/* Copyright (c) 2025 Oracle and/or its affiliates.

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

#pragma once

#include <cstdint>

using ulonglong = unsigned long long;

/**
  Initialize the my_physical_memory function using server_memory option
  @param[in]  memory  Value of the server_memory startup option

  @note The input value of 0 indicates no limits, and underlying container/host
  configuration must be used
  @return true on success, false if input memory value is invalid
*/
bool init_my_physical_memory(ulonglong memory);

/**
  Determine the total physical memory available in bytes.

  If process is running within a container, then memory available is the maximum
  limit set for the container. If the process is not running in a container then
  it uses the appropriate system APIs to determine the available memory.

  If the API is unable to determine the available memory, then it returns 0.

   @return physical memory in bytes or 0
*/
uint64_t my_physical_memory() noexcept;

/**
  Determine the total number of logical CPUs available.

  If process is running within a container, the number of logical CPUs is the
  maximum limit set for the container. If the process is not running in a
  container then it uses the appropriate system APIs to determine the number of
  logical CPUs.

  If the API is unable to determine the number of logical CPUs, then it returns
  0.

  @note: The container set limits are calculated from the CFS quota and period
  as quota/period and is round down. A limit of 0.5 will return the value 0 and
  is treated as though no limits are set.

  @return number of logical CPUs or 0
*/
uint32_t my_num_vcpus() noexcept;
