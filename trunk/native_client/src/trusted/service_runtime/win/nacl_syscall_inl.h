/*
 * Copyright 2008, Google Inc.
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
 * NaCl service runtime syscall inline header file.
 */

#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_WIN_NACL_SYSCALL_INL_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_WIN_NACL_SYSCALL_INL_H_

static INLINE uint32_t NaClAppArg(struct NaClAppThread  *natp,
                                  int                   wordnum) {
  return natp->x_sp[wordnum];
}

/*
 * Syscall return value mapper.  The posix wrappers in windows return
 * -1 on error, store the error code in the thread-specific errno
 * variable, and return -1 instead.  Since we are using these
 * wrappers, we merely detect when any host OS syscall returned -1,
 * and pass -errno back to the NaCl app.  (The syscall wrappers on the
 * NaCl app side will similarly follow the negative-values-are-errors
 * convention).
 */
static INLINE int32_t NaClXlateSysRet(int32_t  rv) {
  return (rv != -1) ? rv : -errno;
}

/*
 * TODO(bsy): NaClXlateSysRetDesc to register returned descriptor in the
 * app's open descriptor table, wrapping it in a native descriptor
 * object.
 */

static INLINE int32_t NaClXlateSysRetAddr(struct NaClApp  *nap,
                                          int32_t         rv) {
  /* if rv is a bad address, we abort */
  return (rv != -1) ? NaClSysToUser(nap, rv) : -errno;
}

#endif
