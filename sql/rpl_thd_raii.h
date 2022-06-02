/* Copyright (c) 2021 Percona LLC and/or its affiliates.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef RPL_THD_RAII_INCLUDED
#define RPL_THD_RAII_INCLUDED 1
#include "sql_class.h" /* THD */

/**
  RAII class to temporarily disable OPTIMIZER_SWITCH_USE_INDEX_EXTENSIONS
  optimizer_switch for replication applier threads.
*/
class Disable_index_extensions_switch_guard
{
public:
  explicit Disable_index_extensions_switch_guard(THD *thd) : m_thd(thd)
  {
    m_save_optimizer_switch = m_thd->variables.optimizer_switch;
    m_thd->variables.optimizer_switch &= ~OPTIMIZER_SWITCH_USE_INDEX_EXTENSIONS;
  }

  ~Disable_index_extensions_switch_guard()
  {
    m_thd->variables.optimizer_switch = m_save_optimizer_switch;
  }

private:
  THD *const m_thd;
  ulonglong m_save_optimizer_switch;
};
#endif /* RPL_THD_RAII_INCLUDED */
