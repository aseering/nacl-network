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
 * NaCl Service Runtime.  I/O Descriptor / Handle abstraction.  Memory
 * mapping using descriptors.
 *
 * Note that we avoid using the thread-specific data / thread local
 * storage access to the "errno" variable, and instead use the raw
 * system call return interface of small negative numbers as errors.
 */

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

#include "native_client/src/include/nacl_platform.h"
#include "native_client/src/include/portability.h"

#include "native_client/src/shared/platform/nacl_host_desc.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/service_runtime/nacl_config.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"
#include "native_client/src/trusted/service_runtime/sel_memory.h"

#include "native_client/src/trusted/service_runtime/include/sys/errno.h"
#include "native_client/src/trusted/service_runtime/include/sys/fcntl.h"
#include "native_client/src/trusted/service_runtime/include/bits/mman.h"
#include "native_client/src/trusted/service_runtime/include/sys/stat.h"

/*
 * Map our ABI to the host OS's ABI.  On linux, this should be a big no-op.
 */
static INLINE int NaClMapOpenFlags(int nacl_flags) {
  int host_os_flags;

  nacl_flags &= (NACL_ABI_O_ACCMODE | NACL_ABI_O_CREAT
                 | NACL_ABI_O_TRUNC | NACL_ABI_O_APPEND);

  host_os_flags = 0;
#define C(H) case NACL_ABI_ ## H: \
  host_os_flags |= H;             \
  break;
  switch (nacl_flags & NACL_ABI_O_ACCMODE) {
    C(O_RDONLY);
    C(O_WRONLY);
    C(O_RDWR);
  }
#undef C
#define M(H) do { \
    if (0 != (nacl_flags & NACL_ABI_ ## H)) {   \
      host_os_flags |= H;                       \
    }                                           \
  } while (0)
  M(O_CREAT);
  M(O_TRUNC);
  M(O_APPEND);
#undef M
  return host_os_flags;
}

static INLINE int NaClMapOpenPerm(int nacl_perm) {
  int host_os_perm;

  host_os_perm = 0;
#define M(H) do { \
    if (0 != (nacl_perm & NACL_ABI_ ## H)) { \
      host_os_perm |= H; \
    } \
  } while (0)
  M(S_IRUSR);
  M(S_IWUSR);
#undef M
  return host_os_perm;
}

static INLINE int NaClMapFlagMap(int nacl_map_flags) {
  int host_os_flags;

  host_os_flags = 0;
#define M(H) do { \
    if (0 != (nacl_map_flags & NACL_ABI_ ## H)) { \
      host_os_flags |= H; \
    } \
  } while (0)
  M(MAP_SHARED);
  M(MAP_PRIVATE);
  M(MAP_FIXED);
  M(MAP_ANONYMOUS);
#undef M

  return host_os_flags;
}

uintptr_t NaClHostDescMap(struct NaClHostDesc *d,
                          void                *start_addr,
                          size_t              len,
                          int                 prot,
                          int                 flags,
                          off_t               offset) {
  int   desc;
  void  *map_addr;
  int   host_prot;
  int   host_flags;

  NaClLog(4,
          ("NaClHostDescMap(0x%08"PRIxPTR", 0x%08"PRIxPTR", 0x%08"PRIxS","
           " 0x%x, 0x%x, 0x%08"PRIx64")\n"),
          (uintptr_t) d,
          (uintptr_t) start_addr,
          len,
          prot,
          flags,
          (int64_t) offset);
  if (NULL == d && 0 == (flags & NACL_ABI_MAP_ANONYMOUS)) {
    NaClLog(LOG_FATAL, "NaClHostDescMap: 'this' is NULL and not anon map\n");
  }
  prot &= (NACL_ABI_PROT_READ | NACL_ABI_PROT_WRITE);
  /* may be PROT_NONE too */
  flags &= NACL_ABI_MAP_ANONYMOUS;
  /* NACL_ABI_MAP_SHARED is ignored */
  flags |= NACL_ABI_MAP_FIXED | NACL_ABI_MAP_PRIVATE;
  /* supplied start_addr must be okay, mapping must be private! */

  if (flags & NACL_ABI_MAP_ANONYMOUS) {
    desc = -1;
  } else {
    desc = d->d;
  }
  /*
   * Translate flags, prot to host_flags, host_prot.
   */
  host_flags = NaClMapFlagMap(flags);
  host_prot = NaClProtMap(prot);

  map_addr = mmap(start_addr, len, host_prot, host_flags, desc, offset);

  if (MAP_FAILED == map_addr) {
    NaClLog(LOG_INFO,
            ("NaClHostDescMap:"
             " mmap(0x%08"PRIxPTR", 0x%"PRIxS", 0x%x, 0x%x, 0x%d, 0x%"PRIx64")"
             " failed, errno %d.\n"),
            (uintptr_t) start_addr, len, host_prot, host_flags, desc,
            (int64_t) offset,
            errno);
    return -NaClXlateErrno(errno);
  }
  if (map_addr != start_addr) {
    NaClLog(LOG_FATAL,
            ("NaClHostDescMap: mmap with MAP_FIXED not fixed:"
             " returned 0x%08"PRIxPTR" instead of 0x%08"PRIxPTR"\n"),
            (uintptr_t) map_addr,
            (uintptr_t) start_addr);
  }
  NaClLog(4, "NaClHostDescMap: returning 0x%08"PRIxPTR"\n",
          (uintptr_t) start_addr);

  return (uintptr_t) start_addr;
}

