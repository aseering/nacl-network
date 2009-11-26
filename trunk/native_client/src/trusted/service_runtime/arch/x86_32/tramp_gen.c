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

#include <stdio.h>
#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/service_runtime/tramp.h"


int main() {
  unsigned char * idx = (unsigned char *) &NaCl_trampoline_seg_code;

  /* print out the offsets used for patching */
  printf("#define NACL_TRAMP_CSEG_PATCH 0x%02"PRIxPTR"\n",
         ((uintptr_t) &NaCl_tramp_cseg_patch -
          (uintptr_t) &NaCl_trampoline_seg_code));
  printf("#define NACL_TRAMP_DSEG_PATCH 0x%02"PRIxPTR"\n",
         ((uintptr_t) &NaCl_tramp_dseg_patch -
          (uintptr_t) &NaCl_trampoline_seg_code));
  printf("\n");

  printf("unsigned char kTrampolineCode[] = {\n");
  while (idx <= (unsigned char *) &NaCl_trampoline_seg_end) {
    printf(" 0x%02x,", *idx);
    idx++;
  }
  printf("\n};\n");

  return 0;
}
