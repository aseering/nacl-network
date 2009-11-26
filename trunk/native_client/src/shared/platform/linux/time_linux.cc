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


#include <sys/time.h>
#include <time.h>

#include "native_client/src/include/portability.h"
#include "native_client/src/include/base/basictypes.h"

#include "native_client/src/shared/platform/time.h"

// The Time routines in this file use standard POSIX routines, or almost-
// standard routines in the case of timegm.
// The TimeTicks routines are Mach-specific.

// Time -----------------------------------------------------------------------

// The internal representation of Time uses time_t directly, so there is no
// offset.  The epoch is 1970-01-01 00:00:00 UTC.
// static
const int64 NaCl::Time::kTimeTToMicrosecondsOffset = GG_INT64_C(0);

// static
int64 NaCl::Time::CurrentWallclockMicroseconds() {
  struct timeval tv;
  struct timezone tz = { 0, 0 };  // UTC
  if (gettimeofday(&tv, &tz) != 0) {
    // DCHECK(0) << "Could not determine time of day";
  }
  // Combine seconds and microseconds in a 64-bit field containing microseconds
  // since the epoch.  That's enough for nearly 600 centuries.
  return tv.tv_sec * GG_UINT64_C(1000000) + tv.tv_usec;
}

// static
NaCl::Time NaCl::Time::FromExploded(bool is_local, const Exploded& exploded) {
  struct tm timestruct;
  timestruct.tm_sec    = exploded.second;
  timestruct.tm_min    = exploded.minute;
  timestruct.tm_hour   = exploded.hour;
  timestruct.tm_mday   = exploded.day_of_month;
  timestruct.tm_mon    = exploded.month - 1;
  timestruct.tm_year   = exploded.year - 1900;
  timestruct.tm_wday   = exploded.day_of_week;  // mktime/timegm ignore this
  timestruct.tm_yday   = 0;     // mktime/timegm ignore this
  timestruct.tm_isdst  = -1;    // attempt to figure it out
  timestruct.tm_gmtoff = 0;     // not a POSIX field, so mktime/timegm ignore
  timestruct.tm_zone   = NULL;  // not a POSIX field, so mktime/timegm ignore

  time_t seconds;
  if (is_local)
    seconds = mktime(&timestruct);
  else
    seconds = timegm(&timestruct);
  // DCHECK(seconds >= 0) << "mktime/timegm could not convert from exploded";

  uint64 milliseconds = seconds * kMillisecondsPerSecond + exploded.millisecond;
  return Time(milliseconds * kMicrosecondsPerMillisecond);
}

void NaCl::Time::Explode(bool is_local, Exploded* exploded) const {
  // Time stores times with microsecond resolution, but Exploded only carries
  // millisecond resolution, so begin by being lossy.
  uint64 milliseconds = us_ / kMicrosecondsPerMillisecond;
  time_t seconds = milliseconds / kMillisecondsPerSecond;

  struct tm timestruct;
  if (is_local)
    localtime_r(&seconds, &timestruct);
  else
    gmtime_r(&seconds, &timestruct);

  exploded->year         = timestruct.tm_year + 1900;
  exploded->month        = timestruct.tm_mon + 1;
  exploded->day_of_week  = timestruct.tm_wday;
  exploded->day_of_month = timestruct.tm_mday;
  exploded->hour         = timestruct.tm_hour;
  exploded->minute       = timestruct.tm_min;
  exploded->second       = timestruct.tm_sec;
  exploded->millisecond  = milliseconds % kMillisecondsPerSecond;
}

// TimeTicks ------------------------------------------------------------------

// static
NaCl::TimeTicks NaCl::TimeTicks::Now() {
  struct timeval tv;
  struct timezone tz = { 0, 0 };  // UTC
  if (gettimeofday(&tv, &tz) != 0) {
    // DCHECK(0) << "Could not determine time of day";
    return TimeTicks(0);
  }
  // Combine seconds and microseconds in a 64-bit field containing microseconds
  // since the epoch.  That's enough for nearly 600 centuries.
  return TimeTicks(tv.tv_sec * GG_UINT64_C(1000000) + tv.tv_usec);
}

// static
NaCl::TimeTicks NaCl::TimeTicks::UnreliableHighResNow() {
  return Now();
}

void NaCl::TimeTicks::InitTimespec(struct timespec *ts) const {
  // from microseconds into seconds
  ts->tv_sec = ticks_ / GG_UINT64_C(1000000);
  // microseconds into nano
  ts->tv_nsec = (ticks_ - ts->tv_sec * GG_UINT64_C(1000000)) * 1000;
}
