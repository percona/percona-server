/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef AUDIT_LOG_FILTER_BASE_COMPONENT_H_INCLUDED
#define AUDIT_LOG_FILTER_BASE_COMPONENT_H_INCLUDED

namespace audit_log_filter {

class AuditBaseMediator;

class AuditBaseComponent {
 private:
  AuditBaseMediator *m_mediator;

 public:
  void set_mediator(AuditBaseMediator *mediator) noexcept {
    this->m_mediator = mediator;
  }

  [[nodiscard]] AuditBaseMediator *get_mediator() noexcept {
    return m_mediator;
  }
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_BASE_COMPONENT_H_INCLUDED
