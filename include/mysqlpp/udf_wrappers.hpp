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

#ifndef MYSQLPP_UDF_WRAPPERS_HPP
#define MYSQLPP_UDF_WRAPPERS_HPP

#include <cassert>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <my_inttypes.h>
#include <mysql_com.h>
#include <mysqld_error.h>

#include <mysqlpp/common_types.hpp>
#include <mysqlpp/udf_context.hpp>
#include <mysqlpp/udf_error_reporter.hpp>
#include <mysqlpp/udf_exception.hpp>

#ifdef WIN32
#define MYSQLPP_UDF_EXPORT extern "C" __declspec(dllexport)
#else
#define MYSQLPP_UDF_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace mysqlpp {

template <typename ImplType>
struct udf_impl_meta_info;

// 'udf_base' is a part of the `generic_udf_base` / `generic_udf` logic that
// does not depend on template parameters. The main reason for doing this
// is to eliminate so called `c++ template code bloat` when functions with
// identical bodies will be present in different instantiations of the class
// template.
class udf_base {
 private:
  static const char *get_function_label(std::string &buffer,
                                        const char *meta_name,
                                        item_result_type item_result) noexcept {
    const char *res = "<function_name>";
    try {
      buffer = meta_name;
      buffer += '<';
      auto item_result_label = get_item_result_label(item_result);
      buffer.append(item_result_label.data(), item_result_label.size());
      buffer += '>';
      res = buffer.c_str();
    } catch (...) {
    }
    return res;
  }
  static void handle_exception(const char *meta_name,
                               item_result_type item_result) noexcept {
    auto error_reporter = udf_error_reporter::instance();
    assert(error_reporter != nullptr);
    std::string buffer;
    try {
      // The following suppression is needed exclusively for Clang 5.0 that
      // has a bug in noexcept specification diagnostics.
#if defined(__clang__) && (__clang_major__ == 5)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexceptions"
#endif
      // Rethrowing the exception that was previously caught with
      // 'catch(...)' in one of the derived classes
      // This is done to write the sequence of the catch blocks
      // in one place.
      throw;
#if defined(__clang__) && (__clang_major__ == 5)
#pragma clang diagnostic pop
#endif
    } catch (const udf_exception &e) {
      if (e.has_error_code()) {
        auto error_code = e.get_error_code();
        if (error_code == ER_QUERY_INTERRUPTED)
          (*error_reporter)(error_code, MYF(0));
        else
          (*error_reporter)(error_code, MYF(0),
                            get_function_label(buffer, meta_name, item_result),
                            e.what());
      }
    } catch (const std::exception &e) {
      (*error_reporter)(ER_UDF_ERROR, MYF(0),
                        get_function_label(buffer, meta_name, item_result),
                        e.what());
    } catch (...) {
      (*error_reporter)(ER_UDF_ERROR, MYF(0),
                        get_function_label(buffer, meta_name, item_result),
                        "unexpected exception");
    }
  }

 protected:
  template <typename ImplType>
  static void handle_exception() noexcept {
    using meta_info = udf_impl_meta_info<ImplType>;
    handle_exception(meta_info::name, meta_info::item_result);
  }
  static void handle_init_exception(char *message,
                                    std::size_t message_size) noexcept {
    try {
      // The following suppression is needed exclusively for Clang 5.0 that
      // has a bug in noexcept specification diagnostics
#if defined(__clang__) && (__clang_major__ == 5)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexceptions"
#endif
      // Rethrowing the exception that was previously caught with
      // 'catch(...)' in one of the derived classes
      // This is done to write the sequence of the catch blocks
      // in one place.
      throw;
#if defined(__clang__) && (__clang_major__ == 5)
#pragma clang diagnostic pop
#endif
    } catch (const std::exception &e) {
      std::strncpy(message, e.what(), message_size);
      message[message_size - 1] = '\0';
    } catch (...) {
      std::strncpy(message, "unexpected exception", message_size);
      message[message_size - 1] = '\0';
    }
  }
  static void validate_argument_nullness(const udf_context &udf_ctx) {
    for (std::size_t index = 0; index < udf_ctx.get_number_of_args(); ++index) {
      if (!udf_ctx.is_arg_nullable(index) && udf_ctx.is_arg_null(index)) {
        throw std::invalid_argument("argument " + std::to_string(index + 1) +
                                    " cannot be null");
      }
    }
  }
};

