#############################################################################
#
# This is the spec file for the distribution specific RPM files
#
##############################################################################

##############################################################################
# Some common macro definitions
##############################################################################

%define mysql_vendor  Percona, Inc
%define distrodescription %(lsb_release -ds)
%define community 1
%define mysqlversion 5.1.70
%define majorversion 14
%define minorversion 7
%define psrelease %{majorversion}.%{minorversion}

%define mysqld_user	mysql
%define mysqld_group	mysql
%define mysqldatadir	/var/lib/mysql
%define see_base For a description of MySQL see the base MySQL RPM or http://www.mysql.com

%define __os_install_post /usr/lib/rpm/brp-compress

# ------------------------------------------------------------------------------
# We don't package all files installed into the build root by intention -
# See BUG#998 for details.
# ------------------------------------------------------------------------------
%define _unpackaged_files_terminate_build 0

# ------------------------------------------------------------------------------
# RPM build tools now automatically detects Perl module dependencies. This 
# detection gives problems as it is broken in some versions, and it also
# give unwanted dependencies from mandatory scripts in our package.
# Might not be possible to disable in all RPM tool versions, but here we
# try. We keep the "AutoReqProv: no" for the "test" sub package, as disabling
# here might fail, and that package has the most problems.
# See http://fedoraproject.org/wiki/Packaging/Perl#Filtering_Requires:_and_Provides
#     http://www.wideopen.com/archives/rpm-list/2002-October/msg00343.html
# ------------------------------------------------------------------------------
%undefine __perl_provides
%undefine __perl_requires

##############################################################################
# Command line handling
##############################################################################

# ----------------------------------------------------------------------
# use "rpmbuild --with yassl" or "rpm --define '_with_yassl 1'" (for RPM 3.x)
# to build with yaSSL support (off by default)
# ----------------------------------------------------------------------
%{?_with_yassl:%define YASSL_BUILD 1}
%{!?_with_yassl:%define YASSL_BUILD 0}

# ----------------------------------------------------------------------
# use "rpmbuild --without libgcc" or "rpm --define '_without_libgcc 1'" (for RPM 3.x)
# to include libgcc (as libmygcc) (on by default)
# ----------------------------------------------------------------------
%{!?_with_libgcc: %{!?_without_libgcc: %define WITH_LIBGCC 1}}
%{?_with_libgcc:%define WITH_LIBGCC 1}
%{?_without_libgcc:%define WITH_LIBGCC 0}


%define __os_install_post /usr/lib/rpm/brp-compress

%define server_suffix  -%{psrelease}
%define package_suffix -51
%define ndbug_comment Percona Server (GPL), %{majorversion}.%{minorversion}, Revision %{gotrevision}
%define debug_comment Percona Server - Debug (GPL), %{majorversion}.%{minorversion}, Revision %{gotrevision}
%define NORMAL_TEST_MODE test-bt
%define DEBUG_TEST_MODE test-bt-debug

%define BUILD_DEBUG 0

%define lic_files COPYING README

##############################################################################
# Main spec file section
##############################################################################

Name:	  Percona-Server%{package_suffix}
Summary:  Drop-in MySQL replacement: fast and reliable
Group:	  Applications/Databases
Version:  %{mysqlversion}
Release:  %{psrelease}.1%{?dist}
License:  GPLv2
Source:	  http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-%{mysqlversion}%{server_suffix}/source/percona-server-%{mysqlversion}%{server_suffix}.tar.gz
URL:      http://www.percona.com/
Provides: msqlormysql MySQL-server Percona-XtraDB-server
BuildRequires: automake
BuildRequires: autoconf
BuildRequires: bison
BuildRequires: gcc-c++
BuildRequires: gperf
BuildRequires: libtool
BuildRequires: ncurses-devel
BuildRequires: perl
BuildRequires: time
BuildRequires: openssl-devel
BuildRequires: procps-ng
BuildRequires: zlib-devel

# Think about what you use here since the first step is to
# run a rm -rf
BuildRoot:    %{_tmppath}/%{name}-%{version}-build

