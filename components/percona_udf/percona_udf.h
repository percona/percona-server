/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef PERCONA_UDF_H
#define PERCONA_UDF_H

#include "my_inttypes.h"  // ulonglong

#define VISIBILITY_DEFAULT __attribute__((visibility("default")))

/* Prototypes */

extern "C" {
bool fnv_64_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void fnv_64_deinit(UDF_INIT *initid);
ulonglong fnv_64(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

bool fnv1a_64_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void fnv1a_64_deinit(UDF_INIT *initid);
ulonglong fnv1a_64(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
                   char *error);

bool murmur_hash_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void murmur_hash_deinit(UDF_INIT *initid);
ulonglong murmur_hash(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
                      char *error);
}

#endif /* PERCONA_UDF_H */
