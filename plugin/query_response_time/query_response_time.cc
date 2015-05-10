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

#include "mysql_version.h"
#include "my_global.h"
#include "mysql_com.h"
#include "rpl_tblmap.h"
#include "table.h"
#include "field.h"
#include "sql_show.h"
#include "query_response_time.h"

#define TIME_STRING_POSITIVE_POWER_LENGTH QRT_TIME_STRING_POSITIVE_POWER_LENGTH
#define TIME_STRING_NEGATIVE_POWER_LENGTH 6
#define TOTAL_STRING_POSITIVE_POWER_LENGTH QRT_TOTAL_STRING_POSITIVE_POWER_LENGTH
#define TOTAL_STRING_NEGATIVE_POWER_LENGTH 6
#define MINIMUM_BASE 2
#define MAXIMUM_BASE QRT_MAXIMUM_BASE
#define POSITIVE_POWER_FILLER QRT_POSITIVE_POWER_FILLER
#define NEGATIVE_POWER_FILLER QRT_NEGATIVE_POWER_FILLER
#define TIME_OVERFLOW   QRT_TIME_OVERFLOW
#define DEFAULT_BASE    QRT_DEFAULT_BASE

#define do_xstr(s) do_str(s)
#define do_str(s) #s
#define do_format(filler,width) "%" filler width "lld"
/*
  Format strings for snprintf. Generate from:
  POSITIVE_POWER_FILLER and TIME_STRING_POSITIVE_POWER_LENGTH
  NEFATIVE_POWER_FILLER and TIME_STRING_NEGATIVE_POWER_LENGTH
*/
#define TIME_STRING_POSITIVE_POWER_FORMAT do_format(POSITIVE_POWER_FILLER,do_xstr(TIME_STRING_POSITIVE_POWER_LENGTH))
#define TIME_STRING_NEGATIVE_POWER_FORMAT do_format(NEGATIVE_POWER_FILLER,do_xstr(TIME_STRING_NEGATIVE_POWER_LENGTH))
#define TIME_STRING_FORMAT		      TIME_STRING_POSITIVE_POWER_FORMAT "." TIME_STRING_NEGATIVE_POWER_FORMAT

#define TOTAL_STRING_POSITIVE_POWER_FORMAT do_format(POSITIVE_POWER_FILLER,do_xstr(TOTAL_STRING_POSITIVE_POWER_LENGTH))
#define TOTAL_STRING_NEGATIVE_POWER_FORMAT do_format(NEGATIVE_POWER_FILLER,do_xstr(TOTAL_STRING_NEGATIVE_POWER_LENGTH))
#define TOTAL_STRING_FORMAT		      TOTAL_STRING_POSITIVE_POWER_FORMAT "." TOTAL_STRING_NEGATIVE_POWER_FORMAT

#define TIME_STRING_LENGTH	QRT_TIME_STRING_LENGTH
#define TIME_STRING_BUFFER_LENGTH	(TIME_STRING_LENGTH + 1 /* '\0' */)

#define TOTAL_STRING_LENGTH	QRT_TOTAL_STRING_LENGTH
#define TOTAL_STRING_BUFFER_LENGTH	(TOTAL_STRING_LENGTH + 1 /* '\0' */)

/*
  Calculate length of "log linear"
  1)
  (MINIMUM_BASE ^ result) <= (10 ^ STRING_POWER_LENGTH) < (MINIMUM_BASE ^ (result + 1))

  2)
  (MINIMUM_BASE ^ result) <= (10 ^ STRING_POWER_LENGTH)
  and
  (MINIMUM_BASE ^ (result + 1)) > (10 ^ STRING_POWER_LENGTH)

  3)
  result     <= LOG(MINIMUM_BASE, 10 ^ STRING_POWER_LENGTH)= STRING_POWER_LENGTH * LOG(MINIMUM_BASE,10)
  result + 1 >  LOG(MINIMUM_BASE, 10 ^ STRING_POWER_LENGTH)= STRING_POWER_LENGTH * LOG(MINIMUM_BASE,10)

  4) STRING_POWER_LENGTH * LOG(MINIMUM_BASE,10) - 1 < result <= STRING_POWER_LENGTH * LOG(MINIMUM_BASE,10)

  MINIMUM_BASE= 2 always, LOG(MINIMUM_BASE,10)= 3.3219280948873626, result= (int)3.3219280948873626 * STRING_POWER_LENGTH

  Last counter always use for time overflow
*/
#define POSITIVE_POWER_COUNT ((int)(3.32192809 * TIME_STRING_POSITIVE_POWER_LENGTH))
#define NEGATIVE_POWER_COUNT ((int)(3.32192809 * TIME_STRING_NEGATIVE_POWER_LENGTH))
#define OVERALL_POWER_COUNT (NEGATIVE_POWER_COUNT + 1 + POSITIVE_POWER_COUNT)