int NaClHostDescUnmapUnsafe(void    *start_addr,
                            size_t  len) {
  int       retval;
  uintptr_t addr;

  addr = (uintptr_t) start_addr;

  return ((-1 == (retval = munmap(start_addr, len)))
          ? -NaClXlateErrno(errno)
          : retval);
}

int NaClHostDescUnmap(void    *start_addr,
                      size_t  len) {
  int       retval;
  uintptr_t addr;

  addr = (uintptr_t) start_addr;

  return ((-1 == (retval = (uintptr_t) mmap(start_addr,
                                            len,
                                            PROT_NONE,
                                            (MAP_PRIVATE
                                             | MAP_ANONYMOUS | MAP_FIXED),
                                            -1,
                                            (off_t) 0)))
          ? -NaClXlateErrno(errno) : retval);
}

int NaClHostDescOpen(struct NaClHostDesc  *d,
                     char                 *path,
                     int                  flags,
                     int                  mode) {
  int host_desc;

  NaClLog(3, "NaClHostDescOpen(0x%08"PRIxPTR", %s, 0x%x, 0x%x)\n",
          (uintptr_t) d, path, flags, mode);
  if (NULL == d) {
    NaClLog(LOG_FATAL, "NaClHostDescOpen: 'this' is NULL\n");
  }
  /*
   * Sanitize access flags.
   */
  if (0 != (flags & ~(NACL_ABI_O_ACCMODE | NACL_ABI_O_CREAT
                      | NACL_ABI_O_TRUNC | NACL_ABI_O_APPEND))) {
    return -NACL_ABI_EINVAL;
  }

  switch (flags & NACL_ABI_O_ACCMODE) {
    case NACL_ABI_O_RDONLY:
    case NACL_ABI_O_WRONLY:
    case NACL_ABI_O_RDWR:
      break;
    default:
      NaClLog(LOG_ERROR,
              "NaClHostDescOpen: bad access flags 0x%x.\n",
              flags);
      return -NACL_ABI_EINVAL;
  }

  flags = NaClMapOpenFlags(flags);
  mode = NaClMapOpenPerm(mode);

  NaClLog(3, "NaClHostDescOpen: invoking POSIX open(%s,0x%x,0%o)\n",
          path, flags, mode);
  host_desc = open(path, flags, mode);
  NaClLog(3, "NaClHostDescOpen: got descriptor %d\n", host_desc);
  if (-1 == host_desc) {
    NaClLog(LOG_ERROR,
            "NaClHostDescOpen: open returned -1, errno %d\n", errno);
    return -NaClXlateErrno(errno);
  }
  d->d = host_desc;
  NaClLog(3, "NaClHostDescOpen: success.\n");
  return 0;
}

int NaClHostDescPosixDup(struct NaClHostDesc  *d,
                         int                  posix_d,
                         int                  mode) {
  int host_desc;

  if (NULL == d) {
    NaClLog(LOG_FATAL, "NaClHostDescPosixDup: 'this' is NULL\n");
  }
  /*
   * Sanitize access modes.
   */
  if (0 != (mode & ~O_ACCMODE)) {
    return -NACL_ABI_EINVAL;
  }

  switch (mode & O_ACCMODE) {
    case O_RDONLY:
    case O_WRONLY:
    case O_RDWR:
      break;
    default:
      NaClLog(LOG_ERROR,
              "NaClHostDescPosixDup: bad access mode 0x%x.\n",
              mode);
      return -NACL_ABI_EINVAL;
  }

  host_desc = dup(posix_d);
  if (-1 == host_desc) {
    return -NACL_ABI_EINVAL;
  }
  d->d = host_desc;
  return 0;
}

int NaClHostDescPosixTake(struct NaClHostDesc *d,
                          int                 posix_d,
                          int                 mode) {
  if (NULL == d) {
    NaClLog(LOG_FATAL, "NaClHostDescPosixTake: 'this' is NULL\n");
  }
  /*
   * Sanitize access modes.
   */
  if (0 != (mode & ~O_ACCMODE)) {
    return -NACL_ABI_EINVAL;
  }

  switch (mode & O_ACCMODE) {
    case O_RDONLY:
    case O_WRONLY:
    case O_RDWR:
      break;
    default:
      NaClLog(LOG_ERROR,
              "NaClHostDescPosixTake: bad access mode 0x%x.\n",
              mode);
      return -NACL_ABI_EINVAL;
  }

  d->d = posix_d;
  return 0;
}

