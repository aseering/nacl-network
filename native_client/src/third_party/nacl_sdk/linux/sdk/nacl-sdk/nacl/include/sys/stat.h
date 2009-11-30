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
 * NaCl kernel / service run-time system call ABI.  stat/fstat.
 */

#ifndef SERVICE_RUNTIME_INCLUDE_SYS_STAT_H_
#define SERVICE_RUNTIME_INCLUDE_SYS_STAT_H_

#ifdef __native_client__
#include <sys/types.h>
#else
#include <machine/_types.h>

#endif
#include <bits/stat.h>


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Linux <bit/stat.h> uses preprocessor to define st_atime to
 * st_atim.tv_sec etc to widen the ABI to use a struct timespec rather
 * than just have a time_t access/modification/inode-change times.
 * this is unfortunate, since that symbol cannot be used as a struct
 * member elsewhere (!).
 *
 * just like with type name conflicts, we avoid it by using 
 * as a prefix for struct members too.  sigh.
 */

struct stat {  /* must be renamed when ABI is exported */
  dev_t     st_dev;      /* not implemented */
  ino_t     st_ino;      /* not implemented */
  mode_t    st_mode;     /* partially implemented. */
  nlink_t   st_nlink;    /* link count */
  uid_t     st_uid;      /* not implemented */
  gid_t     st_gid;      /* not implemented */
  int                __padding;            /* needed to align st_rdev */
  dev_t     st_rdev;     /* not implemented */
  off_t     st_size;     /* object size */
  blksize_t st_blksize;  /* not implemented */
  blkcnt_t  st_blocks;   /* not implemented */
  time_t    st_atime;    /* access time */
  time_t    st_mtime;    /* modification time */
  time_t    st_ctime;    /* inode change time */
};

#ifdef __native_client__
int stat(char const *path, struct stat *stbuf);
int fstat(int d, struct stat *stbuf);
#endif

#ifdef __cplusplus
}
#endif

#endif
