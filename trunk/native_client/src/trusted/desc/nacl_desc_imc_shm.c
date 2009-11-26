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
 * NaCl Service Runtime.  Transferrable shared memory objects.
 */

#include "native_client/src/include/portability.h"
#include "native_client/src/include/nacl_platform.h"

#include <stdlib.h>
#include <string.h>

#include "native_client/src/shared/imc/nacl_imc_c.h"
#include "native_client/src/trusted/desc/nacl_desc_base.h"
#include "native_client/src/trusted/desc/nacl_desc_effector.h"
#include "native_client/src/trusted/desc/nacl_desc_io.h"
#include "native_client/src/trusted/desc/nacl_desc_imc_shm.h"

#include "native_client/src/shared/platform/nacl_host_desc.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_sync_checked.h"

#include "native_client/src/trusted/service_runtime/internal_errno.h"
#include "native_client/src/trusted/service_runtime/nacl_config.h"
#include "native_client/src/trusted/service_runtime/include/sys/errno.h"
#include "native_client/src/trusted/service_runtime/include/sys/mman.h"
#include "native_client/src/trusted/service_runtime/include/sys/stat.h"


/*
 * This file contains the implementation of the NaClDescImcShm
 * subclass of NaClDesc.
 *
 * NaClDescImcShm is the subclass that wraps IMC shm descriptors.
 */

int NaClDescImcShmCtor(struct NaClDescImcShm  *self,
                       NaClHandle             h,
                       off_t                  size) {
  struct NaClDesc *basep = (struct NaClDesc *) self;

  /*
   * off_t is signed, but size_t are not; historically size_t is for
   * sizeof and similar, and off_t is also used for stat structure
   * st_size member.  This runtime test detects large object sizes
   * that are silently converted to negative values.
   */
  basep->vtbl = (struct NaClDescVtbl *) NULL;
  if (size < 0) {
    return 0;
  }

  if (!NaClDescCtor(basep)) {
    return 0;
  }
  self->h = h;
  self->size = size;
  basep->vtbl = &kNaClDescImcShmVtbl;
  return 1;
}

void NaClDescImcShmDtor(struct NaClDesc *vself) {
  struct NaClDescImcShm  *self = (struct NaClDescImcShm *) vself;

  NaClClose(self->h);
  self->h = NACL_INVALID_HANDLE;
  vself->vtbl = (struct NaClDescVtbl *) NULL;
  NaClDescDtor(vself);
}

