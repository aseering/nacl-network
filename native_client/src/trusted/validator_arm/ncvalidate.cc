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

#include "native_client/src/trusted/validator_arm/arm_validate.h"
#include "native_client/src/trusted/validator_arm/ncdecode.h"
#include "native_client/src/trusted/validator_arm/ncvalidate.h"
#include "native_client/src/trusted/validator_arm/branch_patterns.h"
#include "native_client/src/trusted/validator_arm/sequestered_reg_patterns.h"
#include "native_client/src/trusted/validator_arm/stack_adjust_patterns.h"
#include "native_client/src/trusted/validator_arm/store_patterns.h"

void NCValidateInit() {
  InstallBranchPatterns();
  InstallStackAdjustPatterns();
  InstallStorePatterns();
  InstallSequesteredRegisterPatterns();
}

void NCValidateSegment(uint8_t *mbase, uint32_t vbase, size_t size) {
  CodeSegment code_segment;
  CodeSegmentInitialize(&code_segment, mbase, vbase, size);
  ValidateCodeSegment(&code_segment);
}

int NCValidateFinish() {
  return  ValidateExitCode();
}

