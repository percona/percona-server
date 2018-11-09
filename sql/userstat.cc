/* Copyright (c) 2018 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "my_config.h"

#include <time.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include "my_global.h"   // unlikely
#include "userstat.h"

/**
  Get start timers for cpu_time and busy_time.

  @return start_busy_usecs  Start value of busy_time or 0.0 in case of error
  @return start_cpu_nsecs   Start value of cpu_time or 0.0 in case of error
*/
void userstat_start_timer(double *start_busy_usecs,
                          double *start_cpu_nsecs)
{
  *start_busy_usecs = 0.0;
  *start_cpu_nsecs = 0.0;

#ifdef HAVE_CLOCK_GETTIME
  /* Get start cputime */
  struct timespec tp;
  if (!clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tp))
    *start_cpu_nsecs = tp.tv_sec * 1000000000.0 + tp.tv_nsec;
#endif

  /* Gets the start time, in order to measure how long this command takes. */
  struct timeval start_time;
  if (!gettimeofday(&start_time, NULL))
    *start_busy_usecs = start_time.tv_sec * 1000000.0 + start_time.tv_usec;
}

/**
  Get interval time for cpu_time and busy_time between calls to
  userstat_start_timer() and this function.

  @param  start_busy_usecs  Start value of busy_time or 0.0 in case of error
  @param  start_cpu_nsecs   Start value of cpu_time or 0.0 in case of error
  @return busy_sec          Interval time for busy_time in seconds or 0.0 in
                            case of error
  @return cpu_sec           Interval time for cpu_time in seconds or 0.0 in case
                            of error
*/
void userstat_finish_timer(double start_busy_usecs, double start_cpu_nsecs,
                           double *busy_sec, double *cpu_sec)
{
  *busy_sec = 0.0;
  *cpu_sec = 0.0;

  /* Gets the end time. */
  struct timeval end_time;
  double end_busy_usecs = 0.0;
  if (start_busy_usecs > 0.0 && !gettimeofday(&end_time, NULL))
    end_busy_usecs = end_time.tv_sec * 1000000.0 + end_time.tv_usec;

  /* Calculates the difference between the end and start times. */
  if (end_busy_usecs > start_busy_usecs) {
    *busy_sec = (end_busy_usecs - start_busy_usecs) / 1000000.0;
    /* In case there are bad values, 2629743 is the #seconds in a month. */
    if (unlikely(*busy_sec > 2629743.0)) {
      *busy_sec = 0.0;
    }
  }

#ifdef HAVE_CLOCK_GETTIME
  /* Get end cputime */
  struct timespec tp;
  double end_cpu_nsecs = 0.0;
  if (start_cpu_nsecs > 0.0 && !clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tp))
    end_cpu_nsecs = tp.tv_sec * 1000000000.0 + tp.tv_nsec;
#endif

  if (end_cpu_nsecs > start_cpu_nsecs) {
    *cpu_sec = (end_cpu_nsecs - start_cpu_nsecs) / 1000000000.0;
    /* In case there are bad values, 2629743 is the #seconds in a month. */
    if (unlikely(*cpu_sec > 2629743.0)) {
      *cpu_sec = 0.0;
    }
  }
}
