# Copyright (c) 2000, 2010, Oracle and/or its affiliates. All rights reserved.
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
# along with this program; see the file COPYING. If not, write to the
# Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston
# MA  02110-1301  USA.

##############################################################################
# Some common macro definitions
##############################################################################

# NOTE: "vendor" is used in upgrade/downgrade check, so you can't
# change these, has to be exactly as is.
%define mysql_old_vendor        MySQL AB
%define mysql_vendor_2          Sun Microsystems, Inc.
%define mysql_vendor            Oracle and/or its affiliates
%define percona_server_vendor	Percona, Inc

%define mysql_version @@MYSQL_VERSION@@
%define redhatversion %(lsb_release -rs | awk -F. '{ print $1}')
%define percona_server_version @@PERCONA_VERSION@@
%define revision @@REVISION@@
%define tokudb_backup_version @@TOKUDB_BACKUP_VERSION@@

#
%bcond_with tokudb
%bcond_with systemd
#
%if %{with systemd}
  %define systemd 1
%else
  %if 0%{?rhel} > 6
    %define systemd 1
  %else
    %define systemd 0
  %endif
%endif

#
%if %{with tokudb}
  %define TOKUDB_FLAGS -DWITH_VALGRIND=OFF -DUSE_VALGRIND=OFF -DDEBUG_EXTNAME=OFF -DBUILD_TESTING=OFF -DUSE_GTAGS=OFF -DUSE_CTAGS=OFF -DUSE_ETAGS=OFF -DUSE_CSCOPE=OFF -DTOKUDB_BACKUP_PLUGIN_VERSION=%{tokudb_backup_version}
  %define TOKUDB_DEBUG_ON -DTOKU_DEBUG_PARANOID=ON
  %define TOKUDB_DEBUG_OFF -DTOKU_DEBUG_PARANOID=OFF
%else
  %define TOKUDB_FLAGS %{nil}
  %define TOKUDB_DEBUG_ON %{nil}
  %define TOKUDB_DEBUG_OFF %{nil}
%endif
#
%define mysqld_user     mysql
%define mysqld_group    mysql
%define mysqldatadir    /var/lib/mysql

%define release         rel%{percona_server_version}%{?dist}

%if "%rhel" > "6"
%define shared_lib_pri_name libmysqlclient
%define shared_lib_sec_name libperconaserverclient
%else
%define shared_lib_pri_name libperconaserverclient
%define shared_lib_sec_name libmysqlclient
%endif

#
# Macros we use which are not available in all supported versions of RPM
#
# - defined/undefined are missing on RHEL4
#
%if %{expand:%{?defined:0}%{!?defined:1}}
%define defined()       %{expand:%%{?%{1}:1}%%{!?%{1}:0}}
%endif
%if %{expand:%{?undefined:0}%{!?undefined:1}}
%define undefined()     %{expand:%%{?%{1}:0}%%{!?%{1}:1}}
%endif

# ----------------------------------------------------------------------------
# RPM build tools now automatically detect Perl module dependencies.  This
# detection causes problems as it is broken in some versions, and it also
# provides unwanted dependencies from mandatory scripts in our package.
# It might not be possible to disable this in all versions of RPM, but here we
# try anyway.  We keep the "AutoReqProv: no" for the "test" sub package, as
# disabling here might fail, and that package has the most problems.
# See:
#  http://fedoraproject.org/wiki/Packaging/Perl#Filtering_Requires:_and_Provides
#  http://www.wideopen.com/archives/rpm-list/2002-October/msg00343.html
# ----------------------------------------------------------------------------
%undefine __perl_provides
%undefine __perl_requires

##############################################################################
# Command line handling
##############################################################################
#
# To set options:
#
#   $ rpmbuild --define="option <x>" ...
#

# ----------------------------------------------------------------------------
# Commercial builds
# ----------------------------------------------------------------------------
%if %{undefined commercial}
%define commercial 0
%endif

# ----------------------------------------------------------------------------
# Source name
# ----------------------------------------------------------------------------
%if %{undefined src_base}
%define src_base percona-server
%endif
%define src_dir %{src_base}-%{mysql_version}-%{percona_server_version}

# ----------------------------------------------------------------------------
# Feature set (storage engines, options).  Default to community (everything)
# ----------------------------------------------------------------------------
%if %{undefined feature_set}
%define feature_set community
%endif

# ----------------------------------------------------------------------------
# Server comment strings
# ----------------------------------------------------------------------------
%if %{undefined compilation_comment_debug}
%define compilation_comment_debug       Percona Server - Debug (GPL), Release %{percona_server_version}, Revision %{revision}
%endif
%if %{undefined compilation_comment_release}
%define compilation_comment_release     Percona Server (GPL), Release %{percona_server_version}, Revision %{revision}
%endif


# ----------------------------------------------------------------------------
# Product and server suffixes
# ----------------------------------------------------------------------------
%define product_suffix -56
%if %{undefined product_suffix}
  %if %{defined short_product_tag}
    %define product_suffix      -%{short_product_tag}
  %else
    %define product_suffix      %{nil}
  %endif
%endif

%define server_suffix %{product_suffix}
%if %{undefined server_suffix}
%define server_suffix   %{nil}
%endif

# ----------------------------------------------------------------------------
# Distribution support
# ----------------------------------------------------------------------------
%if %{undefined distro_specific}
%define distro_specific 0
%endif
%if %{distro_specific}
  %if %(test -f /etc/enterprise-release && echo 1 || echo 0)
    %define oelver %(rpm -qf --qf '%%{version}\\n' /etc/enterprise-release | sed -e 's/^\\([0-9]*\\).*/\\1/g')
    %if "%oelver" == "4"
      %define distro_description        Oracle Enterprise Linux 4
      %define distro_releasetag         oel4
      %define distro_buildreq           gcc-c++ gperf ncurses-devel perl readline-devel time zlib-devel libaio-devel bison cmake
      %define distro_requires           chkconfig coreutils grep procps shadow-utils
    %else
      %if "%oelver" == "5"
        %define distro_description      Oracle Enterprise Linux 5
        %define distro_releasetag       oel5
        %define distro_buildreq         gcc-c++ gperf ncurses-devel perl readline-devel time zlib-devel libaio-devel bison cmake
        %define distro_requires         chkconfig coreutils grep procps shadow-utils
      %else
        %{error:Oracle Enterprise Linux %{oelver} is unsupported}
      %endif
    %endif
  %else
    %if %(test -f /etc/redhat-release && echo 1 || echo 0)
      %define rhelver %(rpm -qf --qf '%%{version}\\n' /etc/redhat-release | sed -e 's/^\\([0-9]*\\).*/\\1/g')
      %if "%rhelver" == "4"
        %define distro_description      Red Hat Enterprise Linux 4
        %define distro_releasetag       rhel4
        %define distro_buildreq         gcc-c++ gperf ncurses-devel perl readline-devel time zlib-devel libaio-devel bison cmake
        %define distro_requires         chkconfig coreutils grep procps shadow-utils
      %else
        %if "%rhelver" == "5"
          %define distro_description    Red Hat Enterprise Linux 5
          %define distro_releasetag     rhel5
          %define distro_buildreq       gcc-c++ gperf ncurses-devel perl readline-devel time zlib-devel libaio-devel bison cmake
          %define distro_requires       chkconfig coreutils grep procps shadow-utils
        %else
          %{error:Red Hat Enterprise Linux %{rhelver} is unsupported}
        %endif
      %endif
    %else
      %if %(test -f /etc/SuSE-release && echo 1 || echo 0)
        %define susever %(rpm -qf --qf '%%{version}\\n' /etc/SuSE-release)
        %if "%susever" == "10"
          %define distro_description    SUSE Linux Enterprise Server 10
          %define distro_releasetag     sles10
          %define distro_buildreq       gcc-c++ gdbm-devel gperf ncurses-devel openldap2-client readline-devel zlib-devel libaio-devel bison cmake
          %define distro_requires       aaa_base coreutils grep procps pwdutils
        %else
          %if "%susever" == "11"
            %define distro_description  SUSE Linux Enterprise Server 11
            %define distro_releasetag   sles11
            %define distro_buildreq     gcc-c++ gdbm-devel gperf ncurses-devel openldap2-client procps pwdutils readline-devel zlib-devel libaio-devel bison cmake
            %define distro_requires     aaa_base coreutils grep procps pwdutils
          %else
            %{error:SuSE %{susever} is unsupported}
          %endif
        %endif
      %else
        %{error:Unsupported distribution}
      %endif
    %endif
  %endif
%else
  %define generic_kernel %(uname -r | cut -d. -f1-2)
  %define distro_description            Generic Linux (kernel %{generic_kernel})
  %define distro_releasetag             linux%{generic_kernel}
  %define distro_buildreq               gcc-c++ gperf ncurses-devel perl readline-devel time zlib-devel libaio-devel bison cmake
  %define distro_requires               coreutils grep procps /usr/sbin/useradd /usr/sbin/groupadd
%endif

# ----------------------------------------------------------------------------
# Support optional "tcmalloc" library (experimental)
# ----------------------------------------------------------------------------
%if %{defined malloc_lib_target}
%define WITH_TCMALLOC 1
%else
%define WITH_TCMALLOC 0
%endif

##############################################################################
# Configuration based upon above user input, not to be set directly
##############################################################################

%if %{commercial}
%define license_files_server    LICENSE.mysql
%define license_type            Commercial
%else
%define license_files_server    COPYING README
%define license_type            GPL
%endif

##############################################################################
# Main spec file section
##############################################################################

Name:           Percona-Server%{product_suffix}
Summary:        Percona-Server: a very fast and reliable SQL database server
Group:          Applications/Databases
Version:        %{mysql_version}
Release:        %{release}
Distribution:   %{distro_description}
License:        Copyright (c) 2000, 2010, %{mysql_vendor}.  All rights reserved.  Use is subject to license terms.  Under %{license_type} license as shown in the Description field.
Source:         http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-%{mysql_version}-%{percona_server_version}/source/%{src_dir}.tar.gz
URL:            http://www.percona.com/
Packager:       Percona MySQL Development Team <mysqldev@percona.com>
Vendor:         %{percona_server_vendor}
Provides:       mysql-server
BuildRequires:  %{distro_buildreq} pam-devel openssl-devel
%if 0%{?systemd}
BuildRequires:  systemd
%endif
Patch0:         mysql-5.6-sharedlib-rename.patch
Patch1:         mysql-5.6-libmysqlclient-symbols.patch

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

%package -n Percona-Server-server%{product_suffix}
Summary:        Percona Server: a very fast and reliable SQL database server
Group:          Applications/Databases
Requires:       %{distro_requires} Percona-Server-shared%{product_suffix} Percona-Server-client%{product_suffix}
Requires:       perl(Data::Dumper)
%if 0%{?systemd}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd
%else
Requires(post):   /sbin/chkconfig
Requires(preun):  /sbin/chkconfig
Requires(preun):  /sbin/service
%endif
Provides:       mysql-server MySQL-server
Conflicts:	Percona-SQL-server-50 Percona-Server-server-51 Percona-Server-server-55

%description -n Percona-Server-server%{product_suffix}
The Percona Server software delivers a very fast, multi-threaded, multi-user,
and robust SQL (Structured Query Language) database server. Percona Server
is intended for mission-critical, heavy-load production systems.

