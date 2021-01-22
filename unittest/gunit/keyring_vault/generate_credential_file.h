#ifndef MYSQL_GUNIT_GENERATE_CREDENTIAL_FILE_H
#define MYSQL_GUNIT_GENERATE_CREDENTIAL_FILE_H

#include <iosfwd>
#include <string>

enum credentials_validity_type {
  credentials_validity_correct,
  credentials_validity_invalid_token,
};

enum mount_point_version_type {
  mount_point_version_empty,
  mount_point_version_v1,
  mount_point_version_v2,
  mount_point_version_auto
};

std::ostream &operator<<(std::ostream &           os,
                         mount_point_version_type mount_point_version);

std::string generate_uuid();

bool generate_credential_file(const std::string &       credential_file_path,
                              const std::string &       secret_mount_point,
                              mount_point_version_type  mount_point_version,
                              credentials_validity_type generate_credetials=
                                  credentials_validity_correct);

bool is_vault_environment_configured();

#endif  // MYSQL_GUNIT_GENERATE_CREDENTIAL_FILE_H
