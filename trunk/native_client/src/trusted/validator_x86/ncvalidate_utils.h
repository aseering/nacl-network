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

/* Some useful utilities for validator patterns. */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATE_UTILS_H__
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATE_UTILS_H__

#include "native_client/src/trusted/validator_x86/ncopcode_desc.h"

struct NcInstState;
struct ExprNodeVector;

/* Special flag set to find set/use of an operand. */
extern const OperandFlags NcOpSetOrUse;

/* Returns true if the instruction corresponds to a move from
 * REG_USE to REG_SET.
 *
 * Parameters:
 *   opcode - The opcode corresponding to the instruction to check.
 *   vector - The expression vector corresponding to the instruction to check.
 *   reg_set - The register set by the move.
 *   reg_use - The register whose value is used to define the set.
 */
Bool NcIsMovUsingRegisters(struct Opcode* opcode,
                           struct ExprNodeVector* vector,
                           OperandKind reg_set,
                           OperandKind reg_use);

/* Returns true if the instruction corresponds to an OR whose result is
 * put into REG_SET, and the or is of values in REG_SET and REG_USE.
 *
 *
 * Parameters:
 *   opcode - The opcode corresponding to the instruction to check.
 *   vector - The expression vector corresponding to the instruction to check.
 *   reg_set - The register set by the or
 *   reg_use - The register whose value is used (along with reg_set) to generate
 *             the or value.
 */
Bool NcIsOrUsingRegister(struct Opcode* opcode,
                         struct ExprNodeVector* vector,
                         OperandKind reg_set,
                         OperandKind reg_use);


/* Returns true if the given instruction's first operand corresponds to
 * a set of the register with the given name.
 *
 * Parameters:
 *   inst - The instruction to check.
 *   reg_name - The name of the register to check if set.
 */
Bool NcOperandOneIsRegisterSet(struct NcInstState* inst,
                               OperandKind reg_name);

/* Returns true if the given instruction's first operand corresponds to
 * a 32-bit value that is zero extended.
 *
 * Parameters:
 *   inst - The instruction to check.
 */
Bool NcOperandOneZeroExtends(struct NcInstState* inst);

/* Returns true if the given instruction is binary where the first
 * Operand is a register set on the given register, and the
 * second operand corresponds to a 32-bit vlaue that is zero extended.
 */
Bool NcAssignsRegisterWithZeroExtends(struct NcInstState* inst,
                                      OperandKind reg_name);

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATE_UTILS_H__ */
