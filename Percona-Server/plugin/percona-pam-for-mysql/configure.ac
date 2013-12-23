#                                               -*- Autoconf -*-
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
#
AC_PREREQ([2.59])
AC_INIT([Percona PAM plugin], [0.1], [mysql-dev@percona.com])
AC_CONFIG_SRCDIR([src/lib_auth_pam_client.h])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_CHECK_PROGS([BZR], [bzr])

# Remove AC_PROG_LIBTOOL and enable this once CentOS 5.6 support is dropped.
# LT_INIT([disable-static])
AC_PROG_LIBTOOL

AC_ARG_WITH([mysql_config],
    AS_HELP_STRING(
        [--with-mysql_config=PATH],
        [location of the mysql_config program]),
    [MYSQL_CONFIG="$with_mysql_config"])
AC_PATH_PROG([MYSQL_CONFIG], [mysql_config])

AS_IF([test "x${MYSQL_CONFIG}" = x],
      [AC_MSG_ERROR([Unable to find mysql_config. Please install or specify.])])

# Checks for libraries.
AC_CHECK_LIB([pam], [pam_start], [AUTH_PAM_LIBS="$AUTH_PAM_LIBS -lpam"],
    AC_MSG_ERROR(
        [Unable to find PAM. Please install the PAM development libraries])
)
DIALOG_LIBS="$DIALOG_LIBS "`${MYSQL_CONFIG} --libs_r`

# Replace -I with -isystem for the MySQL header include GCC option to silence 
# warnings originating from them
MYSQL_INCLUDES=`"${MYSQL_CONFIG}" --include | sed 's@^-I@-isystem @'`

CPPFLAGS="${CPPFLAGS} ${MYSQL_INCLUDES}"

AC_CHECK_HEADERS([mysql/plugin.h mysql/plugin_auth.h], [],
    AC_MSG_ERROR(
       [Unable to find MySQL development headers. Please install them]))
AC_CHECK_HEADERS([pwd.h grp.h], [],
    AC_MSG_ERROR(
       [Unable to find pwd.h/grp.h headers. Your libc version is not supported]))

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getpass])
AC_CHECK_FUNCS([getpwnam_r getgrgid_r], [],
    AC_MSG_ERROR(
       [Unable to find getpwnam_r/getgrgid_r functions. Your libc version is not supported]))

CFLAGS="-Wall -Wextra ${CFLAGS}"

# Get the plugin dir
PLUGINDIR=$([$MYSQL_CONFIG --plugindir])
AC_SUBST([PLUGINDIR])

# Get the revision if bzr is present
AS_IF([test "x${BZR}" != x],
      [REVISION=`${BZR} revno "$srcdir"`])
REVISION=${REVISION:-0}

AC_SUBST([REVISION])

AC_SUBST([AUTH_PAM_LIBS])
AC_SUBST([DIALOG_LIBS])

# Output files
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 build/build-binary.sh
                 build/percona-pam-plugin.spec])
AC_OUTPUT
