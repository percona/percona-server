/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
   Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.
   Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

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

#include <array>

#include "mysql/components/component_implementation.h"
#include "mysql/components/my_service.h"
#include "mysql/components/services/registry.h"
#include "mysql/components/services/udf_registration.h"

#include "components/masking_functions/include/component.h"
#include "components/masking_functions/include/udf/udf_gen_blocklist.h"
#include "components/masking_functions/include/udf/udf_gen_dictionary.h"
#include "components/masking_functions/include/udf/udf_gen_range.h"
#include "components/masking_functions/include/udf/udf_gen_rnd_email.h"
#include "components/masking_functions/include/udf/udf_gen_rnd_iban.h"
#include "components/masking_functions/include/udf/udf_gen_rnd_pan.h"
#include "components/masking_functions/include/udf/udf_gen_rnd_ssn.h"
#include "components/masking_functions/include/udf/udf_gen_rnd_uk_nin.h"
#include "components/masking_functions/include/udf/udf_gen_rnd_us_phone.h"
#include "components/masking_functions/include/udf/udf_mask_canada_sin.h"
#include "components/masking_functions/include/udf/udf_mask_iban.h"
#include "components/masking_functions/include/udf/udf_mask_inner.h"
#include "components/masking_functions/include/udf/udf_mask_outer.h"
#include "components/masking_functions/include/udf/udf_mask_pan.h"
#include "components/masking_functions/include/udf/udf_mask_pan_relaxed.h"
#include "components/masking_functions/include/udf/udf_mask_ssn.h"
#include "components/masking_functions/include/udf/udf_mask_uk_nin.h"
#include "components/masking_functions/include/udf/udf_mask_uuid.h"
#include "components/masking_functions/include/udf/udf_masking_dictionary_remove.h"
#include "components/masking_functions/include/udf/udf_masking_dictionary_term_add.h"
#include "components/masking_functions/include/udf/udf_masking_dictionary_term_remove.h"
#include "components/masking_functions/include/udf/udf_registration.h"

extern REQUIRES_SERVICE_PLACEHOLDER(udf_registration);

/* The UDFs we will register. */
static std::array<udf_descriptor, 21> udfs = {{
    udf_gen_blocklist(),
    udf_gen_dictionary(),
    udf_gen_range(),
    udf_gen_rnd_email(),
    udf_gen_rnd_pan(),
    udf_gen_rnd_ssn(),
    udf_gen_rnd_uk_nin(),
    udf_gen_rnd_iban(),
    udf_gen_rnd_us_phone(),
    udf_mask_inner(),
    udf_mask_outer(),
    udf_mask_pan(),
    udf_mask_uuid(),
    udf_mask_iban(),
    udf_mask_uk_nin(),
    udf_mask_pan_relaxed(),
    udf_mask_ssn(),
    udf_mask_canada_sin(),
    udf_masking_dictionary_term_add(),
    udf_masking_dictionary_term_remove(),
    udf_masking_dictionary_remove(),
}};

bool register_udfs() {
  bool error = false;

  for (udf_descriptor const &udf : udfs) {
    error = mysql_service_udf_registration->udf_register(
        udf.name, udf.result_type, udf.main_function, udf.init_function,
        udf.deinit_function);
    if (error) {
      /* purecov: begin inspected */
      sql_print_error("DataMasking Plugin: ERROR registering udf ", udf.name);
      break;
      /* purecov: end */
    }
  }

  if (error) {
    /* purecov: begin inspected */
    int was_present;
    for (udf_descriptor const &udf : udfs) {
      // Don't care about errors since we are already erroring out.
      mysql_service_udf_registration->udf_unregister(udf.name, &was_present);
    }
    /* purecov: end */
  }

  return error;
}

bool unregister_udfs() {
  bool error = false;

  int was_present;
  for (udf_descriptor const &udf : udfs) {
    // Don't care about the functions not being there.
    error = error || mysql_service_udf_registration->udf_unregister(
                         udf.name, &was_present);
  }

  if (error) {
    sql_print_error("DataMasking Plugin: ERROR unregistering plugins");
  }

  return error;
}
