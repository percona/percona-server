/* Copyright (c) 2021 Percona LLC and/or its affiliates. All rights reserved.

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

#include "rpl_event_ctx.h"
#include <algorithm>
#include <cstring>
#include "mf_wcomp.h"    // wild_one, wild_many
#include "mysql/strings/m_ctype.h"
#include "sql/mysqld.h"  // system_charset_info
#include "sql/mysqld_cs.h"

Rpl_event_ctx &Rpl_event_ctx::get_instance() {
  static Rpl_event_ctx object;
  return object;
}

int Rpl_event_ctx::process_argument(const char *argument,
                                    std::ostringstream &message) {
  const auto arglen = strlen(argument);

  /* Pattern is empty */
  if (arglen == 0) {
    message.str("Cannot process empty pattern.");
    return 1;
  }

  /*
     If pattern is not in the format <db>.<event>

     i.e, either in one of the below cases
       1. pattern has no dot character
       2. pattern starts with a dot
       3. pattern ends with a dot
       4. pattern has multiple dots
  */
  const char *dot = std::strchr(argument, '.');
  if (dot == nullptr || dot == argument || (dot == (argument + arglen - 1))) {
    message.str(
        "The pattern doesn't follow the format <db_pattern>.<event_pattern>.");
    return 1;
  }

  /* Pattern must not contain multiple dots, search for the next dot character
   */
  dot = std::strchr(dot + 1, '.');
  if (dot != nullptr) {
    message.str(
        "The pattern has multiple dot characters. Please restart with "
        "format <db_pattern>.<event_pattern>.");
    return 1;
  }

  /* As of now, this feature is not supported per-channel basis.
     So, when the pattern contains a channel separator ':' */
  const char *channel_separator = std::strchr(argument, ':');
  if (channel_separator != nullptr) {
    message.str(
        "This server doesn't support per-channel "
        "--replica-enable-event feature.");
    return 1;
  }

  /* All good, add the pattern to the list.  */
  add_to_wild_event_list(argument);
  return 0;
}

void Rpl_event_ctx::add_to_wild_event_list(const char *entry) {
  /* Initialize the list if not initialized. */
  if (!m_event_wild_list_inited) {
    m_event_wild_list = std::make_unique<EVENT_WILD_LIST>();
    m_event_wild_list_inited = true;
  }

  /* Add the pattern if it doesn't exist. */
  std::string pattern(entry);
  auto it =
      std::find(m_event_wild_list->begin(), m_event_wild_list->end(), pattern);
  if (it == m_event_wild_list->end()) {
    m_event_wild_list->push_back(pattern);
  }
}

bool Rpl_event_ctx::event_needs_reenable(const std::string &db,
                                         const std::string &event) const {
  /* Return if not initialized. */
  if (!m_event_wild_list_inited) return false;

  /* Prepare the key for searching the list */
  std::string key = db + "." + event;
  const char *ptr = key.c_str();
  size_t len = key.length();

  for (const auto &pattern : *m_event_wild_list) {
    const char *wild_str = pattern.c_str();
    size_t wild_len = pattern.length();
    if (!my_wildcmp(system_charset_info, ptr, ptr + len, wild_str,
                    wild_str + wild_len, '\\', wild_one, wild_many)) {
      /* Pattern matched */
      return true;
    }
  }
  /* No pattern matched */
  return false;
}

void Rpl_event_ctx::get_events_wild_list(std::string &str) const {
  /* Store the default value. */
  str.assign("");

  /* Return if not initialized. */
  if (!m_event_wild_list_inited) {
    return;
  }

  /* Prepare the status value for reporting */
  for (const auto &pattern : *m_event_wild_list) {
    str += "," + pattern;
  }
  /* Remove the ',' character from the beginning of the string. */
  str.erase(str.begin());
}
