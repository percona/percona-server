/* Copyright (c) 2023, 2024 Percona LLC and/or its affiliates. All rights
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

/*
  This file contains methods of Js_sp class and some helpers which are
  responsible for converting and storing JS values to/from SQL routine
  parameters and return values.
*/

#include "js_lang_core.h"

// Names of important character sets.
static const char *const UTF8_CS_NAME = "utf8mb4";
static const char *const BINARY_CS_NAME = "binary";

// Biggest integer which is safe to store as JS Number value.
static const long long NUMBER_MAX_SAFE_INTEGER = 9007199254740991;  // 2^53-1

/**
  Traits class which encapsulates knowledge how to get metadata for routine
  return value as well as set this value (using mysql_stored_program_*
  family services).

  @note In cases when methods for getting metadata or setting values can fail
        they return false in case of success and true in case of failure (the
        error is reported by the method in the latter case).
*/
class Return_value_handler {
 public:
  static bool get_param_type(stored_program_handle sql_sp, uint /* idx */,
                             uint64_t *sql_type) {
    if (mysql_service_mysql_stored_program_return_metadata_query->get(
            sql_sp, "sql_type", sql_type)) {
      // Getting return value type can fail if the SQL type was added recently
      // and Stored Program Service implementation was not updated to handle
      // it correctly. Since such scenario is not unlikely we try to play
      // safe in such a case.
      my_error(ER_LANGUAGE_COMPONENT, MYF(0),
               "Can't get routine return value metadata (unhandled type?)");
      return true;
    }
    return false;
  }
  static void get_param_signedness(stored_program_handle sql_sp, uint /* idx */,
                                   bool *is_signed) {
    always_ok(mysql_service_mysql_stored_program_return_metadata_query->get(
        sql_sp, "is_signed", is_signed));
  }
  static void get_param_charset(stored_program_handle sql_sp, uint /* idx */,
                                const char **cs_name) {
    always_ok(mysql_service_mysql_stored_program_return_metadata_query->get(
        sql_sp, "charset", cs_name));
  }
  static void get_param_collation(stored_program_handle sql_sp, uint /* idx */,
                                  const char **coll_name) {
    always_ok(mysql_service_mysql_stored_program_return_metadata_query->get(
        sql_sp, "collation", coll_name));
  }
  static void get_param_max_display_length(stored_program_handle sql_sp,
                                           uint /* idx */,
                                           size_t *max_display_length) {
    always_ok(mysql_service_mysql_stored_program_return_metadata_query->get(
        sql_sp, "max_display_length", max_display_length));
  }

  static bool set_param_null(uint /* idx */) {
    return mysql_service_mysql_stored_program_return_value_null->set(nullptr);
  }
  static bool set_param_int(uint /* idx */, int64_t value) {
    return mysql_service_mysql_stored_program_return_value_int->set(nullptr,
                                                                    value);
  }
  static bool set_param_uint(uint /* idx */, uint64_t value) {
    return mysql_service_mysql_stored_program_return_value_unsigned_int->set(
        nullptr, value);
  }
  static bool set_param_string(uint /* idx */, const char *string,
                               size_t length) {
    return mysql_service_mysql_stored_program_return_value_string->set(
        nullptr, string, length);
  }
  static bool set_param_float(uint /* idx */, double value) {
    return mysql_service_mysql_stored_program_return_value_float->set(nullptr,
                                                                      value);
  }
};

/**
  Ancestor for traits classes which encapsulate knowledge how to get metadata
  for routine's IN/INOUT and OUT parameters (using mysql_stored_program_*
  family of services).
*/
class Param_handler {
 public:
  static bool get_param_type(stored_program_handle sql_sp, uint idx,
                             uint64_t *sql_type) {
    if (mysql_service_mysql_stored_program_argument_metadata_query->get(
            sql_sp, idx, "sql_type", sql_type)) {
      // Getting parameter type can fail if the SQL type was added recently
      // and Stored Program Service implementation was not updated to handle
      // it correctly. Since such scenario is not unlikely we try to play
      // safe in such a case.
      my_error(ER_LANGUAGE_COMPONENT, MYF(0),
               "Can't get routine parameter metadata (unhandled type?)");
      return true;
    }
    return false;
  }
  static void get_param_signedness(stored_program_handle sql_sp, uint idx,
                                   bool *is_signed) {
    always_ok(mysql_service_mysql_stored_program_argument_metadata_query->get(
        sql_sp, idx, "is_signed", is_signed));
  }
  static void get_param_charset(stored_program_handle sql_sp, uint idx,
                                const char **cs_name) {
    always_ok(mysql_service_mysql_stored_program_argument_metadata_query->get(
        sql_sp, idx, "charset", cs_name));
  }
  static void get_param_collation(stored_program_handle sql_sp, uint idx,
                                  const char **coll_name) {
    always_ok(mysql_service_mysql_stored_program_argument_metadata_query->get(
        sql_sp, idx, "collation", coll_name));
  }
  static void get_param_max_display_length(stored_program_handle sql_sp,
                                           uint idx,
                                           size_t *max_display_length) {
    always_ok(mysql_service_mysql_stored_program_argument_metadata_query->get(
        sql_sp, idx, "max_display_length", max_display_length));
  }
};

