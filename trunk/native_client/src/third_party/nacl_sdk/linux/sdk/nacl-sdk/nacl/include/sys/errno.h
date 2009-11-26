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
 * NaCl Service Runtime API.
 */

#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_ERRNO_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_ERRNO_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __native_client__
#include <sys/reent.h>

#ifndef _REENT_ONLY
#define errno (*__errno())
  extern int *__errno _PARAMS ((void));
#endif

/* Please don't use these variables directly.
 *    Use strerror instead. */
extern __IMPORT _CONST char * _CONST _sys_errlist[];
extern __IMPORT int _sys_nerr;

#define __errno_r(ptr) ((ptr)->_errno)
#endif  /* __native_client__ */

/*
 * NOTE: when adding new errnos here, check
 * service_runtime/nacl_host_desc_common.[hc] and
 * service_runtime/win/xlate_system_error.h.
 */

/* errno values, mostly from linux/errno.h */

#define EPERM     1  /* Operation not permitted */
#define ENOENT    2  /* No such file or directory */
#define ESRCH     3  /* No such process */
#define EINTR     4  /* Interrupted system call */
#define EIO       5  /* I/O error */
#define ENXIO     6  /* No such device or address */
#define E2BIG     7  /* Argument list too long */
#define ENOEXEC   8  /* Exec format error */
#define EBADF     9  /* Bad file number */
#define ECHILD   10  /* No child processes */
#define EAGAIN   11  /* Try again */
#define ENOMEM   12  /* Out of memory */
#define EACCES   13  /* Permission denied */
#define EFAULT   14  /* Bad address */

#define EBUSY    16  /* Device or resource busy */
#define EEXIST   17  /* File exists */
#define EXDEV    18  /* Cross-device link */
#define ENODEV   19  /* No such device */
#define ENOTDIR  20  /* Not a directory */
#define EISDIR   21  /* Is a directory */
#define EINVAL   22  /* Invalid argument */
#define ENFILE   23  /* File table overflow */
#define EMFILE   24  /* Too many open files */
#define ENOTTY   25  /* Not a typewriter */

#define EFBIG    27  /* File too large */
#define ENOSPC   28  /* No space left on device */
#define ESPIPE   29  /* Illegal seek */
#define EROFS    30  /* Read-only file system */
#define EMLINK   31  /* Too many links */
#define EPIPE    32  /* Broken pipe */

#define ENAMETOOLONG 36  /* File name too long */

#define ENOSYS   38  /* Function not implemented */

#define EDQUOT   122 /* Quota exceeded */

/*
 * Other definitions not needed for NaCl, but needed for newlib build.
 */
