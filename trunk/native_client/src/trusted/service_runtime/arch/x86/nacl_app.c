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
 * NaCl run time.
 */
/* TODO: Is this needed */
#include <fcntl.h>

#include "native_client/src/shared/platform/nacl_host_desc.h"
#include "native_client/src/shared/platform/nacl_sync_checked.h"

#include "native_client/src/trusted/service_runtime/nacl_check.h"
#include "native_client/src/trusted/service_runtime/arch/x86/nacl_ldt_x86.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"


static uint16_t NaClAllocateSegmentForCodeRegion(struct NaClApp *nap) {
  uintptr_t code_start = nap->mem_start;
  size_t    code_bytes = NACL_TRAMPOLINE_END + nap->text_region_bytes;
  size_t    code_pages = code_bytes >> NACL_PAGESHIFT;

  VCHECK((code_bytes & ((1 << NACL_PAGESHIFT) - 1)) == 0,
        ("code_bytes (0x%08"PRIxS") is not page aligned\n",
         code_bytes));

  if (code_pages < 1) {
    NaClLog(LOG_FATAL, "NaClAppPrepareToLaunch: fewer than one code pages?\n");
  }
  NaClLog(2,
          "NaClLdtAllocatePageSelector(code, 1, 0x%08"PRIxPTR", 0x%"PRIxS"\n",
          code_start, code_pages);

  return NaClLdtAllocatePageSelector(NACL_LDT_DESCRIPTOR_CODE,
                                     1,
                                     (void *) code_start,
                                     code_pages);
}


/*
 * NB: in our memory model, we roughly follow standard 7th edition unix but with
 * a >16-bit address space: data and code overlap, and the start of the data
 * segment is the same as the start of the code region; and the data segment
 * actually includes the memory hole between the break and the top of the stack,
 * as well as the stack and environment variables and other things in memory
 * above the stack.
 *
 * The code pages, which is marked read-only via the page protection mechanism,
 * could be viewed as read-only data.  Nothing prevents a NaCl application from
 * looking at its own code.
 *
 * The same segment selector is used for ds, es, and ss, and thus "des_seg".
 * Nuthin' to do with the old Data Encryption Standard.
 */
static uint16_t NaClAllocateSegmentForDataRegion(struct NaClApp *nap) {
  uintptr_t           data_start = nap->mem_start;
  size_t              data_pages = 1 << (nap->addr_bits - NACL_PAGESHIFT);

  CHECK(nap->addr_bits > NACL_PAGESHIFT);

  if (data_pages < 1) {
    NaClLog(LOG_FATAL,
            "NaClAppPrepareToLaunch: address space is fewer than one page?\n");
  }
  NaClLog(2,
          "NaClLdtAllocatePageSelector(data, 1, 0x%08"PRIxPTR", 0x%"PRIxS"\n",
          data_start, data_pages - 1);

  return NaClLdtAllocatePageSelector(NACL_LDT_DESCRIPTOR_DATA,
                                     0,
                                     (void *) data_start,
                                     data_pages);
}


/*
 * Allocate ldt for app, without creating the main thread.
 */
NaClErrorCode NaClAppPrepareToLaunch(struct NaClApp     *nap,
                                     int                in_desc,
                                     int                out_desc,
                                     int                err_desc) {
  uint16_t            cs;
  uint16_t            des_seg;

  int                 i;
  struct NaClHostDesc *nhdp;

  int                 descs[3];

  NaClErrorCode       retval = LOAD_INTERNAL;

  descs[0] = in_desc;
  descs[1] = out_desc;
  descs[2] = err_desc;

  NaClXMutexLock(&nap->mu);

  cs = NaClAllocateSegmentForCodeRegion(nap);

  NaClLog(2, "got 0x%x\n", cs);
  if (0 == cs) {
    retval = SRT_NO_SEG_SEL;
    goto done;
  }

  des_seg = NaClAllocateSegmentForDataRegion(nap);

  NaClLog(2, "got 0x%x\n", des_seg);
  if (0 == des_seg) {
    NaClLdtDeleteSelector(cs);
    retval = SRT_NO_SEG_SEL;
    goto done;
  }

  nap->code_seg_sel = cs;
  nap->data_seg_sel = des_seg;

  /*
   * Note that gs is thread-specific and not global, so that is allocated
   * elsewhere.  See nacl_app_thread.c.
   */

  /*
   * We dup the stdin, stdout, and stderr descriptors and wrap them in
   * NaClHostDesc objects.  Those in turn are wrapped by the
   * NaClDescIoDesc subclass of NaClDesc, and then put into the
   * open-file table.  NaCl app I/O operations will use these shared
   * descriptors, and if they close one of these descriptors it will
   * only be a duplicated descriptor.  NB: some fcntl/ioctl flags
   * apply to the descriptor (e.g., O_CLOEXEC) and some apply to the
   * underlying open file entry (e.g., O_NONBLOCK), so changes by the
   * NaCl app could affect the service runtime.
   */
  for (i = 0; i < 3; ++i) {
    nhdp = malloc(sizeof *nhdp);
    if (NULL == nhdp) {
      NaClLog(LOG_FATAL,
              "NaClAppPrepareToLaunch: no memory for abstract descriptor %d\n",
              i);
    }
    NaClHostDescPosixDup(nhdp, descs[i], (0 == i) ? O_RDONLY : O_WRONLY);
    NaClSetDesc(nap, i, (struct NaClDesc *) NaClDescIoDescMake(nhdp));
  }

  retval = LOAD_OK;
done:
  NaClXMutexUnlock(&nap->mu);
  return retval;
}
