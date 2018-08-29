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

#ifndef USERSTAT_INCLUDED
#define USERSTAT_INCLUDED

void userstat_start_timer(double *start_busy_usecs,
                          double *start_cpu_nsecs) noexcept;
void userstat_finish_timer(double start_busy_usecs, double start_cpu_nsecs,
                           double *busy_sec, double *cpu_sec) noexcept;

#endif