/**
  Traits class which encapsulates knowledge how to get metadata and value
  of routine's IN and INOUT parameters (using mysql_stored_program_* family
  of services).

  @note In cases when methods for getting metadata or getting values can fail
        they return false in case of success and true in case of failure (the
        error is reported by the method in the latter case).
*/
class In_param_handler : public Param_handler {
 public:
  static bool get_param_int(uint idx, int64_t *value, bool *is_null) {
    if (mysql_service_mysql_stored_program_runtime_argument_int->get(
            nullptr, idx, value, is_null)) {
      my_error(ER_LANGUAGE_COMPONENT, MYF(0),
               "Can't get routine parameter value");
      return true;
    }
    return false;
  }
  static bool get_param_uint(uint idx, uint64_t *value, bool *is_null) {
    if (mysql_service_mysql_stored_program_runtime_argument_unsigned_int->get(
            nullptr, idx, value, is_null)) {
      my_error(ER_LANGUAGE_COMPONENT, MYF(0),
               "Can't get routine parameter value");
      return true;
    }
    return false;
  }
  static bool get_param_float(uint idx, double *value, bool *is_null) {
    if (mysql_service_mysql_stored_program_runtime_argument_float->get(
            nullptr, idx, value, is_null)) {
      my_error(ER_LANGUAGE_COMPONENT, MYF(0),
               "Can't get routine parameter value");
      return true;
    }
    return false;
  }
  static bool get_param_string(uint idx, const char **string, size_t *length,
                               bool *is_null) {
    if (mysql_service_mysql_stored_program_runtime_argument_string->get(
            nullptr, idx, string, length, is_null)) {
      my_error(ER_LANGUAGE_COMPONENT, MYF(0),
               "Can't get routine parameter value");
      return true;
    }
    return false;
  }
};

/**
  Traits class which encapsulates knowledge how to get metadata and set value
  of routine's INOUT and OUT parameters (using mysql_stored_program_* family
  of services).

  @note In cases when methods for getting metadata or setting values can fail
        they return false in case of success and true in case of failure (the
        error is reported by the method in the latter case).
*/
class Out_param_handler : public Param_handler {
 public:
  static bool set_param_null(uint idx) {
    return mysql_service_mysql_stored_program_runtime_argument_null->set(
        nullptr, idx);
  }
  static bool set_param_int(uint idx, int64_t value) {
    return mysql_service_mysql_stored_program_runtime_argument_int->set(
        nullptr, idx, value);
  }
  static bool set_param_uint(uint idx, uint64_t value) {
    return mysql_service_mysql_stored_program_runtime_argument_unsigned_int
        ->set(nullptr, idx, value);
  }
  static bool set_param_string(uint idx, const char *string, size_t length) {
    return mysql_service_mysql_stored_program_runtime_argument_string->set(
        nullptr, idx, string, length);
  }
  static bool set_param_float(uint idx, double value) {
    return mysql_service_mysql_stored_program_runtime_argument_float->set(
        nullptr, idx, value);
  }
};

// Helper functions which constructs no-JS-value-error return value for
// the routine parameter getter functions.
static v8::Local<v8::Value> no_js_value_for_param() {
  return v8::Local<v8::Value>();
}

// Helper functions which constructs no-JS-value-error return value for
// the routine parameter getter functions and reports error to the user.
static v8::Local<v8::Value> no_js_value_for_param_with_error() {
  my_error(ER_LANGUAGE_COMPONENT, MYF(0),
           "Can't create JS value from routine parameter value");
  return v8::Local<v8::Value>();
}

/**
  RAII helper which allows to cleanup string objects represented
  by my_h_string handle.
*/
class My_h_string_guard {
 public:
  My_h_string_guard(my_h_string str_h) : m_str_h(str_h) {}
  ~My_h_string_guard() { mysql_service_mysql_string_factory->destroy(m_str_h); }

 private:
  my_h_string m_str_h;
};

