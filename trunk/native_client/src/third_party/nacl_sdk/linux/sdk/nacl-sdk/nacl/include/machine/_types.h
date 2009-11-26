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
 * NaCl kernel / service run-time system call ABI.
 * This file defines nacl target machine dependent types.
 */

#ifndef SERVICE_RUNTIME_INCLUDE_MACHINE__TYPES_H_
#define SERVICE_RUNTIME_INCLUDE_MACHINE__TYPES_H_

#ifdef __native_client__
# include <stdint.h>
# include <machine/_default_types.h>
#else
# include "native_client/src/include/portability.h"
#endif

#define __need_size_t
#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

#define NACL_CONCAT3_(a, b, c) a ## b ## c
#define NACL_PRI_(fmt, size) NACL_CONCAT3_(PRI, fmt, size)

#ifndef __dev_t_defined
#define __dev_t_defined
typedef int64_t __dev_t;
#ifndef __native_client__
typedef __dev_t dev_t;
#endif
#endif

#define PRIdNACL_DEV NACL_PRI_(d, 64)
#define PRIiNACL_DEV NACL_PRI_(i, 64)
#define PRIoNACL_DEV NACL_PRI_(o, 64)
#define PRIuNACL_DEV NACL_PRI_(u, 64)
#define PRIxNACL_DEV NACL_PRI_(x, 64)
#define PRIXNACL_DEV NACL_PRI_(X, 64)

#ifndef __ino_t_defined
#define __ino_t_defined
typedef uint32_t __ino_t;
#ifndef __native_client__
typedef __ino_t ino_t;
#endif
#endif

#define PRIdNACL_INO NACL_PRI_(d, 32)
#define PRIiNACL_INO NACL_PRI_(i, 32)
#define PRIoNACL_INO NACL_PRI_(o, 32)
#define PRIuNACL_INO NACL_PRI_(u, 32)
#define PRIxNACL_INO NACL_PRI_(x, 32)
#define PRIXNACL_INO NACL_PRI_(X, 32)

#ifndef __mode_t_defined
#define __mode_t_defined
typedef uint32_t __mode_t;
#ifndef __native_client__
typedef __mode_t mode_t;
#endif
#endif

#define PRIdNACL_MODE NACL_PRI_(d, 32)
#define PRIiNACL_MODE NACL_PRI_(i, 32)
#define PRIoNACL_MODE NACL_PRI_(o, 32)
#define PRIuNACL_MODE NACL_PRI_(u, 32)
#define PRIxNACL_MODE NACL_PRI_(x, 32)
#define PRIXNACL_MODE NACL_PRI_(X, 32)

#ifndef __nlink_t_defined
#define __nlink_t_defined
typedef uint32_t __nlink_t;
#ifndef __native_client__
typedef __nlink_t nlink_t;
#endif
#endif

#define PRIdNACL_NLINK NACL_PRI_(d, 32)
#define PRIiNACL_NLINK NACL_PRI_(i, 32)
#define PRIoNACL_NLINK NACL_PRI_(o, 32)
#define PRIuNACL_NLINK NACL_PRI_(u, 32)
#define PRIxNACL_NLINK NACL_PRI_(x, 32)
#define PRIXNACL_NLINK NACL_PRI_(X, 32)

#ifndef __uid_t_defined
#define __uid_t_defined
typedef uint32_t __uid_t;
#ifndef __native_client__
typedef __uid_t uid_t;
#endif
#endif

#define PRIdNACL_UID NACL_PRI_(d, 32)
#define PRIiNACL_UID NACL_PRI_(i, 32)
#define PRIoNACL_UID NACL_PRI_(o, 32)
#define PRIuNACL_UID NACL_PRI_(u, 32)
#define PRIxNACL_UID NACL_PRI_(x, 32)
#define PRIXNACL_UID NACL_PRI_(X, 32)

#ifndef __gid_t_defined
#define __gid_t_defined
typedef uint32_t __gid_t;
#ifndef __native_client__
typedef __gid_t gid_t;
#endif
#endif

#define PRIdNACL_GID NACL_PRI_(d, 32)
#define PRIiNACL_GID NACL_PRI_(i, 32)
#define PRIoNACL_GID NACL_PRI_(o, 32)
#define PRIuNACL_GID NACL_PRI_(u, 32)
#define PRIxNACL_GID NACL_PRI_(x, 32)
#define PRIXNACL_GID NACL_PRI_(X, 32)

#ifndef __off_t_defined
#define __off_t_defined
typedef int32_t _off_t;
#ifndef __native_client__
typedef _off_t off_t;
#endif
#endif

#define PRIdNACL_OFF NACL_PRI_(d, 32)
#define PRIiNACL_OFF NACL_PRI_(i, 32)
#define PRIoNACL_OFF NACL_PRI_(o, 32)
#define PRIuNACL_OFF NACL_PRI_(u, 32)
#define PRIxNACL_OFF NACL_PRI_(x, 32)
#define PRIXNACL_OFF NACL_PRI_(X, 32)

#ifndef __blksize_t_defined
#define __blksize_t_defined
typedef int32_t __blksize_t;
typedef __blksize_t blksize_t;
#endif

#define PRIdNACL_BLKSIZE NACL_PRI_(d, 32)
#define PRIiNACL_BLKSIZE NACL_PRI_(i, 32)
#define PRIoNACL_BLKSIZE NACL_PRI_(o, 32)
#define PRIuNACL_BLKSIZE NACL_PRI_(u, 32)
#define PRIxNACL_BLKSIZE NACL_PRI_(x, 32)
#define PRIXNACL_BLKSIZE NACL_PRI_(X, 32)

#ifndef __blkcnt_t_defined
#define __blkcnt_t_defined
typedef int32_t __blkcnt_t;
typedef __blkcnt_t blkcnt_t;
#endif

#define PRIdNACL_BLKCNT NACL_PRI_(d, 32)
#define PRIiNACL_BLKCNT NACL_PRI_(i, 32)
#define PRIoNACL_BLKCNT NACL_PRI_(o, 32)
#define PRIuNACL_BLKCNT NACL_PRI_(u, 32)
#define PRIxNACL_BLKCNT NACL_PRI_(x, 32)
#define PRIXNACL_BLKCNT NACL_PRI_(X, 32)

#ifndef __time_t_defined
#define __time_t_defined
typedef int32_t       __time_t;
typedef __time_t time_t;
#endif

#define PRIdNACL_TIME NACL_PRI_(d, 32)
#define PRIiNACL_TIME NACL_PRI_(i, 32)
#define PRIoNACL_TIME NACL_PRI_(o, 32)
#define PRIuNACL_TIME NACL_PRI_(u, 32)
#define PRIxNACL_TIME NACL_PRI_(x, 32)
#define PRIXNACL_TIME NACL_PRI_(X, 32)

/*
 * stddef.h defines size_t, and we cannot export another definition.
 * see __need_size_t above and stddef.h
 * (BUILD/gcc-4.2.2/gcc/ginclude/stddef.h) contents.
 */
#define NACL_NO_STRIP(t) nacl_ ## abi_ ## t

#ifndef size_t_defined
#define size_t_defined
typedef uint32_t NACL_NO_STRIP(size_t);
#endif

#ifndef ssize_t_defined
#define ssize_t_defined
typedef int32_t NACL_NO_STRIP(ssize_t);
#endif

#undef NACL_NO_STRIP

#endif
