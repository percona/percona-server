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

#ifndef AUDIT_LOG_FILTER_AUDIT_UDF_H_INCLUDED
#define AUDIT_LOG_FILTER_AUDIT_UDF_H_INCLUDED

#include <mysql/udf_registration_types.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace audit_log_filter {

struct UdfFuncInfo {
  const char *udf_name;
  Item_result return_type;
  char *(*udf_str_func)(UDF_INIT *, UDF_ARGS *, char *, unsigned long *,
                        unsigned char *, unsigned char *);
  long long (*udf_int_func)(UDF_INIT *, UDF_ARGS *, unsigned char *,
                            unsigned char *);
  bool (*init_func)(UDF_INIT *, UDF_ARGS *, char *);
  void (*deinit_func)(UDF_INIT *);
};

class AuditUdf {
 public:
  AuditUdf() = default;
  ~AuditUdf();

  /**
   * @brief Init audit log filter UDFs.
   *
   * @param begin Iterator pointing to the first element in the list of
   *              UdfFuncInfo elements describing known UDF functions
   * @param end Iterator pointing to the one past last element in the list of
   *            UdfFuncInfo elements describing known UDF functions
   * @return true in case UDFs are initialized successfully, false otherwise
   */
  bool init(UdfFuncInfo *begin, UdfFuncInfo *end);

  /**
   * @brief De-initialise audit log filter UDFs.
   */
  void deinit() noexcept;

