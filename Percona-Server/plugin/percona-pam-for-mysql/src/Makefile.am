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

plugindir = @PLUGINDIR@

plugin_LTLIBRARIES = auth_pam.la auth_pam_compat.la test_auth_pam_client.la dialog.la
plugin_CPPFLAGS = -DMYSQL_DYNAMIC_PLUGIN
plugin_LDFLAGS = -module -avoid-version -shared

auth_pam_la_SOURCES = auth_pam_common.c auth_pam.c lib_auth_pam_client.h \
                      lib_auth_pam_client.c auth_mapping.c groups.c
auth_pam_la_CPPFLAGS = $(plugin_CPPFLAGS)
auth_pam_la_LDFLAGS = $(plugin_LDFLAGS) $(AUTH_PAM_LIBS)

auth_pam_compat_la_SOURCES = auth_pam_common.c auth_pam_compat.c \
                             lib_auth_pam_client.h lib_auth_pam_client.c \
                             auth_mapping.c groups.c
auth_pam_compat_la_CPPFLAGS = $(plugin_CPPFLAGS)
auth_pam_compat_la_LDFLAGS = $(plugin_LDFLAGS) $(AUTH_PAM_LIBS)

test_auth_pam_client_la_SOURCES = test_auth_pam_client.c \
	lib_auth_pam_client.h lib_auth_pam_client.c
test_auth_pam_client_la_LDFLAGS = $(plugin_LDFLAGS)

dialog_la_SOURCES = dialog.c
dialog_la_CPPFLAGS = $(plugin_CPPFLAGS)
dialog_la_LDFLAGS = $(plugin_LDFLAGS) $(DIALOG_LIBS)
