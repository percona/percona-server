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

#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

enum { max_nss_name_len = 10240 };
enum { max_number_of_groups = 1024 };

/** Groups iterator. It's not exposed outsude */
struct groups_iter {
  char buf[max_nss_name_len];
  gid_t groups[max_number_of_groups];
  int current_group;
  int ngroups;
};

/** Create iterator through user groups.
    Initially iterator set to position before first
    group. On success non-NULL pointer returned, otherwise NULL */
struct groups_iter *groups_iter_new(const char *user_name)
{
  struct passwd pwd, *pwd_result;
  int error;
  struct groups_iter *it;

  it= calloc(1, sizeof(struct groups_iter));
  if (it == NULL)
    return NULL;

  error= getpwnam_r(user_name, &pwd, it->buf, max_nss_name_len, &pwd_result);
  if (error != 0 || pwd_result == NULL)
  {
    free(it);
    return NULL;
  }

  it->ngroups= max_number_of_groups;
  error= getgrouplist(user_name, pwd_result->pw_gid, it->groups, &it->ngroups);
  if (error == -1)
  {
    free(it);
    return NULL;
  }

  return it;
}

/** Move iterator to next group.
    On success group name is returned,
    otherwise NULL */
const char *groups_iter_next(struct groups_iter *it)
{
  struct group grp, *grp_result;
  int error;

  if (it->current_group >= it->ngroups)
    return NULL;

  error= getgrgid_r(it->groups[it->current_group++],
                    &grp, it->buf, max_nss_name_len, &grp_result);
  if (error != 0 || grp_result == NULL)
  {
    return NULL;
  }

  return grp_result->gr_name;
}

/** Make iterator to point to beginning again */
void groups_iter_reset(struct groups_iter *it)
{
  it->current_group= 0;
}

/** Finish iteration and release iterator */
void groups_iter_free(struct groups_iter *it)
{
  free(it);
}