uintptr_t NaClDescImcShmMap(struct NaClDesc         *vself,
                            struct NaClDescEffector *effp,
                            void                    *start_addr,
                            size_t                  len,
                            int                     prot,
                            int                     flags,
                            off_t                   offset) {
  struct NaClDescImcShm  *self = (struct NaClDescImcShm *) vself;

  int       rv;
  int       nacl_prot;
  int       nacl_flags;
  uintptr_t addr;
  uintptr_t end_addr;
  void      *result;
  off_t     tmp_off;

  /*
   * shm must have NACL_ABI_MAP_SHARED in flags, and all calls through
   * this API must supply a start_addr, so NACL_ABI_MAP_FIXED is
   * assumed.
   */
  UNREFERENCED_PARAMETER(flags);
  /*
   * prot must be not be PROT_NONE nor contain other than PROT_{READ|WRITE}
   */
  if (NACL_ABI_PROT_NONE == prot) {
    NaClLog(LOG_INFO, "NaClDescImcShmMap: PROT_NONE not supported\n");
    return -NACL_ABI_EINVAL;
  }
  if (0 != (~(NACL_ABI_PROT_READ | NACL_ABI_PROT_WRITE) & prot)) {
    NaClLog(LOG_INFO,
            "NaClDescImcShmMap: prot has other bits than PROT_{READ|WRITE}\n");
    return -NACL_ABI_EINVAL;
  }
  /*
   * Map from NACL_ABI_ prot and flags bits to IMC library flags,
   * which will later map back into posix-style prot/flags on *x
   * boxen, and to MapViewOfFileEx arguments on Windows.
   */
  nacl_prot = 0;
  if (NACL_ABI_PROT_READ & prot) {
    nacl_prot |= NACL_PROT_READ;
  }
  if (NACL_ABI_PROT_WRITE & prot) {
    nacl_prot |= NACL_PROT_WRITE;
  }
  nacl_flags = NACL_MAP_SHARED | NACL_MAP_FIXED;

  /*
   * For *x, we just map with MAP_FIXED and the kernel takes care of
   * atomically unmapping any existing memory.  For Windows, we must
   * unmap existing memory first, which creates a race condition,
   * where some other innocent thread puts some other memory into the
   * hole, and that memory becomes vulnerable to attack by the
   * untrusted NaCl application.
   *
   * For now, abort the process.  We will need to figure out how to
   * re-architect this code to do the address space move, since it is
   * deep surgery and we'll need to ensure that all threads have
   * stopped and any addresses derived from the old address space
   * would not be on any thread's call stack, i.e., stop the thread in
   * user space or before entering real service runtime code.  This
   * means that no application thread may be indefinitely blocked
   * performing a service call in the service runtime, since otherwise
   * there is no way for us to stop all threads.
   *
   * TODO(bsy): We will probably return an internal error code
   * -NACL_ABI_E_MOVE_ADDRESS_SPACE to ask the caller to do the address space
   * dance.
   */
  for (addr = (uintptr_t) start_addr, end_addr = addr + len, tmp_off = offset;
       addr < end_addr;
       addr += NACL_MAP_PAGESIZE, tmp_off += NACL_MAP_PAGESIZE) {

    /*
     * Minimize the time between the unmap and the map for the same
     * page: we interleave the unmap and map for the pages, rather
     * than do all the unmap first and then do all of the map
     * operations.
     */
    if (0 !=
        (rv = (*effp->vtbl->UnmapMemory)(effp,
                                         addr,
                                         NACL_MAP_PAGESIZE))) {
      NaClLog(LOG_FATAL,
              ("NaClDescImcShmMap: error %d --"
               " could not unmap 0x%08"PRIxPTR", length 0x%x\n"),
              rv,
              addr,
              NACL_MAP_PAGESIZE);
    }

    result = NaClMap((void *) addr, NACL_MAP_PAGESIZE, nacl_prot, nacl_flags,
                     self->h, tmp_off);
    if (NACL_MAP_FAILED == result) {
      return -NACL_ABI_E_MOVE_ADDRESS_SPACE;
    }
    if (result != (void *) addr) {
      NaClLog(LOG_FATAL,
              ("NaClDescImcShmMap: NACL_MAP_FIXED but"
               " got 0x%08"PRIxPTR" instead of 0x%08"PRIxPTR"\n"),
              (uintptr_t) result, addr);
    }
  }
  return (uintptr_t) start_addr;
}

int NaClDescImcShmUnmapCommon(struct NaClDesc         *vself,
                              struct NaClDescEffector *effp,
                              void                    *start_addr,
                              size_t                  len,
                              int                     safe_mode) {
  int       status;
  uintptr_t addr;
  uintptr_t end_addr;

  UNREFERENCED_PARAMETER(vself);

  for (addr = (uintptr_t) start_addr, end_addr = addr + len;
       addr < end_addr;
       addr += NACL_MAP_PAGESIZE) {
    /*
     * Do the unmap "properly" through NaClUnmap, in case that the IMC
     * library is changed to do some bookkeeping.
     */
    status = NaClUnmap((void *) addr, NACL_MAP_PAGESIZE);

    if (0 != status) {
      NaClLog(LOG_FATAL, "NaClDescImcShmUnmapCommon: NaClUnmap failed\n");
      return -NACL_ABI_EINVAL;
    }
    /* there's still a race condition */
    if (safe_mode) {
      if (NaClIsNegErrno((*effp->vtbl->MapAnonymousMemory)(effp,
                                                           addr,
                                                           NACL_MAP_PAGESIZE,
                                                           PROT_NONE))) {
        NaClLog(LOG_ERROR, "NaClDescImcShmUnmapCommon: could not fill hole\n");
        return -NACL_ABI_E_MOVE_ADDRESS_SPACE;
      }
    }
  }
  return 0;
}

int NaClDescImcShmUnmapUnsafe(struct NaClDesc         *vself,
                              struct NaClDescEffector *effp,
                              void                    *start_addr,
                              size_t                  len) {
  return NaClDescImcShmUnmapCommon(vself, effp, start_addr, len, 0);
}

int NaClDescImcShmUnmap(struct NaClDesc         *vself,
                        struct NaClDescEffector *effp,
                        void                    *start_addr,
                        size_t                  len) {
  return NaClDescImcShmUnmapCommon(vself, effp, start_addr, len, 1);
}