ssize_t NaClHostDescRead(struct NaClHostDesc  *d,
                         void                 *buf,
                         size_t               len) {
  ssize_t retval;

  if (NULL == d) {
    NaClLog(LOG_FATAL, "NaClHostDescRead: 'this' is NULL\n");
  }
  return ((-1 == (retval = read(d->d, buf, len)))
          ? -NaClXlateErrno(errno) : retval);
}

ssize_t NaClHostDescWrite(struct NaClHostDesc *d,
                          void const          *buf,
                          size_t              len) {
  ssize_t retval;

  if (NULL == d) {
    NaClLog(LOG_FATAL, "NaClHostDescWrite: 'this' is NULL\n");
  }
  return ((-1 == (retval = write(d->d, buf, len)))
          ? -NaClXlateErrno(errno) : retval);
}

int NaClHostDescSeek(struct NaClHostDesc  *d,
                     off_t                offset,
                     int                  whence) {
  int retval;

  if (NULL == d) {
    NaClLog(LOG_FATAL, "NaClHostDescSeek: 'this' is NULL\n");
  }
  return ((-1 == (retval = lseek(d->d, offset, whence)))
          ? -NaClXlateErrno(errno) : retval);
}

int NaClHostDescIoctl(struct NaClHostDesc *d,
                      int                 request,
                      void                *arg) {
#if 0
  int retval;

  if (NULL == d) {
    NaClLog(LOG_FATAL, "NaClHostDescIoctl: 'this' is NULL\n");
  }
  /*
   * Validate arg according to request.  Arrgh indeed.
   */
  return ((-1 == (retval = ioctl(d->d, request, arg)))
          ? -NaClXlateErrno(errno) : retval);
#else
  UNREFERENCED_PARAMETER(request);
  UNREFERENCED_PARAMETER(arg);

  if (NULL == d) {
    NaClLog(LOG_FATAL, "NaClHostDescIoctl: 'this' is NULL\n");
  }
  return -NACL_ABI_ENOSYS;
#endif
}

void NaClHostDescStatCommon(struct nacl_abi_stat  *nasp,
                            struct stat           *sbp) {
  nacl_abi_mode_t m;

  nasp->nacl_abi_st_dev = 0;
  nasp->nacl_abi_st_ino = 0x6c43614e;

  switch (sbp->st_mode & S_IFMT) {
    case S_IFREG:
      m = NACL_ABI_S_IFREG;
      break;
    case S_IFDIR:
      m = NACL_ABI_S_IFDIR;
      break;
#if defined(S_IFCHR)
    case S_IFCHR:
      /* stdin/out/err can be inherited, so this is okay */
      m = NACL_ABI_S_IFCHR;
      break;
#endif
    default:
      NaClLog(LOG_ERROR,
              ("NaClHostDescStatCommon: how did NaCl app open a file"
               " with st_mode = 0%o?\n"),
              sbp->st_mode);
      m = NACL_ABI_S_UNSUP;
  }
  if (0 != (nasp->nacl_abi_st_mode & S_IRUSR)) {
      m |= NACL_ABI_S_IRUSR;
  }
  if (0 != (nasp->nacl_abi_st_mode & S_IWUSR)) {
      m |= NACL_ABI_S_IWUSR;
  }
  if (0 != (nasp->nacl_abi_st_mode & S_IXUSR)) {
      m |= NACL_ABI_S_IXUSR;
  }
  nasp->nacl_abi_st_mode = m;
  nasp->nacl_abi_st_nlink = sbp->st_nlink;
  nasp->nacl_abi_st_uid = -1;  /* not root */
  nasp->nacl_abi_st_gid = -1;  /* not wheel */
  nasp->nacl_abi_st_rdev = 0;
  nasp->nacl_abi_st_size = sbp->st_size;
  nasp->nacl_abi_st_blksize = 0;
  nasp->nacl_abi_st_blocks = 0;
  nasp->nacl_abi_st_atime = sbp->st_atime;
  nasp->nacl_abi_st_mtime = sbp->st_mtime;
  nasp->nacl_abi_st_ctime = sbp->st_ctime;
}

/*
 * See NaClHostDescStat below.
 */
int NaClHostDescFstat(struct NaClHostDesc  *d,
                      struct nacl_abi_stat *nasp) {
  struct stat stbuf;

  if (fstat(d->d, &stbuf) == -1) {
    return -errno;
  }

  NaClHostDescStatCommon(nasp, &stbuf);

  return 0;
}

int NaClHostDescClose(struct NaClHostDesc *d) {
  int retval;

  if (NULL == d) {
    NaClLog(LOG_FATAL, "NaClHostDescClose: 'this' is NULL\n");
  }
  retval = close(d->d);
  if (-1 != retval) {
    d->d = -1;
  }
  return (-1 == retval) ? -NaClXlateErrno(errno) : retval;
}

/*
 * This is not a host descriptor function, but is closely related to
 * fstat and should behave similarly.
 */
int NaClHostDescStat(char const           *host_os_pathname,
                     struct nacl_abi_stat *nasp) {
  struct stat stbuf;

  if (stat(host_os_pathname, &stbuf) == -1) {
    return -errno;
  }

  NaClHostDescStatCommon(nasp, &stbuf);

  return 0;
}
