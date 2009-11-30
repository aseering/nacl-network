/*
 * Copyright 2008  The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Service Runtime API.  Time types.
 */

#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_SYS_TIME_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_SYS_TIME_H_

#include <machine/_types.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef long int  suseconds_t;

#ifndef __clock_t_defined
#define __clock_t_defined
typedef long int  clock_t;  /* to be deprecated */
#endif

struct timeval {
  time_t      tv_sec;
  suseconds_t tv_usec;
};

struct timespec {
  time_t    tv_sec;
  long int           tv_nsec;
};

/* obsolete.  should not be used */
struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

/*
 * In some places (e.g., the linux man page) the second parameter is defined
 * as a struct timezone *.  The header file says this struct type should
 * never be used, and defines it by default as void *.  The Mac man page says
 * it is void *.
 */
extern int gettimeofday (struct timeval *tv, void *tz);

#ifdef __cplusplus
}
#endif

#endif