#define EDOM 33   /* Math arg out of domain of func */
#define ERANGE 34 /* Math result not representable */
#define ENOMSG 35 /* No message of desired type */
#define ECHRNG 37 /* Channel number out of range */
#define EL3HLT 39 /* Level 3 halted */
#define EL3RST 40 /* Level 3 reset */
#define ELNRNG 41 /* Link number out of range */
#define EUNATCH 42  /* Protocol driver not attached */
#define ENOCSI 43 /* No CSI structure available */
#define EL2HLT 44 /* Level 2 halted */
#define EDEADLK 45  /* Deadlock condition */
#define ENOLCK 46 /* No record locks available */
#define EBADE 50  /* Invalid exchange */
#define EBADR 51  /* Invalid request descriptor */
#define EXFULL 52 /* Exchange full */
#define ENOANO 53 /* No anode */
#define EBADRQC 54  /* Invalid request code */
#define EBADSLT 55  /* Invalid slot */
#define EDEADLOCK EDEADLK  /* File locking deadlock error */
#define EBFONT 57 /* Bad font file fmt */
#define ENOSTR 60 /* Device not a stream */
#define ENODATA 61  /* No data (for no delay io) */
#define ETIME 62  /* Timer expired */
#define ENOSR 63  /* Out of streams resources */
#define ENONET 64 /* Machine is not on the network */
#define ENOPKG 65 /* Package not installed */
#define EREMOTE 66  /* The object is remote */
#define ENOLINK 67  /* The link has been severed */
#define EADV 68   /* Advertise error */
#define ESRMNT 69 /* Srmount error */
#define ECOMM 70  /* Communication error on send */
#define EPROTO 71 /* Protocol error */
#define EMULTIHOP 74  /* Multihop attempted */
#define ELBIN 75  /* Inode is remote (not really error) */
#define EDOTDOT 76  /* Cross mount point (not really error) */
#define EBADMSG 77  /* Trying to read unreadable message */
#define EFTYPE 79 /* Inappropriate file type or format */
#define ENOTUNIQ 80 /* Given log. name not unique */
#define EBADFD 81 /* f.d. invalid for this operation */
#define EREMCHG 82  /* Remote address changed */
#define ELIBACC 83  /* Can't access a needed shared lib */
#define ELIBBAD 84  /* Accessing a corrupted shared lib */
#define ELIBSCN 85  /* .lib section in a.out corrupted */
#define ELIBMAX 86  /* Attempting to link in too many libs */
#define ELIBEXEC 87 /* Attempting to exec a shared library */
#define ENMFILE 89      /* No more files */
#define ENOTEMPTY 90  /* Directory not empty */
#define ELOOP 92  /* Too many symbolic links */
#define EOPNOTSUPP 95 /* Operation not supported on transport endpoint */
#define EPFNOSUPPORT 96 /* Protocol family not supported */
#define ECONNRESET 104  /* Connection reset by peer */
#define ENOBUFS 105 /* No buffer space available */
#define EAFNOSUPPORT 106 /* Address family not supported by protocol family */
#define EPROTOTYPE 107  /* Protocol wrong type for socket */
#define ENOTSOCK 108  /* Socket operation on non-socket */
#define ENOPROTOOPT 109 /* Protocol not available */
#define ESHUTDOWN 110 /* Can't send after socket shutdown */
#define ECONNREFUSED 111  /* Connection refused */
#define EADDRINUSE 112    /* Address already in use */
#define ECONNABORTED 113  /* Connection aborted */
#define ENETUNREACH 114   /* Network is unreachable */
#define ENETDOWN 115    /* Network interface is not configured */
#define ETIMEDOUT 116   /* Connection timed out */
#define EHOSTDOWN 117   /* Host is down */
#define EHOSTUNREACH 118  /* Host is unreachable */
#define EINPROGRESS 119   /* Connection already in progress */
#define EALREADY 120    /* Socket already connected */
#define EDESTADDRREQ 121  /* Destination address required */
#define EPROTONOSUPPORT 123 /* Unknown protocol */
#define ESOCKTNOSUPPORT 124 /* Socket type not supported */
#define EADDRNOTAVAIL 125 /* Address not available */
#define ENETRESET 126
#define EISCONN 127   /* Socket is already connected */
#define ENOTCONN 128    /* Socket is not connected */
#define ETOOMANYREFS 129
#define EPROCLIM 130
#define EUSERS 131
#define ESTALE 133
#define ENOTSUP EOPNOTSUPP   /* Not supported */
#define ENOMEDIUM 135   /* No medium (in tape drive) */
#define ENOSHARE 136    /* No such host or network path */
#define ECASECLASH 137  /* Filename exists with different case */
#define EILSEQ 138
#define EOVERFLOW 139 /* Value too large for defined data type */
#define ECANCELED 140 /* Operation canceled. */

/*
 * Changed due to conflict with NaCl definitions.
 */
#define EL2NSYNC 88 /* Level 2 not synchronized */
#define EIDRM 91  /* Identifier removed */
#define EMSGSIZE 132    /* Message too long */

/* From cygwin32.  */
#define EWOULDBLOCK EAGAIN      /* Operation would block */

#ifdef __cplusplus
}
#endif

#endif