Js_sp::get_param_func_t Js_sp::prepare_get_param_func(uint idx) {
  uint64_t sql_type;
  if (In_param_handler::get_param_type(m_sql_sp, idx, &sql_type))
    return get_param_func_t();

  switch (sql_type) {
    // BOOLEAN type is just alias for TINYINT at the moment, so we
    // should never meet it. If there will be separate boolean type
    // we need to consider mapping it to JS booleans.
    // For now we treat it as integer for compatibility sake.
    case MYSQL_SP_ARG_TYPE_BOOL: {
      assert(false);
      [[fallthrough]];
    }
    // YEAR values are mapped to integers.
    case MYSQL_SP_ARG_TYPE_YEAR: {
      [[fallthrough]];
    }
    case MYSQL_SP_ARG_TYPE_TINY:
    case MYSQL_SP_ARG_TYPE_SHORT:
    case MYSQL_SP_ARG_TYPE_INT24:
    case MYSQL_SP_ARG_TYPE_LONG: {
      bool is_signed;
      In_param_handler::get_param_signedness(m_sql_sp, idx, &is_signed);

      if (is_signed) {
        return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
          int64_t value;
          bool is_null;
          if (In_param_handler::get_param_int(idx, &value, &is_null))
            return no_js_value_for_param();

          if (is_null)
            return v8::Null(isolate);
          else {
            // V8 aborts the process in case of OOM. So we don't try to
            // handle OOM gracefully here either.
            return v8::Integer::New(isolate, static_cast<int32_t>(value));
          }
        };
      } else {
        return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
          uint64_t value;
          bool is_null;
          if (In_param_handler::get_param_uint(idx, &value, &is_null))
            return no_js_value_for_param();

          if (is_null)
            return v8::Null(isolate);
          else {
            // V8 aborts the process in case of OOM. So we don't try to
            // handle OOM gracefully here either.
            return v8::Integer::NewFromUnsigned(isolate,
                                                static_cast<uint32_t>(value));
          }
        };
      }
      break;
    }
    case MYSQL_SP_ARG_TYPE_LONGLONG: {
      bool is_signed;
      In_param_handler::get_param_signedness(m_sql_sp, idx, &is_signed);

      // Lots of integers which are stored in BIGINT fields are probably
      // less than 2^53-1 and might benefit from optimized handling as
      // primitive types.
      //
      // OTOH it might be better to always use BigInt for 64-bit integers,
      // to create less surprises during development of JS routines.
      //
      // We follow the first approach at the moment.
      //
      // TODO: Consider implementing the second approach as an option.
      if (is_signed) {
        return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
          int64_t value;
          bool is_null;
          if (In_param_handler::get_param_int(idx, &value, &is_null))
            return no_js_value_for_param();

          // V8 aborts the process in case of OOM. So we don't try to
          // handle OOM gracefully here either.
          if (is_null)
            return v8::Null(isolate);
          else if (value >= INT32_MIN && value <= INT32_MAX) {
            return v8::Integer::New(isolate, static_cast<int32_t>(value));
          } else if (value >= -NUMBER_MAX_SAFE_INTEGER &&
                     value <= NUMBER_MAX_SAFE_INTEGER) {
            return v8::Number::New(isolate, static_cast<double>(value));
          } else {
            return v8::BigInt::New(isolate, value);
          }
        };
      } else {
        return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
          uint64_t value;
          bool is_null;
          if (In_param_handler::get_param_uint(idx, &value, &is_null))
            return no_js_value_for_param();

          // V8 aborts the process in case of OOM. So we don't try to
          // handle OOM gracefully here either.
          if (is_null)
            return v8::Null(isolate);
          else if (value <= UINT32_MAX) {
            return v8::Integer::NewFromUnsigned(isolate,
                                                static_cast<uint32_t>(value));
          } else if (value <= NUMBER_MAX_SAFE_INTEGER) {
            return v8::Number::New(isolate, static_cast<double>(value));
          } else {
            return v8::BigInt::NewFromUnsigned(isolate, value);
          }
        };
      }
      break;
    }
    case MYSQL_SP_ARG_TYPE_FLOAT:
    case MYSQL_SP_ARG_TYPE_DOUBLE: {
      return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
        // Even though FLOAT and DOUBLE parameters can be UNSIGNED this
        // doesn't affect their storage.
        double value;
        bool is_null;
        if (In_param_handler::get_param_float(idx, &value, &is_null))
          return no_js_value_for_param();

        if (is_null)
          return v8::Null(isolate);
        else {
          // V8 doesn't handle allocation failure gracefully,
          // so neither do we here.
          return v8::Number::New(isolate, value);
        }
      };
      break;
    }
    // We map BIT parameters with width <= 53 bits to integer JS Number
    // values.
    // BIT parameters with size > 53 bits can't be safely represented as
    // JS Number values in generic case, so we map them to BigInt values.
    //
    // TODO: Consider mapping to custom type as an option.
    case MYSQL_SP_ARG_TYPE_BIT: {
      size_t max_display_length;
      In_param_handler::get_param_max_display_length(m_sql_sp, idx,
                                                     &max_display_length);

      // Maximum size of BIT type which safe to store in JS Number.
      const size_t MAX_BIT_WIDTH_SAFE_FOR_NUMBER = 53;
      static_assert((1ULL << MAX_BIT_WIDTH_SAFE_FOR_NUMBER) - 1 <=
                    NUMBER_MAX_SAFE_INTEGER);

      if (max_display_length > MAX_BIT_WIDTH_SAFE_FOR_NUMBER) {
        return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
          uint64_t value;
          bool is_null;
          if (In_param_handler::get_param_uint(idx, &value, &is_null))
            return no_js_value_for_param();
          if (is_null)
            return v8::Null(isolate);
          else {
            // V8 doesn't handle allocation failure gracefully,
            // so neither do we here.
            return v8::BigInt::NewFromUnsigned(isolate, value);
          }
        };
      } else {
        return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
          uint64_t value;
          bool is_null;
          if (In_param_handler::get_param_uint(idx, &value, &is_null))
            return no_js_value_for_param();

          // V8 doesn't handle allocation failure gracefully,
          // so neither do we here.
          if (is_null)
            return v8::Null(isolate);
          else if (value <= UINT32_MAX) {
            return v8::Integer::NewFromUnsigned(isolate,
                                                static_cast<uint32_t>(value));
          } else {
            assert(value <= NUMBER_MAX_SAFE_INTEGER);
            return v8::Number::New(isolate, static_cast<double>(value));
          }
        };
      }
      break;
    }
    // JS has no native fixed precision type. So we map DECIMAL values
    // to strings to avoid loss of precision.
    //
    // TODO: Consider using custom type as possible future option?
    case MYSQL_SP_ARG_TYPE_NEWDECIMAL:
    //
    // TIME, DATE, DATETIME and TIMESTAMP values are mapped to strings.
    //
    // For TIME type there is no corresponding native JS type.
    //
    // While converting DATETIME/TIMESTMAMP and DATE values to JS Date
    // objects might seem like a natural thing, it turns out to be not
    // that good idea in practice.
    // To begin with Date objects are constructed from UTC seconds since
    // the Unix Epoch values which don't present in arguments API.
    // The second issue is time zone influence - we do not have much control
    // over the time zone used for conversions by V8 (most probably it is
    // the same as MySQL's SYSTEM), OTOH each MySQL connection can use its
    // own time zone. Even if we will use SYSTEM timezone/mktime_r() to
    // produce values equivalent to argument values in Isolate time zone,
    // connection vs JS time zone difference still might surprising.
    // So we take the lazy path for now and convert date and datetime values
    // to strings.
    //
    // TODO: Consider using custom types for this as an option?
    case MYSQL_SP_ARG_TYPE_TIME2:
    case MYSQL_SP_ARG_TYPE_TIMESTAMP2:
    case MYSQL_SP_ARG_TYPE_DATETIME2:
    case MYSQL_SP_ARG_TYPE_NEWDATE: {
      return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
        const char *str;
        size_t length;
        bool is_null;
        /*
          Note the below call builds string representation of SQL value
          without a native one, and then copies it to MEM_ROOT-allocated
          buffer to be able to return pointer to it to the caller.

          TODO: Study how much performance overhead this creates and possibly
                optimize.
        */
        if (In_param_handler::get_param_string(idx, &str, &length, &is_null))
          return no_js_value_for_param();
        if (is_null)
          return v8::Null(isolate);
        else {
          // String representation of DECIMAL type is always UTF8 compatible
          // (uses my_charset_numeric).
          //
          // The same is true for datetime types.
          //
          // Also these string representations can never exceed JS string
          // length limit, so the below call can fail only in case of OOM.
          // Since V8 handles OOM by aborting the process we do not try to
          // handle it gracefully here either.
          return v8::String::NewFromUtf8(isolate, str,
                                         v8::NewStringType::kNormal, length)
              .ToLocalChecked();
        }
      };
      break;
    }
    // Internal representation of GEOMETRY objects is based on WKB
    // standard. So we simply provide access to it using JS DataView.
    case MYSQL_SP_ARG_TYPE_GEOMETRY: {
      return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
        const char *str;
        size_t length;
        bool is_null;
        if (In_param_handler::get_param_string(idx, &str, &length, &is_null))
          return no_js_value_for_param();
        if (is_null)
          return v8::Null(isolate);
        else {
          // At the moment V8 simply aborts the process if it fails to
          // allocate ArrayBuffer. It also aborts on other allocation
          // failures. So we don't try to handle allocation failures
          // here gracefully either.
          v8::Local<v8::ArrayBuffer> buffer =
              v8::ArrayBuffer::New(isolate, length);
          v8::Local<v8::DataView> view = v8::DataView::New(buffer, 0, length);
          memcpy(buffer->Data(), str, length);
          return view;
        }
      };
      break;
    }
    // We map arguments of JSON SQL type to JS objects/values which we
    // reconstruct from JSON string using JSON.parse() call.
    case MYSQL_SP_ARG_TYPE_JSON: {
      return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
        const char *str;
        size_t length;
        bool is_null;
        /*
          Note the below call builds string which corresponds to SQL's JSON
          binary value. In then without a native one, and then copies it to
          MEM_ROOT-allocated buffer to be able to return pointer to it to
          the caller. This introduces some performance and memory overhead
          as this MEM_ROOT-allocated buffer will stay around until the end
          of routine execution.

          TODO: Consider improving memory usage and performance in this case
                possibly by introducing new service API which will skip extra
                allocation and copying by just handing over the original
                string buffer to the caller.
        */
        if (In_param_handler::get_param_string(idx, &str, &length, &is_null))
          return no_js_value_for_param();
        if (is_null)
          return v8::Null(isolate);
        else {
          // String representation of JSON SQL type value should be always
          // UTF8 compatible.
          v8::Local<v8::String> json_str;
          if (!v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal,
                                       length)
                   .ToLocal(&json_str)) {
            // String representation of SQL JSON value can exceed V8 string
            // length limit.
            my_error(ER_LANGUAGE_COMPONENT, MYF(0),
                     "Can't create JS value from routine parameter value of "
                     "JSON type (V8 string length limit excedeed)");
            return no_js_value_for_param();
          }

          v8::Local<v8::Value> result;
          if (!v8::JSON::Parse(isolate->GetCurrentContext(), json_str)
                   .ToLocal(&result)) {
            // In theory whatever is stored in JSON SQL type should be always
            // JSON.parse()-able. But we play safe.
            my_error(ER_LANGUAGE_COMPONENT, MYF(0),
                     "Can't create JS value from routine parameter value of "
                     "JSON type (can't parse JSON string)");
            return no_js_value_for_param();
          }
          return result;
        }
      };
      break;
    }
    // Obsolete types which shold not be possible to use as parameter
    // for newly created routine.
    case MYSQL_SP_ARG_TYPE_DECIMAL:
    case MYSQL_SP_ARG_TYPE_DATE:
    case MYSQL_SP_ARG_TYPE_VAR_STRING:
    case MYSQL_SP_ARG_TYPE_TIME:
    case MYSQL_SP_ARG_TYPE_TIMESTAMP:
    case MYSQL_SP_ARG_TYPE_DATETIME: {
      [[fallthrough]];
    }
    // Special types which should not be possible to use as parameter
    // for routine.
    case MYSQL_SP_ARG_TYPE_INVALID:
    case MYSQL_SP_ARG_TYPE_NULL:
    case MYSQL_SP_ARG_TYPE_TYPED_ARRAY: {
      [[fallthrough]];
    }
    // New types will require analysis and handling.
    default: {
      assert(false);
      [[fallthrough]];
    }
    // CHAR/VARCHAR and TEXT types are naturally mapped to strings.
    // BINARY/VARBINARY and BLOBs are string types with binary
    // charset. We map them to JS DataView on top of ArrayBuffer.
    // (as DataViews are more flexible than JS Typed Arrays).
    case MYSQL_SP_ARG_TYPE_VARCHAR:
    case MYSQL_SP_ARG_TYPE_STRING:
    case MYSQL_SP_ARG_TYPE_TINY_BLOB:
    case MYSQL_SP_ARG_TYPE_MEDIUM_BLOB:
    case MYSQL_SP_ARG_TYPE_BLOB:
    case MYSQL_SP_ARG_TYPE_LONG_BLOB:
    // SET parameters are mapped to string values.
    //
    // Ideally, SETs should be mapped to JS Set objects. However, we lack
    // API which allows to get SET values element by element to be able to
    // do this.
    //
    // Another alternative is to map SET values to numbers but we
    // consider this less user friendly.
    case MYSQL_SP_ARG_TYPE_SET:
    // We also map ENUM values to strings.
    case MYSQL_SP_ARG_TYPE_ENUM: {
      const char *cs_name;
      In_param_handler::get_param_charset(m_sql_sp, idx, &cs_name);

      // In theory handling of utf8mb3 and latin1 character sets can be
      // optimized as well. However, we prefer to keep the code simple
      // instead. People should upgrade to utf8mb4 or accept performance
      // penalty.
      if (strcmp(cs_name, UTF8_CS_NAME) == 0) {
        return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
          const char *str;
          size_t length;
          bool is_null;
          /*
            N.B. The below call only works well for parameters with types
            having native string representation internally, so the pointer
            to this internal representation can be returned.

            For types which have non-string native representation, like JSON,
            SET, DECIMAL or datetime, string representation is constructed in
            a heap-allocated memory buffer.
            Ideally, this buffer should have been handed over to and its
            lifetime should have been controlled by code calling the service.
            However, mysql_stored_program_runtime_argument_string service API
            doesn't allow that. Instead SQL-core creates yet another copy of
            this buffer on current memory root (which happens to be in our
            case, "call" memory root (sp_head::execute_function()) and returns
            pointer to this copy.

            This wastes memory since the memory root is kept around until end
            of the call and introduces performance overhead.

            TODO: Consider improving memory usage and performance in this case
                  possibly by introducing new service API which will handover
                  buffer to caller.
          */
          if (In_param_handler::get_param_string(idx, &str, &length, &is_null))
            return no_js_value_for_param();

          if (is_null)
            return v8::Null(isolate);
          else {
            v8::Local<v8::Value> result;
            if (!v8::String::NewFromUtf8(isolate, str,
                                         v8::NewStringType::kNormal, length)
                     .ToLocal(&result)) {
              // Length of SQL LONGTEXT value can exceed V8 string length
              // limit, so we handle it gracefully.
              my_error(ER_LANGUAGE_COMPONENT, MYF(0),
                       "Can't create JS value from routine parameter "
                       "value (V8 string length limit excedeed)");
              return no_js_value_for_param();
            }
            return result;
          }
        };
      } else if (strcmp(cs_name, BINARY_CS_NAME) == 0) {
        if (sql_type == MYSQL_SP_ARG_TYPE_SET) {
          my_error(ER_LANGUAGE_COMPONENT, MYF(0),
                   "SET parameters using binary character set are not "
                   "supported.");
          return get_param_func_t();
        }
        return [](v8::Isolate *isolate, uint idx) -> v8::Local<v8::Value> {
          const char *str;
          size_t length;
          bool is_null;

          // See comment for UTF8 case.
          if (In_param_handler::get_param_string(idx, &str, &length, &is_null))
            return no_js_value_for_param();

          if (is_null)
            return v8::Null(isolate);
          else {
            // At the moment V8 simply aborts the process if it fails to
            // allocate ArrayBuffer. It also aborts on other allocation
            // failures. So we don't try to handle allocation failures
            // here gracefully either.
            v8::Local<v8::ArrayBuffer> buffer =
                v8::ArrayBuffer::New(isolate, length);
            v8::Local<v8::DataView> view = v8::DataView::New(buffer, 0, length);
            memcpy(buffer->Data(), str, length);
            return view;
          }
        };
      } else {
        const char *coll_name;
        In_param_handler::get_param_collation(m_sql_sp, idx, &coll_name);

        CHARSET_INFO_h src_cs_h = mysql_service_mysql_charset->get(coll_name);
        // Collation of parameter must be always available.
        assert(src_cs_h != nullptr);
        CHARSET_INFO_h utf8_cs_h = mysql_service_mysql_charset->get_utf8mb4();
        // UTF8mb4 must be always available.
        assert(utf8_cs_h != nullptr);

        return [src_cs_h, utf8_cs_h](v8::Isolate *isolate,
                                     uint idx) -> v8::Local<v8::Value> {
          const char *str;
          size_t length;
          bool is_null;

          // See comment for UTF8 case.
          if (In_param_handler::get_param_string(idx, &str, &length, &is_null))
            return no_js_value_for_param();

          if (is_null)
            return v8::Null(isolate);
          else {
            my_h_string str_h;
            if (mysql_service_mysql_string_factory->create(&str_h))
              return no_js_value_for_param_with_error();

            My_h_string_guard str_h_guard(str_h);

            uint errors;
            if (mysql_service_mysql_string_copy_converter->copy_convert(
                    str_h, str, length, src_cs_h, utf8_cs_h, &errors) ||
                errors != 0) {
              // In theory should not happed as we are converting to
              // UTF-8.
              return no_js_value_for_param_with_error();
            }

            const char *utf8_buff;
            size_t utf8_length;
            CHARSET_INFO_h not_used;
            always_ok(mysql_service_mysql_string_get_data_in_charset->get_data(
                str_h, &utf8_buff, &utf8_length, &not_used));

            v8::Local<v8::Value> result;
            if (!v8::String::NewFromUtf8(isolate, utf8_buff,
                                         v8::NewStringType::kNormal,
                                         utf8_length)
                     .ToLocal(&result)) {
              // Length of SQL LONGTEXT value can exceed V8 string length
              // limit, so we handle it gracefully.
              my_error(ER_LANGUAGE_COMPONENT, MYF(0),
                       "Can't create JS value from routine parameter "
                       "value (V8 string length limit excedeed)");
              return no_js_value_for_param();
            }
            return result;
          }
        };
      }
      break;
    }
  }
}

