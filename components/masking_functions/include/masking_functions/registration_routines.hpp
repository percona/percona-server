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

#ifndef MASKING_FUNCTIONS_REGISTRATION_ROUTINES_HPP
#define MASKING_FUNCTIONS_REGISTRATION_ROUTINES_HPP

namespace masking_functions {

// This function tries to register all the dynamic privileges
// needed by the 'component_masking_functions' (currently only
// 'MASKING_DICTIONARIES_ADMIN').
// Returns 'true' on success, 'false' otherwise.
// The function can be called several times until it sucseeds.
bool register_dynamic_privileges();
// This function tries to unregister all the dynamic privileges
// needed by the 'component_masking_functions' (currently only
// 'MASKING_DICTIONARIES_ADMIN').
// Returns 'true' on success, 'false' otherwise.
// The function can be called several times until it sucseeds.
bool unregister_dynamic_privileges();

// This function tries to register all UDFs available in the
// 'component_masking_functions'.
// It keeps an internal bitmask of successfully registered UDFs.
// Returns 'true', if all the bits in this internal bitmask are set.
// If some of the bits of this internal bitmask are still not set,
// the 'false' is returned. In this case, the function can be called again
// and it will try to register the remaining UDFs.
bool register_udfs();
// This function tries to unregister all UDFs available in the
// 'component_masking_functions'.
// It has access to the same internal bitmask of successfully registered
// UDFs as 'register_udfs()'.
// Returns 'true', if all the bits in this internal bitmask are not set.
// If some of the bits of this internal bitmask are still set,
// the 'false' is returned. In this case, the function can be called again
// and it will try to unregister the remaining UDFs.
bool unregister_udfs();

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_UDF_REGISTRATION_ROUTINES_HPP