template <typename ImplType, item_result_type ItemResult>
class generic_udf_base : private udf_base {
 public:
  static bool init(UDF_INIT *initid, UDF_ARGS *args, char *message) noexcept {
    udf_context udf_ctx{initid, args};
    extended_impl_t *impl = nullptr;
    try {
      impl = new extended_impl_t{udf_ctx};
    } catch (...) {
      handle_init_exception(message, MYSQL_ERRMSG_SIZE);
      return true;
    }

    initid->ptr = reinterpret_cast<char *>(impl);

    return false;
  }

  static void deinit(UDF_INIT *initid) noexcept {
    delete get_extended_impl_from_udf_initid(initid);
  }

 protected:
  using extended_impl_t = impl_with_mixin<udf_mixin_t<ItemResult>, ImplType>;

  static extended_impl_t *get_extended_impl_from_udf_initid(
      UDF_INIT *initid) noexcept {
    return reinterpret_cast<extended_impl_t *>(initid->ptr);
  }
};

template <typename ImplType, item_result_type ItemResult>
class generic_udf;

template <typename ImplType>
class generic_udf<ImplType, STRING_RESULT>
    : public generic_udf_base<ImplType, STRING_RESULT> {
 public:
  static char *func(UDF_INIT *initid, UDF_ARGS *args, char * /* result */,
                    unsigned long *length, unsigned char *is_null,
                    unsigned char *error) noexcept {
    auto &extended_impl = *generic_udf_base<
        ImplType, STRING_RESULT>::get_extended_impl_from_udf_initid(initid);
    udf_result_t<STRING_RESULT> res;
    const udf_context udf_ctx{initid, args};
    try {
      udf_base::validate_argument_nullness(udf_ctx);
      res = extended_impl.impl.calculate(udf_ctx);
    } catch (...) {
      udf_base::template handle_exception<ImplType>();
      *error = 1;
      return nullptr;
    }

    *error = 0;
    if (!res) {
      assert(udf_ctx.is_result_nullabale());
      *is_null = 1;
      return nullptr;
    } else {
      *is_null = 0;
      extended_impl.mixin = std::move(res.value());
      *length = extended_impl.mixin.size();
      return const_cast<char *>(extended_impl.mixin.c_str());
    }
  }
};

template <typename ImplType>
class generic_udf<ImplType, REAL_RESULT>
    : public generic_udf_base<ImplType, REAL_RESULT> {
 public:
  static double func(UDF_INIT *initid, UDF_ARGS *args, unsigned char *is_null,
                     unsigned char *error) noexcept {
    auto &extended_impl = *generic_udf_base<
        ImplType, REAL_RESULT>::get_extended_impl_from_udf_initid(initid);
    udf_result_t<REAL_RESULT> res;
    const udf_context udf_ctx{initid, args};
    try {
      udf_base::validate_argument_nullness(udf_ctx);
      res = extended_impl.impl.calculate(udf_ctx);
    } catch (...) {
      udf_base::template handle_exception<ImplType>();
      *error = 1;
      return 0.0;
    }

    *error = 0;
    if (!res) {
      assert(udf_ctx.is_result_nullabale());
      *is_null = 1;
      return 0.0;
    } else {
      *is_null = 0;
      return res.value();
    }
  }
};

template <typename ImplType>
class generic_udf<ImplType, INT_RESULT>
    : public generic_udf_base<ImplType, INT_RESULT> {
 public:
  static long long func(UDF_INIT *initid, UDF_ARGS *args,
                        unsigned char *is_null, unsigned char *error) noexcept {
    auto &extended_impl = *generic_udf_base<
        ImplType, INT_RESULT>::get_extended_impl_from_udf_initid(initid);
    udf_result_t<INT_RESULT> res;
    const udf_context udf_ctx{initid, args};
    try {
      udf_base::validate_argument_nullness(udf_ctx);
      res = extended_impl.impl.calculate(udf_ctx);
    } catch (...) {
      udf_base::template handle_exception<ImplType>();
      *error = 1;
      return 0;
    }

    *error = 0;
    if (!res) {
      assert(udf_ctx.is_result_nullabale());
      *is_null = 1;
      return 0;
    } else {
      *is_null = 0;
      return res.value();
    }
  }
};

}  // namespace mysqlpp

#define DECLARE_UDF_META_INFO(IMPL, RESULT_TYPE, NAME)           \
  namespace mysqlpp {                                            \
  template <>                                                    \
  struct udf_impl_meta_info<IMPL> {                              \
    static constexpr item_result_type item_result = RESULT_TYPE; \
    static constexpr const char *const name = #NAME;             \
    using type = IMPL;                                           \
  };                                                             \
  }

