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


// NaCl Handle Transfer Protocol

#include "native_client/src/shared/imc/nacl_htp_c.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "native_client/src/shared/imc/nacl_htp.h"
#include "native_client/src/trusted/service_runtime/include/sys/nacl_imc_api.h"

#ifndef __native_client__

int NaClHtpSendDatagram(NaClHtpHandle socket, const NaClHtpHeader* message,
                        int flags) {
  return nacl::SendDatagram(socket,
                            reinterpret_cast<const nacl::HtpHeader*>(message),
                            flags);
}

int NaClHtpReceiveDatagram(NaClHtpHandle socket, NaClHtpHeader* message,
                           int flags) {
  return nacl::ReceiveDatagram(socket,
                               reinterpret_cast<nacl::HtpHeader*>(message),
                               flags);
}

int NaClHtpClose(NaClHtpHandle handle) {
  return nacl::Close(*reinterpret_cast<nacl::HtpHandle*>(&handle));
}

#endif  // __native_client__