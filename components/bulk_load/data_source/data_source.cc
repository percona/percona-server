/* Copyright (c) 2024, Percona and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation. The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA */

#include "data_source.h"

#include "data_source_local/data_source_local.h"
#include "data_source_s3/data_source_s3.h"

namespace Bulk_load::Data_source {

template <Bulk_source BulkSource>
std::unique_ptr<DataSourceBase> create_helper(const std::string &path) {
  return std::make_unique<DataSourceImpl<BulkSource>>(path);
}

std::unique_ptr<DataSourceBase> create(Bulk_source src,
                                       const std::string &path) noexcept {
  if (src == Bulk_source::OCI) {
    return nullptr;   // not supported
  }

  using CreateFunc = std::unique_ptr<DataSourceBase> (*)(
      const std::string &path);
  static const CreateFunc funcs[static_cast<int>(Bulk_source::S3) + 1] = {
    create_helper<Bulk_source::LOCAL>,
    nullptr,
    create_helper<Bulk_source::S3>};
  return (*funcs[static_cast<int>(src)])(path);
}

}  // namespace Bulk_load::Data_source
