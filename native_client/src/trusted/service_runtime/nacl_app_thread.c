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
 * NaCl Server Runtime user thread state.
 */
#include "native_client/src/shared/platform/nacl_sync_checked.h"
#include "native_client/src/trusted/desc/nacl_desc_effector_ldr.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_tls.h"
#include "native_client/src/trusted/service_runtime/nacl_switch_to_app.h"


void WINAPI NaClThreadLauncher(void *state) {
  struct NaClAppThread  *natp;

  NaClLog(4, "NaClThreadLauncher: entered\n");
  natp = (struct NaClAppThread *) state;
  NaClLog(4, "natp = 0x%08"PRIxPTR"\n", (uintptr_t) natp);
  NaClLog(4, "prog_ctr  = 0x%08"PRIx32"\n", natp->user.prog_ctr);

  NaClLog(4, "Obtaining thread_num\n");
  /*
   * We have to hold the threads_mu lock until after thread_num field
   * in this thread has been initialized.  All other threads can only
   * find and examine this natp through the threads table, so the fact
   * that natp is not consistent (no thread_num) will not be visible.
   */
  NaClXMutexLock(&natp->nap->threads_mu);
  natp->thread_num = NaClAddThreadMu(natp->nap, natp);
  NaClXMutexUnlock(&natp->nap->threads_mu);
  NaClLog(4, "thread num %d\n", natp->thread_num);

  /*
   * We need to set an exception handler in every thread we start,
   * otherwise the system's default handler is called and a message box is
   * shown.
   */
  WINDOWS_EXCEPTION_TRY;
  NaClStartThreadInApp(natp, natp->user.prog_ctr);
  WINDOWS_EXCEPTION_CATCH;
}

int NaClAppThreadCtor(struct NaClAppThread  *natp,
                      struct NaClApp        *nap,
                      int                   is_privileged,
                      uintptr_t             usr_entry,
                      uintptr_t             usr_stack_ptr,
                      uint32_t              tls_idx) {
  int                         rv;
  uint32_t                    thread_idx;
  struct NaClDescEffectorLdr  *effp;

  NaClLog(4, "natp = 0x%08"PRIxPTR"\n", (uintptr_t) natp);
  NaClLog(4, "nap = 0x%08"PRIxPTR"\n", (uintptr_t) nap);

  NaClThreadContextCtor(&natp->user, nap, usr_entry, usr_stack_ptr, tls_idx);

  effp = NULL;

  if (!NaClMutexCtor(&natp->mu)) {
    return 0;
  }
  if (!NaClCondVarCtor(&natp->cv)) {
    goto cleanup_mutex;
  }

  natp->is_privileged = is_privileged;
  natp->refcount = 1;

  if (!NaClClosureResultCtor(&natp->result)) {
    goto cleanup_cv;
  }
  natp->sysret = 0;
  natp->nap = nap;

  effp = (struct NaClDescEffectorLdr *) malloc(sizeof *effp);
  if (NULL == effp) {
    goto cleanup_cv;
  }

  if (!NaClDescEffectorLdrCtor(effp, natp)) {
    goto cleanup_cv;
  }
  natp->effp = (struct NaClDescEffector *) effp;
  effp = NULL;

  natp->holding_sr_locks = 0;
  natp->state = NACL_APP_THREAD_ALIVE;

  natp->thread_num = -1;  /* illegal index */

  thread_idx = NaClGetThreadIdx(natp);

  nacl_thread[thread_idx] = natp;
  nacl_user[thread_idx] = &natp->user;
  nacl_sys[thread_idx] = &natp->sys;

  rv = NaClThreadCtor(&natp->thread,
                      NaClThreadLauncher,
                      (void *) natp,
                      NACL_KERN_STACK_SIZE);
  if (rv != 0) {
    return rv;
  }
  NaClClosureResultDtor(&natp->result);
 cleanup_cv:
  NaClCondVarDtor(&natp->cv);
 cleanup_mutex:
  NaClMutexDtor(&natp->mu);
  free(effp);
  natp->effp = NULL;
  return 0;
}


void NaClAppThreadDtor(struct NaClAppThread *natp) {
  /*
   * the thread must not be still running, else this crashes the system
   */
  NaClThreadDtor(&natp->thread);
  NaClClosureResultDtor(&natp->result);
  (*natp->effp->vtbl->Dtor)(natp->effp);
  free(natp->effp);
  natp->effp = NULL;
  NaClTlsFree(natp);
  NaClCondVarDtor(&natp->cv);
  NaClMutexDtor(&natp->mu);
}


int NaClAppThreadAllocSegCtor(struct NaClAppThread  *natp,
                              struct NaClApp        *nap,
                              int                   is_privileged,
                              uintptr_t             usr_entry,
                              uintptr_t             usr_stack_ptr,
                              uintptr_t             sys_tdb_base,
                              size_t                tdb_size) {
  uint32_t  tls_idx;

  /*
   * Even though we don't know what segment base/range should gs/r9/nacl_tls_idx
   * select, we still need one, since it identifies the thread when we context
   * switch back.  This use of a dummy tls is only needed for the main thread,
   * which is expected to invoke the tls_init syscall from its crt code (before
   * main or much of libc can run).  Other threads are spawned with the tdb
   * address and size as a parameter.
   */
  tls_idx = NaClTlsAllocate(natp, (void *)sys_tdb_base, tdb_size);

  NaClLog(4,
        "NaClAppThreadAllocSegCtor: stack_ptr 0x%08"PRIxPTR", tls_idx 0x%02x\n",
         usr_stack_ptr, tls_idx);

  if (0 == tls_idx) {
    NaClLog(LOG_ERROR, "No tls for thread, num_thread %d\n", nap->num_threads);
    return 0;
  }

  return NaClAppThreadCtor(natp,
                           nap,
                           is_privileged,
                           usr_entry,
                           usr_stack_ptr,
                           tls_idx);
}


int NaClAppThreadIncRef(struct NaClAppThread *natp) {
  int refcount;

  NaClXMutexLock(&natp->mu);
  if (natp->refcount == 0) {
    NaClLog(LOG_FATAL, "NaClAppThreadIncRef: count was already 0!\n");
  }
  if (++natp->refcount == 0) {
    NaClLog(LOG_FATAL, "NaClAppThreadIncRef: refcount overflow\n");
  }
  refcount = natp->refcount;
  NaClXMutexUnlock(&natp->mu);

  return refcount;
}


int NaClAppThreadDecRef(struct NaClAppThread *natp) {
  int refcount;

  NaClXMutexLock(&natp->mu);
  refcount = --natp->refcount;
  NaClXMutexUnlock(&natp->mu);

  if (0 == refcount) {
    NaClAppThreadDtor(natp);
    free(natp);
  }
  return refcount;
}
