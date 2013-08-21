# (C) 2011-2013 Percona LLC and/or its affiliates
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
IF(WITH_PAM)
INCLUDE (CheckLibraryExists)
CHECK_LIBRARY_EXISTS(pam pam_authenticate "" HAVE_PAM) 
IF(NOT HAVE_PAM)
	MESSAGE(FATAL_ERROR "Required PAM dev library not found. Please install PAM development files!")
ENDIF(NOT HAVE_PAM)
CHECK_SYMBOL_EXISTS(getpwnam_r "pwd.h" HAVE_GETPWNAM_R)
CHECK_SYMBOL_EXISTS(getgrgid_r "grp.h" HAVE_GETGRGID_R)
CHECK_INCLUDE_FILES (security/pam_misc.h HAVE_SECURITY_PAM_MISC_H)
CHECK_INCLUDE_FILES (security/openpam.h HAVE_SECURITY_OPENPAM_H)
IF(HAVE_PAM AND HAVE_GETPWNAM_R AND HAVE_GETGRGID_R)
  SET(AUTH_PAM_COMMON_SOURCES 
    src/auth_pam_common.c src/lib_auth_pam_client.c src/lib_auth_pam_client.h
    src/auth_mapping.h src/auth_mapping.c src/groups.c src/groups.h)
  SET(AUTH_PAM_SOURCES ${AUTH_PAM_COMMON_SOURCES} src/auth_pam.c)
  SET(AUTH_PAM_COMPAT_SOURCES ${AUTH_PAM_COMMON_SOURCES} src/auth_pam_compat.c)
  MYSQL_ADD_PLUGIN(auth_pam ${AUTH_PAM_SOURCES} LINK_LIBRARIES pam MODULE_ONLY)
  MYSQL_ADD_PLUGIN(auth_pam_compat ${AUTH_PAM_COMPAT_SOURCES} LINK_LIBRARIES pam MODULE_ONLY)
  MYSQL_ADD_PLUGIN(dialog src/dialog.c LINK_LIBRARIES mysqlclient MODULE_ONLY)
ENDIF(HAVE_PAM AND HAVE_GETPWNAM_R AND HAVE_GETGRGID_R)
ENDIF(WITH_PAM)
