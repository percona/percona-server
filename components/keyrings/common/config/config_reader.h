/* Copyright (c) 2021, 2024, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef CONFIG_READER_INCLUDED
#define CONFIG_READER_INCLUDED

#include <string>

#include "my_rapidjson_size_t.h"

#include <rapidjson/document.h>

namespace keyring_common {
namespace config {

class Config_reader {
 public:
  /**
    Constructor

    Reads JSON from config file and stores it in memory.

    @param [in] config_file_path Full path to configuration file
  */
  inline explicit Config_reader(const std::string config_file_path);

  /**
    Get an element value from JSON document.
    Assumption: Type is compatible with Get() function and
                type of element is matching with template argument.

    @param [in]  element_name  Name of the element being searched
    @param [out] element_value Value of the element

    @returns status of search operation
      @retval false Element found. Refer to element_value
      @retval true  Element missing.
  */
  template <typename T>
  bool get_element(const std::string element_name, T &element_value) {
    if (!valid_ || !data_.HasMember(element_name)) return true;
    element_value = data_[element_name].Get<T>();
    return false;
  }

  /**
    Check if an element with the provided name exists.

    @param [in]  element_name  Name of the element being checked

    @returns status of operation
      @retval false Element found.
      @retval true  Element is not found.
  */

  bool has_element(const std::string &element_name);

  /**
    Check if an element value is of numeric type.

    @param [in]  element_name  Name of the element being checked

    @returns status of type check operation
      @retval false Element found and it is of numeric type.
      @retval true  Element type is not a string or element is not found.
  */
  bool is_number(const std::string &element_name);

  /**
    Check if an element value is of string type.

    @param [in]  element_name  Name of the element being checked

    @returns status of type check operation
      @retval false Element found and it is of string type.
      @retval true  Element type is not a string or element is not found.
  */
  bool is_string(const std::string &element_name);

 private:
  /** Configuration file path */
  std::string config_file_path_;
  /** Configuration data in JSON */
  rapidjson::Document data_;
  /** Validity of configuration data */
  bool valid_;
};

}  // namespace config
}  // namespace keyring_common

#endif  // !CONFIG_READER_INCLUDED

#include "config_reader.cc"
