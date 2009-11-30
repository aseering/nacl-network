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

#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_FCNTL_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_FCNTL_H_

#ifdef __native_client__
#include <sys/types.h>
#endif

/* from bits/fcntl.h */
#define O_ACCMODE     0003
#define O_RDONLY        00
#define O_WRONLY        01
#define O_RDWR          02

#define O_CREAT       0100 /* not fcntl */
#define O_TRUNC      01000 /* not fcntl */
#define O_APPEND     02000

/*
 * Features not implemented by NaCl, but required by the newlib build.
 */
#define O_EXCL        0200
#define O_NONBLOCK   04000
#define O_NDELAY      O_NONBLOCK
#define O_SYNC      010000
#define O_FSYNC       O_SYNC
#define O_ASYNC     020000

/* XXX close on exec request; must match UF_EXCLOSE in user.h */
#define FD_CLOEXEC  1 /* posix */

/* fcntl(2) requests */
#define F_DUPFD   0 /* Duplicate fildes */
#define F_GETFD   1 /* Get fildes flags (close on exec) */
#define F_SETFD   2 /* Set fildes flags (close on exec) */
#define F_GETFL   3 /* Get file flags */
#define F_SETFL   4 /* Set file flags */
#ifndef _POSIX_SOURCE
#define F_GETOWN  5 /* Get owner - for ASYNC */
#define F_SETOWN  6 /* Set owner - for ASYNC */
#endif  /* !_POSIX_SOURCE */
#define F_GETLK   7 /* Get record-locking information */
#define F_SETLK   8 /* Set or Clear a record-lock (Non-Blocking) */
#define F_SETLKW  9 /* Set or Clear a record-lock (Blocking) */
#ifndef _POSIX_SOURCE
#define F_RGETLK  10  /* Test a remote lock to see if it is blocked */
#define F_RSETLK  11  /* Set or unlock a remote lock */
#define F_CNVT    12  /* Convert a fhandle to an open fd */
#define F_RSETLKW   13  /* Set or Clear remote record-lock(Blocking) */
#endif  /* !_POSIX_SOURCE */

/* fcntl(2) flags (l_type field of flock structure) */
#define F_RDLCK   1 /* read lock */
#define F_WRLCK   2 /* write lock */
#define F_UNLCK   3 /* remove lock(s) */
#ifndef _POSIX_SOURCE
#define F_UNLKSYS 4 /* remove remote locks for a given system */
#endif  /* !_POSIX_SOURCE */

#endif
