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
 * Model control flow patterns.
 */

#include "native_client/src/trusted/validator_arm/branch_patterns.h"
#include "native_client/src/trusted/validator_arm/arm_insts_rt.h"
#include "native_client/src/trusted/validator_arm/arm_validate.h"
#include "native_client/src/trusted/validator_arm/register_set_use.h"
#include "native_client/src/trusted/validator_arm/validator_patterns.h"
#include "native_client/src/trusted/validator_arm/masks.h"

/*
 * Validator pattern for a safe mask-and-branch sequence.
 */
class SafeIndirectBranchPattern : public ValidatorPattern {
 public:
  SafeIndirectBranchPattern() : ValidatorPattern("indirect branch", 2, 1) {}
  virtual ~SafeIndirectBranchPattern() {}

  virtual bool MayBeUnsafe(const NcDecodeState &state) {
    return state.CurrentInstructionIs(ARM_BRANCH_RS);
  }
  virtual bool IsSafe(const NcDecodeState &state) {
    NcDecodeState pred(state);
    pred.PreviousInstruction();

    // Make sure we haven't walked out of the text segment.
    if (!pred.HasValidPc()) return false;

    const NcDecodedInstruction &branch = state.CurrentInstruction();

    return CheckControlMask(pred, branch.values.arg4, branch.values.cond);
  }
};

static bool isNaclHalt(const NcDecodeState &state) {
  if (state.CurrentInstructionIs(ARM_MOV)
      && state.CurrentInstructionIs(ARM_DP_I)) {
    const NcDecodedInstruction &inst = state.CurrentInstruction();
    // We don't need to process the shift field: 0 << x == 0.
    return inst.values.immediate == 0;
  } else {
    return false;
  }
}

/*
 * Validator pattern for non-branch writes to PC.
 */
class NonBranchPcUpdatePattern : public ValidatorPattern {
 public:
  NonBranchPcUpdatePattern()
      : ValidatorPattern("non-branch PC update", 1, 0) {}
  virtual ~NonBranchPcUpdatePattern() {}

  virtual bool MayBeUnsafe(const NcDecodeState &state) {
    const NcDecodedInstruction &inst = state.CurrentInstruction();
    return GetBit(RegisterSets(&inst), PC_INDEX)
        && !isNaclHalt(state)
        && !state.CurrentInstructionIs(ARM_BRANCH_RS)
        && !state.CurrentInstructionIs(ARM_BRANCH);
  }
  virtual bool IsSafe(const NcDecodeState &state) {
    return CheckControlMask(state, PC_INDEX, kIgnoreCondition);
  }
};

void InstallBranchPatterns() {
  RegisterValidatorPattern(new SafeIndirectBranchPattern());
  RegisterValidatorPattern(new NonBranchPcUpdatePattern());
}