#define MILLION ((unsigned long)1000 * 1000)

namespace query_response_time
{

class utility
{
public:
  utility() : m_base(0)
  {
    m_max_dec_value= MILLION;
    for(int i= 0; TIME_STRING_POSITIVE_POWER_LENGTH > i; ++i)
      m_max_dec_value *= 10;
    setup(DEFAULT_BASE);
  }
public:
  uint      base()            const { return m_base; }
  uint      negative_count()  const { return m_negative_count; }
  uint      positive_count()  const { return m_positive_count; }
  uint      bound_count()     const { return m_bound_count; }
  ulonglong max_dec_value()   const { return m_max_dec_value; }
  ulonglong bound(uint index) const { return m_bound[ index ]; }
public:
  void setup(uint base)
  {
    if(base != m_base)
    {
      m_base= base;

      const ulonglong million= 1000 * 1000;
      ulonglong value= million;
      m_negative_count= 0;
      while(value > 0)
      {
	m_negative_count += 1;
	value /= m_base;
      }
      m_negative_count -= 1;

      value= million;
      m_positive_count= 0;
      while(value < m_max_dec_value)
      {
	m_positive_count += 1;
	value *= m_base;
      }
      m_bound_count= m_negative_count + m_positive_count;

      value= million;
      for(uint i= 0; i < m_negative_count; ++i)
      {
	value /= m_base;
	m_bound[m_negative_count - i - 1]= value;
      }
      value= million;
      for(uint i= 0; i < m_positive_count;  ++i)
      {
	m_bound[m_negative_count + i]= value;
	value *= m_base;
      }
    }
  }
private:
  uint      m_base;
  uint      m_negative_count;
  uint      m_positive_count;
  uint      m_bound_count;
  ulonglong m_max_dec_value; /* for TIME_STRING_POSITIVE_POWER_LENGTH=7 is 10000000 */
  ulonglong m_bound[OVERALL_POWER_COUNT];
};

static
void print_time(char* buffer, std::size_t buffer_size, const char* format,
                uint64 value)
{
  ulonglong second=      (value / MILLION);
  ulonglong microsecond= (value % MILLION);
  my_snprintf(buffer, buffer_size, format, second, microsecond);
}

class time_collector
{
public:
  time_collector(utility& u) : m_utility(&u)
  {
    my_atomic_rwlock_init(&time_collector_lock);
  }
  ~time_collector()
  {
    my_atomic_rwlock_destroy(&time_collector_lock);
  }
  uint32 count(QUERY_TYPE type, uint index)
  {
    my_atomic_rwlock_rdlock(&time_collector_lock);
    uint32 result= my_atomic_load32((int32*)&m_count[type][index]);
    my_atomic_rwlock_rdunlock(&time_collector_lock);
    return result;
  }
  uint64 total(QUERY_TYPE type, uint index)
  {
    my_atomic_rwlock_rdlock(&time_collector_lock);
    uint64 result= my_atomic_load64((int64*)&m_total[type][index]);
    my_atomic_rwlock_rdunlock(&time_collector_lock);
    return result;
  }
public:
  void flush()
  {
    my_atomic_rwlock_wrlock(&time_collector_lock);
    memset((void*)&m_count,0,sizeof(m_count));
    memset((void*)&m_total,0,sizeof(m_total));
    my_atomic_rwlock_wrunlock(&time_collector_lock);
  }
  void collect(QUERY_TYPE type, uint64 time)
  {
    int i= 0;
    for(int count= m_utility->bound_count(); count > i; ++i)
    {
      if(m_utility->bound(i) > time)
      {
        my_atomic_rwlock_wrlock(&time_collector_lock);
        my_atomic_add32((int32*)(&m_count[0][i]), 1);
        my_atomic_add64((int64*)(&m_total[0][i]), time);
        my_atomic_add32((int32*)(&m_count[type][i]), 1);
        my_atomic_add64((int64*)(&m_total[type][i]), time);
        my_atomic_rwlock_wrunlock(&time_collector_lock);
        break;
      }
    }
  }
private:
  utility* m_utility;
  /* The lock for atomic operations on
  m_count, m_total, m_r_count, m_r_total, m_w_count, m_w_total.
  Only actually used on architectures that do not have atomic
  implementation of atomic operations. */
  my_atomic_rwlock_t time_collector_lock;
  /*
   The first row is for overall statistics,
   the second row is for 'read' queries,
   the third row is for 'write' queries.
  */
  uint32   m_count[3][OVERALL_POWER_COUNT + 1];
  uint64   m_total[3][OVERALL_POWER_COUNT + 1];
};

class collector
{
public:
  collector() : m_time(m_utility)
  {
    m_utility.setup(DEFAULT_BASE);
    m_time.flush();
  }
public:
  void flush()
  {
    m_utility.setup(opt_query_response_time_range_base);
    m_time.flush();
  }
  int fill(QUERY_TYPE type,
           THD* thd,
           TABLE_LIST *tables, COND *cond)
  {
    DBUG_ENTER("fill_schema_query_response_time");
    TABLE        *table= static_cast<TABLE*>(tables->table);
    Field        **fields= table->field;
    for(uint i= 0, count= bound_count() + 1 /* with overflow */; count > i; ++i)
    {
      char time[TIME_STRING_BUFFER_LENGTH];
      char total[TOTAL_STRING_BUFFER_LENGTH];
      if(i == bound_count())
      {        
        assert(sizeof(TIME_OVERFLOW) <= TIME_STRING_BUFFER_LENGTH);
        assert(sizeof(TIME_OVERFLOW) <= TOTAL_STRING_BUFFER_LENGTH);
        memcpy(time,TIME_OVERFLOW,sizeof(TIME_OVERFLOW));
        memcpy(total,TIME_OVERFLOW,sizeof(TIME_OVERFLOW));
      }
      else
      {
        print_time(time, sizeof(time), TIME_STRING_FORMAT, this->bound(i));
        print_time(total, sizeof(total), TOTAL_STRING_FORMAT, this->total(type, i));
      }
      fields[0]->store(time,strlen(time),system_charset_info);
      fields[1]->store(this->count(type, i));
      fields[2]->store(total,strlen(total),system_charset_info);
      if (schema_table_store_record(thd, table))
      {
	DBUG_RETURN(1);
      }
    }
    DBUG_RETURN(0);
  }
  void collect(QUERY_TYPE type, ulonglong time)
  {
    m_time.collect(type, time);
  }
  uint bound_count() const
  {
    return m_utility.bound_count();
  }
  ulonglong bound(uint index)
  {
    return m_utility.bound(index);
  }
  ulonglong count(QUERY_TYPE type, uint index)
  {
    return m_time.count(type, index);
  }
  ulonglong total(QUERY_TYPE type, uint index)
  {
    return m_time.total(type, index);
  }
private:
  utility          m_utility;
  time_collector   m_time;
};

static collector g_collector;

} // namespace query_response_time

void query_response_time_init()
{
  query_response_time_flush();
}

void query_response_time_free()
{
  query_response_time::g_collector.flush();
}

void query_response_time_flush()
{
  query_response_time::g_collector.flush();
}

void query_response_time_collect(QUERY_TYPE type,
                                 ulonglong query_time)
{
  query_response_time::g_collector.collect(type, query_time);
}

int query_response_time_fill(THD* thd, TABLE_LIST *tables, COND *cond)
{
  QUERY_TYPE query_type= ANY;
  if (!strncmp(tables->table->alias,
              "QUERY_RESPONSE_TIME_READ",
              sizeof("QUERY_RESPONSE_TIME_READ") - 1))
    query_type= READ;
  else if (!strncmp(tables->table->alias,
                   "QUERY_RESPONSE_TIME_WRITE",
                   sizeof("QUERY_RESPONSE_TIME_WRITE") - 1))
    query_type= WRITE;
  return query_response_time::g_collector.fill(query_type, thd, tables, cond);
}
