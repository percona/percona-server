/* Copyright (C) 2014 Percona and Sergey Vojtovich

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifndef QUERY_RESPONSE_TIME_H
#define QUERY_RESPONSE_TIME_H

/*
  Settings for query response time
*/

/*
  Maximum string length for (10 ^ (-1 * QRT_STRING_NEGATIVE_POWER_LENGTH)) in text representation.
  Example: for 6 is 0.000001
  Always 2

  Maximum string length for (10 ^ (QRT_STRING_POSITIVE_POWER_LENGTH + 1) - 1) in text representation.
  Example: for 7 is 9999999.0
*/
#define QRT_TIME_STRING_POSITIVE_POWER_LENGTH 7
#define QRT_TOTAL_STRING_POSITIVE_POWER_LENGTH 7

/*
  Minimum base for log - ALWAYS 2
  Maximum base for log:
*/
#define QRT_MAXIMUM_BASE 1000

/*
  Filler for whole number (positive power)
  Example: for
  QRT_POSITIVE_POWER_FILLER ' '
  QRT_POSITIVE_POWER_LENGTH 7
  and number 7234 result is:
  '   7234'
*/
#define QRT_POSITIVE_POWER_FILLER ""
/*
  Filler for fractional number. Similiary to whole number
*/
#define QRT_NEGATIVE_POWER_FILLER "0"

/*
  Message if time too big for statistic collecting (very long query)
*/
#define QRT_TIME_OVERFLOW "TOO LONG"

#define QRT_DEFAULT_BASE 10

#define QRT_TIME_STRING_LENGTH				\
  MY_MAX( (QRT_TIME_STRING_POSITIVE_POWER_LENGTH + 1 /* '.' */ + 6 /*QRT_TIME_STRING_NEGATIVE_POWER_LENGTH*/), \
       (sizeof(QRT_TIME_OVERFLOW) - 1) )

#define QRT_TOTAL_STRING_LENGTH				\
  MY_MAX( (QRT_TOTAL_STRING_POSITIVE_POWER_LENGTH + 1 /* '.' */ + 6 /*QRT_TOTAL_STRING_NEGATIVE_POWER_LENGTH*/), \
       (sizeof(QRT_TIME_OVERFLOW) - 1) )

extern ST_SCHEMA_TABLE query_response_time_table;

typedef class Item COND;

extern void query_response_time_init   ();
extern void query_response_time_free   ();
extern void query_response_time_flush  ();
extern void query_response_time_collect(ulonglong query_time);
extern int  query_response_time_fill   (THD* thd, TABLE_LIST *tables, COND *cond);

extern ulong   opt_query_response_time_range_base;
extern my_bool opt_query_response_time_stats;

#endif // QUERY_RESPONSE_TIME_H
