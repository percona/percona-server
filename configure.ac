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

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_LIBTOOL

AC_CHECK_PROGS([BZR], [bzr])

# mysql_config
AC_ARG_WITH([mysql_config],
    AC_HELP_STRING(
        [--with-mysql_config=PROGRAM],
        [location of the mysql_config program]),
    [mysql_config_prog="$withval"])
AC_PATH_PROG([MYSQL_CONFIG], [mysql_config], [$mysql_config_prog])

if test "x$MYSQL_CONFIG" = "x"
then
    AC_MSG_ERROR([Unable to find mysql_config. Please install or specify.])
fi

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpam':
AC_CHECK_LIB([pam], [pam_start])

# Checks for header files.
MYSQL_INCLUDES="$(mysql_config --include)"

save_CFLAGS="${CFLAGS}"
save_CPPFLAGS="${CPPFLAGS}"

CFLAGS="${CFLAGS} ${MYSQL_INCLUDES}"
CPPFLAGS="${CPPFLAGS} ${MYSQL_INCLUDES}"

AC_CHECK_HEADERS([string.h unistd.h])
AC_CHECK_HEADER([mysql/plugin.h], [],
    AC_MSG_ERROR(
       [Unable to find mysql/plugin.h. Please install the mysql
        development headers]))

CFLAGS="${save_CFLAGS}"
CPPFLAGS="${save_CPPFLAGS}"

AC_SUBST([MYSQL_INCLUDES])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getpass strchr strdup])

# Get the plugin dir
PLUGINDIR=$([$MYSQL_CONFIG --plugindir])
AC_SUBST([PLUGINDIR])

# Get the revision if bzr is present
if test "x${BZR}" != "x"
then
    if cd "$(dirname "$0")" && bzr status >& /dev/null
    then
        REVISION="$(cd "$(dirname "$0")"; ${BZR} log -r-1 | grep ^revno: | cut -d ' ' -f 2)"
    else
        REVISION=0
    fi
else
    REVISION=0
fi

AC_SUBST([REVISION])

# Output files
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 build/percona-pam-plugin.spec])
AC_OUTPUT
