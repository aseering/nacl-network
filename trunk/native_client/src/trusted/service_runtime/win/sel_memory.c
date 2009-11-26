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
 * NaCl Service Runtime memory allocation code
 */
#include "native_client/src/include/portability.h"

#include <errno.h>
#include <windows.h>
#include <string.h>

#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/win/xlate_system_error.h"

#include "native_client/src/trusted/service_runtime/win/mman.h"
#include "native_client/src/trusted/service_runtime/nacl_config.h"
#include "native_client/src/trusted/service_runtime/sel_memory.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"

/*
 * NaCl_page_free: free pages allocated with NaCl_page_alloc.
 * Must start at allocation granularity (NACL_MAP_PAGESIZE) and
 * number of bytes must be a multiple of allocation granularity.
 */
void  NaCl_page_free(void     *p,
                     size_t   num_bytes) {
  void  *end_addr;

  end_addr = (void *) (((char *) p) + num_bytes);
  while (p < end_addr) {
    if (!VirtualFree(p, 0, MEM_RELEASE)) {
      NaClLog(0,
              "NaCl_page_free: VirtualFree(0x%08x, 0, MEM_RELEASE) failed\n",
              (uintptr_t) p);
    }
    p = (void *) (((char *) p) + NACL_MAP_PAGESIZE);
  }
}


int   NaCl_page_alloc(void    **p,
                      size_t  num_bytes) {
  SYSTEM_INFO sys_info;

  int         attempt_count;

  void        *addr;
  void        *end_addr;
  void        *chunk;
  void        *unroll;

  /*
   * We have to allocate every 64KB -- the windows allocation
   * granularity -- because VirtualFree will only accept an address
   * that was returned by a call to VirtualAlloc.  NB: memory pages
   * put into the address space via MapViewOfFile{,Ex} must be
   * released by UnmapViewOfFile.  Thus, in order for us to open up a
   * hole in the NaCl application's address space to map in a file, we
   * must allocate the entire address space in 64KB chunks, so we can
   * later pick an arbitrary range of addresses (in 64KB chunks) to
   * free up and map in a file later.
   *
   * First, we verify via GetSystemInfo that the allocation
   * granularity matches NACL_MAP_PAGESIZE.
   *
   * Next, we VirtualAlloc the entire chunk desired.  This essentially
   * asks the kernel where there is space in the virtual address
   * space.  Then, we free this back, and repeat the allocation
   * starting at the returned address, but in 64KB chunks.  If any of
   * these smaller allocations fail, we roll back and try again.
   */

  NaClLog(3, "NaCl_page_alloc(*, 0x%x)\n", num_bytes);
  GetSystemInfo(&sys_info);
  if (NACL_PAGESIZE != sys_info.dwPageSize) {
    NaClLog(2, "page size is 0x%x; expected 0x%x\n",
            sys_info.dwPageSize,
            NACL_PAGESIZE);
  }
  if (NACL_MAP_PAGESIZE != sys_info.dwAllocationGranularity) {
    NaClLog(0, "allocation granularity is 0x%x; expected 0x%x\n",
            sys_info.dwAllocationGranularity,
            NACL_MAP_PAGESIZE);
  }

  /*
   * Round allocation request up to next NACL_MAP_PAGESIZE.  This is
   * assumed to have taken place in NaCl_page_free.
   */
  num_bytes = NaClRoundAllocPage(num_bytes);

  for (attempt_count = 0;
       attempt_count < NACL_MEMORY_ALLOC_RETRY_MAX;
       ++attempt_count) {

    addr = VirtualAlloc(NULL,
                        num_bytes,
                        MEM_RESERVE,
                        PAGE_EXECUTE_READWRITE);
    if (addr == NULL) {
      NaClLog(0,
              ("NaCl_page_alloc:"
               "  VirtualAlloc(*,0x%x,MEM_COMMIT,PAGE_EXECUTE_READWRITE)"
               " failed\n"),
              num_bytes);
      return -ENOMEM;
    }
    NaClLog(3,
            ("NaCl_page_alloc:"
             "  VirtualAlloc(*,0x%x,MEM_COMMIT,PAGE_EXECUTE_READWRITE)"
             " succeeded, 0x%08x,"
             " releasing and re-allocating in 64K chunks....\n"),
            num_bytes, addr);
    (void) VirtualFree(addr, 0, MEM_RELEASE);
    /*
     * We now know [addr, addr + num_bytes) is available in our addr space.
     * Get that memory again, but in 64KB chunks.
     */
    end_addr = (void *) (((char *) addr) + num_bytes);
    for (chunk = addr;
         chunk < end_addr;
         chunk = (void *) (((char *) chunk) + NACL_MAP_PAGESIZE)) {
      if (NULL == VirtualAlloc(chunk,
                               NACL_MAP_PAGESIZE,
                               MEM_COMMIT | MEM_RESERVE,
                               PAGE_EXECUTE_READWRITE)) {
        NaClLog(0, ("NaCl_page_alloc: re-allocation failed at 0x%08x,"
                    " error %d\n"),
                chunk, GetLastError());
        for (unroll = addr;
             unroll < chunk;
             unroll = (void *) (((char *) unroll) + NACL_MAP_PAGESIZE)) {
          (void) VirtualFree(unroll, 0, MEM_RELEASE);
        }
        goto retry;
        /* double break */
      }
    }
    NaClLog(2, "NaCl_page_alloc: *p = 0x%08x, returning success.\n", addr);
    *p = addr;
    return 0;
 retry:
    ;
  }

  return -ENOMEM;
}


