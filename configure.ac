#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ([2.63b])
AC_INIT([handlersocket-plugin], [1.0.6], [Yoshinori.Matsunobu@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([libhsclient/fatal.cpp])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL

AC_DEFUN([CONFIG_OPTION_MYSQL],[
  AC_MSG_CHECKING([mysql source])

  MYSQL_SOURCE_VERSION=
  MYSQL_INC=
  ac_mysql_source_dir=
  AC_ARG_WITH([mysql-source],
    [AS_HELP_STRING([--with-mysql-source=PATH], [MySQL source directory PATH])],
    [
      ac_mysql_source_dir=`cd $withval && pwd`
      if test -f "$ac_mysql_source_dir/sql/handler.h" ; then
        MYSQL_INC="-I$ac_mysql_source_dir/sql"
        MYSQL_INC="$MYSQL_INC -I$ac_mysql_source_dir/include"
        MYSQL_INC="$MYSQL_INC -I$ac_mysql_source_dir/regex"
        MYSQL_INC="$MYSQL_INC -I$ac_mysql_source_dir"
        AC_SUBST(MYSQL_INC)
        MYSQL_SOURCE_VERSION=`cat $ac_mysql_source_dir/configure.in | grep "\[[MySQL Server\]]" | sed -e "s|.*\([[0-9]]\+\.[[0-9]]\+\.[[0-9]]\+[[0-9a-zA-Z\_\-]]*\).*|\1|"`
        AC_MSG_RESULT([yes: Using $ac_mysql_source_dir, version $MYSQL_SOURCE_VERSION])
      else
        AC_MSG_ERROR([invalid MySQL source directory: $ac_mysql_source_dir])
      fi
    ],
    [AC_MSG_ERROR([--with-mysql-source=PATH is required for standalone build])]
  )

  AC_MSG_CHECKING([mysql binary])

  MYSQL_BIN_VERSION=
  ac_mysql_bin_dir=
  AC_ARG_WITH([mysql-bindir],
    [AS_HELP_STRING([--with-mysql-bindir=PATH], [MySQL biarny directory PATH. This should be the directory where MySQL binary executables (i.e. mysql_config) are located])],
    [
      ac_mysql_bin_dir=`cd $withval && pwd`
      if test ! -f "$ac_mysql_bin_dir/mysql_config" ; then
        AC_MSG_ERROR([mysql_config not found! invalid MySQL base directory: $ac_mysql_bin_dir])
      fi

      MYSQL_INC_ADD=`(cd $ac_mysql_bin_dir; ./mysql_config --include)`
      MYSQL_INC="$MYSQL_INC $MYSQL_INC_ADD"
      AC_SUBST(MYSQL_INC)

      MYSQL_BIN_VERSION=`(cd $ac_mysql_bin_dir; ./mysql_config --version)`
      AC_MSG_RESULT([yes: Using $ac_mysql_bin_dir, version $MYSQL_BIN_VERSION])

      MYSQL_LIB=`(cd $ac_mysql_bin_dir; ./mysql_config --libs_r)`
      LIB_DIR=`echo $MYSQL_LIB | sed -e "s|.* -L||" | sed -e "s| .*||"`
      if test a`basename $LIB_DIR` = amysql ; then
        MYSQL_LIB="-L`dirname $LIB_DIR` $MYSQL_LIB" 
      fi
      AC_SUBST(MYSQL_LIB)
    ],
    [AC_MSG_ERROR([--with-mysql-bindir=PATH is required for standalone build])]
  )

  if test a$MYSQL_SOURCE_VERSION != a$MYSQL_BIN_VERSION ; then
    AC_MSG_ERROR([MySQL source version does not match MySQL binary version])
  fi

  AC_MSG_CHECKING([mysql plugin dir])
  ac_mysql_plugin_dir=
  AC_ARG_WITH([mysql-plugindir],
    [AS_HELP_STRING([--with-mysql-plugindir=PATH], [MySQL plugin directory where handlersocket.so to be copied])],
    [
      ac_mysql_plugin_dir=`cd $withval && pwd`
      if test -d "$ac_mysql_plugin_dir/" ; then
        PLUGIN_DIR="$ac_mysql_plugin_dir"
        AC_SUBST(PLUGIN_DIR)
        AC_MSG_RESULT([yes: Using $ac_mysql_plugin_dir])
      else
        AC_MSG_ERROR([invalid MySQL plugin directory : $ac_mysql_plugin_dir])
      fi
    ],
    [
      LIB_DIR_TMP1=`(cd $ac_mysql_bin_dir; ./mysql_config --libs_r)`
      LIB_DIR_TMP2=`echo $LIB_DIR_TMP1 | sed -e "s|.* -L||" | sed -e "s| .*||"`
      ac_mysql_plugin_dir=$LIB_DIR_TMP2/plugin
      PLUGIN_DIR="$ac_mysql_plugin_dir"
      AC_SUBST(PLUGIN_DIR)
      AC_MSG_RESULT([--with-mysql-plugindir was not set. Using $ac_mysql_plugin_dir])
    ]
  )
])

CONFIG_OPTION_MYSQL

CFLAGS="$CFLAGS -Werror"
CXXFLAGS="$CXXFLAGS -Wall -g -fno-rtti -fno-exceptions -fPIC -DPIC"

#TODO
AC_CONFIG_FILES([Makefile
                 handlersocket/Makefile
                 libhsclient/Makefile
                 client/Makefile])
#                 handlersocket/Makefile
#                 libhsclient/Makefile
#                 regtest/Makefile
#                 regtest/test_01_lib/Makefile])

AC_OUTPUT
