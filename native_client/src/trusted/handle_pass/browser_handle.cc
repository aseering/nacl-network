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


// C/C++ library for handle passing in the Windows Chrome sandbox
// browser plugin interface.

#include <windows.h>
#include <map>
#include "native_client/src/trusted/handle_pass/browser_handle.h"
#include "native_client/src/shared/platform/nacl_sync.h"
#include "native_client/src/shared/platform/nacl_threads.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"
#include "native_client/src/trusted/desc/nacl_desc_base.h"
#include "native_client/src/trusted/desc/nrd_xfer.h"
#include "native_client/src/trusted/desc/nrd_xfer_effector.h"

static const HANDLE kInvalidHandle = reinterpret_cast<HANDLE>(-1);

// All APIs are guarded by the single mutex.
static struct NaClMutex pid_handle_map_mu = { NULL };

// The map.
static std::map<DWORD, HANDLE>* pid_handle_map = NULL;

// The NaClDesc for the bound socket used to accept connections from
// sel_ldr instances and the corresponding socket address.
// NOTE: we are not closing these descriptors when no instances remain alive.
static struct NaClDesc* handle_descs[2] = { NULL, NULL };

int NaClHandlePassBrowserCtor() {
  int retval = 1;

  NaClMutexLock(&pid_handle_map_mu);
  if (NULL != pid_handle_map) {
    pid_handle_map = new(std::nothrow) std::map<DWORD, HANDLE>;
    if (NULL == pid_handle_map) {
      retval = 0;
    }
  }
  NaClMutexUnlock(&pid_handle_map_mu);
  return retval;
}


static NaClSrpcError Lookup(NaClSrpcChannel* channel,
                            NaClSrpcArg** in_args,
                            NaClSrpcArg** out_args) {
  NaClMutexLock(&pid_handle_map_mu);
  // The PID of the process wanting to send a descriptor.
  int sender_pid = in_args[0]->u.ival;
  // The PID of the process to be sent a descriptor.
  int recipient_pid = in_args[1]->u.ival;
  // The HANDLE in the sender process.  This is the duplicate of the
  // HANDLE contained in the mapping for recipient_pid.
  HANDLE recipient_handle;
  if (pid_handle_map->find(sender_pid) == pid_handle_map->end() ||
      pid_handle_map->find(recipient_pid) == pid_handle_map->end() ||
      FALSE == DuplicateHandle(GetCurrentProcess(),
                               (*pid_handle_map)[recipient_pid],
                               (*pid_handle_map)[sender_pid],
                               &recipient_handle,
                               0,
                               FALSE,
                               DUPLICATE_SAME_ACCESS)) {
    out_args[0]->u.ival = reinterpret_cast<int>(kInvalidHandle);
  } else {
    out_args[0]->u.ival = reinterpret_cast<int>(recipient_handle);
  }
  NaClMutexUnlock(&pid_handle_map_mu);
  return NACL_SRPC_RESULT_OK;
}

static NaClSrpcError Shutdown(NaClSrpcChannel* channel,
                              NaClSrpcArg** in_args,
                              NaClSrpcArg** out_args) {
  return NACL_SRPC_RESULT_BREAK;
}

static void WINAPI HandleServer(void* dummy) {
  // Set up an effector.
  struct NaClDesc* pair[2];
  struct NaClNrdXferEffector effector;
  struct NaClDescEffector* effp;
  struct NaClDesc* lookup_desc;
  struct NaClSrpcHandlerDesc handlers[] = { { "lookup:ii:i", Lookup },
                                            { "shutdown::", Shutdown } };

  // Create a bound socket for use by the effector.
  if (0 != NaClCommonDescMakeBoundSock(pair)) {
    goto no_state;
  }
  // Create an effector to use to receive the connected socket.
  if (!NaClNrdXferEffectorCtor(&effector, pair[0])) {
    goto bound_sock_created;
  }
  effp = (struct NaClDescEffector*) &effector;
  // Accept on the bound socket.
  if (0 != (handle_descs[0]->vtbl->AcceptConn)(handle_descs[0], effp)) {
    goto effector_constructed;
  }
  // Get the connected socket from the effector.
  lookup_desc = NaClNrdXferEffectorTakeDesc(&effector);
  // Create an SRPC client and start the message loop.
  NaClSrpcServerLoop(lookup_desc, handlers, NULL);
  // Success.  Clean up.

 effector_constructed:
  effp->vtbl->Dtor(effp);
 bound_sock_created:
  NaClDescUnref(pair[1]);
  NaClDescUnref(pair[0]);
 no_state:
  NaClThreadExit();
}

struct NaClDesc* NaClHandlePassBrowserGetSocketAddress() {
  struct NaClDesc* socket_address = NULL;
  struct NaClThread thread;

  NaClMutexLock(&pid_handle_map_mu);
  // If the socket address is already set, bump the ref count and return it.
  if (NULL == handle_descs[1]) {
    // Otherwise, create the bound socket and socket address.
    if (0 != NaClCommonDescMakeBoundSock(handle_descs)) {
      goto no_cleanup;
    }
    socket_address = NaClDescRef(handle_descs[1]);
    // Create the acceptor/server thread.
    NaClThreadCtor(&thread, HandleServer, NULL, 65536);
    // And return to the caller.
  }
 no_cleanup:
  NaClMutexUnlock(&pid_handle_map_mu);
  return socket_address;
}

void NaClHandlePassBrowserRememberHandle(DWORD pid, HANDLE handle) {
  NaClMutexLock(&pid_handle_map_mu);
  (*pid_handle_map)[pid] = handle;
  NaClMutexUnlock(&pid_handle_map_mu);
}

void NaClHandlePassBrowserDtor() {
  // Nothing for now.
}
