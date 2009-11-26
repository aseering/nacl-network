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

#ifndef NATIVE_CLIENT_SRC_TRUSTED_HANDLE_PASS_BROWSER_HANDLE_H_
#define NATIVE_CLIENT_SRC_TRUSTED_HANDLE_PASS_BROWSER_HANDLE_H_

#include <windows.h>
#include "native_client/src/include/nacl_base.h"
#include "native_client/src/trusted/desc/nacl_desc_base.h"

EXTERN_C_BEGIN

// Initializes the maps, etc., maintained by the handle passing library.
extern int NaClHandlePassBrowserCtor();

// Sel_ldr instances will look up PIDs to get HANDLEs by means of an SRPC
// connection.  This method returns the socket address that a sel_ldr instance
// uses to connect to the lookup server.  This will start a thread that
// waits to accept a connection from a sel_ldr.
extern struct NaClDesc* NaClHandlePassBrowserGetSocketAddress();

// Whenever the browser plugin causes a sel_ldr process to be started by
// the browser, it will return the PID and the process HANDLE of the sel_ldr.
// For lookups this pair is remembered by the library.
// NB: It is important to note that the lookup service exported by this
// library will duplicate handles between any two processes remembered by
// it.  If higher capability processes are remembered this way then it may
// be possible to copy higher capability HANDLEs into lower capability
// processes.
extern void NaClHandlePassBrowserRememberHandle(DWORD pid, HANDLE handle);

// Destroys this instance of the handle passing service.
extern void NaClHandlePassBrowserDtor();

EXTERN_C_END

#endif  // NATIVE_CLIENT_SRC_TRUSTED_HANDLE_PASS_BROWSER_HANDLE_H_