Percona recommends that all production deployments be protected with a support
contract (http://www.percona.com/mysql-suppport/) to ensure the highest uptime,
be eligible for hot fixes, and boost your team's productivity.

This package includes the Percona Server with XtraDB binary 
as well as related utilities to run and administer Percona Server.

If you want to access and work with the database, you have to install
package "Percona-Server-client%{product_suffix}" as well!

%if %{with tokudb}
# ----------------------------------------------------------------------------
%package -n Percona-Server-tokudb%{product_suffix}
Summary:        Percona Server - TokuDB
Group:          Applications/Databases
Requires:       Percona-Server-server%{product_suffix} = %{version}-%{release}
Requires:       Percona-Server-shared%{product_suffix} = %{version}-%{release}
Requires:       Percona-Server-client%{product_suffix} = %{version}-%{release}
Requires:       jemalloc >= 3.3.0
Provides:       tokudb-plugin = %{version}-%{release}

%description -n Percona-Server-tokudb%{product_suffix}
This package contains the TokuDB plugin for Percona Server %{version}-%{release}
%endif

# ----------------------------------------------------------------------------
%package -n Percona-Server-selinux%{product_suffix}
Summary: 		Percona Server - Selinux policy module
Group:          	Applications/Databases
%if 0%{?rhel} >= 6
BuildArch:		noarch
%endif
Requires:		selinux-policy
Requires(post):		policycoreutils
Requires(postun):	policycoreutils

%if 0%{?rhel} == 6
BuildRequires: 		selinux-policy
%else
BuildRequires: 		selinux-policy-devel
%endif

%description -n Percona-Server-selinux%{product_suffix}
This package contains SELinux policy module for Percona Server package.

For a description of Percona Server see http://www.percona.com/software/percona-server/

# ----------------------------------------------------------------------------
%package -n Percona-Server-client%{product_suffix}
Summary:        Percona Server - Client
Group:          Applications/Databases
Requires:       Percona-Server-shared%{product_suffix}
Provides:       mysql-client MySQL-client mysql MySQL
Conflicts:      Percona-SQL-client-50 Percona-Server-client-51 Percona-Server-client-55

%description -n Percona-Server-client%{product_suffix}
This package contains the standard Percona Server client and administration tools.

For a description of Percona Server see http://www.percona.com/software/percona-server/

# ----------------------------------------------------------------------------
%package -n Percona-Server-test%{product_suffix}
Requires:       Percona-Server-client%{product_suffix} perl
Requires:       perl(Socket), perl(Time::HiRes), perl(Data::Dumper), perl(Test::More)
Summary:        Percona Server - Test suite
Group:          Applications/Databases
Provides:       mysql-test
Conflicts:      Percona-SQL-test-50 Percona-Server-test-51 Percona-Server-test-55
AutoReqProv:    no

%description -n Percona-Server-test%{product_suffix}
This package contains the Percona Server regression test suite.

For a description of Percona Server see http://www.percona.com/software/percona-server/

# ----------------------------------------------------------------------------
%package -n Percona-Server-devel%{product_suffix}
Summary:        Percona Server - Development header files and libraries
Group:          Applications/Databases
Provides:       mysql-devel
Conflicts:      Percona-SQL-devel-50 Percona-Server-devel-51 Percona-Server-devel-55

%description -n Percona-Server-devel%{product_suffix}
This package contains the development header files and libraries necessary
to develop Percona Server client applications.

For a description of Percona Server see http://www.percona.com/software/percona-server/

# ----------------------------------------------------------------------------
%package -n Percona-Server-shared%{product_suffix}
Summary:        Percona Server - Shared libraries
Group:          Applications/Databases
%if "%rhel" > "6"
Provides:       mysql-shared mysql-libs
Obsoletes:      mariadb-libs
Conflicts:      Percona-Server-shared-55
%else
%ifarch x86_64
Provides:       libmysqlclient.so.18()(64bit)
Provides:       libmysqlclient.so.18(libmysqlclient_18)(64bit)
%endif
%ifarch i386 i686
Provides:       libmysqlclient.so.18()(32bit)
Provides:       libmysqlclient.so.18(libmysqlclient_18)(32bit)
%endif
%endif

%description -n Percona-Server-shared%{product_suffix}
This package contains the shared libraries (*.so*) which certain languages
and applications need to dynamically load and use Percona Server.

##############################################################################
%prep
%setup -n %{src_dir}

%if "%rhel" > "6"
%patch0 -p1
%patch1 -p1
%endif

##############################################################################
%build

# Optional package files
touch optional-files-devel

#
# Set environment in order of preference, MYSQL_BUILD_* first, then variable
# name, finally a default.  RPM_OPT_FLAGS is assumed to be a part of the
# default RPM build environment.
#

# This is a hack, $RPM_OPT_FLAGS on ia64 hosts contains flags which break
# the compile in cmd-line-utils/readline - needs investigation, but for now
# we simply unset it and use those specified directly in cmake.
%if "%{_arch}" == "ia64"
RPM_OPT_FLAGS=
%endif
#
RPM_OPT_FLAGS=$(echo ${RPM_OPT_FLAGS} | sed -e 's|-march=i386|-march=i686|g')
#
# Needed on centos5 to force debug symbols compatibility with older gdb version
%if "%rhel" == "5"
RPM_OPT_FLAGS="${RPM_OPT_FLAGS} -gdwarf-2"
%endif
#
export PATH=${MYSQL_BUILD_PATH:-$PATH}
export CC=${MYSQL_BUILD_CC:-${CC:-gcc}}
export CXX=${MYSQL_BUILD_CXX:-${CXX:-g++}}
export CFLAGS=${MYSQL_BUILD_CFLAGS:-${CFLAGS:-$RPM_OPT_FLAGS}}
export CXXFLAGS=${MYSQL_BUILD_CXXFLAGS:-${CXXFLAGS:-$RPM_OPT_FLAGS -felide-constructors -fno-rtti}}
export LDFLAGS=${MYSQL_BUILD_LDFLAGS:-${LDFLAGS:-}}
export CMAKE=${MYSQL_BUILD_CMAKE:-${CMAKE:-cmake}}

# "Fix" cmake directories in case we're crosscompiling.
# We detect crosscompiles to i686 if uname is x86_64 however _libdir does not
# contain lib64.
# In this case, we cannot instruct cmake to change CMAKE_SYSTEM_PROCESSOR, so
# we need to alter the directories in cmake/install_layout.cmake manually.
if test "x$(uname -m)" = "xx86_64" && echo "%{_libdir}" | fgrep -vq lib64
then
    sed -i 's/lib64/lib/' "cmake/install_layout.cmake"
fi

# Build debug mysqld and libmysqld.a
mkdir debug
(
  cd debug
  # Attempt to remove any optimisation flags from the debug build
  # BLD-238 - bug1408232
  CFLAGS=`echo " ${CFLAGS} " | \
            sed -e 's/ -O[0-9]* / /' \
                -e 's/-Wp,-D_FORTIFY_SOURCE=2/ /' \
                -e 's/ -unroll2 / /' \
                -e 's/ -ip / /' \
                -e 's/^ //' \
                -e 's/ $//'`
  CXXFLAGS=`echo " ${CXXFLAGS} " | \
              sed -e 's/ -O[0-9]* / /' \
                  -e 's/-Wp,-D_FORTIFY_SOURCE=2/ /' \
                  -e 's/ -unroll2 / /' \
                  -e 's/ -ip / /' \
                  -e 's/^ //' \
                  -e 's/ $//'`
  # XXX: MYSQL_UNIX_ADDR should be in cmake/* but mysql_version is included before
  # XXX: install_layout so we can't just set it based on INSTALL_LAYOUT=RPM
  ${CMAKE} ../ -DBUILD_CONFIG=mysql_release -DINSTALL_LAYOUT=RPM \
           -DCMAKE_BUILD_TYPE=Debug \
           -DENABLE_DTRACE=OFF \
           -DWITH_EMBEDDED_SERVER=OFF \
           -DWITH_INNODB_MEMCACHED=ON \
           -DWITH_SSL=system -DWITH_PAM=ON \
           -DINSTALL_MYSQLSHAREDIR=share/percona-server \
           -DINSTALL_SUPPORTFILESDIR=share/percona-server \
           -DMYSQL_UNIX_ADDR="/var/lib/mysql/mysql.sock" \
           -DFEATURE_SET="%{feature_set}" \
           -DCOMPILATION_COMMENT="%{compilation_comment_debug}" %{TOKUDB_FLAGS} %{TOKUDB_DEBUG_ON}

  echo BEGIN_DEBUG_CONFIG ; egrep '^#define' include/config.h ; echo END_DEBUG_CONFIG
  make %{?_smp_mflags}
)
# Build full release
mkdir release
(
  cd release
  # XXX: MYSQL_UNIX_ADDR should be in cmake/* but mysql_version is included before
  # XXX: install_layout so we can't just set it based on INSTALL_LAYOUT=RPM
  ${CMAKE} ../ -DBUILD_CONFIG=mysql_release -DINSTALL_LAYOUT=RPM \
           -DCMAKE_BUILD_TYPE=RelWithDebInfo \
           -DENABLE_DTRACE=OFF \
           -DWITH_EMBEDDED_SERVER=OFF \
           -DWITH_INNODB_MEMCACHED=ON \
           -DWITH_SSL=system -DWITH_PAM=ON \
           -DINSTALL_MYSQLSHAREDIR=share/percona-server \
           -DINSTALL_SUPPORTFILESDIR=share/percona-server \
           -DMYSQL_UNIX_ADDR="/var/lib/mysql/mysql.sock" \
           -DFEATURE_SET="%{feature_set}" \
           -DCOMPILATION_COMMENT="%{compilation_comment_release}" %{TOKUDB_FLAGS} %{TOKUDB_DEBUG_OFF}
  
  echo BEGIN_NORMAL_CONFIG ; egrep '^#define' include/config.h ; echo END_NORMAL_CONFIG
  make %{?_smp_mflags}
)

# For the debuginfo extraction stage, some source files are not located in the release
# and debug dirs, but in the source dir. Make a link there to avoid errors in the
# strip phase.
for d in debug release
do
    for f in pars/lexyy.cc pars/pars0grm.cc pars/pars0grm.y pars/pars0lex.l \
        fts/fts0pars.cc fts/fts0pars.y fts/fts0blex.l fts/fts0blex.cc \
        include/fts0pars.h fts/fts0tlex.cc fts/fts0tlex.l
    do
        ln -s "../../../%{src_dir}/storage/innobase/$f" "$d/storage/innobase/"
    done
    mkdir -p "$d/storage/include/"
    ln -s "../../../%{src_dir}/storage/innobase/include/fts0tlex.h" \
            "$d/storage/include/"
    ln -s "../../../%{src_dir}/storage/innobase/include/fts0blex.h" \
            "$d/storage/include/"
done

# Use the build root for temporary storage of the shared libraries.
RBR=$RPM_BUILD_ROOT

# Clean up the BuildRoot first
[ "$RBR" != "/" ] && [ -d "$RBR" ] && rm -rf "$RBR";

##############################################################################
%install

RBR=$RPM_BUILD_ROOT
MBD=$RPM_BUILD_DIR/%{src_dir}

# Ensure that needed directories exists
install -d $RBR%{_sysconfdir}/{logrotate.d,init.d}
install -d $RBR%{mysqldatadir}/mysql
install -d $RBR%{_datadir}/mysql-test
install -d $RBR%{_datadir}/percona-server/SELinux/RHEL4
install -d $RBR%{_includedir}
install -d $RBR%{_libdir}
install -d $RBR%{_mandir}
install -d $RBR%{_sbindir}
install -d $RBR%{_libdir}/mysql/plugin

# SElinux
pushd ${MBD}/policy
make -f /usr/share/selinux/devel/Makefile
install -D -m 0644 $MBD/policy/percona-server.pp $RBR%{_datadir}/selinux/packages/percona-server/percona-server.pp
popd
# SElinux END

(
  cd $MBD/release
  make DESTDIR=$RBR benchdir_root=%{_datadir} install
)

# Install all binaries
(
  cd $MBD/release
  make DESTDIR=$RBR install
)

# FIXME: at some point we should stop doing this and just install everything
# FIXME: directly into %{_libdir}/mysql - perhaps at the same time as renaming
# FIXME: the shared libraries to use libmysql*-$major.$minor.so syntax
mv -v $RBR/%{_libdir}/*.a $RBR/%{_libdir}/mysql/

# Install logrotate and autostart
install -m 644 $MBD/release/support-files/mysql-log-rotate $RBR%{_sysconfdir}/logrotate.d/mysql
%if 0%{?systemd}
install -D -m 0755 $MBD/build-ps/rpm/mysql-systemd $RBR%{_bindir}/mysql-systemd
install -D -m 0644 $MBD/build-ps/rpm/mysqld.service $RBR%{_unitdir}/mysqld.service
install -D -m 0644 $MBD/build-ps/rpm/mysql.conf $RBR%{_tmpfilesdir}/mysql.conf
%else
install -m 755 $MBD/release/support-files/mysql.server $RBR%{_sysconfdir}/init.d/mysql
%endif

%if "%rhel" > "6"
install -D -m 0644 $MBD/build-ps/rpm/my.cnf $RBR%{_sysconfdir}/my.cnf
%endif

#
%{__rm} -f $RBR/%{_prefix}/README
%if %{with tokudb}
%{__rm} -f $RBR/%{_prefix}/README.md
%{__rm} -f $RBR/%{_prefix}/COPYING.AGPLv3
%{__rm} -f $RBR/%{_prefix}/COPYING.GPLv2
%{__rm} -f $RBR/%{_prefix}/PATENTS
%endif
#
# Delete the symlinks to the libraries from the libdir. These are created by
# ldconfig(8) afterwards.
rm -f $RBR%{_libdir}/libmysqlclient*.so.18

# Create a symlink "rcmysql", pointing to the init.script. SuSE users
# will appreciate that, as all services usually offer this.
ln -s %{_sysconfdir}/init.d/mysql $RBR%{_sbindir}/rcmysql

# Touch the place where the my.cnf config file might be located
# Just to make sure it's in the file list and marked as a config file
touch $RBR%{_sysconfdir}/my.cnf

# Install SELinux files in datadir
install -m 600 $MBD/support-files/RHEL4-SElinux/mysql.{fc,te} \
  $RBR%{_datadir}/percona-server/SELinux/RHEL4

%if %{WITH_TCMALLOC}
# Even though this is a shared library, put it under /usr/lib*/mysql, so it
# doesn't conflict with possible shared lib by the same name in /usr/lib*.  See
# `mysql_config --variable=pkglibdir` and mysqld_safe for how this is used.
install -m 644 "%{malloc_lib_source}" \
  "$RBR%{_libdir}/mysql/%{malloc_lib_target}"
%endif

# Remove files we explicitly do not want to package, avoids 'unpackaged
# files' warning.
rm -f $RBR%{_mandir}/man1/make_win_bin_dist.1*
%if 0%{?systemd}
rm -rf $RBR%{_sysconfdir}/init.d/mysql
%endif
# Not needed if TokuDB package is not created
%if ! %{with tokudb}
rm -rf $RBR%{_bindir}/ps_tokudb_admin
%endif

##############################################################################
#  Post processing actions, i.e. when installed
##############################################################################

%pre -n Percona-Server-server%{product_suffix}

# ATTENTION: Parts of this are duplicated in the "triggerpostun" !

# There are users who deviate from the default file system layout.
# Check local settings to support them.
if [ -x %{_bindir}/my_print_defaults ]
then
  mysql_datadir=`%{_bindir}/my_print_defaults server mysqld | grep '^--datadir=' | sed -n 's/--datadir=//p' | tail -n 1`
  PID_FILE_PATT=`%{_bindir}/my_print_defaults server mysqld | grep '^--pid-file=' | sed -n 's/--pid-file=//p'`
fi
if [ -z "$mysql_datadir" ]
then
  mysql_datadir=%{mysqldatadir}
fi
if [ -z "$PID_FILE_PATT" ]
then
  PID_FILE_PATT="$mysql_datadir/*.pid"
fi

# Check if we can safely upgrade.  An upgrade is only safe if it's from one
# of our RPMs in the same version family.

installed=`rpm -q --whatprovides mysql-server 2> /dev/null`
if [ $? -eq 0 -a -n "$installed" ]; then
  vendor=`rpm -q --queryformat='%{VENDOR}' "$installed" 2>&1`
  version=`rpm -q --queryformat='%{VERSION}' "$installed" 2>&1`
  myoldvendor='%{mysql_old_vendor}'
  myvendor_2='%{mysql_vendor_2}'
  myvendor='%{mysql_vendor}'
  perconaservervendor='%{percona_server_vendor}'
  myversion='%{mysql_version}'

  old_family=`echo $version \
    | sed -n -e 's,^\([1-9][0-9]*\.[0-9][0-9]*\)\..*$,\1,p'`
  new_family=`echo $myversion \
    | sed -n -e 's,^\([1-9][0-9]*\.[0-9][0-9]*\)\..*$,\1,p'`

  [ -z "$vendor" ] && vendor='<unknown>'
  [ -z "$old_family" ] && old_family="<unrecognized version $version>"
  [ -z "$new_family" ] && new_family="<bad package specification: version $myversion>"

  error_text=
  if [ "$vendor" != "$myoldvendor" \
    -a "$vendor" != "$myvendor_2" \
    -a "$vendor" != "$myvendor" \
    -a "$vendor" != "$perconaservervendor" ]; then
    error_text="$error_text
The current MySQL server package is provided by a different
vendor ($vendor) than $myoldvendor, $myvendor_2,
$myvendor, or $perconaservervendor.
Some files may be installed to different locations, including log
files and the service startup script in %{_sysconfdir}/init.d/.
"
  fi

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
A MySQL server package ($installed) is installed.
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
  library will be reinstalled by the MySQL-shared-compat package.
- Install the new MySQL packages supplied by $myvendor
- Ensure that the MySQL server is started
- Run the 'mysql_upgrade' program

This is a brief description of the upgrade process.  Important details
can be found in the MySQL manual, in the Upgrading section.
******************************************************************
HERE
    exit 1
  fi
fi

# We assume that if there is exactly one ".pid" file,
# it contains the valid PID of a running MySQL server.
NR_PID_FILES=`ls -1 $PID_FILE_PATT 2>/dev/null | wc -l`
case $NR_PID_FILES in
	0 ) SERVER_TO_START=''  ;;  # No "*.pid" file == no running server
	1 ) SERVER_TO_START='true' ;;
	* ) SERVER_TO_START=''      # Situation not clear
	    SEVERAL_PID_FILES=true ;;