bool Js_sp::prepare_get_param_funcs() {
  for (uint param_idx = 0; param_idx < m_arg_count; ++param_idx) {
    get_param_func_t func = prepare_get_param_func(param_idx);
    if (!func) return true;
    m_get_param_func.push_back(func);
  }
  return false;
}

static void my_error_js_value_to_string_param() {
  my_error(ER_LANGUAGE_COMPONENT, MYF(0),
           "Can't convert JS value to string to be stored in out parameter or "
           "return value");
}

template <class H>
Js_sp::set_param_func_t Js_sp::prepare_set_param_func(uint param_idx) {
  uint64_t sql_type;
  if (H::get_param_type(m_sql_sp, param_idx, &sql_type))
    return set_param_func_t();

  switch (sql_type) {
    // BOOLEAN type is just alias for TINYINT at the moment, so we
    // should never meet it. If there will be separate boolean type
    // we need to consider special handling for it.
    // For now we treat it as integer for compatibility sake.
    case MYSQL_SP_ARG_TYPE_BOOL: {
      assert(false);
      [[fallthrough]];
    }
    // We treat YEAR type similarly to integer types.
    case MYSQL_SP_ARG_TYPE_YEAR: {
      [[fallthrough]];
    }
    // In theory, we can handle return values of integer SQL-types by
    // simply converting JS value to string and then letting SQL core
    // to try to extract value from this string.
    // However, this might be sub-optimal in many cases, so we try to
    // convert JS integers to SQL integers directly.
    case MYSQL_SP_ARG_TYPE_TINY:
    case MYSQL_SP_ARG_TYPE_SHORT:
    case MYSQL_SP_ARG_TYPE_INT24:
    case MYSQL_SP_ARG_TYPE_LONG:
    case MYSQL_SP_ARG_TYPE_LONGLONG: {
      bool is_signed;
      H::get_param_signedness(m_sql_sp, param_idx, &is_signed);

      if (is_signed) {
        return [](v8::Local<v8::Context> context, uint idx,
                  v8::Local<v8::Value> result) -> bool {
          if (result->IsUndefined() || result->IsNull()) {
            return H::set_param_null(idx);
          } else if (result->IsInt32()) {
            // Number already, so no real conversion that can fail.
            return H::set_param_int(idx,
                                    result->Int32Value(context).ToChecked());
          } else if (result->IsBigInt()) {
            v8::Local<v8::BigInt> bigint_result = result.As<v8::BigInt>();
            bool lossless;
            int64_t int64_result = bigint_result->Int64Value(&lossless);
            if (lossless) {
              return H::set_param_int(idx, int64_result);
            }
            // If direct conversion to int64_t is lossy, resort to
            // converting through string.
          }
          // A natural thing would be to have alternative for JS Number as well
          // (i.e. get it as double and pass as double to SQL core).
          //
          // However, such behavior can be confusing in some cases as MySQL
          // uses different rounding when storing in integer field values like:
          // 4.5 (decimal literal, rounded to 5, round()-style conversion),
          // 4.5e+0 (floating-point literal, rounded to 4 [sic!], rint()-style
          // conversion), "4.5" (decimal value in string, rounded to 5), and
          // "4.5e+0" (floating-point value in string, rounded to 5).
          //
          // To avoid the confusion we stick to converting JS Numbers to
          // SQL integers through strings, i.e. sticking to round()-style
          // conversion.
          //
          // TODO: Discuss this, perhaps this should be yet another mode.

          // Convert JS value to JS string and get it as UTF-8.
          v8::String::Utf8Value utf8(context->GetIsolate(), result);
          if (!*utf8) {
            // Conversion of JS value to JS string can fail (e.g. throw).
            my_error_js_value_to_string_param();
            return true;
          }
          return H::set_param_string(idx, *utf8, utf8.length());
        };
      } else {
        return [](v8::Local<v8::Context> context, uint idx,
                  v8::Local<v8::Value> result) -> bool {
          if (result->IsUndefined() || result->IsNull()) {
            return H::set_param_null(idx);
          } else if (result->IsUint32()) {
            // Number already, so no real conversion that can fail.
            return H::set_param_uint(idx,
                                     result->Uint32Value(context).ToChecked());
          } else if (result->IsBigInt()) {
            v8::Local<v8::BigInt> bigint_result = result.As<v8::BigInt>();
            bool lossless;
            uint64_t uint64_result = bigint_result->Uint64Value(&lossless);
            if (lossless) {
              return H::set_param_uint(idx, uint64_result);
            }
            // If direct conversion to uint64_t is lossy, resort to
            // converting through string.
          }
          // Convert JS value to JS string and get it as UTF-8.
          v8::String::Utf8Value utf8(context->GetIsolate(), result);
          if (!*utf8) {
            // Conversion of JS value to JS string can fail (e.g. throw).
            my_error_js_value_to_string_param();
            return true;
          }
          return H::set_param_string(idx, *utf8, utf8.length());
        };
      }
      break;
    }
    // We try to handle OUT parameters/return values of SET type similarly
    // to how SQL core handles integer/floating-point/string values which
    // stored in SET type.
    //
    // Unsigned 32-bit integer Number values and unsigned 64-bit BigInt
    // values are passed to SQL core as integers. The latter interprets
    // such values as bitmaps representing SET.
    // Other JS Number values (including integers which do not fit into
    // 32 bits and negative integers) are passed to SQL core as floating-
    // point values. SQL core casts them to integers, which again are
    // interpreted as bitmaps for corresponding SETs.
    //
    // JS string values and other JS values are passed to SQL core as strings.
    // The latter are interpreted as a comma-separated lists of SET elements.
    // Additionally if string contains integer value SQL core will try to
    // interpret it as SET's bitmap.
    //
    // Thanks to the above any integer value, be it represented as Number,
    // BigInt or a string will be normally treated as SET's bitmap.
    //
    // Handling of Number and string representation of non-integer number
    // values is not consistent, but it is not the case for SQL core either.
    //
    // Also some confusion is possible if SET uses strings with integers as
    // its elements, but this case is problematic in SQL core as well.
    case MYSQL_SP_ARG_TYPE_SET: {
      return [](v8::Local<v8::Context> context, uint idx,
                v8::Local<v8::Value> result) -> bool {
        if (result->IsUndefined() || result->IsNull()) {
          return H::set_param_null(idx);
        } else if (result->IsUint32()) {
          // By having separate branch for Uint32 we probably save a few
          // cycles for the most common case when number is really is a SMI.
          //
          // Number already, so no real conversion that can fail.
          return H::set_param_uint(idx,
                                   result->Uint32Value(context).ToChecked());
        } else if (result->IsNumber()) {
          // Number already, so no real conversion that can fail.
          double float_result = result->NumberValue(context).ToChecked();
          return H::set_param_float(idx, float_result);
        } else if (result->IsBigInt()) {
          v8::Local<v8::BigInt> bigint_result = result.As<v8::BigInt>();
          bool lossless;
          uint64_t uint64_result = bigint_result->Uint64Value(&lossless);
          if (lossless) {
            return H::set_param_uint(idx, uint64_result);
          }
          // If direct conversion to uint64_t is lossy, resort to converting
          // through string.
        }

        // Convert JS value to JS string and get it as UTF-8.
        v8::String::Utf8Value utf8(context->GetIsolate(), result);
        if (!*utf8) {
          // Conversion of JS value to JS string can fail (e.g. throw).
          my_error_js_value_to_string_param();
          return true;
        }

        // SQL-core code does conversion from UTF8 to the parameter charset.
        return H::set_param_string(idx, *utf8, utf8.length());
      };
    }
    // We handle BIT type by converting JS value to Number and then
    // letting SQL core to convert this value into bitmap. We do not
    // fallback to going through string if conversion to Number fails.
    //
    // The problem is that for BIT type integer and floating-point
    // values are interpreted differently than strings representing
    // the same integer/floating-point values.
    //
    // To avoid confusion we decided not to allow storing string JS
    // values which are not convertible to Number in BIT parameters.
    case MYSQL_SP_ARG_TYPE_BIT: {
      return [](v8::Local<v8::Context> context, uint idx,
                v8::Local<v8::Value> result) -> bool {
        if (result->IsUndefined() || result->IsNull()) {
          return H::set_param_null(idx);
        } else if (result->IsBigInt()) {
          v8::Local<v8::BigInt> bigint_result = result.As<v8::BigInt>();
          bool lossless;
          uint64_t uint64_result = bigint_result->Uint64Value(&lossless);
          if (lossless) {
            return H::set_param_uint(idx, uint64_result);
          } else {
            my_error(ER_LANGUAGE_COMPONENT, MYF(0),
                     "Can't convert BigInt value to BIT type (value "
                     "out of range)");
            return true;
          }
        } else {
          double fp_result;
          if (!result->NumberValue(context).To(&fp_result) ||
              std::isnan(fp_result)) {
            // Conversion to Number might fail by throwing or result in NaN,
            // we bark in either case.
            my_error(ER_LANGUAGE_COMPONENT, MYF(0),
                     "Can't convert JS value to BIT type (possibly non-numeric"
                     " value)");
            return true;
          }
          return H::set_param_float(idx, fp_result);
        }
      };
    }
    // Again, we can handle return values of floating-point SQL types
    // by converting any JS value to string and then letting SQL core
    // parse the string. But we optimize by trying to convert JS Number
    // values to SQL type directly.
    case MYSQL_SP_ARG_TYPE_FLOAT:
    case MYSQL_SP_ARG_TYPE_DOUBLE: {
      return [](v8::Local<v8::Context> context, uint idx,
                v8::Local<v8::Value> result) -> bool {
        if (result->IsUndefined() || result->IsNull()) {
          return H::set_param_null(idx);
        } else if (result->IsNumber()) {
          // Number already, so no real conversion that can fail.
          double float_result = result->NumberValue(context).ToChecked();
          return H::set_param_float(idx, float_result);
        } else {
          // Convert JS value to JS string and get it as UTF-8.
          v8::String::Utf8Value utf8(context->GetIsolate(), result);
          if (!*utf8) {
            // Conversion of JS value to JS string can fail (e.g. throw).
            my_error_js_value_to_string_param();
            return true;
          }
          return H::set_param_string(idx, *utf8, utf8.length());
        }
      };
    }
    // We use JSON.stringify() to get JSON representation of JS value and
    // then try to store the resulting UTF8 string into OUT parameter or
    // return value of JSON SQL type.
    //
    // Both the former and the latter steps can fail.
    case MYSQL_SP_ARG_TYPE_JSON: {
      return [](v8::Local<v8::Context> context, uint idx,
                v8::Local<v8::Value> result) -> bool {
        if (result->IsUndefined() || result->IsNull()) {
          return H::set_param_null(idx);
        } else {
          v8::Local<v8::String> json_str;
          if (!v8::JSON::Stringify(context, result).ToLocal(&json_str)) {
            my_error(ER_LANGUAGE_COMPONENT, MYF(0),
                     "For JSON return type only values supported by "
                     "JSON.stringify() are allowed.");
            return true;
          }

          v8::String::Utf8Value utf8(context->GetIsolate(), json_str);

          if (!*utf8) {
            // Should not happen in theory, but we play safe.
            my_error_js_value_to_string_param();
            return true;
          }

          return H::set_param_string(idx, *utf8, utf8.length());
        }
      };
    }
    // Since MySQL GEOMETRY type accepts only binary data in certain
    // format we only allow ArrayBuffer-based values for it.
    case MYSQL_SP_ARG_TYPE_GEOMETRY: {
      return [](v8::Local<v8::Context>, uint idx,
                v8::Local<v8::Value> result) -> bool {
        if (result->IsUndefined() || result->IsNull()) {
          return H::set_param_null(idx);
        } else if (result->IsArrayBufferView()) {
          v8::Local<v8::ArrayBufferView> buff_view_result =
              result.As<v8::ArrayBufferView>();
          v8::Local<v8::ArrayBuffer> arr_buff = buff_view_result->Buffer();
          const char *ptr = static_cast<const char *>(arr_buff->Data()) +
                            buff_view_result->ByteOffset();
          return H::set_param_string(idx, ptr, buff_view_result->ByteLength());
        } else if (result->IsArrayBuffer()) {
          // We also support direct passing of ArrayBuffer objects.
          v8::Local<v8::ArrayBuffer> arr_buff = result.As<v8::ArrayBuffer>();
          const char *ptr = static_cast<const char *>(arr_buff->Data());
          return H::set_param_string(idx, ptr, arr_buff->ByteLength());
        } else {
          my_error(ER_LANGUAGE_COMPONENT, MYF(0),
                   "Only ArrayBuffer-based values are supported for GEOMETRY "
                   "return type.");
          return true;
        }
      };
    }
    // We handle datetime types by converting JS value to UTF-8 string and
    // then letting SQL core parse the result.
    // Note that we don't want to use generic code that used for string
    // types in this case. Datetime types report "numeric" (i.e. "latin1")
    // as their charset, so using generic code in their case would introduce
    // overhead due to conversion from UTF-8 to latin1.
    // OTOH implementation of datetime types in SQL core handles string input
    // in any ASCII-compatible charset (including UTF-8) directly, without
    // conversion.
    case MYSQL_SP_ARG_TYPE_NEWDATE:
    case MYSQL_SP_ARG_TYPE_TIMESTAMP2:
    case MYSQL_SP_ARG_TYPE_DATETIME2:
    case MYSQL_SP_ARG_TYPE_TIME2: {
      return [](v8::Local<v8::Context> context, uint idx,
                v8::Local<v8::Value> result) -> bool {
        if (result->IsUndefined() || result->IsNull()) {
          return H::set_param_null(idx);
        } else {
          // Convert JS value to JS string and get it as UTF-8.
          v8::String::Utf8Value utf8(context->GetIsolate(), result);
          if (!*utf8) {
            // Conversion of JS value to JS string can fail (e.g. throw).
            my_error_js_value_to_string_param();
            return true;
          }
          // Implementation of datetime types in SQL core accepts input
          // in UTF-8 without requiring conversion.
          return H::set_param_string(idx, *utf8, utf8.length());
        }
      };
    }
    // Obsolete types which shold not be possible to use as return
    // value for newly created routine.
    case MYSQL_SP_ARG_TYPE_DECIMAL:
    case MYSQL_SP_ARG_TYPE_DATE:
    case MYSQL_SP_ARG_TYPE_VAR_STRING:
    case MYSQL_SP_ARG_TYPE_TIME:
    case MYSQL_SP_ARG_TYPE_TIMESTAMP:
    case MYSQL_SP_ARG_TYPE_DATETIME: {
      [[fallthrough]];
    }
    // Special types which should not be possible to use as return
    // value for routine.
    case MYSQL_SP_ARG_TYPE_INVALID:
    case MYSQL_SP_ARG_TYPE_NULL:
    case MYSQL_SP_ARG_TYPE_TYPED_ARRAY: {
      [[fallthrough]];
    }
    // New SQL-types need analysis to be handled correctly.
    default: {
      assert(false);
      [[fallthrough]];
    }
    // We handle DECIMAL SQL-type by converting JS value to string and
    // then letting SQL core to extract fixed point value.
    //
    // Unlike for integer or floating-point values it doesn't make sense
    // to optimize conversion from Number type, as SQL core handles int/
    // double -> DECIMAL conversions through strings.
    case MYSQL_SP_ARG_TYPE_NEWDECIMAL: {
#ifndef NDEBUG
      // It is important to avoid unnecessary work in the generic string
      // handling code below and not to do charset conversions.
      // Code which does parsing of decimal values only cares about latin1
      // digits and spaces, so no conversion from UTF8 should be required.
      const char *cs_name;
      H::get_param_charset(m_sql_sp, param_idx, &cs_name);
      assert(strcmp(cs_name, UTF8_CS_NAME) == 0);
#endif
      [[fallthrough]];
    }
    // We handle ENUMs similarly to strings.
    case MYSQL_SP_ARG_TYPE_ENUM: {
      [[fallthrough]];
    }
    case MYSQL_SP_ARG_TYPE_VARCHAR:
    case MYSQL_SP_ARG_TYPE_STRING:
    case MYSQL_SP_ARG_TYPE_TINY_BLOB:
    case MYSQL_SP_ARG_TYPE_MEDIUM_BLOB:
    case MYSQL_SP_ARG_TYPE_BLOB:
    case MYSQL_SP_ARG_TYPE_LONG_BLOB: {
      const char *cs_name;
      H::get_param_charset(m_sql_sp, param_idx, &cs_name);

      const bool is_binary = (strcmp(cs_name, BINARY_CS_NAME) == 0);

      return [is_binary](v8::Local<v8::Context> context, uint idx,
                         v8::Local<v8::Value> result) -> bool {
        if (result->IsUndefined() || result->IsNull()) {
          return H::set_param_null(idx);
        } else if (is_binary && result->IsArrayBufferView()) {
          v8::Local<v8::ArrayBufferView> buff_view_result =
              result.As<v8::ArrayBufferView>();
          v8::Local<v8::ArrayBuffer> arr_buff = buff_view_result->Buffer();
          const char *ptr = static_cast<const char *>(arr_buff->Data()) +
                            buff_view_result->ByteOffset();
          return H::set_param_string(idx, ptr, buff_view_result->ByteLength());
        } else if (is_binary && result->IsArrayBuffer()) {
          // We also support direct passing of ArrayBuffer objects.
          v8::Local<v8::ArrayBuffer> arr_buff = result.As<v8::ArrayBuffer>();
          const char *ptr = static_cast<const char *>(arr_buff->Data());
          return H::set_param_string(idx, ptr, arr_buff->ByteLength());
        } else {
          // Convert JS value to JS string and get it as UTF-8.
          v8::String::Utf8Value utf8(context->GetIsolate(), result);
          if (!*utf8) {
            // Conversion of JS value to JS string can fail (e.g. throw).
            my_error_js_value_to_string_param();
            return true;
          }

          // SQL-core code does conversion from UTF8 to the parameter charset.
          return H::set_param_string(idx, *utf8, utf8.length());
        }
      };
    }
  }
  // We didn't manage to build setter. Report error.
  return set_param_func_t();
}

bool Js_sp::prepare_set_retval_func() {
  m_set_retval_func = prepare_set_param_func<Return_value_handler>(0);
  return !m_set_retval_func;
}

bool Js_sp::prepare_set_out_param_funcs() {
  for (auto param_idx : m_out_param_indexes) {
    set_param_func_t func =
        prepare_set_param_func<Out_param_handler>(param_idx);
    if (!func) return true;
    m_set_out_param_func.push_back(func);
  }
  return false;
}
