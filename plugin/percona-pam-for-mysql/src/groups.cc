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

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "auth_pam_common.h"

#include "my_sys.h"

static int gr_buf_size = 0;

#ifdef __APPLE__
using my_gid_t = int;
#else
using my_gid_t = gid_t;
#endif

/** Groups iterator. It's not exposed outsude */
struct groups_iter {
  char *buf;
  my_gid_t *groups;
  int current_group;
  int ngroups;
  int buf_size;
};

/** Create iterator through user groups.
    Initially iterator set to position before first
    group. On success non-NULL pointer returned, otherwise NULL */
struct groups_iter *groups_iter_new(const char *user_name) {
  if (gr_buf_size <= 0) {
    long gr_size_max, pw_size_max;
    gr_size_max = sysconf(_SC_GETGR_R_SIZE_MAX);
    pw_size_max = sysconf(_SC_GETPW_R_SIZE_MAX);
    gr_buf_size = gr_size_max > pw_size_max ? gr_size_max : pw_size_max;
  }

  struct groups_iter *const it = (struct groups_iter *)my_malloc(
      key_memory_pam_group_iter, sizeof(struct groups_iter),
      MYF(MY_FAE | MY_ZEROFILL));

  it->buf_size = gr_buf_size;
  if (it->buf_size <= 0) it->buf_size = 1024;

  it->buf =
      (char *)my_malloc(key_memory_pam_group_iter, it->buf_size, MYF(MY_FAE));

  struct passwd pwd, *pwd_result;
  int error;
  while ((error = getpwnam_r(user_name, &pwd, it->buf, it->buf_size,
                             &pwd_result)) == ERANGE) {
    it->buf_size = it->buf_size * 2;
    it->buf = (char *)my_realloc(key_memory_pam_group_iter, it->buf,
                                 it->buf_size, MYF(MY_FAE));
  }
  if (error != 0 || pwd_result == nullptr) {
    my_plugin_log_message(&auth_pam_plugin_info, MY_ERROR_LEVEL,
                          "Unable to obtain the passwd entry for the user "
                          "'%s'.",
                          user_name);
    my_free(it->buf);
    my_free(it);
    return nullptr;
  }

  gr_buf_size = it->buf_size;

  it->ngroups = 1024;
  it->groups = static_cast<my_gid_t *>(my_malloc(
      key_memory_pam_group_iter, it->ngroups * sizeof(gid_t), MYF(MY_FAE)));
  error = getgrouplist(user_name, pwd_result->pw_gid, it->groups, &it->ngroups);
  if (error == -1) {
    it->groups = static_cast<my_gid_t *>(
        my_realloc(key_memory_pam_group_iter, it->groups,
                   it->ngroups * sizeof(gid_t), MYF(MY_FAE)));
    error =
        getgrouplist(user_name, pwd_result->pw_gid, it->groups, &it->ngroups);
    if (error == -1) {
      my_plugin_log_message(&auth_pam_plugin_info, MY_ERROR_LEVEL,
                            "Unable to obtain the group access list for "
                            "the user '%s'.",
                            user_name);
      my_free(it->buf);
      my_free(it->groups);
      my_free(it);
      return nullptr;
    }
  }

  return it;
}

/** Move iterator to next group.
    On success group name is returned,
    otherwise NULL */
const char *groups_iter_next(struct groups_iter *it) {
  if (it->current_group >= it->ngroups) return nullptr;

  int error;
  struct group grp, *grp_result;
  while ((error = getgrgid_r(it->groups[it->current_group], &grp, it->buf,
                             it->buf_size, &grp_result)) == ERANGE) {
    it->buf_size = it->buf_size * 2;
    it->buf = (char *)my_realloc(key_memory_pam_group_iter, it->buf,
                                 it->buf_size, MYF(MY_FAE));
  }
  if (error != 0 || grp_result == nullptr) {
    my_plugin_log_message(&auth_pam_plugin_info, MY_ERROR_LEVEL,
                          "Unable to obtain the group record for the group "
                          "id %d.",
                          (int)it->groups[it->current_group]);
    return nullptr;
  }
  ++it->current_group;

  return grp_result->gr_name;
}

/** Make iterator to point to the beginning again */
void groups_iter_reset(struct groups_iter *it) { it->current_group = 0; }

/** Finish iteration and release iterator */
void groups_iter_free(struct groups_iter *it) {
  my_free(it->buf);
  my_free(it->groups);
  my_free(it);
}