# From the manual
%description
The Percona Server software delivers a very fast, multi-threaded, multi-user,
and robust SQL (Structured Query Language) database server. Percona Server
is intended for mission-critical, heavy-load production systems.

Percona recommends that all production deployments be protected with a support
contract (http://www.percona.com/mysql-suppport/) to ensure the highest uptime,
be eligible for hot fixes, and boost your team's productivity.

##############################################################################
# Sub package definition
##############################################################################

%package -n Percona-Server-server%{package_suffix}
Summary:	%{ndbug_comment}
Group:		Applications/Databases
Requires:	Percona-Server-shared%{package_suffix} Percona-Server-client%{package_suffix} chkconfig coreutils shadow-utils grep procps
Provides:	msqlormysql mysql-server MySQL-server Percona-XtraDB-server
Conflicts:	Percona-SQL-server-50

%description -n Percona-Server-server%{package_suffix}
The Percona Server software delivers a very fast, multi-threaded, multi-user,
and robust SQL (Structured Query Language) database server. Percona Server
is intended for mission-critical, heavy-load production systems.

Percona recommends that all production deployments be protected with a support
contract (http://www.percona.com/mysql-suppport/) to ensure the highest uptime,
be eligible for hot fixes, and boost your team's productivity.

This package includes the Percona Server with XtraDB binary 
as well as related utilities to run and administer Percona Server.

If you want to access and work with the database, you have to install
package "Percona-Server-client%{package_suffix}" as well!

# ------------------------------------------------------------------------------

%package -n Percona-Server-client%{package_suffix}
Summary: Percona-Server - Client
Group: Applications/Databases
Provides: mysql-client MySQL-client Percona-XtraDB-client mysql MySQL
Conflicts: Percona-SQL-client-50

%description -n Percona-Server-client%{package_suffix}
This package contains the standard Percona Server client and administration tools. 

%{see_base}


# ------------------------------------------------------------------------------

%package -n Percona-Server-test%{package_suffix}
Requires: mysql-client perl
Summary: Percona-Server - Test suite
Group: Applications/Databases
Provides: mysql-test MySQL-test Percona-XtraDB-test
Conflicts: Percona-SQL-test-50
AutoReqProv: no

%description -n Percona-Server-test%{package_suffix}
This package contains the Percona-Server regression test suite.

%{see_base}

# ------------------------------------------------------------------------------

%package -n Percona-Server-devel%{package_suffix}
Summary: Percona-Server - Development header files and libraries
Group: Applications/Databases
Provides: mysql-devel MySQL-devel Percona-XtraDB-devel
Conflicts: Percona-SQL-devel-50

%description -n Percona-Server-devel%{package_suffix}
This package contains the development header files and libraries
necessary to develop Percona Server client applications.

%{see_base}

# ------------------------------------------------------------------------------

%package -n Percona-Server-shared%{package_suffix}
Summary: Percona-Server - Shared libraries
Group: Applications/Databases
Provides: mysql-shared MySQL-shared Percona-XtraDB-shared mysql-libs
Obsoletes: mysql-libs

%description -n Percona-Server-shared%{package_suffix}
This package contains the shared libraries (*.so*) which certain
languages and applications need to dynamically load and use MySQL.

# ------------------------------------------------------------------------------


##############################################################################
# 
##############################################################################

%prep

%setup -n percona-server-%{mysqlversion}%{server_suffix}


##############################################################################
# The actual build
##############################################################################

%build

BuildMySQL() {
# Get flags from environment. RPM_OPT_FLAGS seems not to be set anywhere.
CFLAGS=${CFLAGS:-$RPM_OPT_FLAGS}
CXXFLAGS=${CXXFLAGS:-$RPM_OPT_FLAGS}
# Evaluate current setting of $DEBUG
if [ $DEBUG -gt 0 ] ; then
	OPT_COMMENT='--with-comment="%{debug_comment}"'
	OPT_DEBUG='--with-debug'
	CFLAGS=`echo   " $CFLAGS "   | \
	    sed -e 's/ -O[0-9]* / /' -e 's/ -unroll2 / /' -e 's/ -ip / /' \
	        -e 's/^ //' -e 's/ $//'`
	CXXFLAGS=`echo " $CXXFLAGS " | \
	    sed -e 's/ -O[0-9]* / /' -e 's/ -unroll2 / /' -e 's/ -ip / /' \
	        -e 's/^ //' -e 's/ $//'`
else
	OPT_COMMENT='--with-comment="%{ndbug_comment}"'
	OPT_DEBUG=''
fi

echo "BUILD =================="
echo $*

# The --enable-assembler simply does nothing on systems that does not
# support assembler speedups.
sh -c  "CFLAGS=\"$CFLAGS\" \
	CXXFLAGS=\"$CXXFLAGS\" \
	AM_CPPFLAGS=\"$AM_CPPFLAGS\" \
	LDFLAGS=\"$LDFLAGS\" \
	./configure \
 	    $* \
	    --with-plugins=partition,archive,blackhole,csv,example,federated,innodb_plugin \
	    --enable-assembler \
	    --enable-local-infile \
            --with-mysqld-user=%{mysqld_user} \
            --with-unix-socket-path=/var/lib/mysql/mysql.sock \
	    --with-pic \
            -prefix=/usr \
	    --with-extra-charsets=complex \
	    --with-ssl=/usr \
            --exec-prefix=%{_exec_prefix} \
            --libexecdir=%{_sbindir} \
            --libdir=%{_libdir} \
            --sysconfdir=%{_sysconfdir} \
            --datadir=%{_datadir} \
            --localstatedir=%{mysqldatadir} \
            --infodir=%{_infodir} \
            --includedir=%{_includedir} \
            --mandir=%{_mandir} \
	    --enable-thread-safe-client \
        --enable-profiling \
%if %{?ndbug_comment:1}0
	    $OPT_COMMENT \
%endif
	    $OPT_DEBUG \
	    --with-readline \
	    ; make %{?_smp_mflags}"
}
# end of function definition "BuildMySQL"

BuildHandlerSocket() {
cd storage/HandlerSocket-Plugin-for-MySQL
./autogen.sh
CXX=${HS_CXX:-g++} ./configure --with-mysql-source=../../ \
	--with-mysql-bindir=../../scripts \
	--with-mysql-plugindir=%{_libdir}/percona-server/plugin \
	--libdir=%{_libdir} \
	--prefix=%{_prefix}
make %{?_smp_mflags}
cd -
}

BuildUDF() {
MYSQL_SOURCE=`pwd`
cd UDF
CXX=${UDF_CXX:-g++} ./configure --includedir=$MYSQL_SOURCE/include --libdir=%{_libdir}/percona-server/plugin
make %{?_smp_mflags} all
cd -
}
# end of function definition "BuildHandlerSocket"

BuildServer() {
BuildMySQL "--enable-shared \
        --with-server-suffix='%{server_suffix}' \
		--without-embedded-server \
		--without-bench \
		--with-zlib-dir=bundled \
		--with-big-tables"

if [ -n "$MYSQL_CONFLOG_DEST" ] ; then
	cp -fp config.log "$MYSQL_CONFLOG_DEST"
fi

#if [ -f sql/.libs/mysqld ] ; then
#	nm --numeric-sort sql/.libs/mysqld > sql/mysqld.sym
#else
#	nm --numeric-sort sql/mysqld > sql/mysqld.sym
#fi
}
# end of function definition "BuildServer"

# For the debuginfo extraction stage, make a link there to avoid errors in the
# strip phase.
for f in lexyy.c pars0grm.c pars0grm.y pars0lex.l
do
    for d in innobase innodb_plugin
    do
        ln -s "pars/$f" "storage/$d/"
    done
done

RBR=$RPM_BUILD_ROOT
MBD=$RPM_BUILD_DIR/

# Move the test suite to /usr/share/mysql
sed -i 's@[$][(]prefix[)]@\0/share@' mysql-test/Makefile.am \
    mysql-test/lib/My/SafeProcess/Makefile.am

# Clean up the BuildRoot first
[ "$RBR" != "/" ] && [ -d $RBR ] && rm -rf $RBR;
mkdir -p $RBR%{_libdir}/mysql $RBR%{_sbindir}

# Use gcc for C and C++ code (to avoid a dependency on libstdc++ and
# including exceptions into the code
if [ -z "$CXX" -a -z "$CC" ] ; then
	export CC="gcc" CXX="gcc"
fi

# Create the shared libs seperately to avoid a dependency for the client utilities
DEBUG=0
BuildMySQL "--enable-shared"

# Install shared libraries
cp -av libmysql/.libs/*.so*   $RBR/%{_libdir}
cp -av libmysql_r/.libs/*.so* $RBR/%{_libdir}

##############################################################################

# Include libgcc.a in the devel subpackage (BUG 4921)
%if %{WITH_LIBGCC}
libgcc=`$CC $CFLAGS --print-libgcc-file`
mkdir -p $RBR%{_libdir}/percona-server/
install -m 644 "$libgcc" $RBR%{_libdir}/percona-server/libmygcc.a
%endif

##############################################################################

# Now create a debug server
%if %{BUILD_DEBUG}
DEBUG=1
make clean

( BuildServer )   # subshell, so that CFLAGS + CXXFLAGS are modified only locally

if [ "$MYSQL_RPMBUILD_TEST" != "no" ] ; then
	MTR_BUILD_THREAD=auto make %{?_smp_mflags} %{DEBUG_TEST_MODE}
fi

# Get the debug server and its .sym file from the build tree
#if [ -f sql/.libs/mysqld ] ; then
#	cp sql/.libs/mysqld $RBR%{_sbindir}/mysqld-debug
#else
#	cp sql/mysqld       $RBR%{_sbindir}/mysqld-debug
#fi
#cp libmysqld/libmysqld.a    $RBR%{_libdir}/percona-server/libmysqld-debug.a
#cp sql/mysqld.sym           $RBR%{_libdir}/percona-server/mysqld-debug.sym

%endif

# Now, the default server
DEBUG=0
make clean

BuildServer
BuildHandlerSocket
BuildUDF
if [ "$MYSQL_RPMBUILD_TEST" != "no" ] ; then
	MTR_BUILD_THREAD=auto make %{?_smp_mflags} %{NORMAL_TEST_MODE}
fi

# Now, build plugin 
#BUILDSO=0
#make clean

#BuildServer

#if [ "$MYSQL_RPMBUILD_TEST" != "no" ] ; then
#	MTR_BUILD_THREAD=auto make %{NORMAL_TEST_MODE}
#fi

# Move temporarily the saved files to the BUILD directory since the BUILDROOT
# dir will be cleaned at the start of the install phase
mkdir -p "$(dirname $RPM_BUILD_DIR/%{_libdir})"
mv $RBR%{_libdir} $RPM_BUILD_DIR/%{_libdir}

%install
RBR=$RPM_BUILD_ROOT
MBD=$RPM_BUILD_DIR/percona-server-%{mysqlversion}%{server_suffix}

# Move back the libdir from BUILD dir to BUILDROOT
mkdir -p "$(dirname $RBR%{_libdir})"
mv $RPM_BUILD_DIR/%{_libdir} $RBR%{_libdir}

# Ensure that needed directories exists
install -d $RBR%{_sysconfdir}/{logrotate.d,init.d}
install -d $RBR%{mysqldatadir}/mysql
install -d $RBR%{_datadir}/mysql-test
install -d $RBR%{_datadir}/percona-server/SELinux/RHEL4
install -d $RBR%{_includedir}
install -d $RBR%{_libdir}
install -d $RBR%{_mandir}
install -d $RBR%{_sbindir}
install -d $RBR%{_libdir}/percona-server/plugin

make DESTDIR=$RBR benchdir_root=%{_datadir} install
cd storage/HandlerSocket-Plugin-for-MySQL
make DESTDIR=$RBR benchdir_root=%{_datadir} install
cd -
cd UDF
make DESTDIR=$RBR benchdir_root=%{_datadir} install
cd -

# install symbol files ( for stack trace resolution)
#install -m644 $MBD/sql/mysqld.sym $RBR%{_libdir}/percona-server/mysqld.sym

# Install logrotate and autostart
install -m644 $MBD/support-files/mysql-log-rotate \
        $RBR%{_sysconfdir}/logrotate.d/mysql
install -m755 $MBD/support-files/mysql.server \
        $RBR%{_sysconfdir}/init.d/mysql

# in RPMs, it is unlikely that anybody should use "sql-bench"
rm -fr $RBR%{_datadir}/sql-bench

# Create a symlink "rcmysql", pointing to the init.script. SuSE users
# will appreciate that, as all services usually offer this.
ln -s %{_sysconfdir}/init.d/mysql $RBR%{_sbindir}/rcmysql

# Touch the place where the my.cnf config file and mysqlmanager.passwd
# (MySQL Instance Manager password file) might be located
# Just to make sure it's in the file list and marked as a config file
touch $RBR%{_sysconfdir}/my.cnf
touch $RBR%{_sysconfdir}/mysqlmanager.passwd

# Install SELinux files in datadir
install -m600 $MBD/support-files/RHEL4-SElinux/mysql.{fc,te} \
	$RBR%{_datadir}/percona-server/SELinux/RHEL4

##############################################################################
#  Post processing actions, i.e. when installed
##############################################################################

%pre -n Percona-Server-server%{package_suffix}
# Check if we can safely upgrade.  An upgrade is only safe if it's from one
# of our RPMs in the same version family.

installed=`rpm -q --whatprovides mysql-server 2> /dev/null`
if [ $? -eq 0 -a -n "$installed" ]; then
  vendor=`rpm -q --queryformat='%{VENDOR}' "$installed" 2>&1`
  version=`rpm -q --queryformat='%{VERSION}' "$installed" 2>&1`
  myvendor='%{mysql_vendor}'
  myversion='%{mysqlversion}'

  old_family=`echo $version   | sed -n -e 's,^\([1-9][0-9]*\.[0-9][0-9]*\)\..*$,\1,p'`
  new_family=`echo $myversion | sed -n -e 's,^\([1-9][0-9]*\.[0-9][0-9]*\)\..*$,\1,p'`

  [ -z "$vendor" ] && vendor='<unknown>'
  [ -z "$old_family" ] && old_family="<unrecognized version $version>"
  [ -z "$new_family" ] && new_family="<bad package specification: version $myversion>"

  error_text=
#  if [ "$vendor" != "$myvendor" ]; then
#    error_text="$error_text
#The current MySQL server package is provided by a different
#vendor ($vendor) than $myvendor.  Some files may be installed
#to different locations, including log files and the service
#startup script in %{_sysconfdir}/init.d/.
#"
#  fi

  if [ "$old_family" != "$new_family" ]; then
    error_text="$error_text
Upgrading directly from MySQL $old_family to MySQL $new_family may not
be safe in all cases.  A manual dump and restore using mysqldump is
recommended.  It is important to review the MySQL manual's Upgrading
section for version-specific incompatibilities.
"
  fi

  if [ -n "$error_text" ]; then
    cat <<HERE >&2

******************************************************************
A MySQL  package ($installed) is installed.
$error_text
A manual upgrade is required.

- Ensure that you have a complete, working backup of your data and my.cnf
  files
- Shut down the MySQL server cleanly
- Remove the existing MySQL packages.  Usually this command will
  list the packages you should remove:
  rpm -qa | grep -i '^mysql-'

  You may choose to use 'rpm --nodeps -ev <package-name>' to remove
  the package which contains the perconaserverclient shared library.  The
  library will be reinstalled by the Percona-shared-compat package.
- Install the new Percona Server packages supplied by $myvendor
- Ensure that the Percona Server is started
- Run the 'mysql_upgrade' program

This is a brief description of the upgrade process.  Important details
can be found in the MySQL manual, in the Upgrading section.
For additional details please visit Percona Documentation page
at http://www.percona.com/software/documentation/

******************************************************************
HERE
    exit 1
  fi
fi

# Shut down a previously installed server first
if [ -x %{_sysconfdir}/init.d/mysql ] ; then
	%{_sysconfdir}/init.d/mysql stop > /dev/null 2>&1
	echo "Giving mysqld 5 seconds to exit nicely"
	sleep 5
fi

%post -n Percona-Server-server%{package_suffix}
if [ X${PERCONA_DEBUG} == X1 ]; then
        set -x
fi

# There are users who deviate from the default file system layout.
# Check local settings to support them.
if [ -x %{_bindir}/my_print_defaults ]
then
  mysql_datadir=`%{_bindir}/my_print_defaults server mysqld | grep '^--datadir=' | sed -n 's/--datadir=//p'`
fi
if [ -z "$mysql_datadir" ]
then
  mysql_datadir=%{mysqldatadir}
fi

# ----------------------------------------------------------------------
# Make MySQL start/shutdown automatically when the machine does it.
# ----------------------------------------------------------------------
if [ -x /sbin/chkconfig ] ; then
	/sbin/chkconfig --add mysql
fi
#
# ----------------------------------------------------------------------
# Create a MySQL user and group. Do not report any problems if it already
# exists.
# ----------------------------------------------------------------------
groupadd -r %{mysqld_group} 2> /dev/null || true
useradd -M -r -d $mysql_datadir -s /bin/bash -c "Percona Server" -g %{mysqld_group} %{mysqld_user} 2> /dev/null || true 
# The user may already exist, make sure it has the proper group nevertheless (BUG#12823)
usermod -g %{mysqld_group} %{mysqld_user} 2> /dev/null || true

# ----------------------------------------------------------------------
# Initiate databases
# ----------------------------------------------------------------------
if [ $1 -eq 1 ]; then #clean installation
        mkdir -p $mysql_datadir/{mysql,test}
        %{_bindir}/mysql_install_db --rpm --user=%{mysqld_user}
fi
# ----------------------------------------------------------------------
# FIXME upgrade databases if needed would go here - but it cannot be
# automated yet
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
# Change permissions again to fix any new files.
# ----------------------------------------------------------------------
chown -R %{mysqld_user}:%{mysqld_group} $mysql_datadir

# ----------------------------------------------------------------------
# Fix permissions for the permission database so that only the user
# can read them.
# ----------------------------------------------------------------------
chmod -R og-rw $mysql_datadir/mysql

# ----------------------------------------------------------------------
# install SELinux files - but don't override existing ones
# ----------------------------------------------------------------------
SETARGETDIR=/etc/selinux/targeted/src/policy
SEDOMPROG=$SETARGETDIR/domains/program
SECONPROG=$SETARGETDIR/file_contexts/program

if [ -x sbin/restorecon ] ; then
	sbin/restorecon -R var/lib/mysql
fi

# Restart in the same way that mysqld will be started normally.
if [ -x %{_sysconfdir}/init.d/mysql ] ; then
	%{_sysconfdir}/init.d/mysql start
	echo "Giving mysqld 2 seconds to start"
	sleep 2
fi

echo "Percona Server is distributed with several useful UDF (User Defined Function) from Maatkit."
echo "Run the following commands to create these functions:"
echo "mysql -e \"CREATE FUNCTION fnv1a_64 RETURNS INTEGER SONAME 'libfnv1a_udf.so'\""
echo "mysql -e \"CREATE FUNCTION fnv_64 RETURNS INTEGER SONAME 'libfnv_udf.so'\""
echo "mysql -e \"CREATE FUNCTION murmur_hash RETURNS INTEGER SONAME 'libmurmur_udf.so'\""
echo "See http://code.google.com/p/maatkit/source/browse/trunk/udf for more details"

# Allow mysqld_safe to start mysqld and print a message before we exit
sleep 2

%preun -n Percona-Server-server%{package_suffix}
if [ $1 = 0 ] ; then
	# Stop MySQL before uninstalling it
	if [ -x %{_sysconfdir}/init.d/mysql ] ; then
		%{_sysconfdir}/init.d/mysql stop > /dev/null
		# Don't start it automatically anymore
		if [ -x /sbin/chkconfig ] ; then
			/sbin/chkconfig --del mysql
		fi
	fi
fi

# We do not remove the mysql user since it may still own a lot of
# database files.

# ----------------------------------------------------------------------
# Clean up the BuildRoot after build is done
# ----------------------------------------------------------------------
%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT;

##############################################################################
#  Files section
##############################################################################

%files -n Percona-Server-server%{package_suffix}
%defattr(-,root,root,0755)

%doc %{lic_files}
%doc support-files/my-*.cnf

%doc %attr(644, root, root) %{_infodir}/mysql.info*
%doc %attr(644, root, man) %{_mandir}/man1/innochecksum.1*
%doc %attr(644, root, man) %{_mandir}/man1/myisam_ftdump.1*
%doc %attr(644, root, man) %{_mandir}/man1/myisamchk.1*
%doc %attr(644, root, man) %{_mandir}/man1/myisamlog.1*
%doc %attr(644, root, man) %{_mandir}/man1/myisampack.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_convert_table_format.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_fix_extensions.1*
%doc %attr(644, root, man) %{_mandir}/man8/mysqld.8*
%doc %attr(644, root, man) %{_mandir}/man1/mysqld_multi.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqld_safe.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_fix_privilege_tables.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_install_db.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_secure_installation.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_setpermission.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_upgrade.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlhotcopy.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlman.1*
%doc %attr(644, root, man) %{_mandir}/man8/mysqlmanager.8*
%doc %attr(644, root, man) %{_mandir}/man1/mysql.server.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqltest.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_tzinfo_to_sql.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_zap.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlbug.1*
%doc %attr(644, root, man) %{_mandir}/man1/perror.1*
%doc %attr(644, root, man) %{_mandir}/man1/replace.1*
%doc %attr(644, root, man) %{_mandir}/man1/resolve_stack_dump.1*
%doc %attr(644, root, man) %{_mandir}/man1/resolveip.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqldumpslow.1*

%ghost %config(noreplace,missingok) %{_sysconfdir}/my.cnf
%ghost %config(noreplace,missingok) %{_sysconfdir}/mysqlmanager.passwd

%attr(755, root, root) %{_bindir}/innochecksum
%attr(755, root, root) %{_bindir}/myisam_ftdump
%attr(755, root, root) %{_bindir}/myisamchk
%attr(755, root, root) %{_bindir}/myisamlog
%attr(755, root, root) %{_bindir}/myisampack
%attr(755, root, root) %{_bindir}/mysql_convert_table_format
%attr(755, root, root) %{_bindir}/mysql_fix_extensions
%attr(755, root, root) %{_bindir}/mysql_fix_privilege_tables
%attr(755, root, root) %{_bindir}/mysql_install_db
%attr(755, root, root) %{_bindir}/mysql_secure_installation
%attr(755, root, root) %{_bindir}/mysql_setpermission
%attr(755, root, root) %{_bindir}/mysql_tzinfo_to_sql
%attr(755, root, root) %{_bindir}/mysql_upgrade
%attr(755, root, root) %{_bindir}/mysql_zap
%attr(755, root, root) %{_bindir}/mysqlbug
%attr(755, root, root) %{_bindir}/mysqld_multi
%attr(755, root, root) %{_bindir}/mysqld_safe
%attr(755, root, root) %{_bindir}/mysqldumpslow
%attr(755, root, root) %{_bindir}/mysqlhotcopy
%attr(755, root, root) %{_bindir}/mysqltest
%attr(755, root, root) %{_bindir}/perror
%attr(755, root, root) %{_bindir}/replace
%attr(755, root, root) %{_bindir}/resolve_stack_dump
%attr(755, root, root) %{_bindir}/resolveip

%attr(755, root, root) %{_sbindir}/mysqld
%if %{BUILD_DEBUG}
%attr(755, root, root) %{_sbindir}/mysqld-debug
%endif
%attr(755, root, root) %{_sbindir}/mysqlmanager
%attr(755, root, root) %{_sbindir}/rcmysql
#%attr(644, root, root) %{_libdir}/percona-server/mysqld.sym
%if %{BUILD_DEBUG}
#%attr(644, root, root) %{_libdir}/percona-server/mysqld-debug.sym
%endif

%attr(644, root, root) %config(noreplace,missingok) %{_sysconfdir}/logrotate.d/mysql
%attr(755, root, root) %{_sysconfdir}/init.d/mysql

%attr(755, root, root) %{_datadir}/percona-server/

%attr(644, root, root) %{_libdir}/percona-server/plugin/*

%files -n Percona-Server-client%{package_suffix}
%defattr(-, root, root, 0755)
%attr(755, root, root) %{_bindir}/msql2mysql
%attr(755, root, root) %{_bindir}/mysql
%attr(755, root, root) %{_bindir}/my_print_defaults
%attr(755, root, root) %{_bindir}/mysql_find_rows
%attr(755, root, root) %{_bindir}/mysql_waitpid
%attr(755, root, root) %{_bindir}/mysqlaccess
%attr(755, root, root) %{_bindir}/mysqladmin
%attr(755, root, root) %{_bindir}/mysqlbinlog
%attr(755, root, root) %{_bindir}/mysqlcheck
%attr(755, root, root) %{_bindir}/mysqldump
%attr(755, root, root) %{_bindir}/mysqlimport
%attr(755, root, root) %{_bindir}/mysqlshow
%attr(755, root, root) %{_bindir}/mysqlslap
%attr(755, root, root) %{_bindir}/hsclient

%doc %attr(644, root, man) %{_mandir}/man1/msql2mysql.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql.1*
%doc %attr(644, root, man) %{_mandir}/man1/my_print_defaults.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_find_rows.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_waitpid.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlaccess.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqladmin.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlbinlog.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlcheck.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqldump.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlimport.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlshow.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlslap.1*

%post -n Percona-Server-shared%{package_suffix}
/sbin/ldconfig

%postun -n Percona-Server-shared%{package_suffix}
/sbin/ldconfig


%files -n Percona-Server-devel%{package_suffix}
%defattr(-, root, root, 0755)
%doc %attr(644, root, man) %{_mandir}/man1/comp_err.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_config.1*
%attr(755, root, root) %{_bindir}/mysql_config
%dir %attr(755, root, root) %{_libdir}/mysql
%{_includedir}/percona-server
%{_includedir}/handlersocket
%{_datadir}/aclocal/mysql.m4
%{_libdir}/percona-server/libdbug.a
%{_libdir}/percona-server/libheap.a
%if %{WITH_LIBGCC}
%{_libdir}/percona-server/libmygcc.a
%endif
%{_libdir}/percona-server/libmyisam.a
%{_libdir}/percona-server/libmyisammrg.a
%{_libdir}/percona-server/libperconaserverclient.a
%{_libdir}/percona-server/libperconaserverclient.la
%{_libdir}/percona-server/libperconaserverclient_r.a
%{_libdir}/percona-server/libperconaserverclient_r.la
%{_libdir}/percona-server/libmystrings.a
%{_libdir}/percona-server/libmysys.a
%{_libdir}/percona-server/libvio.a
%{_libdir}/percona-server/libz.a
%{_libdir}/percona-server/libz.la
%{_libdir}/libhsclient.a
%{_libdir}/libhsclient.la

%{_libdir}/*.so
%{_libdir}/percona-server/*.so

%files -n Percona-Server-shared%{package_suffix}
%defattr(-, root, root, 0755)
# Shared libraries (omit for architectures that don't support them)
%{_libdir}/*.so.*
%{_libdir}/percona-server/*.so.*

%files -n Percona-Server-test%{package_suffix}
%defattr(-, root, root, 0755)
/usr/share/mysql-test/*
%attr(755, root, root) %{_bindir}/mysql_client_test
%doc %attr(644, root, man) %{_mandir}/man1/mysql_client_test.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql-stress-test.pl.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql-test-run.pl.1*

%changelog
* Thu Dec 12 2013 Stewart Smith <stewart.smith@percona.com>

Percona Server 5.1.70-14.8

