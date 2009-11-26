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


// C/C++ library for handler passing in the Windows Chrome sandbox.
// sel_ldr side interface.

#include "native_client/src/trusted/handle_pass/ldr_handle.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"

static const HANDLE kInvalidHandle = reinterpret_cast<HANDLE>(-1);

static struct NaClDesc* lookup_desc = NULL;
static NaClSrpcChannel lookup_channel;

int NaClHandlePassLdrCtor(struct NaClDesc* socket_address) {
  struct NaClDesc* pair[2];
  struct NaClNrdXferEffector effector;
  struct NaClDescEffector* effp;
  int retval = 0;

  // Create a bound socket for use by the effector.
  if (0 != NaClCommonDescMakeBoundSock(pair)) {
    goto no_state;
  }
  // Create an effector to use to receive the connected socket.
  if (!NaClNrdXferEffectorCtor(&effector, pair[0])) {
    goto bound_sock_created;
  }
  effp = reinterpret_cast<struct NaClDescEffector*>(&effector);
  // Connect to the given socket address.
  if (0 != (socket_address->vtbl->ConnectAddr)(socket_address, effp)) {
    goto effector_constructed;
  }
  // Get the connected socket from the effector.
  lookup_desc = NaClNrdXferEffectorTakeDesc(&effector);
  // Create an SRPC client for lookup requests.
  if (!NaClSrpcClientCtor(&lookup_channel, lookup_desc)) {
    goto error_connected;
  }
  // Success.  Clean up everything but lookup_channel and lookup_desc.
  retval = 1;
  goto effector_constructed;

 error_connected:
  NaClDescUnref(lookup_desc);
 effector_constructed:
  // Clean up the effector.
  effp->vtbl->Dtor(effp);
 bound_sock_created:
  NaClDescUnref(pair[1]);
  NaClDescUnref(pair[0]);
 no_state:
  return 0;
}

HANDLE NaClHandlePassLdrLookupHandle(DWORD pid) {
  int int_handle;
  // Optimization: try lookup in a local map first.
  if (NACL_SRPC_RESULT_OK != NaClSrpcInvokeByName(&lookup_channel,
                                                  "lookup",
                                                  GetCurrentProcessId(),
                                                  pid,
                                                  &int_handle)) {
    return kInvalidHandle;
  }
  return reinterpret_cast<HANDLE>(int_handle);
}

void NaClHandlePassLdrDtor() {
  // Tell the server to shut down the thread used for this sel_ldr process.
  NaClSrpcInvokeByName(&lookup_channel, "shutdown");
  // And destroy the SRPC client (which also unrefs lookup_desc).
  NaClSrpcDtor(&lookup_channel);
}
