/*
(C) 2013, 2016 Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <my_sys.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int gr_buf_size= 0;

/** Groups iterator. It's not exposed outsude */
struct groups_iter {
  char *buf;
  gid_t *groups;
  int current_group;
  int ngroups;
  int buf_size;
};

/** Create iterator through user groups.
    Initially iterator set to position before first
    group. On success non-NULL pointer returned, otherwise NULL */
struct groups_iter *groups_iter_new(const char *user_name)
{
  struct passwd pwd, *pwd_result;
  int error;
  struct groups_iter *it;

  if (gr_buf_size <= 0)
  {
    long gr_size_max, pw_size_max;
    gr_size_max= sysconf(_SC_GETGR_R_SIZE_MAX);
    pw_size_max= sysconf(_SC_GETPW_R_SIZE_MAX);
    gr_buf_size= gr_size_max > pw_size_max ? gr_size_max : pw_size_max;
  }

  it= (struct groups_iter *) my_malloc(sizeof(struct groups_iter),
                                       MYF(MY_FAE | MY_ZEROFILL));

  it->buf_size= gr_buf_size;
  if (it->buf_size <= 0)
    it->buf_size= 1024;

  it->buf= (char *) my_malloc(it->buf_size, MYF(MY_FAE));

  while ((error= getpwnam_r(user_name, &pwd, it->buf, it->buf_size,
                            &pwd_result)) == ERANGE)
  {
    it->buf_size= it->buf_size * 2;
    it->buf= (char *) my_realloc(it->buf, it->buf_size, MYF(MY_FAE));
  }
  if (error != 0 || pwd_result == NULL)
  {
    fprintf(stderr, "auth_pam: Unable to obtain the passwd entry for the user "
                    "'%s'.", user_name);
    my_free(it->buf);
    my_free(it);
    return NULL;
  }

  gr_buf_size= it->buf_size;

  it->ngroups= 1024;
  it->groups= (gid_t *) my_malloc(it->ngroups * sizeof(gid_t), MYF(MY_FAE));
  error= getgrouplist(user_name, pwd_result->pw_gid, it->groups, &it->ngroups);
  if (error == -1)
  {
    it->groups= (gid_t *) my_realloc(it->groups, it->ngroups * sizeof(gid_t),
                                     MYF(MY_FAE));
    error= getgrouplist(user_name, pwd_result->pw_gid, it->groups,
                        &it->ngroups);
    if (error == -1)
    {
      fprintf(stderr, "auth_pam: Unable to obtain the group access list for "
                      "the user '%s'.", user_name);
      my_free(it->buf);
      my_free(it->groups);
      my_free(it);
      return NULL;
    }
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

  while ((error= getgrgid_r(it->groups[it->current_group], &grp, it->buf,
                            it->buf_size, &grp_result)) == ERANGE)
  {
    it->buf_size= it->buf_size * 2;
    it->buf= (char *) my_realloc(it->buf, it->buf_size, MYF(MY_FAE));
  }
  if (error != 0 || grp_result == NULL)
  {
    fprintf(stderr, "auth_pam: Unable to obtain the group record for the group "
                    "id %d.", (int) it->groups[it->current_group]);
    return NULL;
  }
  ++it->current_group;

  return grp_result->gr_name;
}

/** Make iterator to point to the beginning again */
void groups_iter_reset(struct groups_iter *it)
{
  it->current_group= 0;
}

/** Finish iteration and release iterator */
void groups_iter_free(struct groups_iter *it)
{
  my_free(it->buf);
  my_free(it->groups);
  my_free(it);
}

