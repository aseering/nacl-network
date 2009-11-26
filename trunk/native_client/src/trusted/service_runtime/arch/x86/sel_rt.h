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

/*
 * NaCl Secure Runtime
 */

#ifndef __NATIVE_CLIENT_SERVICE_RUNTIME_ARCH_X86_SEL_RT_H__
#define __NATIVE_CLIENT_SERVICE_RUNTIME_ARCH_X86_SEL_RT_H__ 1

#include "native_client/src/include/portability.h"

uint16_t NaClGetGlobalCs(void);

uint16_t NaClGetGlobalDs(void);

uint16_t NaClGetCs(void);
/* setting CS is done using an lcall */

uint16_t NaClGetDs(void);

void    NaClSetDs(uint16_t v);

uint16_t NaClGetEs(void);

void    NaClSetEs(uint16_t v);

uint16_t NaClGetFs(void);

void    NaClSetFs(uint16_t v);

uint16_t NaClGetGs(void);

void    NaClSetGs(uint16_t v);

uint16_t NaClGetSs(void);

/* not really a segment registers, but ... */

uint32_t NaClGetEsp(void);

uint32_t NaClGetEbx(void);

/*
 * On a context switch through the syscall interface, not all
 * registers are saved.  We assume that C calling convention is used,
 * so %ecx and %edx are caller-saved and the NaCl service runtime does
 * not have to bother saving them; %eax (or %edx:%eax pair) should
 * have the return value, so its old value is also not saved.  (We
 * should, however, ensure that there is not an accidental covert
 * channel leaking information via these registers on syscall return.)
 * The eflags register is also caller saved.
 *
 * TODO(bsy) if/when we do pre-emptive thread switching (to multiplex
 * user threads on top of kernel threads, for example), we will have
 * to add the full CPU state back in and figure out how to do a full
 * context switch completely in ring 3 code.
 *
 * We assume that the following is packed.  This is true for gcc and
 * msvc for x86, but we will include a check that sizeof(struct
 * NaClThreadContext) == 64 bytes. (32-bit and 64-bit mode)
 */

/* 8-bytes */
union PtrAbstraction {
  uint64_t ptr_64;
  struct {
    uint32_t ptr_padding;
    uint32_t ptr;
  } ptr_32;
};

struct NaClThreadContext {
  /* ecx, edx, eax, eflags not saved */
  uint32_t    ebx, esi, edi, prog_ctr; /* return addr */
  /*          0    4    8    c */
  union PtrAbstraction frame_ptr;
  /*          10 */
  union PtrAbstraction stack_ptr;
  /*          18 */
  uint16_t    ss; /* stack_ptr and ss must be adjacent */
  /*          20 */
  char        dummy[6];
  /*
   * gs is our TLS base in the app; on the host side it's either fs or gs.
   */
  uint16_t    ds, es, fs, gs;
  /*          28  2a  2c  2e */
  /*
   * spring_addr, sys_ret and new_eip are not a part of the thread's
   * register set, but are needed by NaClSwitch. By including them
   * here, the two use the same interface.
   */
  uint32_t    new_eip;
  /*          30 */
  uint32_t    sysret;
  /*          34 */
  uint32_t    spring_addr;
  /*          38 */
  uint16_t    cs; /* spring_addr and cs must be adjacent */
  /*          3c */
};

#endif /* __NATIVE_CLIENT_SERVICE_RUNTIME_ARCH_X86_SEL_RT_H__ */
