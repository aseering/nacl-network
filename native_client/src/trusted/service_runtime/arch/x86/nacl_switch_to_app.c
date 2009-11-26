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
 * NaCl Service Runtime, C-level context switch code.
 */

#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/arch/x86/sel_rt.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_switch_to_app.h"


NORETURN void NaClStartThreadInApp(struct NaClAppThread *natp,
                                   uint32_t             new_prog_ctr) {
  struct NaClApp  *nap;
  struct NaClThreadContext  *context;
  /*
   * Save service runtime segment registers; fs/gs is used for TLS
   * on Windows and Linux respectively, so will change.  The others
   * should be global, but we save them from the thread anyway.
   */
#if 0 /* restored by trampoline code */
  natp->sys.cs = NaClGetCs();
  natp->sys.ds = NaClGetDs();
#endif
  natp->sys.es = NaClGetEs();
  natp->sys.fs = NaClGetFs();
  natp->sys.gs = NaClGetGs();
  natp->sys.ss = NaClGetSs();
  /*
   * Preserves stack alignment.  The trampoline code loads this value
   * to %esp, then pushes the thread ID (LDT index) onto the stack as
   * argument to NaClSyscallCSegHook.  See nacl_syscall.S.
   */
  NaClSetThreadCtxSp(&natp->sys, (NaClGetEsp() & ~0xf) + 4);

  nap = natp->nap;
  context = &natp->user;
  context->spring_addr = NaClSysToUser(nap,
                                       nap->mem_start + nap->springboard_addr);
  context->new_eip = new_prog_ctr;
  context->sysret = 0; /* %eax not used to return */

  NaClSwitch(context);
}


/*
 * syscall return
 */
NORETURN void NaClSwitchToApp(struct NaClAppThread *natp,
                              uint32_t             new_prog_ctr) {
  struct NaClApp  *nap;
  struct NaClThreadContext  *context;

  nap = natp->nap;
  context = &natp->user;
  context->spring_addr = NaClSysToUser(nap,
                                       nap->mem_start + nap->springboard_addr);
  context->new_eip = new_prog_ctr;
  context->sysret = natp->sysret;

  NaClSwitch(context);
}
