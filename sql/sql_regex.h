/* Copyright (c) 2016, Percona and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef _regex_h_
#define _regex_h_

#include <string>
#include <algorithm>

#include "my_global.h"
#include "my_pthread.h"
#include "my_regex.h"

/*
  Wrapper class around my_regex that manages concurrent access and the lifetime
  of the compiled regex.
*/
class Regex
{
private:
#if defined(HAVE_PSI_INTERFACE)
  const PSI_rwlock_key& m_key;
#endif

  std::string m_pattern;
  my_regex_t m_expr;
  bool m_compiled;

  mutable mysql_rwlock_t m_rwlock;

  // No implementations, should never be called.
  Regex(const Regex& other);
  Regex& operator=(const Regex& other);

  void reset()
  {
    // assumes object is write locked where necessary
    if (m_compiled)
    {
      m_pattern.clear();
      my_regfree(&m_expr);
      m_compiled= false;
    }
  }

public:
#if defined(HAVE_PSI_INTERFACE)
  Regex(const PSI_rwlock_key& key) :
    m_key(key),
#else
  Regex() :
#endif
    m_pattern(""),
    m_compiled(false)
  {
#if defined(HAVE_PSI_INTERFACE)
    mysql_rwlock_init(m_key, &m_rwlock);
#else
    mysql_rwlock_init(nullptr, &m_rwlock);
#endif
  }

  ~Regex()
  {
    reset();
    mysql_rwlock_destroy(&m_rwlock);
  }

  bool compiled() const
  {
    return m_compiled;
  }

  const std::string& pattern() const
  {
    return m_pattern;
  }

  // Compile the pattern into an expression
  // see regex/my_regex.h for definition of flags
  bool compile(const char *pattern, int flags, const CHARSET_INFO *charset)
  {
    int error= 0;

    mysql_rwlock_wrlock(&m_rwlock);

    reset();

    if (pattern)
    {
      m_pattern.assign(pattern);
    }

    if (!m_pattern.empty())
    {
      error= my_regcomp(&m_expr,
                        m_pattern.c_str(),
                        flags,
                        charset);
      m_compiled= !error;
    }

    mysql_rwlock_unlock(&m_rwlock);

    return !error;
  }

  // See if a string matches at least one pattern
  bool match(const std::string& str) const
  {
    if (!m_compiled)
    {
      return false;
    }

    mysql_rwlock_rdlock(&m_rwlock);

    int found= my_regexec(&m_expr, str.c_str(), 0, nullptr, 0);

    mysql_rwlock_unlock(&m_rwlock);

    return !found;
  }
};

#endif // _regex_h_
