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

#ifndef RPL_EVENT_CTX_H
#define RPL_EVENT_CTX_H

#include <list>
#include <memory>
#include <sstream>
#include <string>

class Rpl_event_ctx {
 public:
  static Rpl_event_ctx &get_instance();

  /* Parser function */
  int process_argument(const char *argument, std::ostringstream &message);

  /* Checks - returns if the event should be turned on on replica side. */
  bool event_needs_reenable(const std::string &db,
                            const std::string &event) const;

  /* Get the values for reporting */
  void get_events_wild_list(std::string &str) const;

 private:
  Rpl_event_ctx() = default;
  Rpl_event_ctx(const Rpl_event_ctx &) = delete;
  Rpl_event_ctx &operator=(const Rpl_event_ctx &) = delete;
  ~Rpl_event_ctx() = default;

  /* Adds the wild event name to the wild list. */
  void add_to_wild_event_list(const char *entry);

  using EVENT_WILD_LIST = std::list<std::string>;
  std::unique_ptr<EVENT_WILD_LIST> m_event_wild_list{nullptr};

  bool m_event_wild_list_inited{false};
};
#endif /* RPL_EVENT_CTX_H */
