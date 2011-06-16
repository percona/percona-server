#ifndef AUTH_PAM_GROUPS_INCLUDED
#define AUTH_PAM_GROUPS_INCLUDED
/*
  (C) 2013 Percona LLC and/or its affiliates

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

 PAM authentication for MySQL, interface for groups enumeration.

*/

#ifdef __cplusplus
extern "C" {
#endif

struct groups_iter;

/** Create iterator through user groups.
    Initially iterator set to position before first
    group. On success non-NULL pointer returned, otherwise NULL */
struct groups_iter *groups_iter_new(const char *user_name);

/** Move iterator to next group.
    On success group name is returned,
    otherwise NULL */
const char *groups_iter_next(struct groups_iter *it);

/** Make iterator to point to beginning again */
void groups_iter_reset(struct groups_iter *it);

/** Finish iteration and release iterator */
void groups_iter_free(struct groups_iter *it);

#ifdef __cplusplus
}
#endif

#endif
