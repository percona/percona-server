#ifndef _UTILS_H
/* Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#define _UTILS_H

#include <my_inttypes.h>
#include <mysql/components/services/registry.h>
#include <mysql/components/services/udf_metadata.h>
#include <mysql/udf_registration_types.h>
#include <random>
#include <string>

namespace mysql {
namespace plugins {
std::string random_string(unsigned long length, bool letter_start);

std::string random_number(const unsigned int length);

long random_number(const long min, const long max);

std::string random_credit_card();

std::string random_ssn();

std::string random_us_phone();

/**
 @class Charset_service

 Class that acquire/release the udf_metadata_service from registry service.
 It provides the APIs to set the character set of return value and arguments
 of UDFs using the udf_metadata service.
*/
class Charset_service {
 public:
  /**
    Acquires the udf_metadata_service from the registry  service.
    @param[in]  reg_srv Registry service from which udf_metadata service
                        will be acquired

    @retval true if service could not be acquired
    @retval false Otherwise
  */
  static bool init(SERVICE_TYPE(registry) * reg_srv);

  /**
    Release the udf_metadata service

    @param[in]  reg_srv Registry service from which the udf_metadata
                        service will be released.

    @retval true if service could not be released
    @retval false Otherwise
  */
  static bool deinit(SERVICE_TYPE(registry) * reg_srv);

  /**
    Set the specified character set of UDF return value

    @param[in] initid  UDF_INIT structure
    @param[in] charset_name Character set that has to be set.
               The default charset is set to 'latin1'

    @retval true Could not set the character set of return value
    @retval false Otherwise
  */
  static bool set_return_value_charset(
      UDF_INIT *initid, const std::string &charset_name = "latin1");
  /**
    Set the specified character set of all UDF arguments

    @param[in] args UDF_ARGS structure
    @param[in] charset_name Character set that has to be set.
               The default charset is set to 'latin1'

    @retval true Could not set the character set of any of the argument
    @retval false Otherwise
  */
  static bool set_args_charset(UDF_ARGS *args,
                               const std::string &charset_name = "latin1");

 private:
  /* Argument type to specify in the metadata service methods */
  static const char *arg_type;
  /* udf_metadata service name */
  static const char *service_name;
  /* Handle of udf_metadata_service */
  static SERVICE_TYPE(mysql_udf_metadata) * udf_metadata_service;
};
}  // namespace plugins
}  // namespace mysql

#endif  //_UTILS_H
