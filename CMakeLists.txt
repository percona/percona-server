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
IF(HAVE_PAM)
  SET(AUTH_PAM_SOURCES 
    src/auth_pam.c src/lib_auth_pam_client.c src/lib_auth_pam_client.h)
  MYSQL_ADD_PLUGIN(auth_pam ${AUTH_PAM_SOURCES} LINK_LIBRARIES pam)
  MYSQL_ADD_PLUGIN(dialog src/dialog.c)
ENDIF(HAVE_PAM)
