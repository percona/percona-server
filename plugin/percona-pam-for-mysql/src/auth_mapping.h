#ifndef AUTH_MAPPING_INCLUDED
#define AUTH_MAPPING_INCLUDED
/*
  (C) 2012, 2013 Percona LLC and/or its affiliates

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/**
 @file

 PAM authentication for MySQL, interface for user mapping.

*/

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Mapping iterator. It's not exposed outsude */
struct mapping_iter;

/** Create iterator through mapping string.
    Initially iterator set to position before first
    key-value pair. On success non-NULL pointer returned, otherwise NULL */
struct mapping_iter *mapping_iter_new(const char *mapping_string);

/** Move iterator to next key-value pair.
    On success pointer to key position in string returned,
    otherwise NULL */
const char *mapping_iter_next(struct mapping_iter *it);

/** Finish iteration and release iterator */
void mapping_iter_free(struct mapping_iter *it);

/** Get key at current iterator position. On success buf returned,
    otherwise NULL */
char *mapping_iter_get_key(struct mapping_iter *it, char *buf, size_t buf_len);

/** Get value at current iterator position. On success buf returned,
    otherwise NULL */
char *mapping_iter_get_value(struct mapping_iter *it, char *buf, size_t buf_len);

/** Get value by given key. On success value_buf returned,
    otherwise NULL */
char *mapping_lookup_user(const char *key, char *value_buf, size_t value_buf_len,
                        const char *mapping_string);

/** Get service name for auth_string. On success buf returned,
    otherwise NULL */
char *mapping_get_service_name(char *buf, size_t buf_len,
                               const char *mapping_string);

#ifdef __cplusplus
}
#endif

#endif
