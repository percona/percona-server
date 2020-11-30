/* Copyright (c) 2020 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef MYSQLPP_UDF_CONTEXT_CHARSET_EXTENSION_HPP
#define MYSQLPP_UDF_CONTEXT_CHARSET_EXTENSION_HPP

#include <cstddef>
#include <stdexcept>

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/udf_metadata.h>

#include <mysqlpp/udf_context.hpp>

namespace mysqlpp {

// A helper class which simplifies handling of charsets / collations for the
// string arguments and the result of a UDF.
// This functionality is put into a separate class (rather than directly into
// 'udf_context') deliberately because the implementation depends on the
// 'mysql_udf_metadata' service and this dependency may not be desired in all
// use cases.
class udf_context_charset_extension {
 private:
  static constexpr const char *const charset_attribute_name = "charset";
  static constexpr const char *const collation_attribute_name = "collation";

 public:
  explicit udf_context_charset_extension(SERVICE_TYPE(mysql_udf_metadata) *
                                         udf_metadata_service)
      : udf_metadata_service_{udf_metadata_service} {}

  const char *get_return_value_charset(const udf_context &ctx) const {
    void *output = nullptr;
    if (udf_metadata_service_->result_get(ctx.initid_, charset_attribute_name,
                                          &output))
      throw std::runtime_error{"cannot get return value character set"};

    return static_cast<char *>(output);
  }

  void set_return_value_charset(udf_context &ctx, const char *charset) const {
    void *cs = const_cast<char *>(charset);
    if (udf_metadata_service_->result_set(ctx.initid_, charset_attribute_name,
                                          cs))
      throw std::runtime_error{"cannot set return value character set"};
  }

  const char *get_arg_charset(const udf_context &ctx, std::size_t index) const {
    void *output = nullptr;
    if (ctx.args_->arg_type[index] != STRING_RESULT)
      throw std::runtime_error{
          "cannot get character set of a non-string argument"};

    if (udf_metadata_service_->argument_get(ctx.args_, charset_attribute_name,
                                            index, &output))
      throw std::runtime_error{"cannot get argument character set"};

    return static_cast<char *>(output);
  }

  void set_arg_value_charset(udf_context &ctx, std::size_t index,
                             const char *charset) const {
    void *cs = const_cast<char *>(charset);
    if (ctx.args_->arg_type[index] != STRING_RESULT)
      throw std::runtime_error{
          "cannot set character set of a non-string argument"};

    if (udf_metadata_service_->argument_set(ctx.args_, charset_attribute_name,
                                            index, cs))
      throw std::runtime_error{"cannot set argument value character set"};
  }

  const char *get_return_value_collation(const udf_context &ctx) const {
    void *output = nullptr;
    if (udf_metadata_service_->result_get(ctx.initid_, collation_attribute_name,
                                          &output))
      throw std::runtime_error{"cannot get return value collation"};

    return static_cast<char *>(output);
  }

  void set_return_value_collation(udf_context &ctx,
                                  const char *collation) const {
    void *cs = const_cast<char *>(collation);
    if (udf_metadata_service_->result_set(ctx.initid_, collation_attribute_name,
                                          cs))
      throw std::runtime_error{"cannot set return value collation"};
  }

  const char *get_arg_collation(const udf_context &ctx,
                                std::size_t index) const {
    void *output = nullptr;
    if (ctx.args_->arg_type[index] != STRING_RESULT)
      throw std::runtime_error{"cannot get collation of a non-string argument"};

    if (udf_metadata_service_->argument_get(ctx.args_, collation_attribute_name,
                                            index, &output))
      throw std::runtime_error{"cannot get argument collation"};

    return static_cast<char *>(output);
  }

  void set_arg_value_collation(udf_context &ctx, std::size_t index,
                               const char *collation) const {
    void *cs = const_cast<char *>(collation);
    if (ctx.args_->arg_type[index] != STRING_RESULT)
      throw std::runtime_error{"cannot set collation of a non-string argument"};

    if (udf_metadata_service_->argument_set(ctx.args_, collation_attribute_name,
                                            index, cs))
      throw std::runtime_error{"cannot set argument value collation"};
  }

 private:
  SERVICE_TYPE(mysql_udf_metadata) * udf_metadata_service_;
};

}  // namespace mysqlpp

#endif