uintptr_t NaClProtectChunkSize(uintptr_t start,
                               uintptr_t end) {
  uintptr_t chunk_end;

  chunk_end = NaClRoundAllocPage(start + 1);
  if (chunk_end > end) {
    chunk_end = end;
  }
  return chunk_end - start;
}


/*
 * This is critical to make the text region non-writable, and the data
 * region read/write but no exec.  Of course, some kernels do not
 * respect the lack of PROT_EXEC.
 */
int   NaCl_mprotect(void          *addr,
                    size_t        len,
                    int           prot) {
  uintptr_t start_addr;
  uintptr_t end_addr;
  uintptr_t cur_addr;
  uintptr_t cur_chunk_size;
  DWORD     newProtection, oldProtection;
  BOOL      res;

  NaClLog(2, "NaCl_mprotect(0x%08x, 0x%x, 0x%x)\n",
          (uintptr_t) addr, len, prot);

  if (len == 0) {
    return 0;
  }

  start_addr = (uintptr_t) addr;
  if (!NaClIsPageMultiple(start_addr)) {
    NaClLog(2, "NaCl_mprotect: start address not a multiple of page size\n");
    return -EINVAL;
  }
  if (!NaClIsPageMultiple(len)) {
    NaClLog(2, "NaCl_mprotect: length not a multiple of page size\n");
    return -EINVAL;
  }
  end_addr = start_addr + len;

  switch (prot) {
    case PROT_EXEC: {
      newProtection = PAGE_EXECUTE;
      break;
    }
    case PROT_EXEC|PROT_READ: {
      newProtection = PAGE_EXECUTE_READ;
      break;
    }
    case PROT_EXEC|PROT_READ|PROT_WRITE: {
      newProtection = PAGE_EXECUTE_READWRITE;
      break;
    }
    case PROT_READ: {
      newProtection = PAGE_READONLY;
      break;
    }
    case PROT_READ|PROT_WRITE: {
      newProtection = PAGE_READWRITE;
      break;
    }
    case PROT_NONE: {
      newProtection = PAGE_NOACCESS;
      break;
    }
    default: {
      NaClLog(2, "NaCl_mprotect: invalid protection mode\n");
      return -EINVAL;
    }
  }
  NaClLog(2, "NaCl_mprotect: newProtection = 0x%x\n", newProtection);
  /*
   * VirtualProtect region cannot span allocations: all addresses from
   * [lpAddress, lpAddress+dwSize) must be in one region of memory
   * returned from VirtualAlloc or VirtualAllocEx
   */
  for (cur_addr = start_addr,
           cur_chunk_size = NaClProtectChunkSize(cur_addr, end_addr);
       cur_addr < end_addr;
       cur_addr += cur_chunk_size,
           cur_chunk_size = NaClProtectChunkSize(cur_addr, end_addr)) {
    NaClLog(4, "NaCl_mprotect: VirtualProtect(0x%08x, 0x%x, 0x%x, *)\n",
            cur_addr, cur_chunk_size, newProtection);
    res = VirtualProtect((void*) cur_addr, cur_chunk_size,
                         newProtection, &oldProtection);
    if (!res) {
      NaClLog(2, "NaCl_mprotect: ... failed\n");
      return -NaClXlateSystemError(GetLastError());
    }
  }
  NaClLog(2, "NaCl_mprotect: done\n");
  return 0;
}


int   NaCl_madvise(void           *start,
                   size_t         length,
                   int            advice) {
  int       err;
  uintptr_t start_addr;
  uintptr_t end_addr;
  uintptr_t cur_addr;
  uintptr_t cur_chunk_size;

  /*
   * MADV_DONTNEED and MADV_NORMAL are needed
   */
  NaClLog(2, "NaCl_madvise(0x%08x, 0x%x, 0x%x)\n",
          (uintptr_t) start, length, advice);
  switch (advice) {
    case MADV_DONTNEED:
      start_addr = (uintptr_t) start;
      if (!NaClIsPageMultiple(start_addr)) {
        NaClLog(2,
                "NaCl_madvise: start address not a multiple of page size\n");
        return -EINVAL;
      }
      if (!NaClIsPageMultiple(length)) {
        NaClLog(2, "NaCl_madvise: length not a multiple of page size\n");
        return -EINVAL;
      }
      end_addr = start_addr + length;
      for (cur_addr = start_addr,
               cur_chunk_size = NaClProtectChunkSize(cur_addr, end_addr);
           cur_addr < end_addr;
           cur_addr += cur_chunk_size,
               cur_chunk_size = NaClProtectChunkSize(cur_addr, end_addr)) {
        NaClLog(4,
                ("NaCl_madvise: MADV_DONTNEED"
                 " -> VirtualAlloc(0x%08x, 0x%x, MEM_RESET, PAGE_NOACCESS)\n"),
                cur_addr, cur_chunk_size);
        if (NULL == VirtualAlloc((void*) cur_addr, cur_chunk_size, MEM_RESET,
                                 PAGE_NOACCESS)) {
          err = NaClXlateSystemError(GetLastError());
          NaClLog(2, "NaCl_madvise: VirtualAlloc failed: 0x%x\n", err);
          return -err;
        }
      }
      break;
    case MADV_NORMAL:
      memset(start, 0, length);
      break;
    default:
      return -EINVAL;
  }
  NaClLog(2, "NaCl_madvise: done\n");
  return 0;
}
