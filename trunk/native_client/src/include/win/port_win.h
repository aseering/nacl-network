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


#ifndef NATIVE_CLIENT_SRC_INCLUDE_WIN_PORT_WIN_H_
#define NATIVE_CLIENT_SRC_INCLUDE_WIN_PORT_WIN_H_ 1

#include "native_client/src/include/nacl_base.h"

/* TODO: eliminated this file and move its contents to portability*.h */

/* wchar_t and unsigned short are not always equivalent*/
#pragma warning(disable : 4255)
/* padding added after struct member */
#pragma warning(disable: 4820)
/* sign extended conversion */
#pragma warning(disable: 4826)
/* conditional expression is constant */
#pragma warning(disable : 4127)

/* TODO: limit this include to files that really need it */
#include <windows.h>

// types missing on Windows
typedef long              off_t;
//typedef __int64 intptr_t;
typedef unsigned int      size_t;
typedef int               ssize_t;
typedef unsigned int      uint32_t;
typedef int               int32_t;
typedef unsigned char     uint8_t;
typedef unsigned int      u_int32_t;
typedef unsigned short    __uint16_t;
typedef unsigned short    uint16_t;
typedef short             int16_t;
typedef int               mode_t;
typedef long              _off_t;
typedef long int          __loff_t;
typedef signed char       int8_t;
typedef unsigned long     DWORD;
typedef long              clock_t;
typedef __int64           int64_t;
typedef unsigned __int64  uint64_t;

#if !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS)
/* only what we need so far */
# define UINT8_MAX       (255)
#endif

EXTERN_C_BEGIN

//arguments processing
int ffs(int x);
int getopt(int argc, char *argv[], char *optstring);
extern char *optarg;  // global argument pointer
extern int optind;   // global argv index

EXTERN_C_END


/*************************** stdio.h  ********************/
#ifndef NULL
#if defined(__cplusplus)
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

/************************************************************/

/* missing from Windows <errno.h> */
#define EDQUOT 122 /* Quota exceeded */


/* from linux/limits.h, via sys/param.h */
#define PATH_MAX 4096

#endif  /* NATIVE_CLIENT_SRC_INCLUDE_WIN_PORT_WIN_H_ */