esac
# That logic may be debated: We might check whether it is non-empty,
# contains exactly one number (possibly a PID), and whether "ps" finds it.
# OTOH, if there is no such process, it means a crash without a cleanup -
# is that a reason not to start a new server after upgrade?

STATUS_FILE=$mysql_datadir/RPM_UPGRADE_MARKER

if [ -f $STATUS_FILE ]; then
	echo "Some previous upgrade was not finished:"
	ls -ld $STATUS_FILE
	echo "Please check its status, then do"
	echo "    rm $STATUS_FILE"
	echo "before repeating the MySQL upgrade."
	exit 1
elif [ -n "$SEVERAL_PID_FILES" ] ; then
	echo "You have more than one PID file:"
	ls -ld $PID_FILE_PATT
	echo "Please check which one (if any) corresponds to a running server"
	echo "and delete all others before repeating the MySQL upgrade."
	exit 1
fi

NEW_VERSION=%{mysql_version}-%{release}

# The "pre" section code is also run on a first installation,
# when there  is no data directory yet. Protect against error messages.
if [ -d $mysql_datadir ] ; then
	echo "MySQL RPM upgrade to version $NEW_VERSION"  > $STATUS_FILE
	echo "'pre' step running at `date`"          >> $STATUS_FILE
	echo                                         >> $STATUS_FILE
	fcount=`ls -ltr $mysql_datadir/*.err 2>/dev/null | wc -l`
	if [ $fcount -gt 0 ] ; then
		echo "ERR file(s):"                          >> $STATUS_FILE
		ls -ltr $mysql_datadir/*.err                 >> $STATUS_FILE
		echo                                         >> $STATUS_FILE
		echo "Latest 'Version' line in latest file:" >> $STATUS_FILE
		grep '^Version' `ls -tr $mysql_datadir/*.err | tail -1` | \
			tail -1                              >> $STATUS_FILE
		echo                                         >> $STATUS_FILE
	fi

	if [ -n "$SERVER_TO_START" ] ; then
		# There is only one PID file, race possibility ignored
		echo "PID file:"                           >> $STATUS_FILE
		ls -l   $PID_FILE_PATT                     >> $STATUS_FILE
		cat     $PID_FILE_PATT                     >> $STATUS_FILE
		echo                                       >> $STATUS_FILE
		echo "Server process:"                     >> $STATUS_FILE
		ps -fp `cat $PID_FILE_PATT`                >> $STATUS_FILE
		echo                                       >> $STATUS_FILE
		echo "SERVER_TO_START=$SERVER_TO_START"    >> $STATUS_FILE
	else
		# Take a note we checked it ...
		echo "PID file:"                           >> $STATUS_FILE
		ls -l   $PID_FILE_PATT                     >> $STATUS_FILE 2>&1
	fi
fi

# Shut down a previously installed server first
# Note we *could* make that depend on $SERVER_TO_START, but we rather don't,
# so a "stop" is attempted even if there is no PID file.
# (Maybe the "stop" doesn't work then, but we might fix that in itself.)
if [ -x %{_sysconfdir}/init.d/mysql ] ; then
        %{_sysconfdir}/init.d/mysql stop > /dev/null 2>&1
        echo "Giving mysqld 5 seconds to exit nicely"
        sleep 5
fi

# SElinux
%post -n Percona-Server-selinux%{product_suffix}
/usr/sbin/semodule -i %{_datadir}/selinux/packages/percona-server/percona-server.pp >/dev/null 2>&1 || :

%postun -n Percona-Server-selinux%{product_suffix}
if [ $1 -eq 0 ] ; then
    /usr/sbin/semodule -r percona-server >/dev/null 2>&1 || :
fi

#SElinux

%post -n Percona-Server-server%{product_suffix}

%if 0%{?systemd}
  %systemd_post mysqld
%endif

# ATTENTION: Parts of this are duplicated in the "triggerpostun" !

# There are users who deviate from the default file system layout.
# Check local settings to support them.
if [ -x %{_bindir}/my_print_defaults ]
then
  mysql_datadir=`%{_bindir}/my_print_defaults server mysqld | grep '^--datadir=' | sed -n 's/--datadir=//p' | tail -n 1`
fi
if [ -z "$mysql_datadir" ]
then
  mysql_datadir=%{mysqldatadir}
fi
NEW_VERSION=%{mysql_version}-%{release}
STATUS_FILE=$mysql_datadir/RPM_UPGRADE_MARKER

if [ -f $STATUS_FILE ] ; then
	SERVER_TO_START=`grep '^SERVER_TO_START=' $STATUS_FILE | cut -c17-`
else
	SERVER_TO_START=''
fi

# ----------------------------------------------------------------------
# Create data directory if needed, check whether upgrade or install
# ----------------------------------------------------------------------
if [ ! -d $mysql_datadir ] ; then mkdir -m 755 $mysql_datadir; fi
# echo "Analyzed: SERVER_TO_START=$SERVER_TO_START"
if [ ! -d $mysql_datadir/mysql ] ; then
	echo "MySQL RPM installation of version $NEW_VERSION" >> $STATUS_FILE
else
	# If the directory exists, we may assume it is an upgrade.
	echo "MySQL RPM upgrade to version $NEW_VERSION" >> $STATUS_FILE
fi

# ----------------------------------------------------------------------
# Make MySQL start/shutdown automatically when the machine does it.
# ----------------------------------------------------------------------
# NOTE: This still needs to be debated. Should we check whether these links
# for the other run levels exist(ed) before the upgrade?
# use chkconfig on Enterprise Linux and newer SuSE releases
%if 0%{?systemd}
if [ -x %{_bindir}/systemctl ] ; then
	%{_bindir}/systemctl enable mysqld >/dev/null 2>&1
fi
%else
if [ -x /sbin/chkconfig ] ; then
        /sbin/chkconfig --add mysql
# use insserv for older SuSE Linux versions
elif [ -x /sbin/insserv ] ; then
        /sbin/insserv %{_sysconfdir}/init.d/mysql
fi
%endif

# ----------------------------------------------------------------------
# Create a MySQL user and group. Do not report any problems if it already
# exists.
# ----------------------------------------------------------------------
groupadd -r %{mysqld_group} 2> /dev/null || true
useradd -M -r -d $mysql_datadir -s /bin/bash -c "MySQL server" \
  -g %{mysqld_group} %{mysqld_user} 2> /dev/null || true
# The user may already exist, make sure it has the proper group nevertheless
# (BUG#12823)
usermod -g %{mysqld_group} %{mysqld_user} 2> /dev/null || true

# ----------------------------------------------------------------------
# Change permissions so that the user that will run the MySQL daemon
# owns all database files.
# ----------------------------------------------------------------------
chown -R %{mysqld_user}:%{mysqld_group} $mysql_datadir

# ----------------------------------------------------------------------
# Initiate databases if needed
# ----------------------------------------------------------------------
    # Does $mysql_datadir/mysql exist? In this case, this is probably an
    # upgrade from a previous version or a reinstall. It's best not to
    # call mysql_install_db in this case since the test db would be
    # possibly recreated (bug #1169522).
    if test ! -e $mysql_datadir/mysql
    then
        %{_bindir}/mysql_install_db --rpm --user=%{mysqld_user} \
            --datadir=$mysql_datadir
    fi

%if 0%{?systemd}
  %tmpfiles_create mysql.conf
%endif

# ----------------------------------------------------------------------
# Upgrade databases if needed would go here - but it cannot be automated yet
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

# For systemd check postun
%if 0%{?systemd} == 0
# Was the server running before the upgrade? If so, restart the new one.
# Don't start it if TokuDB package is installed - it will be started
# after TokuDB package is upgraded - prevents TokuDB init error
tokudb_installed=`rpm -q Percona-Server-tokudb-56 2>/dev/null`
if [ $? -eq 1 -a "$SERVER_TO_START" = "true" ] ; then
	# Restart in the same way that mysqld will be started normally.
	if [ -x %{_sysconfdir}/init.d/mysql ] ; then
		%{_sysconfdir}/init.d/mysql start
		echo "Giving mysqld 5 seconds to start"
		sleep 5
	fi
fi
%endif

echo "Percona Server is distributed with several useful UDF (User Defined Function) from Percona Toolkit."
echo "Run the following commands to create these functions:"
echo "mysql -e \"CREATE FUNCTION fnv1a_64 RETURNS INTEGER SONAME 'libfnv1a_udf.so'\""
echo "mysql -e \"CREATE FUNCTION fnv_64 RETURNS INTEGER SONAME 'libfnv_udf.so'\""
echo "mysql -e \"CREATE FUNCTION murmur_hash RETURNS INTEGER SONAME 'libmurmur_udf.so'\""
echo "See http://www.percona.com/doc/percona-server/5.6/management/udf_percona_toolkit.html for more details"

# Collect an upgrade history ...
echo "Upgrade/install finished at `date`"        >> $STATUS_FILE
echo                                             >> $STATUS_FILE
echo "====="                                     >> $STATUS_FILE
STATUS_HISTORY=$mysql_datadir/RPM_UPGRADE_HISTORY
cat $STATUS_FILE >> $STATUS_HISTORY
mv -f  $STATUS_FILE ${STATUS_FILE}-LAST  # for "triggerpostun" and TokuDB package


#echo "Thank you for installing the MySQL Community Server! For Production
#systems, we recommend MySQL Enterprise, which contains enterprise-ready
#software, intelligent advisory services, and full production support with
#scheduled service packs and more.  Visit www.mysql.com/enterprise for more
#information."

%preun -n Percona-Server-server%{product_suffix}

# Which '$1' does this refer to?  Fedora docs have info:
# " ... a count of the number of versions of the package that are installed.
#   Action                           Count
#   Install the first time           1
#   Upgrade                          2 or higher (depending on the number of versions installed)
#   Remove last version of package   0 "
#
#  http://docs.fedoraproject.org/en-US/Fedora_Draft_Documentation/0.1/html/RPM_Guide/ch09s04s05.html

if [ $1 = 0 ] ; then
%if 0%{?systemd}
	%systemd_preun mysqld
%else
       # Stop MySQL before uninstalling it
        if [ -x %{_sysconfdir}/init.d/mysql ] ; then
                %{_sysconfdir}/init.d/mysql stop > /dev/null
                # Remove autostart of MySQL
                # use chkconfig on Enterprise Linux and newer SuSE releases
                if [ -x /sbin/chkconfig ] ; then
                        /sbin/chkconfig --del mysql
                # For older SuSE Linux versions
                elif [ -x /sbin/insserv ] ; then
                        /sbin/insserv -r %{_sysconfdir}/init.d/mysql
                fi
        fi
%endif
fi

# We do not remove the mysql user since it may still own a lot of
# database files.

%triggerpostun -n Percona-Server-server%{product_suffix} --MySQL-server-community

# Setup: We renamed this package, so any existing "server-community"
#   package will be removed when this "server" is installed.
# Problem: RPM will first run the "pre" and "post" sections of this script,
#   and only then the "preun" of that old community server.
#   But this "preun" includes stopping the server and uninstalling the service,
#   "chkconfig --del mysql" which removes the symlinks to the start script.
# Solution: *After* the community server got removed, restart this server
#   and re-install the service.
#
# For information about triggers in spec files, see the Fedora docs:
#   http://docs.fedoraproject.org/en-US/Fedora_Draft_Documentation/0.1/html/RPM_Guide/ch10s02.html
# For all details of this code, see the "pre" and "post" sections.

# There are users who deviate from the default file system layout.
# Check local settings to support them.
if [ -x %{_bindir}/my_print_defaults ]
then
  mysql_datadir=`%{_bindir}/my_print_defaults server mysqld | grep '^--datadir=' | sed -n 's/--datadir=//p' | tail -n 1`
fi
if [ -z "$mysql_datadir" ]
then
  mysql_datadir=%{mysqldatadir}
fi
NEW_VERSION=%{mysql_version}-%{release}
STATUS_FILE=$mysql_datadir/RPM_UPGRADE_MARKER-LAST  # Note the difference!
STATUS_HISTORY=$mysql_datadir/RPM_UPGRADE_HISTORY

if [ -f $STATUS_FILE ] ; then
	SERVER_TO_START=`grep '^SERVER_TO_START=' $STATUS_FILE | cut -c17-`
else
	# This should never happen, but let's be prepared
	SERVER_TO_START=''
fi
echo "Analyzed: SERVER_TO_START=$SERVER_TO_START"

%if 0%{?systemd}
if [ -x %{_bindir}/systemctl ] ; then
	%{_bindir}/systemctl enable mysqld >/dev/null 2>&1
fi
%else
if [ -x /sbin/chkconfig ] ; then
        /sbin/chkconfig --add mysql
# use insserv for older SuSE Linux versions
elif [ -x /sbin/insserv ] ; then
        /sbin/insserv %{_sysconfdir}/init.d/mysql
fi
%endif

# Was the server running before the upgrade? If so, restart the new one.
if [ "$SERVER_TO_START" = "true" ] ; then
# Restart in the same way that mysqld will be started normally.
%if 0%{?systemd}
	if [ -x %{_bindir}/systemctl ] ; then 
		%{_bindir}/systemctl start mysqld
		echo "Giving mysqld 5 seconds to start"
		sleep 5
	fi
%else
	if [ -x %{_sysconfdir}/init.d/mysql ] ; then
		%{_sysconfdir}/init.d/mysql start
		echo "Giving mysqld 5 seconds to start"
		sleep 5
	fi
%endif
fi

echo "Trigger 'postun --community' finished at `date`"        >> $STATUS_HISTORY
echo                                             >> $STATUS_HISTORY
echo "====="                                     >> $STATUS_HISTORY


%if %{with tokudb}
# ----------------------------------------------------------------------------
%post -n Percona-Server-tokudb%{product_suffix}

if [ $1 -eq 1 ] ; then
  echo -e "\n\n * This release of Percona Server is distributed with TokuDB storage engine."
  echo -e " * Run the following script to enable the TokuDB storage engine in Percona Server:\n"
  echo -e "\tps_tokudb_admin --enable -u <mysql_admin_user> -p[mysql_admin_pass] [-S <socket>] [-h <host> -P <port>]\n"
  echo -e " * See http://www.percona.com/doc/percona-server/5.6/tokudb/tokudb_installation.html for more installation details\n"
  echo -e " * See http://www.percona.com/doc/percona-server/5.6/tokudb/tokudb_intro.html for an introduction to TokuDB\n\n"
fi
# If upgrade is in question and the server was started before upgrade we need to start it
# after upgrading TokuDB package and not before because TokuDB will fail on init
%if 0%{?systemd} == 0
if [ $1 -eq 2 ]; then
	# There are users who deviate from the default file system layout.
	# Check local settings to support them.
	if [ -x %{_bindir}/my_print_defaults ]
	then
	  mysql_datadir=`%{_bindir}/my_print_defaults server mysqld | grep '^--datadir=' | sed -n 's/--datadir=//p' | tail -n 1`
	fi
	if [ -z "$mysql_datadir" ]
	then
	  mysql_datadir=%{mysqldatadir}
	fi

	STATUS_FILE=$mysql_datadir/RPM_UPGRADE_MARKER-LAST

	if [ -f $STATUS_FILE ] ; then
	  SERVER_TO_START=`grep '^SERVER_TO_START=' $STATUS_FILE | cut -c17-`
	else
	  SERVER_TO_START=''
	fi

	# Was the server running before the upgrade? If so, restart the new one.
	if [ "$SERVER_TO_START" = "true" ] ; then 
	  # Restart in the same way that mysqld will be started normally.
	  if [ -x %{_sysconfdir}/init.d/mysql ] ; then 
	    %{_sysconfdir}/init.d/mysql start
	    echo "Giving mysqld 5 seconds to start"
	    sleep 5
	  fi
	fi
fi
%endif
# ----------------------------------------------------------------------------
%endif

%postun -n Percona-Server-server%{product_suffix}
%if 0%{?systemd}
%systemd_postun_with_restart mysqld
%endif

# ----------------------------------------------------------------------
# Clean up the BuildRoot after build is done
# ----------------------------------------------------------------------
%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] \
  && rm -rf $RPM_BUILD_ROOT;

##############################################################################
#  Files section
##############################################################################

%files -n Percona-Server-selinux%{product_suffix}
%dir %attr(755, root, root) %{_datadir}/selinux/packages/percona-server
%attr(644, root, root) %{_datadir}/selinux/packages/percona-server/percona-server.pp

%files -n Percona-Server-server%{product_suffix}
%defattr(-,root,root,0755)

%if %{defined license_files_server}
%doc %{license_files_server}
%endif
%doc release/Docs/INFO_SRC
%doc release/Docs/INFO_BIN
%doc release/support-files/my-*.cnf

%doc %attr(644, root, root) %{_infodir}/mysql.info*

%doc %attr(644, root, man) %{_mandir}/man1/innochecksum.1*
%doc %attr(644, root, man) %{_mandir}/man1/my_print_defaults.1*
%doc %attr(644, root, man) %{_mandir}/man1/myisam_ftdump.1*
%doc %attr(644, root, man) %{_mandir}/man1/myisamchk.1*
%doc %attr(644, root, man) %{_mandir}/man1/myisamlog.1*
%doc %attr(644, root, man) %{_mandir}/man1/myisampack.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_convert_table_format.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_fix_extensions.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqld_multi.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqld_safe.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqldumpslow.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_install_db.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_secure_installation.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_setpermission.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_upgrade.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlhotcopy.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlman.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql.server.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqltest.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_tzinfo_to_sql.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_zap.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqlbug.1*
%doc %attr(644, root, man) %{_mandir}/man1/perror.1*
%doc %attr(644, root, man) %{_mandir}/man1/replace.1*
%doc %attr(644, root, man) %{_mandir}/man1/resolve_stack_dump.1*
%doc %attr(644, root, man) %{_mandir}/man1/resolveip.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_plugin.1*
%doc %attr(644, root, man) %{_mandir}/man8/mysqld.8*

%if "%rhel" < "7"
%ghost %config(noreplace,missingok) %{_sysconfdir}/my.cnf
%endif

%attr(755, root, root) %{_bindir}/innochecksum
%attr(755, root, root) %{_bindir}/my_print_defaults
%attr(755, root, root) %{_bindir}/myisam_ftdump
%attr(755, root, root) %{_bindir}/myisamchk
%attr(755, root, root) %{_bindir}/myisamlog
%attr(755, root, root) %{_bindir}/myisampack
%attr(755, root, root) %{_bindir}/mysql_convert_table_format
%attr(755, root, root) %{_bindir}/mysql_fix_extensions
%attr(755, root, root) %{_bindir}/mysql_install_db
%attr(755, root, root) %{_bindir}/mysql_secure_installation
%attr(755, root, root) %{_bindir}/mysql_setpermission
%attr(755, root, root) %{_bindir}/mysql_tzinfo_to_sql
%attr(755, root, root) %{_bindir}/mysql_upgrade
%attr(755, root, root) %{_bindir}/mysql_plugin
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
%if 0%{?systemd}
%attr(755, root, root) %{_bindir}/mysql-systemd
%endif

%attr(755, root, root) %{_sbindir}/mysqld
%attr(755, root, root) %{_sbindir}/mysqld-debug
%attr(755, root, root) %{_sbindir}/rcmysql
%attr(644, root, root) %{_libdir}/mysql/plugin/daemon_example.ini

#plugins
%attr(755, root, root) %{_libdir}/mysql/plugin/adt_null.so
%attr(755, root, root) %{_libdir}/mysql/plugin/auth.so
%attr(755, root, root) %{_libdir}/mysql/plugin/auth_socket.so
%attr(755, root, root) %{_libdir}/mysql/plugin/auth_test_plugin.so
%attr(755, root, root) %{_libdir}/mysql/plugin/innodb_engine.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libdaemon_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libfnv1a_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/libfnv_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/libmemcached.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libmurmur_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/mypluglib.so
%attr(755, root, root) %{_libdir}/mysql/plugin/qa_auth_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/qa_auth_interface.so
%attr(755, root, root) %{_libdir}/mysql/plugin/qa_auth_server.so
%attr(755, root, root) %{_libdir}/mysql/plugin/semisync_master.so
%attr(755, root, root) %{_libdir}/mysql/plugin/semisync_slave.so
%attr(755, root, root) %{_libdir}/mysql/plugin/validate_password.so
%attr(755, root, root) %{_libdir}/mysql/plugin/auth_pam.so
%attr(755, root, root) %{_libdir}/mysql/plugin/auth_pam_compat.so
%attr(755, root, root) %{_libdir}/mysql/plugin/dialog.so
%attr(755, root, root) %{_libdir}/mysql/plugin/handlersocket.so
%attr(755, root, root) %{_libdir}/mysql/plugin/query_response_time.so
%attr(755, root, root) %{_libdir}/mysql/plugin/mysql_no_login.so

# %attr(755, root, root) %{_libdir}/mysql/plugin/debug/*.so*
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/adt_null.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/auth.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/auth_pam.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/auth_pam_compat.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/auth_socket.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/auth_test_plugin.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/dialog.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/innodb_engine.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libdaemon_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libfnv1a_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libfnv_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libmemcached.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libmurmur_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/mypluglib.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/qa_auth_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/qa_auth_interface.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/qa_auth_server.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/semisync_master.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/semisync_slave.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/validate_password.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/handlersocket.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/query_response_time.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/mysql_no_login.so
# Audit Log and Scalability Metrics files
%attr(755, root, root) %{_libdir}/mysql/plugin/audit_log.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/audit_log.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/scalability_metrics.so
%attr(755, root, root) %{_libdir}/mysql/plugin/scalability_metrics.so

%if %{WITH_TCMALLOC}
%attr(755, root, root) %{_libdir}/mysql/%{malloc_lib_target}
%endif

%attr(644, root, root) %config(noreplace,missingok) %{_sysconfdir}/logrotate.d/mysql
%if 0%{?systemd}
%attr(755, root, root) %{_bindir}/mysql-systemd
%attr(644, root, root) %{_unitdir}/mysqld.service
%attr(644, root, root) %{_tmpfilesdir}/mysql.conf
%else
%attr(755, root, root) %{_sysconfdir}/init.d/mysql
%endif

# %attr(755, root, root) %{_datadir}/percona-server/
%attr(755, root, root) %{_datadir}/percona-server/binary-configure
%attr(755, root, root) %{_datadir}/percona-server/bulgarian
%attr(755, root, root) %{_datadir}/percona-server/charsets
%attr(755, root, root) %{_datadir}/percona-server/czech
%attr(755, root, root) %{_datadir}/percona-server/danish
%attr(755, root, root) %{_datadir}/percona-server/dictionary.txt
%attr(755, root, root) %{_datadir}/percona-server/dutch
%attr(755, root, root) %{_datadir}/percona-server/english
%attr(755, root, root) %{_datadir}/percona-server/errmsg-utf8.txt
%attr(755, root, root) %{_datadir}/percona-server/estonian
%attr(755, root, root) %{_datadir}/percona-server/fill_help_tables.sql
%attr(755, root, root) %{_datadir}/percona-server/french
%attr(755, root, root) %{_datadir}/percona-server/german
%attr(755, root, root) %{_datadir}/percona-server/greek
%attr(755, root, root) %{_datadir}/percona-server/hungarian
%attr(755, root, root) %{_datadir}/percona-server/innodb_memcached_config.sql
%attr(755, root, root) %{_datadir}/percona-server/italian
%attr(755, root, root) %{_datadir}/percona-server/japanese
%attr(755, root, root) %{_datadir}/percona-server/korean
%attr(755, root, root) %{_datadir}/percona-server/magic
%attr(755, root, root) %{_datadir}/percona-server/my-default.cnf
%attr(755, root, root) %{_datadir}/percona-server/mysqld_multi.server
%attr(755, root, root) %{_datadir}/percona-server/mysql-log-rotate
%attr(755, root, root) %{_datadir}/percona-server/mysql_security_commands.sql
%attr(755, root, root) %{_datadir}/percona-server/mysql.server
%attr(755, root, root) %{_datadir}/percona-server/mysql_system_tables_data.sql
%attr(755, root, root) %{_datadir}/percona-server/mysql_system_tables.sql
%attr(755, root, root) %{_datadir}/percona-server/mysql_test_data_timezone.sql
%attr(755, root, root) %{_datadir}/percona-server/norwegian
%attr(755, root, root) %{_datadir}/percona-server/norwegian-ny
%attr(755, root, root) %{_datadir}/percona-server/polish
%attr(755, root, root) %{_datadir}/percona-server/portuguese
%attr(755, root, root) %{_datadir}/percona-server/romanian
%attr(755, root, root) %{_datadir}/percona-server/russian
%attr(755, root, root) %{_datadir}/percona-server/SELinux
%attr(755, root, root) %{_datadir}/percona-server/serbian
%attr(755, root, root) %{_datadir}/percona-server/slovak
%attr(755, root, root) %{_datadir}/percona-server/spanish
%attr(755, root, root) %{_datadir}/percona-server/swedish
%attr(755, root, root) %{_datadir}/percona-server/ukrainian

# ----------------------------------------------------------------------------
%files -n Percona-Server-client%{product_suffix}

%defattr(-, root, root, 0755)
%attr(755, root, root) %{_bindir}/msql2mysql
%attr(755, root, root) %{_bindir}/mysql
%attr(755, root, root) %{_bindir}/mysql_find_rows
%attr(755, root, root) %{_bindir}/mysql_waitpid
%attr(755, root, root) %{_bindir}/mysqlaccess
# XXX: This should be moved to %{_sysconfdir}
%attr(644, root, root) %{_bindir}/mysqlaccess.conf
%attr(755, root, root) %{_bindir}/mysqladmin
%attr(755, root, root) %{_bindir}/mysqlbinlog
%attr(755, root, root) %{_bindir}/mysqlcheck
%attr(755, root, root) %{_bindir}/mysqldump
%attr(755, root, root) %{_bindir}/mysqlimport
%attr(755, root, root) %{_bindir}/mysqlshow
%attr(755, root, root) %{_bindir}/mysqlslap
%attr(755, root, root) %{_bindir}/mysql_config_editor

%doc %attr(644, root, man) %{_mandir}/man1/msql2mysql.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql.1*
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
%doc %attr(644, root, man) %{_mandir}/man1/mysql_config_editor.1*

# ----------------------------------------------------------------------------
%files -n Percona-Server-devel%{product_suffix} -f optional-files-devel
%defattr(-, root, root, 0755)
%doc %attr(644, root, man) %{_mandir}/man1/comp_err.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_config.1*
%attr(755, root, root) %{_bindir}/mysql_config
%dir %attr(755, root, root) %{_includedir}/mysql
%dir %attr(755, root, root) %{_libdir}/mysql
%{_includedir}/mysql/*
%{_datadir}/aclocal/mysql.m4
%{_libdir}/mysql/%{shared_lib_pri_name}.a
%{_libdir}/mysql/%{shared_lib_pri_name}_r.a
%{_libdir}/mysql/libmysqlservices.a
%{_libdir}/%{shared_lib_pri_name}.so
%{_libdir}/%{shared_lib_pri_name}_r.so

%post -n Percona-Server-devel%{product_suffix}
# For compatibility after reverting name to libmysql
for lib in %{shared_lib_sec_name}{.a,_r.a}; do
if [ ! -f %{_libdir}/mysql/$lib ]; then
	ln -s %{shared_lib_pri_name}.a %{_libdir}/mysql/$lib;
fi
done

%postun -n Percona-Server-devel%{product_suffix}
# Cleanup of symlinks after uninstall
for lib in %{shared_lib_sec_name}{.a,_r.a}; do
if [ -h %{_libdir}/mysql/$lib ]; then
	rm -f %{_libdir}/mysql/$lib;
fi
done

# ----------------------------------------------------------------------------
%if %{with tokudb}
%files -n Percona-Server-tokudb%{product_suffix}
%attr(-, root, root) 
%{_bindir}/tokuftdump
%{_libdir}/mysql/plugin/ha_tokudb.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/ha_tokudb.so
%attr(755, root, root) %{_bindir}/ps_tokudb_admin
%attr(755, root, root) %{_bindir}/tokuft_logprint
%attr(755, root, root) %{_libdir}/mysql/plugin/tokudb_backup.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/tokudb_backup.so
%attr(755, root, root) %{_libdir}/libHotBackup.so
%{_includedir}/backup.h
%doc storage/tokudb/PerconaFT/README.md
%doc storage/tokudb/PerconaFT/COPYING.AGPLv3
%doc storage/tokudb/PerconaFT/COPYING.GPLv2
%doc storage/tokudb/PerconaFT/PATENTS
%endif

# ----------------------------------------------------------------------------
%files -n Percona-Server-shared%{product_suffix}
%defattr(-, root, root, 0755)
# Shared libraries (omit for architectures that don't support them)
%{_libdir}/%{shared_lib_pri_name}*.so.*

%if "%rhel" > "6"
%attr(644, root, root) %config(noreplace) %{_sysconfdir}/my.cnf
%endif

%post -n Percona-Server-shared%{product_suffix}
%if "%rhel" > "6"
# Added for compatibility
for lib in %{shared_lib_pri_name}{.so.18,_r.so.18}; do
    if [ ! -f %{_libdir}/$lib ]; then
        ln -s %{shared_lib_pri_name}.so.18.1.0 %{_libdir}/$lib
    fi
done
%endif
# For compatibility between different names of library
for lib in %{shared_lib_sec_name}{.so.18.0.0,.so.18,_r.so.18.0.0,_r.so.18,.so,_r.so}; do
if [ ! -f %{_libdir}/$lib ]; then
        ln -s %{shared_lib_pri_name}.so.18 %{_libdir}/$lib;
fi
done
for lib in %{shared_lib_sec_name}{.so.18.1.0,_r.so.18.1.0}; do
if [ ! -f %{_libdir}/$lib ]; then
        ln -s %{shared_lib_pri_name}.so.18.1.0 %{_libdir}/$lib;
fi
done

/sbin/ldconfig

%postun -n Percona-Server-shared%{product_suffix}
# Cleanup of symlinks after uninstall
for lib in %{shared_lib_sec_name}{.so.18.0.0,.so.18,_r.so.18.0.0,_r.so.18,.so,_r.so,.so.18.1.0,_r.so.18.1.0}; do
if [ -h %{_libdir}/$lib ]; then
	rm -f %{_libdir}/$lib;
fi
done
%if "%rhel" > "6"
for lib in %{shared_lib_pri_name}{.so.18,_r.so.18}; do
if [ -h %{_libdir}/$lib ]; then
	rm -f %{_libdir}/$lib;
fi
done
%endif
/sbin/ldconfig

# ----------------------------------------------------------------------------
%files -n Percona-Server-test%{product_suffix}
%defattr(-, root, root, 0755)
%attr(-, root, root) %{_datadir}/mysql-test
%attr(755, root, root) %{_bindir}/mysql_client_test
#%attr(755, root, root) %{_bindir}/mysql_client_test_embedded
#%attr(755, root, root) %{_bindir}/mysqltest_embedded
%doc %attr(644, root, man) %{_mandir}/man1/mysql_client_test.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql-stress-test.pl.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql-test-run.pl.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysql_client_test_embedded.1*
%doc %attr(644, root, man) %{_mandir}/man1/mysqltest_embedded.1*

%changelog
* Thu Sep 10 2015 Tomislav Plavcic <tomislav.plavcic@percona.com>

- Included Percona-TokuBackup into TokuDB package

* Wed Jul 15 2015 Tomislav Plavcic <tomislav.plavcic@percona.com>

- Fixed symbol versioning for rhel7 in 5.6 rpm (bug1420691)

* Wed May 06 2015 Tomislav Plavcic <tomislav.plavcic@percona.com>

- mysql client is now built with readline (bug1266386)

* Fri Feb 27 2015 Tomislav Plavcic <tomislav.plavcic@percona.com>

- Fixed RPMs assumes that .pid file is located in datadir (bug1201896)
- Fixed errors in debug build when maintainer mode is on (bug1408232)

* Wed Feb 04 2015 Tomislav Plavcic <tomislav.plavcic@percona.com>

- Added ps_tokudb_admin script for TokuDB plugin installation (BLD-212)
- Fixed TokuDB engine fails after upgrade on centos 5/6 (bug1413956)

* Fri Jan 09 2015 Tomislav Plavcic <tomislav.plavcic@percona.com>

- Upgrading a running server does not restart the service (bug1311840)
- Set MYSQL_MAINTAINER_MODE=OFF for debug build (bug1408232)

* Thu Nov 20 2014 Tomislav Plavcic <tomislav.plavcic@percona.com>

- Fixed debug symbols on rhel5 (bug 1388972)

* Tue Aug 26 2014 Tomislav Plavcic <tomislav.plavcic@percona.com>

- Added support for centos7

* Mon May 26 2014 Tomislav Plavcic <tomislav.plavcic@percona.com>

- Added packaging changes regarding TokuDB

* Thu Feb 10 2011 Ignacio Nin <ignacio.nin@percona.com>

- Removed lines which prevented -debuginfo packages from being built.

* Tue Nov 23 2010 Jonathan Perkin <jonathan.perkin@oracle.com>

- EXCEPTIONS-CLIENT has been deleted, remove it from here too
- Support MYSQL_BUILD_MAKE_JFLAG environment variable for passing
  a '-j' argument to make.

* Mon Nov 1 2010 Georgi Kodinov <georgi.godinov@oracle.com>

- Added test authentication (WL#1054) plugin binaries

* Wed Oct 6 2010 Georgi Kodinov <georgi.godinov@oracle.com>

- Added example external authentication (WL#1054) plugin binaries

* Wed Aug 11 2010 Joerg Bruehe <joerg.bruehe@oracle.com>

- With a recent spec file cleanup, names have changed: A "-community" part was dropped.
  Reflect that in the "Obsoletes" specifications.
- Add a "triggerpostun" to handle the uninstall of the "-community" server RPM.
- This fixes bug#55015 "MySQL server is not restarted properly after RPM upgrade".

* Tue Jun 15 2010 Joerg Bruehe <joerg.bruehe@sun.com>

- Change the behaviour on installation and upgrade:
  On installation, do not autostart the server.
  *Iff* the server was stopped before the upgrade is started, this is taken as a
  sign the administrator is handling that manually, and so the new server will
  not be started automatically at the end of the upgrade.
  The start/stop scripts will still be installed, so the server will be started
  on the next machine boot.
  This is the 5.5 version of fixing bug#27072 (RPM autostarting the server).

* Tue Jun 1 2010 Jonathan Perkin <jonathan.perkin@oracle.com>

- Implement SELinux checks from distribution-specific spec file.

* Wed May 12 2010 Jonathan Perkin <jonathan.perkin@oracle.com>

- Large number of changes to build using CMake
- Introduce distribution-specific RPMs
- Drop debuginfo, build all binaries with debug/symbols
- Remove __os_install_post, use native macro
- Remove _unpackaged_files_terminate_build, make it an error to have
  unpackaged files
- Remove cluster RPMs

* Wed Mar 24 2010 Joerg Bruehe <joerg.bruehe@sun.com>

- Add "--with-perfschema" to the configure options.

* Mon Mar 22 2010 Joerg Bruehe <joerg.bruehe@sun.com>

- User "usr/lib*" to allow for both "usr/lib" and "usr/lib64",
  mask "rmdir" return code 1.
- Remove "ha_example.*" files from the list, they aren't built.

* Wed Mar 17 2010 Joerg Bruehe <joerg.bruehe@sun.com>

- Fix a wrong path name in handling the debug plugins.

* Wed Mar 10 2010 Joerg Bruehe <joerg.bruehe@sun.com>

- Take the result of the debug plugin build and put it into the optimized tree,
  so that it becomes part of the final installation;
  include the files in the packlist. Part of the fixes for bug#49022.

* Mon Mar 01 2010 Joerg Bruehe <joerg.bruehe@sun.com>

- Set "Oracle and/or its affiliates" as the vendor and copyright owner,
  accept upgrading from packages showing MySQL or Sun as vendor.

* Fri Feb 12 2010 Joerg Bruehe <joerg.bruehe@sun.com>

- Formatting changes:
  Have a consistent structure of separator lines and of indentation
  (8 leading blanks => tab).
- Introduce the variable "src_dir".
- Give the environment variables "MYSQL_BUILD_CC(CXX)" precedence
  over "CC" ("CXX").
- Drop the old "with_static" argument analysis, this is not supported
  in 5.1 since ages.
- Introduce variables to control the handlers individually, as well
  as other options.
- Use the new "--with-plugin" notation for the table handlers.
- Drop handling "/etc/rc.d/init.d/mysql", the switch to "/etc/init.d/mysql"
  was done back in 2002 already.
- Make "--with-zlib-dir=bundled" the default, add an option to disable it.
- Add missing manual pages to the file list.
- Improve the runtime check for "libgcc.a", protect it against being tried
  with the Intel compiler "icc".

* Mon Jan 11 2010 Joerg Bruehe <joerg.bruehe@sun.com>

- Change RPM file naming:
  - Suffix like "-m2", "-rc" becomes part of version as "_m2", "_rc".
  - Release counts from 1, not 0.

* Wed Dec 23 2009 Joerg Bruehe <joerg.bruehe@sun.com>

- The "semisync" plugin file name has lost its introductory "lib",
  adapt the file lists for the subpackages.
  This is a part missing from the fix for bug#48351.
- Remove the "fix_privilege_tables" manual, it does not exist in 5.5
  (and likely, the whole script will go, too).

* Mon Nov 16 2009 Joerg Bruehe <joerg.bruehe@sun.com>

- Fix some problems with the directives around "tcmalloc" (experimental),
  remove erroneous traces of the InnoDB plugin (that is 5.1 only).

* Fri Oct 06 2009 Magnus Blaudd <mvensson@mysql.com>

- Removed mysql_fix_privilege_tables

* Fri Oct 02 2009 Alexander Nozdrin <alexander.nozdrin@sun.com>

- "mysqlmanager" got removed from version 5.4, all references deleted.

* Fri Aug 28 2009 Joerg Bruehe <joerg.bruehe@sun.com>

- Merge up from 5.1 to 5.4: Remove handling for the InnoDB plugin.

* Thu Aug 27 2009 Joerg Bruehe <joerg.bruehe@sun.com>

- This version does not contain the "Instance manager", "mysqlmanager":
  Remove it from the spec file so that packaging succeeds.

* Mon Aug 24 2009 Jonathan Perkin <jperkin@sun.com>

- Add conditionals for bundled zlib and innodb plugin

* Fri Aug 21 2009 Jonathan Perkin <jperkin@sun.com>

- Install plugin libraries in appropriate packages.
- Disable libdaemon_example and ftexample plugins.

* Thu Aug 20 2009 Jonathan Perkin <jperkin@sun.com>

- Update variable used for mysql-test suite location to match source.

* Fri Nov 07 2008 Joerg Bruehe <joerg@mysql.com>

- Correct yesterday's fix, so that it also works for the last flag,
  and fix a wrong quoting: un-quoted quote marks must not be escaped.

* Thu Nov 06 2008 Kent Boortz <kent.boortz@sun.com>

- Removed "mysql_upgrade_shell"
- Removed some copy/paste between debug and normal build

* Thu Nov 06 2008 Joerg Bruehe <joerg@mysql.com>

- Modify CFLAGS and CXXFLAGS such that a debug build is not optimized.
  This should cover both gcc and icc flags.  Fixes bug#40546.

* Fri Aug 29 2008 Kent Boortz <kent@mysql.com>

- Removed the "Federated" storage engine option, and enabled in all

* Tue Aug 26 2008 Joerg Bruehe <joerg@mysql.com>

- Get rid of the "warning: Installed (but unpackaged) file(s) found:"
  Some generated files aren't needed in RPMs:
  - the "sql-bench/" subdirectory
  Some files were missing:
  - /usr/share/aclocal/mysql.m4  ("devel" subpackage)
  - Manual "mysqlbug" ("server" subpackage)
  - Program "innochecksum" and its manual ("server" subpackage)
  - Manual "mysql_find_rows" ("client" subpackage)
  - Script "mysql_upgrade_shell" ("client" subpackage)
  - Program "ndb_cpcd" and its manual ("ndb-extra" subpackage)
  - Manuals "ndb_mgm" + "ndb_restore" ("ndb-tools" subpackage)

* Mon Mar 31 2008 Kent Boortz <kent@mysql.com>

- Made the "Federated" storage engine an option
- Made the "Cluster" storage engine and sub packages an option

* Wed Mar 19 2008 Joerg Bruehe <joerg@mysql.com>

- Add the man pages for "ndbd" and "ndb_mgmd".

* Mon Feb 18 2008 Timothy Smith <tim@mysql.com>

- Require a manual upgrade if the alread-installed mysql-server is
  from another vendor, or is of a different major version.

* Wed May 02 2007 Joerg Bruehe <joerg@mysql.com>

- "ndb_size.tmpl" is not needed any more,
  "man1/mysql_install_db.1" lacked the trailing '*'.

* Sat Apr 07 2007 Kent Boortz <kent@mysql.com>

- Removed man page for "mysql_create_system_tables"

* Wed Mar 21 2007 Daniel Fischer <df@mysql.com>

- Add debug server.

* Mon Mar 19 2007 Daniel Fischer <df@mysql.com>

- Remove Max RPMs; the server RPMs contain a mysqld compiled with all
  features that previously only were built into Max.

* Fri Mar 02 2007 Joerg Bruehe <joerg@mysql.com>

- Add several man pages for NDB which are now created.

* Fri Jan 05 2007 Kent Boortz <kent@mysql.com>

- Put back "libmygcc.a", found no real reason it was removed.

- Add CFLAGS to gcc call with --print-libgcc-file, to make sure the
  correct "libgcc.a" path is returned for the 32/64 bit architecture.

* Mon Dec 18 2006 Joerg Bruehe <joerg@mysql.com>

- Fix the move of "mysqlmanager" to section 8: Directory name was wrong.

* Thu Dec 14 2006 Joerg Bruehe <joerg@mysql.com>

- Include the new man pages for "my_print_defaults" and "mysql_tzinfo_to_sql"
  in the server RPM.
- The "mysqlmanager" man page got moved from section 1 to 8.

* Thu Nov 30 2006 Joerg Bruehe <joerg@mysql.com>

- Call "make install" using "benchdir_root=%{_datadir}",
  because that is affecting the regression test suite as well.

* Thu Nov 16 2006 Joerg Bruehe <joerg@mysql.com>

- Explicitly note that the "MySQL-shared" RPMs (as built by MySQL AB)
  replace "mysql-shared" (as distributed by SuSE) to allow easy upgrading
  (bug#22081).

* Mon Nov 13 2006 Joerg Bruehe <joerg@mysql.com>

- Add "--with-partition" to all server builds.

- Use "--report-features" in one test run per server build.

* Tue Aug 15 2006 Joerg Bruehe <joerg@mysql.com>

- The "max" server is removed from packages, effective from 5.1.12-beta.
  Delete all steps to build, package, or install it.

* Mon Jul 10 2006 Joerg Bruehe <joerg@mysql.com>

- Fix a typing error in the "make" target for the Perl script to run the tests.

* Tue Jul 04 2006 Joerg Bruehe <joerg@mysql.com>

- Use the Perl script to run the tests, because it will automatically check
  whether the server is configured with SSL.

* Tue Jun 27 2006 Joerg Bruehe <joerg@mysql.com>

- move "mysqldumpslow" from the client RPM to the server RPM (bug#20216)

- Revert all previous attempts to call "mysql_upgrade" during RPM upgrade,
  there are some more aspects which need to be solved before this is possible.
  For now, just ensure the binary "mysql_upgrade" is delivered and installed.

* Thu Jun 22 2006 Joerg Bruehe <joerg@mysql.com>

- Close a gap of the previous version by explicitly using
  a newly created temporary directory for the socket to be used
  in the "mysql_upgrade" operation, overriding any local setting.

* Tue Jun 20 2006 Joerg Bruehe <joerg@mysql.com>

- To run "mysql_upgrade", we need a running server;
  start it in isolation and skip password checks.

* Sat May 20 2006 Kent Boortz <kent@mysql.com>

- Always compile for PIC, position independent code.

* Wed May 10 2006 Kent Boortz <kent@mysql.com>

- Use character set "all" when compiling with Cluster, to make Cluster
  nodes independent on the character set directory, and the problem
  that two RPM sub packages both wants to install this directory.

* Mon May 01 2006 Kent Boortz <kent@mysql.com>

- Use "./libtool --mode=execute" instead of searching for the
  executable in current directory and ".libs".

* Fri Apr 28 2006 Kent Boortz <kent@mysql.com>

- Install and run "mysql_upgrade"

* Wed Apr 12 2006 Jim Winstead <jimw@mysql.com>

- Remove sql-bench, and MySQL-bench RPM (will be built as an independent
  project from the mysql-bench repository)

* Tue Apr 11 2006 Jim Winstead <jimw@mysql.com>

- Remove old mysqltestmanager and related programs
* Sat Apr 01 2006 Kent Boortz <kent@mysql.com>

- Set $LDFLAGS from $MYSQL_BUILD_LDFLAGS

* Wed Mar 07 2006 Kent Boortz <kent@mysql.com>

- Changed product name from "Community Edition" to "Community Server"

* Mon Mar 06 2006 Kent Boortz <kent@mysql.com>

- Fast mutexes is now disabled by default, but should be
  used in Linux builds.

* Mon Feb 20 2006 Kent Boortz <kent@mysql.com>

- Reintroduced a max build
- Limited testing of 'debug' and 'max' servers
- Berkeley DB only in 'max'

* Mon Feb 13 2006 Joerg Bruehe <joerg@mysql.com>

- Use "-i" on "make test-force";
  this is essential for later evaluation of this log file.

* Thu Feb 09 2006 Kent Boortz <kent@mysql.com>

- Pass '-static' to libtool, link static with our own libraries, dynamic
  with system libraries.  Link with the bundled zlib.

* Wed Feb 08 2006 Kristian Nielsen <knielsen@mysql.com>

- Modified RPM spec to match new 5.1 debug+max combined community packaging.

* Sun Dec 18 2005 Kent Boortz <kent@mysql.com>

- Added "client/mysqlslap"

* Mon Dec 12 2005 Rodrigo Novo <rodrigo@mysql.com>

- Added zlib to the list of (static) libraries installed
- Added check against libtool wierdness (WRT: sql/mysqld || sql/.libs/mysqld)
- Compile MySQL with bundled zlib
- Fixed %packager name to "MySQL Production Engineering Team"

* Mon Dec 05 2005 Joerg Bruehe <joerg@mysql.com>

- Avoid using the "bundled" zlib on "shared" builds:
  As it is not installed (on the build system), this gives dependency
  problems with "libtool" causing the build to fail.
  (Change was done on Nov 11, but left uncommented.)

* Tue Nov 22 2005 Joerg Bruehe <joerg@mysql.com>

- Extend the file existence check for "init.d/mysql" on un-install
  to also guard the call to "insserv"/"chkconfig".

* Thu Oct 27 2005 Lenz Grimmer <lenz@grimmer.com>

- added more man pages

* Wed Oct 19 2005 Kent Boortz <kent@mysql.com>

- Made yaSSL support an option (off by default)

* Wed Oct 19 2005 Kent Boortz <kent@mysql.com>

- Enabled yaSSL support

* Sat Oct 15 2005 Kent Boortz <kent@mysql.com>

- Give mode arguments the same way in all places
- Moved copy of mysqld.a to "standard" build, but
  disabled it as we don't do embedded yet in 5.0

* Fri Oct 14 2005 Kent Boortz <kent@mysql.com>

- For 5.x, always compile with --with-big-tables
- Copy the config.log file to location outside
  the build tree

* Fri Oct 14 2005 Kent Boortz <kent@mysql.com>

- Removed unneeded/obsolete configure options
- Added archive engine to standard server
- Removed the embedded server from experimental server
- Changed suffix "-Max" => "-max"
- Changed comment string "Max" => "Experimental"

* Thu Oct 13 2005 Lenz Grimmer <lenz@mysql.com>

- added a usermod call to assign a potential existing mysql user to the
  correct user group (BUG#12823)
- Save the perror binary built during Max build so it supports the NDB
  error codes (BUG#13740)
- added a separate macro "mysqld_group" to be able to define the
  user group of the mysql user seperately, if desired.

* Thu Sep 29 2005 Lenz Grimmer <lenz@mysql.com>

- fixed the removing of the RPM_BUILD_ROOT in the %clean section (the
  $RBR variable did not get expanded, thus leaving old build roots behind)

* Thu Aug 04 2005 Lenz Grimmer <lenz@mysql.com>

- Fixed the creation of the mysql user group account in the postinstall
  section (BUG 12348)
- Fixed enabling the Archive storage engine in the Max binary

* Tue Aug 02 2005 Lenz Grimmer <lenz@mysql.com>

- Fixed the Requires: tag for the server RPM (BUG 12233)

* Fri Jul 15 2005 Lenz Grimmer <lenz@mysql.com>

- create a "mysql" user group and assign the mysql user account to that group
  in the server postinstall section. (BUG 10984)

* Tue Jun 14 2005 Lenz Grimmer <lenz@mysql.com>

- Do not build statically on i386 by default, only when adding either "--with
  static" or "--define '_with_static 1'" to the RPM build options. Static
  linking really only makes sense when linking against the specially patched
  glibc 2.2.5.

* Mon Jun 06 2005 Lenz Grimmer <lenz@mysql.com>

- added mysql_client_test to the "bench" subpackage (BUG 10676)
- added the libndbclient static and shared libraries (BUG 10676)

* Wed Jun 01 2005 Lenz Grimmer <lenz@mysql.com>

- use "mysqldatadir" variable instead of hard-coding the path multiple times
- use the "mysqld_user" variable on all occasions a user name is referenced
- removed (incomplete) Brazilian translations
- removed redundant release tags from the subpackage descriptions

* Wed May 25 2005 Joerg Bruehe <joerg@mysql.com>

- Added a "make clean" between separate calls to "BuildMySQL".

* Thu May 12 2005 Guilhem Bichot <guilhem@mysql.com>

- Removed the mysql_tableinfo script made obsolete by the information schema

* Wed Apr 20 2005 Lenz Grimmer <lenz@mysql.com>

- Enabled the "blackhole" storage engine for the Max RPM

* Wed Apr 13 2005 Lenz Grimmer <lenz@mysql.com>

- removed the MySQL manual files (html/ps/texi) - they have been removed
  from the MySQL sources and are now available seperately.

* Mon Apr 4 2005 Petr Chardin <petr@mysql.com>

- old mysqlmanager, mysqlmanagerc and mysqlmanager-pwger renamed into
  mysqltestmanager, mysqltestmanager and mysqltestmanager-pwgen respectively

* Fri Mar 18 2005 Lenz Grimmer <lenz@mysql.com>

- Disabled RAID in the Max binaries once and for all (it has finally been
  removed from the source tree)

* Sun Feb 20 2005 Petr Chardin <petr@mysql.com>

- Install MySQL Instance Manager together with mysqld, touch mysqlmanager
  password file

* Mon Feb 14 2005 Lenz Grimmer <lenz@mysql.com>

- Fixed the compilation comments and moved them into the separate build sections
  for Max and Standard

* Mon Feb 7 2005 Tomas Ulin <tomas@mysql.com>

- enabled the "Ndbcluster" storage engine for the max binary
- added extra make install in ndb subdir after Max build to get ndb binaries
- added packages for ndbcluster storage engine

* Fri Jan 14 2005 Lenz Grimmer <lenz@mysql.com>

- replaced obsoleted "BuildPrereq" with "BuildRequires" instead

* Thu Jan 13 2005 Lenz Grimmer <lenz@mysql.com>

- enabled the "Federated" storage engine for the max binary

* Tue Jan 04 2005 Petr Chardin <petr@mysql.com>

- ISAM and merge storage engines were purged. As well as appropriate
  tools and manpages (isamchk and isamlog)

* Thu Dec 31 2004 Lenz Grimmer <lenz@mysql.com>

- enabled the "Archive" storage engine for the max binary
- enabled the "CSV" storage engine for the max binary
- enabled the "Example" storage engine for the max binary

* Thu Aug 26 2004 Lenz Grimmer <lenz@mysql.com>

- MySQL-Max now requires MySQL-server instead of MySQL (BUG 3860)

* Fri Aug 20 2004 Lenz Grimmer <lenz@mysql.com>

- do not link statically on IA64/AMD64 as these systems do not have
  a patched glibc installed

* Tue Aug 10 2004 Lenz Grimmer <lenz@mysql.com>

- Added libmygcc.a to the devel subpackage (required to link applications
  against the the embedded server libmysqld.a) (BUG 4921)

* Mon Aug 09 2004 Lenz Grimmer <lenz@mysql.com>

- Added EXCEPTIONS-CLIENT to the "devel" package

* Thu Jul 29 2004 Lenz Grimmer <lenz@mysql.com>

- disabled OpenSSL in the Max binaries again (the RPM packages were the
  only exception to this anyway) (BUG 1043)

* Wed Jun 30 2004 Lenz Grimmer <lenz@mysql.com>

- fixed server postinstall (mysql_install_db was called with the wrong
  parameter)

* Thu Jun 24 2004 Lenz Grimmer <lenz@mysql.com>

- added mysql_tzinfo_to_sql to the server subpackage
- run "make clean" instead of "make distclean"

* Mon Apr 05 2004 Lenz Grimmer <lenz@mysql.com>

- added ncurses-devel to the build prerequisites (BUG 3377)

* Thu Feb 12 2004 Lenz Grimmer <lenz@mysql.com>

- when using gcc, _always_ use CXX=gcc
- replaced Copyright with License field (Copyright is obsolete)

* Tue Feb 03 2004 Lenz Grimmer <lenz@mysql.com>

- added myisam_ftdump to the Server package

* Tue Jan 13 2004 Lenz Grimmer <lenz@mysql.com>

- link the mysql client against libreadline instead of libedit (BUG 2289)

* Mon Dec 22 2003 Lenz Grimmer <lenz@mysql.com>

- marked /etc/logrotate.d/mysql as a config file (BUG 2156)

* Fri Dec 13 2003 Lenz Grimmer <lenz@mysql.com>

- fixed file permissions (BUG 1672)

* Thu Dec 11 2003 Lenz Grimmer <lenz@mysql.com>

- made testing for gcc3 a bit more robust

* Fri Dec 05 2003 Lenz Grimmer <lenz@mysql.com>

- added missing file mysql_create_system_tables to the server subpackage

* Fri Nov 21 2003 Lenz Grimmer <lenz@mysql.com>

- removed dependency on MySQL-client from the MySQL-devel subpackage
  as it is not really required. (BUG 1610)

* Fri Aug 29 2003 Lenz Grimmer <lenz@mysql.com>

- Fixed BUG 1162 (removed macro names from the changelog)
- Really fixed BUG 998 (disable the checking for installed but
  unpackaged files)

* Tue Aug 05 2003 Lenz Grimmer <lenz@mysql.com>

- Fixed BUG 959 (libmysqld not being compiled properly)
- Fixed BUG 998 (RPM build errors): added missing files to the
  distribution (mysql_fix_extensions, mysql_tableinfo, mysqldumpslow,
  mysql_fix_privilege_tables.1), removed "-n" from install section.

* Wed Jul 09 2003 Lenz Grimmer <lenz@mysql.com>

- removed the GIF Icon (file was not included in the sources anyway)
- removed unused variable shared_lib_version
- do not run automake before building the standard binary
  (should not be necessary)
- add server suffix '-standard' to standard binary (to be in line
  with the binary tarball distributions)
- Use more RPM macros (_exec_prefix, _sbindir, _libdir, _sysconfdir,
  _datadir, _includedir) throughout the spec file.
- allow overriding CC and CXX (required when building with other compilers)

* Fri May 16 2003 Lenz Grimmer <lenz@mysql.com>

- re-enabled RAID again

* Wed Apr 30 2003 Lenz Grimmer <lenz@mysql.com>

- disabled MyISAM RAID (--with-raid) - it throws an assertion which
  needs to be investigated first.

* Mon Mar 10 2003 Lenz Grimmer <lenz@mysql.com>

- added missing file mysql_secure_installation to server subpackage
  (BUG 141)

* Tue Feb 11 2003 Lenz Grimmer <lenz@mysql.com>

- re-added missing pre- and post(un)install scripts to server subpackage
- added config file /etc/my.cnf to the file list (just for completeness)
- make sure to create the datadir with 755 permissions

* Mon Jan 27 2003 Lenz Grimmer <lenz@mysql.com>

- removed unused CC and CXX variables
- CFLAGS and CXXFLAGS should honor RPM_OPT_FLAGS

* Fri Jan 24 2003 Lenz Grimmer <lenz@mysql.com>

- renamed package "MySQL" to "MySQL-server"
- fixed Copyright tag
- added mysql_waitpid to client subpackage (required for mysql-test-run)

* Wed Nov 27 2002 Lenz Grimmer <lenz@mysql.com>

- moved init script from /etc/rc.d/init.d to /etc/init.d (the majority of
  Linux distributions now support this scheme as proposed by the LSB either
  directly or via a compatibility symlink)
- Use new "restart" init script action instead of starting and stopping
  separately
- Be more flexible in activating the automatic bootup - use insserv (on
  older SuSE versions) or chkconfig (Red Hat, newer SuSE versions and
  others) to create the respective symlinks

* Wed Sep 25 2002 Lenz Grimmer <lenz@mysql.com>

- MySQL-Max now requires MySQL >= 4.0 to avoid version mismatches
  (mixing 3.23 and 4.0 packages)

* Fri Aug 09 2002 Lenz Grimmer <lenz@mysql.com>

- Turn off OpenSSL in MySQL-Max for now until it works properly again
- enable RAID for the Max binary instead
- added compatibility link: safe_mysqld -> mysqld_safe to ease the
  transition from 3.23

* Thu Jul 18 2002 Lenz Grimmer <lenz@mysql.com>

- Reworked the build steps a little bit: the Max binary is supposed
  to include OpenSSL, which cannot be linked statically, thus trying
  to statically link against a special glibc is futile anyway
- because of this, it is not required to make yet another build run
  just to compile the shared libs (saves a lot of time)
- updated package description of the Max subpackage
- clean up the BuildRoot directory afterwards

* Mon Jul 15 2002 Lenz Grimmer <lenz@mysql.com>

- Updated Packager information
- Fixed the build options: the regular package is supposed to
  include InnoDB and linked statically, while the Max package
  should include BDB and SSL support

* Fri May 03 2002 Lenz Grimmer <lenz@mysql.com>

- Use more RPM macros (e.g. infodir, mandir) to make the spec
  file more portable
- reorganized the installation of documentation files: let RPM
  take care of this
- reorganized the file list: actually install man pages along
  with the binaries of the respective subpackage
- do not include libmysqld.a in the devel subpackage as well, if we
  have a special "embedded" subpackage
- reworked the package descriptions

* Mon Oct  8 2001 Monty

- Added embedded server as a separate RPM

* Fri Apr 13 2001 Monty

- Added mysqld-max to the distribution

* Tue Jan 2  2001  Monty

- Added mysql-test to the bench package

* Fri Aug 18 2000 Tim Smith <tim@mysql.com>

- Added separate libmysql_r directory; now both a threaded
  and non-threaded library is shipped.

* Wed Sep 28 1999 David Axmark <davida@mysql.com>

- Added the support-files/my-example.cnf to the docs directory.

- Removed devel dependency on base since it is about client
  development.

* Wed Sep 8 1999 David Axmark <davida@mysql.com>

- Cleaned up some for 3.23.

* Thu Jul 1 1999 David Axmark <davida@mysql.com>

- Added support for shared libraries in a separate sub
  package. Original fix by David Fox (dsfox@cogsci.ucsd.edu)

- The --enable-assembler switch is now automatically disables on
  platforms there assembler code is unavailable. This should allow
  building this RPM on non i386 systems.

* Mon Feb 22 1999 David Axmark <david@detron.se>

- Removed unportable cc switches from the spec file. The defaults can
  now be overridden with environment variables. This feature is used
  to compile the official RPM with optimal (but compiler version
  specific) switches.

- Removed the repetitive description parts for the sub rpms. Maybe add
  again if RPM gets a multiline macro capability.

- Added support for a pt_BR translation. Translation contributed by
  Jorge Godoy <jorge@bestway.com.br>.

* Wed Nov 4 1998 David Axmark <david@detron.se>

- A lot of changes in all the rpm and install scripts. This may even
  be a working RPM :-)

* Sun Aug 16 1998 David Axmark <david@detron.se>

- A developers changelog for MySQL is available in the source RPM. And
  there is a history of major user visible changed in the Reference
  Manual.  Only RPM specific changes will be documented here.