  /**
   * @brief Init function for audit_log_filter_set_filter UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_filter_set_filter_udf_init(AuditUdf *udf,
                                                   UDF_INIT *initid,
                                                   UDF_ARGS *udf_args,
                                                   char *message) noexcept;

  /**
   * @brief Main function for audit_log_filter_set_filter UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param result UDFs result buffer
   * @param length Result length
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return Pointer to the result buffer
   */
  static char *audit_log_filter_set_filter_udf(AuditUdf *udf, UDF_INIT *initid,
                                               UDF_ARGS *udf_args, char *result,
                                               unsigned long *length,
                                               unsigned char *is_null,
                                               unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_filter_set_filter UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_filter_set_filter_udf_deinit(UDF_INIT *initid);

  /**
   * @brief Init function for audit_log_filter_remove_filter UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_filter_remove_filter_udf_init(AuditUdf *udf,
                                                      UDF_INIT *initid,
                                                      UDF_ARGS *udf_args,
                                                      char *message) noexcept;

  /**
   * @brief Main function for audit_log_filter_remove_filter UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param result UDFs result buffer
   * @param length Result length
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return Pointer to the result buffer
   */
  static char *audit_log_filter_remove_filter_udf(
      AuditUdf *udf, UDF_INIT *initid, UDF_ARGS *udf_args, char *result,
      unsigned long *length, unsigned char *is_null,
      unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_filter_remove_filter UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_filter_remove_filter_udf_deinit(UDF_INIT *initid);

  /**
   * @brief Init function for audit_log_filter_set_user UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_filter_set_user_udf_init(AuditUdf *udf,
                                                 UDF_INIT *initid,
                                                 UDF_ARGS *udf_args,
                                                 char *message) noexcept;

  /**
   * @brief Main function for audit_log_filter_set_user UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param result UDFs result buffer
   * @param length Result length
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return Pointer to the result buffer
   */
  static char *audit_log_filter_set_user_udf(AuditUdf *udf, UDF_INIT *initid,
                                             UDF_ARGS *udf_args, char *result,
                                             unsigned long *length,
                                             unsigned char *is_null,
                                             unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_filter_set_user UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_filter_set_user_udf_deinit(UDF_INIT *initid);

  /**
   * @brief Init function for audit_log_filter_remove_user UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_filter_remove_user_udf_init(AuditUdf *udf,
                                                    UDF_INIT *initid,
                                                    UDF_ARGS *udf_args,
                                                    char *message) noexcept;

  /**
   * @brief Main function for audit_log_filter_remove_user UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param result UDFs result buffer
   * @param length Result length
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return Pointer to the result buffer
   */
  static char *audit_log_filter_remove_user_udf(AuditUdf *udf, UDF_INIT *initid,
                                                UDF_ARGS *udf_args,
                                                char *result,
                                                unsigned long *length,
                                                unsigned char *is_null,
                                                unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_filter_remove_user UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_filter_remove_user_udf_deinit(UDF_INIT *initid);

  /**
   * @brief Init function for audit_log_filter_flush UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_filter_flush_udf_init(AuditUdf *udf, UDF_INIT *initid,
                                              UDF_ARGS *udf_args,
                                              char *message) noexcept;

  /**
   * @brief Main function for audit_log_filter_flush UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param result UDFs result buffer
   * @param length Result length
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return Pointer to the result buffer
   */
  static char *audit_log_filter_flush_udf(AuditUdf *udf, UDF_INIT *initid,
                                          UDF_ARGS *udf_args, char *result,
                                          unsigned long *length,
                                          unsigned char *is_null,
                                          unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_filter_flush UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_filter_flush_udf_deinit(UDF_INIT *initid);

  /**
   * @brief Init function for audit_log_read UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_read_udf_init(AuditUdf *udf, UDF_INIT *initid,
                                      UDF_ARGS *udf_args,
                                      char *message) noexcept;

  /**
   * @brief Main function for audit_log_read UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param result UDFs result buffer
   * @param length Result length
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return Pointer to the result buffer
   */
  static char *audit_log_read_udf(AuditUdf *udf, UDF_INIT *initid,
                                  UDF_ARGS *udf_args, char *result,
                                  unsigned long *length, unsigned char *is_null,
                                  unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_read UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_read_udf_deinit(UDF_INIT *initid);

  /**
   * @brief Init function for audit_log_read_bookmark UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_read_bookmark_udf_init(AuditUdf *udf, UDF_INIT *initid,
                                               UDF_ARGS *udf_args,
                                               char *message) noexcept;

  /**
   * @brief Main function for audit_log_read_bookmark UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param result UDFs result buffer
   * @param length Result length
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return Pointer to the result buffer
   */
  static char *audit_log_read_bookmark_udf(AuditUdf *udf, UDF_INIT *initid,
                                           UDF_ARGS *udf_args, char *result,
                                           unsigned long *length,
                                           unsigned char *is_null,
                                           unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_read_bookmark UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_read_bookmark_udf_deinit(UDF_INIT *initid);

  /**
   * @brief Init function for audit_log_rotate UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_rotate_udf_init(AuditUdf *udf, UDF_INIT *initid,
                                        UDF_ARGS *udf_args,
                                        char *message) noexcept;

  /**
   * @brief Main function for audit_log_rotate UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param result UDFs result buffer
   * @param length Result length
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return Pointer to the result buffer
   */
  static char *audit_log_rotate_udf(AuditUdf *udf, UDF_INIT *initid,
                                    UDF_ARGS *udf_args, char *result,
                                    unsigned long *length,
                                    unsigned char *is_null,
                                    unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_rotate UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_rotate_udf_deinit(UDF_INIT *initid);

  /**
   * @brief Init function for audit_log_encryption_password_get UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_encryption_password_get_udf_init(
      AuditUdf *udf, UDF_INIT *initid, UDF_ARGS *udf_args,
      char *message) noexcept;

  /**
   * @brief Main function for audit_log_encryption_password_get UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param result UDFs result buffer
   * @param length Result length
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return Pointer to the result buffer
   */
  static char *audit_log_encryption_password_get_udf(
      AuditUdf *udf, UDF_INIT *initid, UDF_ARGS *udf_args, char *result,
      unsigned long *length, unsigned char *is_null,
      unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_encryption_password_get UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_encryption_password_get_udf_deinit(UDF_INIT *initid);

  /**
   * @brief Init function for audit_log_encryption_password_set UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_encryption_password_set_udf_init(
      AuditUdf *udf, UDF_INIT *initid, UDF_ARGS *udf_args,
      char *message) noexcept;

  /**
   * @brief Main function for audit_log_encryption_password_set UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param result UDFs result buffer
   * @param length Result length
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return Pointer to the result buffer
   */
  static char *audit_log_encryption_password_set_udf(
      AuditUdf *udf, UDF_INIT *initid, UDF_ARGS *udf_args, char *result,
      unsigned long *length, unsigned char *is_null,
      unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_encryption_password_set UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_encryption_password_set_udf_deinit(UDF_INIT *initid);

  /**
   * @brief Init function for audit_log_session_filter_id UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param message Error message in case of error
   * @retval false Success
   * @retval true  Failure. Error in the message argument
   */
  static bool audit_log_session_filter_id_udf_init(AuditUdf *udf,
                                                   UDF_INIT *initid,
                                                   UDF_ARGS *udf_args,
                                                   char *message) noexcept;

  /**
   * @brief Main function for audit_log_session_filter_id UDF.
   *
   * @param udf Pointer to UDFs handler instance
   * @param initid Pointer to UDF_INIT argument
   * @param udf_args Pointer to the UDF arguments struct
   * @param is_null Indicates a return value of NULL in the UDF
   * @param error Indicates if there was an error
   * @return ID of a filtering rule assigned to a session
   */
  static long long audit_log_session_filter_id_udf(
      AuditUdf *udf, UDF_INIT *initid, UDF_ARGS *udf_args,
      unsigned char *is_null, unsigned char *error) noexcept;

  /**
   * @brief De-init function for audit_log_session_filter_id UDF.
   *
   * @param initid Pointer to UDF_INIT argument
   */
  static void audit_log_session_filter_id_udf_deinit(UDF_INIT *initid);

 private:
  /**
   * @brief Set a character set name of a UDF return value.
   *
   * @param initid Pointer to UDF_INIT argument
   * @param charset_name Character set name, defaults to utf8mb4
   * @return true in case of success, false otherwise
   */
  static bool set_return_value_charset(
      UDF_INIT *initid, const std::string &charset_name = "utf8mb4") noexcept;

  /**
   * @brief Set a character set name of a UDF argument.
   *
   * @param udf_args Pointer to the UDF arguments struct
   * @param charset_name Character set name, defaults to utf8mb4
   * @return true in case of success, false otherwise
   */
  static bool set_args_charset(
      UDF_ARGS *udf_args, const std::string &charset_name = "utf8mb4") noexcept;

 private:
  std::vector<std::string> m_active_udf_names;
};

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_AUDIT_UDF_H_INCLUDED
