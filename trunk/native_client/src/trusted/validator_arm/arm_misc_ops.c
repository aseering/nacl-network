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
 * Define miscellaneous ARM instructions.
 */

#include "native_client/src/trusted/validator_arm/arm_misc_ops.h"
#include "native_client/src/trusted/validator_arm/arm_insts.h"
#include "native_client/src/trusted/validator_arm/arm_inst_modeling.h"

static const ModeledOpInfo kMiscellaneousOps[] = {
  { "clz",
    ARM_CLZ,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\t%x, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x0, 0x16, 0xF, CONTAINS_ZERO, 0xF,
      CONTAINS_ZERO, 0x0, 0x1 }},
  { "usad8",
    ARM_USAD8,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\t%r, %z, %y",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x18, CONTAINS_ZERO, 0xF, CONTAINS_ZERO,
      CONTAINS_ZERO, 0x0, 0x1 }},
  { "usada8",
    ARM_USADA8,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\t%r, %z, %y, %x",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x18, CONTAINS_ZERO, CONTAINS_ZERO, CONTAINS_ZERO,
      CONTAINS_ZERO, 0x0, 0x1 }},
  { "pkhbt",
    ARM_PKHBT_1_LSL,
    kArmUndefinedAccessModeName,
    ARM_LS_RO,
    TRUE,
    "%C\t%x, %r, %z, lsl #%s",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x8, CONTAINS_ZERO, CONTAINS_ZERO, NON_ZERO,
      CONTAINS_ZERO, 0x1, NOT_USED }},
  { "pkhbt",
    ARM_PKHBT_1,
    kArmUndefinedAccessModeName,
    ARM_LS_RO, TRUE,
    "%C\t%x, %r, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x8, CONTAINS_ZERO, CONTAINS_ZERO, 0x0,
      CONTAINS_ZERO, 0x1, NOT_USED }},
  { "pkhtb",
    ARM_PKHBT_2_ASR,
    kArmUndefinedAccessModeName,
    ARM_LS_RO,
    TRUE,
    "%C\t, %x, %r, %z, asr #%s",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x8, CONTAINS_ZERO, CONTAINS_ZERO, NON_ZERO,
      CONTAINS_ZERO, 0x5, NOT_USED }},
  { "pkhtb",
    ARM_PKHBT_2,
    kArmUndefinedAccessModeName,
    ARM_LS_RO,
    TRUE,
    "%C\t, %x, %r, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x8, CONTAINS_ZERO, CONTAINS_ZERO, 0x0,
      CONTAINS_ZERO, 0x5, NOT_USED }},
  { "rev",
    ARM_REV,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\t%x, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0xB, 0xF, CONTAINS_ZERO, 0xF,
      CONTAINS_ZERO, 0x1, 0x1 }},
  { "rev16",
    ARM_REV16,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\t%x, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0xB, 0xF, CONTAINS_ZERO, 0xF,
      CONTAINS_ZERO, 0x1, 0x9 }},
  { "revsh",
    ARM_REVSH,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\t%x, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0xF, 0xF, CONTAINS_ZERO, 0xF,
      CONTAINS_ZERO, 0x1, 0x9 }},
  { "sel",
    ARM_SEL,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\t%x, %r, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x8, CONTAINS_ZERO, CONTAINS_ZERO, 0xF,
      CONTAINS_ZERO, 0x1, 0x9 }},
  { "ssat",
    ARM_SSAT_LSL,
    kArmUndefinedAccessModeName,
    ARM_MISC_SAT,
    TRUE,
    "%C\t%x, #%1, %z, lsl #%3",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x5, ANY, CONTAINS_ZERO, NON_ZERO,
      CONTAINS_ZERO, 0x1, NOT_USED }},
  { "ssat",
    ARM_SSAT,
    kArmUndefinedAccessModeName,
    ARM_MISC_SAT,
    TRUE,
    "%C\t%x, #%1, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x5, ANY, CONTAINS_ZERO, 0x0,
      CONTAINS_ZERO, 0x1, NOT_USED }},
  { "ssat",
    ARM_SSAT_ASR,
    kArmUndefinedAccessModeName,
    ARM_MISC_SAT,
    TRUE,
    "%C\t%x, #%1, %z, asr #%3",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x5, ANY, CONTAINS_ZERO, ANY,
      CONTAINS_ZERO, 0x5, NOT_USED }},
  { "ssat16",
    ARM_SSAT16,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\t%x, #%+, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0xA, ANY, CONTAINS_ZERO, 0xF,
      CONTAINS_ZERO, 0x1, 0x1 }},
  { "usat",
    ARM_USAT_LSL,
    kArmUndefinedAccessModeName,
    ARM_MISC_SAT,
    TRUE,
    "%C\t%x, #%+, %z, lsl #%3",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x7, ANY, CONTAINS_ZERO, NON_ZERO,
      CONTAINS_ZERO, 0x1, NOT_USED }},
  { "usat",
    ARM_USAT,
    kArmUndefinedAccessModeName,
    ARM_MISC_SAT,
    TRUE,
    "%C\t%x, #%+, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x7, ANY, CONTAINS_ZERO, 0x0,
      CONTAINS_ZERO, 0x1, NOT_USED }},
  { "usat",
    ARM_USAT_ASR,
    kArmUndefinedAccessModeName,
    ARM_MISC_SAT,
    TRUE,
    "%C\t%x, #%+, %z, asr #%3",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0x7, ANY, CONTAINS_ZERO, ANY,
      CONTAINS_ZERO, 0x5, NOT_USED }},
  { "usat16",
    ARM_USAT16,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\t%x, #%1, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x3, 0xE, ANY, CONTAINS_ZERO, 0xF,
      CONTAINS_ZERO, 0x1, 0x1 }},
  { "mrs",
    ARM_MRS_CPSR,
    kArmUndefinedAccessModeName,
    ARM_LS_IO,
    TRUE,
    "%C\t%x, cpsr",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x0, 0x10, 0xF, CONTAINS_ZERO, NOT_USED,
      NOT_USED, NOT_USED, 0x0 }},
  { "mrs",
    ARM_MRS_SPSR,
    kArmUndefinedAccessModeName,
    ARM_LS_IO,
    TRUE,
    "%C\t%x, spsr",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x0, 0x14, 0xF, CONTAINS_ZERO, NOT_USED,
      NOT_USED, NOT_USED, 0x0 }},
  { "msr",
    ARM_MSR_CPSR_IMMEDIATE,
    kArmUndefinedAccessModeName,
    ARM_DP_I,
    TRUE,
    "%C\tcspr_%f, #%I",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x1, 0x12, ANY, 0xF, NOT_USED, NOT_USED, ANY, ANY }},
  { "msr",
    ARM_MSR_SPSR_IMMEDIATE,
    kArmUndefinedAccessModeName,
    ARM_DP_I,
    TRUE,
    "%C\tsspr_%f, #%I",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x1, 0x16, ANY, 0xF, NOT_USED, NOT_USED, ANY, ANY }},
  { "msr",
    ARM_MSR_CPSR_REGISTER,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\tcpsr_%f, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x0, 0x12, ANY, 0xF, 0x0, ANY, 0x0, 0x0 }},
  { "msr",
    ARM_MSR_SPSR_REGISTER,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%C\tspsr_%f, %z",
    ARM_WORD_LENGTH,
    NULLName,
    { CONTAINS_ZERO, 0x0, 0x16, ANY, 0xF, 0x0, ANY, 0x0, 0x0 }},
  /* TODO(kschimpf) cps: arg4 must be one of 0x10, 0x11, 0x12, 0x13, 0x17,
   *                     0x1B, or 0x1F.
   */
  { "cps",
    ARM_CPS_FLAGS,
    kArmUndefinedAccessModeName,
    ARM_CPS,
    TRUE,
    "%n%e\t%F",
    ARM_WORD_LENGTH,
    NULLName,
    { 0xF, 0x0, 0x10, ANY, 0x0, NON_ZERO, 0x0, 0x0, 0x0 }},
  { "cps",
    ARM_CPS_FLAGS_MODE,
    kArmUndefinedAccessModeName,
    ARM_CPS,
    TRUE,
    "%n%e\t%F, #%4",
    ARM_WORD_LENGTH,
    NULLName,
    { 0xF, 0x0, 0x10, ANY, 0x1, NON_ZERO, ANY, 0x0, 0x0 }},
  { "cps",
    ARM_CPS_MODE,
    kArmUndefinedAccessModeName,
    ARM_CPS,
    TRUE,
    "%n\t#%4",
    ARM_WORD_LENGTH,
    NULLName,
    { 0xF, 0x0, 0x10, 0x0, 0x1, 0x0, ANY, 0x0, 0x0 }},
  { "setend",
    ARM_SETEND_BE,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%n\tbe",
    ARM_WORD_LENGTH,
    NULLName,
    { 0xF, 0x3, 0x10, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0 }},
  { "setend",
    ARM_SETEND_LE,
    kArmUndefinedAccessModeName,
    ARM_DP_RS,
    TRUE,
    "%n\tle",
    ARM_WORD_LENGTH,
    NULLName,
    { 0xF, 0x3, 0x10, 0x1, 0x0, 0x2, 0x0, 0x0, 0x0 }},
  END_OPINFO_LIST
};

void BuildMiscellaneousOps() {
  int i;
  for (i = 0; ; ++i) {
    const ModeledOpInfo* op = &(kMiscellaneousOps[i]);
    if (ARM_INST_TYPE_SIZE == op->inst_type) break;
    AddInstruction(op);
  }
}