#define DECLARE_UDF_INIT(IMPL, RESULT_TYPE, NAME)                        \
  MYSQLPP_UDF_EXPORT                                                     \
  bool NAME##_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {    \
    static_assert(std::is_same_v<decltype(&NAME##_init), Udf_func_init>, \
                  "Invalid UDF init function signature");                \
    return mysqlpp::generic_udf<IMPL, RESULT_TYPE>::init(initid, args,   \
                                                         message);       \
  }

#define DECLARE_UDF_DEINIT(IMPL, RESULT_TYPE, NAME)                          \
  MYSQLPP_UDF_EXPORT                                                         \
  void NAME##_deinit(UDF_INIT *initid) {                                     \
    static_assert(std::is_same_v<decltype(&NAME##_deinit), Udf_func_deinit>, \
                  "Invalid UDF deinit function signature");                  \
    mysqlpp::generic_udf<IMPL, RESULT_TYPE>::deinit(initid);                 \
  }

#define DECLARE_UDF_STRING_FUNC(IMPL, NAME)                         \
  MYSQLPP_UDF_EXPORT                                                \
  char *NAME(UDF_INIT *initid, UDF_ARGS *args, char *result,        \
             unsigned long *length, unsigned char *is_null,         \
             unsigned char *error) {                                \
    static_assert(std::is_same_v<decltype(&NAME), Udf_func_string>, \
                  "Invalid string UDF function signature");         \
    return mysqlpp::generic_udf<IMPL, STRING_RESULT>::func(         \
        initid, args, result, length, is_null, error);              \
  }

#define DECLARE_UDF_REAL_FUNC(IMPL, NAME)                                 \
  MYSQLPP_UDF_EXPORT                                                      \
  double NAME(UDF_INIT *initid, UDF_ARGS *args, unsigned char *is_null,   \
              unsigned char *error) {                                     \
    static_assert(std::is_same_v<decltype(&NAME), Udf_func_double>,       \
                  "Invalid real UDF function signature");                 \
    return mysqlpp::generic_udf<IMPL, REAL_RESULT>::func(initid, args,    \
                                                         is_null, error); \
  }

#define DECLARE_UDF_INT_FUNC(IMPL, NAME)                                       \
  MYSQLPP_UDF_EXPORT                                                           \
  long long NAME(UDF_INIT *initid, UDF_ARGS *args, unsigned char *is_null,     \
                 unsigned char *error) {                                       \
    static_assert(std::is_same<decltype(&NAME), Udf_func_longlong>::value,     \
                  "Invalid int UDF function signature");                       \
    return mysqlpp::generic_udf<IMPL, INT_RESULT>::func(initid, args, is_null, \
                                                        error);                \
  }

#define DECLARE_STRING_UDF(IMPL, NAME)             \
  DECLARE_UDF_META_INFO(IMPL, STRING_RESULT, NAME) \
  DECLARE_UDF_INIT(IMPL, STRING_RESULT, NAME)      \
  DECLARE_UDF_STRING_FUNC(IMPL, NAME)              \
  DECLARE_UDF_DEINIT(IMPL, STRING_RESULT, NAME)

#define DECLARE_REAL_UDF(IMPL, NAME)             \
  DECLARE_UDF_META_INFO(IMPL, REAL_RESULT, NAME) \
  DECLARE_UDF_INIT(IMPL, REAL_RESULT, NAME)      \
  DECLARE_UDF_REAL_FUNC(IMPL, NAME)              \
  DECLARE_UDF_DEINIT(IMPL, REAL_RESULT, NAME)

#define DECLARE_INT_UDF(IMPL, NAME)             \
  DECLARE_UDF_META_INFO(IMPL, INT_RESULT, NAME) \
  DECLARE_UDF_INIT(IMPL, INT_RESULT, NAME)      \
  DECLARE_UDF_INT_FUNC(IMPL, NAME)              \
  DECLARE_UDF_DEINIT(IMPL, INT_RESULT, NAME)

// A simplified versions of the DECLARE_STRING_UDF. DECLARE_REAL_UDF and
// DECLARE_INT_UDF macros that rely on the fact that the logic of the
// '<custom_udf>' UDF is put into the '<custom_udf>_impl' class
#define DECLARE_STRING_UDF_AUTO(NAME) DECLARE_STRING_UDF(NAME##_impl, NAME)
#define DECLARE_REAL_UDF_AUTO(NAME) DECLARE_REAL_UDF(NAME##_impl, NAME)
#define DECLARE_INT_UDF_AUTO(NAME) DECLARE_INT_UDF(NAME##_impl, NAME)

#endif
