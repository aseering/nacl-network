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

/* NOTE: for comments see: crti_arm.S */

/* TODO(robertm): find a header to put this in */
    .set ARM_BUNDLE_SIZE, 4

/* ====================================================================== */
/* END .init section */
/* ====================================================================== */
    .section .init
    .align ARM_BUNDLE_SIZE
    pop {r4, lr}
    nop
    bic lr, lr, #0xf000000f
    bx lr

/* ====================================================================== */
/* END .fini section */
/* ====================================================================== */
    .section .fini
    .align ARM_BUNDLE_SIZE
    pop {r4, lr}
    nop
    bic lr, lr, #0xf000000f
    bx lr

/* ====================================================================== */
/* END .text section */
/* ====================================================================== */
    .text
    .weak __NaClSrpcListServices
__NaClSrpcListServices:

/* ====================================================================== */
/* END .nacl_rpc_methods section */
/* ====================================================================== */

    .section .nacl_rpc_methods, ""
    /* TODO(robertm): add comment regarding padding, probably terminator */
    .4byte 0
    .4byte 0
    .global __kNaClSrpcHandlerEnd
__kNaClSrpcHandlerEnd:

