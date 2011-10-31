#                                               -*- Autoconf -*-
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
#
AC_PREREQ([2.61])
AC_INIT([Percona PAM plugin], [0.1], [mysql-dev@percona.com])
AC_CONFIG_SRCDIR([src/lib_auth_pam_client.h])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_SED
AC_CHECK_PROGS([BZR], [bzr])

AC_ARG_WITH([mysql_config],
    AC_HELP_STRING(
        [--with-mysql_config=PATH],
        [location of the mysql_config program]),
    [MYSQL_CONFIG="$with_mysql_config"])
AC_PATH_PROG([MYSQL_CONFIG], [mysql_config])

AS_IF([test "x${MYSQL_CONFIG}" = x],
      [AC_MSG_ERROR([Unable to find mysql_config. Please install or specify.])])

# Checks for libraries.
AC_CHECK_LIB([pam], [pam_start], ,
    AC_MSG_ERROR(
        [Unable to find PAM. Please install the PAM development libraries])
)

# Replace -I with -isystem MySQL header include GCC options to silence warnings
# originating from them
MYSQL_INCLUDES=`"${MYSQL_CONFIG}" --include | "${SED}" 's@^-I@-isystem @'`

CPPFLAGS="${CPPFLAGS} ${MYSQL_INCLUDES}"

AC_CHECK_HEADERS([mysql/plugin.h mysql/plugin_auth.h], [],
    AC_MSG_ERROR(
       [Unable to find MySQL development headers. Please install them]))

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getpass])

# Get the plugin dir
PLUGINDIR=$([$MYSQL_CONFIG --plugindir])
AC_SUBST([PLUGINDIR])

# Get the revision if bzr is present
AS_IF([test "x${BZR}" != x],
      [REVISION=`${BZR} revno "$srcdir"`])
REVISION=${REVISION:-0}

AC_SUBST([REVISION])

# Output files
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 build/build-binary.sh
                 build/percona-pam-plugin.spec])
AC_OUTPUT
