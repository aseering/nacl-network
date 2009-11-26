/*
 * Copyright 2009, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * nc_jumps.c - Validate where valid jumps can occur.
 */

#include <assert.h>
#include <stdlib.h>

#include "native_client/src/trusted/validator_x86/nc_jumps.h"

#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/validator_x86/nc_inst_trans.h"
#include "native_client/src/trusted/validator_x86/ncop_exps.h"
#include "native_client/src/trusted/validator_x86/ncvalidate_iter.h"
#include "native_client/src/trusted/validator_x86/ncvalidate_iter_internal.h"

/* To turn on debugging of instruction decoding, change value of
 * DEBUGGING to 1.
 */
#define DEBUGGING 0

#include "native_client/src/shared/utils/debugging.h"

/* Models a set of addresses using an an array of possible addresses,
 * where the last 3 bits are unioned together using a bit mask. This cuts
 * down on the memory footprint of the address table.
 */
typedef uint8_t* AddressSet;

/* Model the set of possible 3-bit tails of possible PcAddresses. */
static const uint8_t pc_address_masks[8] = {
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

/* Model the offset created by removing the bottom three bits of a PcAddress. */
typedef PcAddress PcOffset;

/* Convert an address into the corresponding offset in an address table.
 * That is, strip off the last three bits, since these remaining bits
 * will be encoded using the union of address masks in the address table.
 */
static INLINE PcOffset PcAddressToOffset(PcAddress address) {
  return address >> 3;
}

/* Convert the 3 lower bits of an address into the corresponding address mask
 * to use.
 */
static INLINE uint8_t PcAddressToMask(PcAddress address) {
  return pc_address_masks[(int) (address & (PcAddress)0x7)];
}

/* Return true if the corresponding address is in the given address set. */
static INLINE uint8_t AddressSetContains(AddressSet set,
                                         PcAddress address,
                                         NcValidatorState* state) {
  PcAddress offset = address - state->vbase;
  assert(address >= state->vbase);
  assert(address < state->vlimit);
  return set[PcAddressToOffset(offset)] & PcAddressToMask(offset);
}

/* Adds the given address to the given address set. */
static void AddressSetAdd(AddressSet set, PcAddress address,
                          NcValidatorState* state) {
  PcAddress offset = address - state->vbase;
  assert(address >= state->vbase);
  assert(address < state->vlimit);
  DEBUG(printf("Address set add: %"PRIxPcAddress"\n", address));
  set[PcAddressToOffset(offset)] |= PcAddressToMask(offset);
}

/* Removes the given address from the given address set. */
static void AddressSetRemove(AddressSet set, PcAddress address,
                             NcValidatorState* state) {
  PcAddress offset = address - state->vbase;
  assert(address >= state->vbase);
  assert(address < state->vlimit);
  set[PcAddressToOffset(offset)] &= ~(PcAddressToMask(offset));
}

/* Create an address set for the range 0..Size. */
static INLINE AddressSet AddressSetCreate(MemorySize size) {
  /* Be sure to add an element for partial overlaps. */
  /* TODO(karl) The cast to size_t for the number of elements may
   * cause loss of data. We need to fix this. This is a security
   * issue when doing cross-platform (32-64 bit) generation.
   */
  return (AddressSet) calloc((size_t) PcAddressToOffset(size) + 1,
                             sizeof(uint8_t));
}

/* frees the memory of the address set back to the system. */
static INLINE void AddressSetDestroy(AddressSet set) {
  free(set);
}

/* Holds information collected about each instruction, and the
 * targets of possible jumps. Then, after the code has been processed,
 * this information is processed to see if there are any invalid jumps.
 */
typedef struct JumpSets {
  /* Holds the set of possible target addresses that can be the result of
   * a jump.
   */
  AddressSet actual_targets;
  /* Holds the set of valid instruction entry points (whenever a pattern of
   * multiple instructions are used, the sequence will be treated as atomic,
   * only having the first address in the set).
   */
  AddressSet possible_targets;
} JumpSets;

/* Generates a jump validator. */
JumpSets* NcJumpValidatorCreate(NcValidatorState* state) {
  PcAddress align_base = state->vbase & (~state->alignment);
  JumpSets* jump_sets = (JumpSets*) malloc(sizeof(JumpSets));
  if (jump_sets != NULL) {
    jump_sets->actual_targets = AddressSetCreate(state->vlimit - align_base);
    jump_sets->possible_targets = AddressSetCreate(state->vlimit - align_base);
    if (jump_sets->actual_targets == NULL ||
        jump_sets->possible_targets == NULL) {
      NcValidatorMessage(LOG_FATAL, state, "unable to allocate jump sets");
    }
  }
  return jump_sets;
}

/* Record that there is an explicit jump from the from_address to the
 * to_address, for the validation defined by the validator state.
 * Parameters:
 *   state - The state of the validator.
 *   from_address - The address of the instruction that does the jump.
 *   to_address - The address that the instruction jumps to.
 *   jump_sets - The collected information on (explicit) jumps.
 *   inst - The instruction that does the jump (may be null - only used
 *      to give more informative error messages).
 */
static void NcAddJumpToJumpSets(NcValidatorState* state,
                                PcAddress from_address,
                                PcAddress to_address,
                                JumpSets* jump_sets,
                                NcInstState* inst) {
  Bool is_good = TRUE;
  if (to_address < state->vlimit) {
    if (to_address < state->vbase) {
      /*
       * the trampolines need to be aligned 0mod32 regardless of the
       * program's elf flags.  This allows the same library to be used
       * with both 16 and 32 byte aligned clients.
       */
      if (to_address < state->vbase - NACL_TRAMPOLINE_END ||
          (to_address & (NACL_INSTR_BLOCK_SIZE-1))) {
        /*
         * TODO(sehr): once we fully support 16/32 alignment, remove this
         * in favor of however we communicate the fixed block size.
         */
        /* this is an unaligned target in the trampoline area. */
        /* vprint(("ignoring target %08x in trampoline\n", target)); */
        is_good = FALSE;
      }
    }
  } else {
    is_good = FALSE;
  }
  if (is_good) {
    DEBUG(printf("Add jump to target: %"PRIxPcAddress" -> %"PRIxPcAddress"\n",
                 from_address, to_address));
    AddressSetAdd(jump_sets->actual_targets, to_address, state);
  } else {
    if (inst == NULL) {
      NcValidatorPcAddressMessage(LOG_ERROR, state, from_address,
                                  "Instruction jumps to bad address\n");
      NcValidatorPcAddressMessage(LOG_ERROR, state, to_address,
                                  "Target of bad jump\n");
    } else {
      NcValidatorInstMessage(LOG_ERROR, state, inst,
                             "Instruction jumps to bad address\n");
    }
  }
}

static Bool ExtractBinaryOperandIndices(
    NcInstState* inst,
    int* op_1,
    int* op_2) {
  uint32_t index;
  ExprNodeVector* nodes = NcInstStateNodeVector(inst);
  *op_1 = -1;
  *op_2 = -1;

  for (index = 0; index < nodes->number_expr_nodes; ++index) {
    if (OperandReference == nodes->node[index].kind) {
      if (-1 == *op_1) {
        *op_1 = index + 1;
      } else {
        *op_2 = index + 1;
        return TRUE;
      }
    }
  }
  return FALSE;
}

/* Given the index of a memory offset in the given expression vector, return
 * true iff the memory offset is of the form [base+index].
 *
 * Parameters:
 *   vector - The expression vector for the instruction being examined.
 *   index - The index into the expression vector where the memory offset
 *           occurs.
 *   base_register - The base register expected in the memory offset.
 *   index_register - The index register expected in the memory offset.
 */
static Bool MemOffsetMatchesBaseIndex(ExprNodeVector* vector,
                                      int memoffset_index,
                                      OperandKind base_register,
                                      OperandKind index_register) {
  int r1_index = memoffset_index + 1;
  int r2_index = r1_index + ExprNodeWidth(vector, r1_index);
  int scale_index = r2_index + ExprNodeWidth(vector, r2_index);
  int disp_index = scale_index + ExprNodeWidth(vector, scale_index);
  OperandKind r1 = GetNodeVectorRegister(vector, r1_index);
  OperandKind r2 = GetNodeVectorRegister(vector, r2_index);
  uint64_t scale = GetExprConstant(vector, scale_index);
  uint64_t disp = GetExprConstant(vector, disp_index);
  assert(ExprMemOffset == vector->node[memoffset_index].kind);
  return r1 == base_register &&
      r2 == index_register &&
      1 == scale &&
      0 == disp;
}

/* Checks if an indirect jump (in 64-bit mode) is native client compliant.
 *
 * Expects pattern:
 *    and %REG32-A, MASK
 *    lea %REG64-B, [%RBASE + %REG64-A]
 *    jmp %REG64-B
 *
 * where MASK is all 1/s except for the alignment mask bits, which must be zero.
 *
 * REG32 is the corresponding 32-bit register that whose value will get zero
 * extended by the AND operation into the corresponding 64-bit register REG64.
 *
 * It is assumed that opcode 0x83 is used for the AND operation, and hence, the
 * mask is a single byte.
 *
 * Note: applies to all kinds of jumps and calls.
 *
 * Parameters:
 *   state - The state of the validator.
 *   iter - The instruction iterator being used.
 *   inst_pc - The (virtual) address of the instruction that jumps.
 *   inst - The instruction that does the jump.
 *   reg - The register used in the jump instruction.
 *   jump_sets - The current information collected about (explicit) jumps
 *     by the jump validator.
 */
static void AddRegisterJumpIndirect64(NcValidatorState* state,
                                      NcInstIter* iter,
                                      PcAddress inst_pc,
                                      NcInstState* inst,
                                      ExprNode* reg,
                                      JumpSets* jump_sets) {
  uint8_t mask;
  NcInstState* and_inst;
  NcInstState* lea_inst;
  Opcode* and_opcode;
  Opcode* lea_opcode;
  int op_1, op_2;
  OperandKind and_reg, lea_reg, jump_reg;
  ExprNodeVector* nodes;
  ExprNode* node;
  jump_reg = GetNodeRegister(reg);

  /* Do the following block exactly once. */
  do {
    /* Check and in 3 instruction sequence. */
    if (!NcInstIterHasLookbackState(iter, 2)) break;
    and_inst = NcInstIterGetLookbackState(iter, 2);
    and_opcode = NcInstStateOpcode(and_inst);
    if (0x83 != and_opcode->opcode[0] || InstAnd != and_opcode->name) break;

    /* Extract the values of the two operands for the and. */
    if (!ExtractBinaryOperandIndices(and_inst, &op_1, &op_2)) break;

    /* Extract the destination register of the and. */
    nodes = NcInstStateNodeVector(and_inst);
    node = &nodes->node[op_1];
    if (ExprRegister != node->kind) break;
    and_reg = GetNodeRegister(node);

    /* Check that the mask is ok. */
    mask = (uint8_t) (~state->alignment_mask);
    assert(0 != mask);  /* alignment must be either 16 or 32. */
    node = &nodes->node[op_2];
    if (ExprConstant != node->kind || mask != node->value) break;

    /* Check lea in 3 instruction sequence. */
    lea_inst = NcInstIterGetLookbackState(iter, 1);
    lea_opcode = NcInstStateOpcode(lea_inst);
    if (InstLea != lea_opcode->name) break;

    /* Extract the values of the two operands for the lea. */
    if (!ExtractBinaryOperandIndices(lea_inst, &op_1, &op_2)) break;

    /* Extract the destination register of the lea, and compare to
     * the jump register.
     */
    nodes = NcInstStateNodeVector(lea_inst);
    node = &nodes->node[op_1];
    if (ExprRegister != node->kind) break;
    lea_reg = GetNodeRegister(node);
    if (lea_reg != jump_reg) break;

    /* Check that we have [%RBASE + %REG64-A]. */
    node = &nodes->node[op_2];
    if (ExprMemOffset != node->kind ||
        !MemOffsetMatchesBaseIndex(nodes, op_2, state->base_register,
                                   NcGet64For32BitRegister(and_reg)))
      break;

    /* If reached, indirect jump is properly masked. */
    NcMarkInstructionJumpIllegal(state, lea_inst);
    NcMarkInstructionJumpIllegal(state, inst);
    return;
  } while(0);

  /* If reached, mask was not found. */
  NcValidatorInstMessage(LOG_ERROR, state, inst, "Invalid indirect jump\n");
}

/* Checks if an indirect jump (in 32-bit mode) is native client compliant.
 *
 * Expects pattern:
 *    and %REG, MASK
 *    jmp %REG
 *
 * where the MASK is all 1's except for the alignment mask bits, which must
 * be zero.
 *
 * It is assumed that opcode 0x83 is used for the AND operation, and hence, the
 * mask is a single byte.
 *
 * Note: applies to all kinds of jumps and calls.
 *
 * Parameters:
 *   state - The state of the validator.
 *   iter - The instruction iterator being used.
 *   inst_pc - The (virtual) address of the instruction that jumps.
 *   inst - The instruction that does the jump.
 *   reg - The register used in the jump instruction.
 *   jump_sets - The current information collected about (explicit) jumps
 *     by the jump validator.
 */
static void AddRegisterJumpIndirect32(NcValidatorState* state,
                                      NcInstIter* iter,
                                      PcAddress inst_pc,
                                      NcInstState* inst,
                                      ExprNode* reg,
                                      JumpSets* jump_sets
                                      ) {
  uint8_t mask;
  NcInstState* and_inst;
  Opcode* and_opcode;
  int op_1, op_2;
  OperandKind and_reg, jump_reg;
  ExprNodeVector* nodes;
  ExprNode* node;
  assert(ExprRegister == reg->kind);
  jump_reg = GetNodeRegister(reg);

  /* Do the following block exactly once. */
  do {
    /* Check that the jump is preceded with an AND. */
    if (!NcInstIterHasLookbackState(iter, 1)) break;
    and_inst = NcInstIterGetLookbackState(iter, 1);
    and_opcode = NcInstStateOpcode(and_inst);
    if (0x83 != and_opcode->opcode[0] || InstAnd  != and_opcode->name) break;

    /* Extract the values of the two operands for the and. */
    if (!ExtractBinaryOperandIndices(and_inst, &op_1, &op_2)) break;


    /* Check that the register of the AND matches the jump. */
    nodes = NcInstStateNodeVector(and_inst);
    node = &nodes->node[op_1];
    if (ExprRegister != node->kind) break;
    and_reg = GetNodeRegister(node);
    if (jump_reg != and_reg) break;

    /* Check that the mask is ok. */
    mask = (uint8_t) (~state->alignment_mask);
    assert(0 != mask);  /* alignment must be either 16 or 32. */
    node = &nodes->node[op_2];
    if (ExprConstant != node->kind || mask != node->value) break;

    /* If reached, indirect jump is properly masked. */
    NcMarkInstructionJumpIllegal(state, inst);
    return;
  } while(0);

  /* If reached, mask was not found. */
  NcValidatorInstMessage(LOG_ERROR, state, inst, "Invalid indirect jump\n");
}

/* Given a jump statement, add the corresponding (explicit) jump value
 * to the set of actual jump targets.
 * Parameters:
 *   state - The state of the validator.
 *   iter - The instruction iterator being used.
 *   inst_pc - The (virtual) address of the instruction that jumps.
 *   inst - The instruction that does the jump.
 *   jump_sets - The current information collected about (explicit) jumps
 *     by the jump validator.
 */
static void AddExprJumpTarget(NcValidatorState* state,
                              NcInstIter* iter,
                              PcAddress inst_pc,
                              NcInstState* inst,
                              JumpSets* jump_sets) {
  uint32_t i;
  ExprNodeVector* vector = NcInstStateNodeVector(inst);
  for (i = 0; i < vector->number_expr_nodes; ++i) {
    if (vector->node[i].flags & ExprFlag(ExprJumpTarget)) {
      switch (vector->node[i].kind) {
        case ExprRegister:
          if (64 == NACL_TARGET_SUBARCH) {
            AddRegisterJumpIndirect64(state,
                                      iter,
                                      inst_pc,
                                      inst,
                                      &vector->node[i],
                                      jump_sets);
          } else {
            AddRegisterJumpIndirect32(state,
                                      iter,
                                      inst_pc,
                                      inst,
                                      &vector->node[i],
                                      jump_sets);
          }
          break;
        case ExprConstant:
        case ExprConstant64:
          /* Explicit jump value. */
          NcAddJumpToJumpSets(state,
                              inst_pc,
                              (PcAddress) GetExprConstant(vector, i),
                              jump_sets,
                              inst);
          break;
        default:
          NcValidatorInstMessage(
              LOG_ERROR, state, inst,
              "Jump not native client compliant\n");
      }
    }
  }
}

/* Given an instruction corresponding to a call, validate that the generated
 * return address is safe.
 * Parameters:
 *   state - The state of the validator.
 *   pc - The (virtual) address of the instruciton that does the call.
 *   inst - The instruction that does the call.
 */
static void ValidateCallAlignment(NcValidatorState* state,
                                  PcAddress pc,
                                  NcInstState* inst) {
  /* The return is safe only if it begins at an aligned address (since
   * return instructions are not explicit jumps).
   */
  PcAddress next_pc = pc + NcInstStateLength(inst);
  if (next_pc & state->alignment_mask) {
    DEBUG(printf("Call alignment: pc = %"PRIxPcAddress", next_pc=%"PRIxPcAddress
                 ", mask = %"PRIxPcAddress"\n",
                 pc, next_pc, state->alignment_mask));
    NcValidatorInstMessage(
        LOG_ERROR, state, inst,
        "Bad call alignment, return pc = %"PRIxPcAddress"\n", next_pc);
  }
}

/* Record that the given address of the given instruction is the beginning of
 * a disassembled instruction.
 * Parameters:
 *   State - The state of the validator.
 *   pc - The (virtual) address of a disassembled instruction.
 *   inst - The disassembled instruction.
 *   jump_sets - The current information collected about (explicit) jumps
 *     by the jump validator.
 */
static void RememberIp(NcValidatorState* state,
                       PcAddress pc,
                       NcInstState* inst,
                       JumpSets* jump_sets) {
  if (pc < state->vbase || pc >= state->vlimit) {
    NcValidatorInstMessage(LOG_ERROR, state, inst,
                           "Instruction pc out of range\n");
  } else {
    AddressSetAdd(jump_sets->possible_targets, pc, state);
  }
}

void NcJumpValidator(NcValidatorState* state,
                     NcInstIter* iter,
                     JumpSets* jump_sets) {
  NcInstState* inst = NcInstIterGetState(iter);
  PcAddress pc = NcInstStateVpc(inst);
  Opcode* opcode = NcInstStateOpcode(inst);
  RememberIp(state, pc, inst, jump_sets);
  switch (opcode->name) {
    case InstJb:
    case InstJbe:
    case InstJcxz:
    case InstJnl:
    case InstJnle:
    case InstJl:
    case InstJle:
    case InstJmp:
    case InstJnb:
    case InstJnbe:
    case InstJno:
    case InstJnp:
    case InstJns:
    case InstJnz:
    case InstJo:
    case InstJp:
    case InstJs:
    case InstJz:
      AddExprJumpTarget(state, iter, pc, inst, jump_sets);
      break;
    case InstCall:
      AddExprJumpTarget(state, iter, pc, inst, jump_sets);
      ValidateCallAlignment(state, pc, inst);
      break;
    default:
      break;
  }
}

void NcJumpValidatorSummarize(FILE* file,
                              NcValidatorState* state,
                              JumpSets* jump_sets) {
  /* Check that any explicit jump is to a possible (atomic) sequence
   * of disassembled instructions.
   */
  PcAddress addr;
  NcValidatorMessage(
      LOG_INFO, state,
      "Checking jump targets: %"PRIxPcAddress" to %"PRIxPcAddress"\n",
      state->vbase, state->vlimit);
  for (addr = state->vbase; addr < state->vlimit; addr++) {
    if (AddressSetContains(jump_sets->actual_targets, addr, state)) {
      if (!AddressSetContains(jump_sets->possible_targets, addr, state)) {
        NcValidatorPcAddressMessage(LOG_ERROR, state, addr, "Bad jump target\n");
      }
    }
  }

  /* Check that all block boundaries are accessable at an aligned address. */
  NcValidatorMessage(
      LOG_INFO, state, "Checking that basic blocks are aligned\n");
  if (state->vbase & state->alignment_mask) {
    NcValidatorMessage(LOG_ERROR, state,
                       "Code segment starts at 0x%"PRIxPcAddress
                       ", which isn't aligned properly.\n",
                       state->vbase);
  } else {
    for (addr = state->vbase; addr < state->vlimit; addr += state->alignment) {
      if (!AddressSetContains(jump_sets->possible_targets, addr, state)) {
        NcValidatorPcAddressMessage(
            LOG_ERROR, state, addr, "Bad basic block alignment.\n");
      }
    }
  }
}

void NcJumpValidatorDestroy(NcValidatorState* state,
                            JumpSets* jump_sets) {
  AddressSetDestroy(jump_sets->actual_targets);
  AddressSetDestroy(jump_sets->possible_targets);
  free(jump_sets);
}

void NcAddJump(NcValidatorState* state,
               PcAddress from_address,
               PcAddress to_address) {
  JumpSets* jump_sets =
      (JumpSets*) NcGetValidatorLocalMemory((NcValidator) NcJumpValidator,
                                            state);
  NcAddJumpToJumpSets(state, from_address, to_address, jump_sets, NULL);
}

void NcMarkInstructionJumpIllegal(struct NcValidatorState* state,
                                  struct NcInstState* inst) {
  PcAddress pc = NcInstStateVpc(inst);
  if (pc < state->vbase || pc >= state->vlimit) {
    /* ERROR instruction out of range.
     * Note: Not reported here, because this will already be reported by
     * the call to RememberIp in JumpValidator.
     */
  } else {
    JumpSets* jump_sets =
        (JumpSets*) NcGetValidatorLocalMemory((NcValidator) NcJumpValidator,
                                              state);
    DEBUG(printf("Mark instruction as jump illegal: %"PRIxPcAddress"\n",
                 pc));
    AddressSetRemove(jump_sets->possible_targets, pc, state);
  }
}
