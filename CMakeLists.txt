# (C) 2011 Percona Inc.
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
INCLUDE (CheckLibraryExists)
CHECK_LIBRARY_EXISTS(pam pam_authenticate "" HAVE_PAM) 
CHECK_LIBRARY_EXISTS(mysqlclient mysql_init "" HAVE_MYSQLCLIENT) 
CHECK_SYMBOL_EXISTS(getpwnam_r "pwd.h" HAVE_GETPWNAM_R)
CHECK_SYMBOL_EXISTS(getgrgid_r "grp.h" HAVE_GETGRGID_R)
IF(HAVE_PAM AND HAVE_MYSQLCLIENT AND HAVE_GETPWNAM_R AND HAVE_GETGRGID_R)
  SET(AUTH_PAM_SOURCES 
    src/auth_pam.c src/lib_auth_pam_client.c src/lib_auth_pam_client.h
    src/auth_mapping.h src/auth_mapping.c)
  MYSQL_ADD_PLUGIN(auth_pam ${AUTH_PAM_SOURCES} LINK_LIBRARIES pam)
  MYSQL_ADD_PLUGIN(dialog src/dialog.c LINK_LIBRARIES mysqlclient)
ENDIF(HAVE_PAM AND HAVE_MYSQLCLIENT AND HAVE_GETPWNAM_R AND HAVE_GETGRGID_R)