int NaClDescImcShmFstat(struct NaClDesc         *vself,
                        struct NaClDescEffector *effp,
                        struct nacl_abi_stat    *stbp) {
  struct NaClDescImcShm  *self = (struct NaClDescImcShm *) vself;

  UNREFERENCED_PARAMETER(effp);

  stbp->nacl_abi_st_dev = 0;
  stbp->nacl_abi_st_ino = 0x6c43614e;
  stbp->nacl_abi_st_mode = NACL_ABI_S_IFREG | NACL_ABI_S_IRWXU;
  stbp->nacl_abi_st_nlink = 1;
  stbp->nacl_abi_st_uid = -1;
  stbp->nacl_abi_st_gid = -1;
  stbp->nacl_abi_st_rdev = 0;
  stbp->nacl_abi_st_size = self->size;  /* the only real reason for fstat */
  stbp->nacl_abi_st_blksize = 0;
  stbp->nacl_abi_st_blocks = 0;
  stbp->nacl_abi_st_atime = 0;
  stbp->nacl_abi_st_mtime = 0;
  stbp->nacl_abi_st_ctime = 0;

  return 0;
}

int NaClDescImcShmClose(struct NaClDesc         *vself,
                        struct NaClDescEffector *effp) {
  UNREFERENCED_PARAMETER(effp);

  NaClDescUnref(vself);
  return 0;
}

int NaClDescImcShmExternalizeSize(struct NaClDesc *vself,
                                  size_t          *nbytes,
                                  size_t          *nhandles) {
  struct NaClDescImcShm  *self = (struct NaClDescImcShm *) vself;

  *nbytes = sizeof self->size;
  *nhandles = 1;

  return 0;
}

int NaClDescImcShmExternalize(struct NaClDesc           *vself,
                              struct NaClDescXferState  *xfer) {
  struct NaClDescImcShm  *self = (struct NaClDescImcShm *) vself;

  *xfer->next_handle++ = self->h;
  memcpy(xfer->next_byte, &self->size, sizeof self->size);
  xfer->next_byte += sizeof self->size;
  return 0;
}

struct NaClDescVtbl const kNaClDescImcShmVtbl = {
  NaClDescImcShmDtor,
  NaClDescImcShmMap,
  NaClDescImcShmUnmapUnsafe,
  NaClDescImcShmUnmap,
  NaClDescReadNotImplemented,
  NaClDescWriteNotImplemented,
  NaClDescSeekNotImplemented,
  NaClDescIoctlNotImplemented,
  NaClDescImcShmFstat,
  NaClDescImcShmClose,
  NaClDescGetdentsNotImplemented,
  NACL_DESC_SHM,
  NaClDescImcShmExternalizeSize,
  NaClDescImcShmExternalize,
  NaClDescLockNotImplemented,
  NaClDescTryLockNotImplemented,
  NaClDescUnlockNotImplemented,
  NaClDescWaitNotImplemented,
  NaClDescTimedWaitAbsNotImplemented,
  NaClDescSignalNotImplemented,
  NaClDescBroadcastNotImplemented,
  NaClDescSendMsgNotImplemented,
  NaClDescRecvMsgNotImplemented,
  NaClDescConnectAddrNotImplemented,
  NaClDescAcceptConnNotImplemented,
  NaClDescPostNotImplemented,
  NaClDescSemWaitNotImplemented,
  NaClDescGetValueNotImplemented,
};

int NaClDescImcShmInternalize(struct NaClDesc           **baseptr,
                              struct NaClDescXferState  *xfer) {
  int                   rv;
  struct NaClDescImcShm *ndisp;
  NaClHandle            h;
  off_t                 hsize;

  rv = -NACL_ABI_EIO;
  ndisp = NULL;

  if (xfer->next_handle == xfer->handle_buffer_end) {
    rv = -NACL_ABI_EIO;
    goto cleanup;
  }
  if (xfer->next_byte + sizeof ndisp->size > xfer->byte_buffer_end) {
    rv = -NACL_ABI_EIO;
    goto cleanup;
  }

  ndisp = malloc(sizeof *ndisp);
  if (NULL == ndisp) {
    rv = -NACL_ABI_ENOMEM;
    goto cleanup;
  }

  h = *xfer->next_handle;
  *xfer->next_handle++ = NACL_INVALID_HANDLE;
  memcpy(&hsize, xfer->next_byte, sizeof hsize);
  xfer->next_byte += sizeof hsize;

  NaClDescImcShmCtor(ndisp, h, hsize);

  *baseptr = (struct NaClDesc *) ndisp;
  rv = 0;

cleanup:
  if (rv < 0) {
    free(ndisp);
  }
  return rv;
}
