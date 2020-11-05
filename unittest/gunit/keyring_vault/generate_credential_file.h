/* Copyright (c) 2018, 2021 Percona LLC and/or its affiliates. All rights
   reserved.

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

#ifndef MYSQL_GUNIT_GENERATE_CREDENTIAL_FILE_H
#define MYSQL_GUNIT_GENERATE_CREDENTIAL_FILE_H

#include <iosfwd>
#include <string>

enum class credentials_validity_type {
  credentials_validity_correct,
  credentials_validity_invalid_token,
};

enum class mount_point_version_type {
  mount_point_version_empty,
  mount_point_version_v1,
  mount_point_version_v2,
  mount_point_version_auto
};

std::ostream &operator<<(std::ostream &os,
                         mount_point_version_type mount_point_version);

std::string generate_uuid();

bool generate_credential_file(
    const std::string &credential_file_path,
    const std::string &secret_mount_point,
    mount_point_version_type mount_point_version,
    credentials_validity_type generate_credetials =
        credentials_validity_type::credentials_validity_correct);

bool is_vault_environment_configured();

std::string extract_admin_token();

#endif  // MYSQL_GUNIT_GENERATE_CREDENTIAL_FILE_H
