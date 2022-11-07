# Copyright (c) 2000, 2015, Oracle and/or its affiliates. All rights reserved.
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

# Rebuild on OL5/RHEL5 needs following rpmbuild options:
#  rpmbuild --define 'dist .el5' --define 'rhel 5' --define 'el5 1' mysql.spec

# Install cmake28 from EPEL when building on OL5/RHEL5 and OL6/RHEL6.

# NOTE: "vendor" is used in upgrade/downgrade check, so you can't
# change these, has to be exactly as is.

%undefine _missing_build_ids_terminate_build
%global mysql_vendor Oracle and/or its affiliates
%global percona_server_vendor Percona, Inc
%global mysqldatadir /var/lib/mysql

%global mysql_version @@MYSQL_VERSION@@
%global percona_server_version @@PERCONA_VERSION@@
%global revision @@REVISION@@
%global tokudb_backup_version %{mysql_version}-%{percona_server_version}
%global rpm_release @@RPM_RELEASE@@

%global release %{percona_server_version}.%{rpm_release}%{?dist}

# By default, a build will be done using the system SSL library
%{?with_ssl: %global ssl_option -DWITH_SSL=%{with_ssl}}
%{!?with_ssl: %global ssl_option -DWITH_SSL=system}

# By default a build will be done excluding the TokuDB
%{!?with_tokudb: %global tokudb 0}

# By default a build will be done including the RocksDB
%{!?with_rocksdb: %global rocksdb 1}

# Pass path to mecab lib
%{?with_mecab: %global mecab_option -DWITH_MECAB=%{with_mecab}}
%{?with_mecab: %global mecab 1}

# Regression tests may take a long time, override the default to skip them
%{!?runselftest:%global runselftest 0}

%{!?with_systemd:                %global systemd 0}
%{?el7:                          %global systemd 1}
%{?el8:                          %global systemd 1}
%{?el9:                          %global systemd 1}
%{!?with_debuginfo:              %global nodebuginfo 0}
%{!?product_suffix:              %global product_suffix -80}
%{!?feature_set:                 %global feature_set community}
%{!?compilation_comment_release: %global compilation_comment_release Percona Server (GPL), Release %{percona_server_version}, Revision %{revision}}
%{!?compilation_comment_debug:   %global compilation_comment_debug Percona Server - Debug (GPL), Release %{percona_server_version}, Revision %{revision}}
%{!?src_base:                    %global src_base percona-server}

# Setup cmake flags for TokuDB
%if 0%{?tokudb}
  %global TOKUDB_FLAGS -DWITH_VALGRIND=OFF -DUSE_VALGRIND=OFF -DDEBUG_EXTNAME=OFF -DBUILD_TESTING=OFF -DUSE_GTAGS=OFF -DUSE_CTAGS=OFF -DUSE_ETAGS=OFF -DUSE_CSCOPE=OFF -DTOKUDB_BACKUP_PLUGIN_VERSION=%{tokudb_backup_version}
  %global TOKUDB_DEBUG_ON -DTOKU_DEBUG_PARANOID=ON
  %global TOKUDB_DEBUG_OFF -DTOKU_DEBUG_PARANOID=OFF
%else
  %global TOKUDB_FLAGS -DWITHOUT_TOKUDB=1
  %global TOKUDB_DEBUG_ON %{nil}
  %global TOKUDB_DEBUG_OFF %{nil}
%endif

# Setup cmake flags for RocksDB
%if 0%{?rocksdb}
  %global ROCKSDB_FLAGS -DWITH_ROCKSDB=1
%else
  %global ROCKSDB_FLAGS -DWITH_ROCKSDB=0
%endif

# On rhel 5/6 we still have renamed library to libperconaserverclient
%if 0%{?rhel} > 6
  %global shared_lib_pri_name mysqlclient
  %global shared_lib_sec_name perconaserverclient
%else
  %global shared_lib_pri_name mysqlclient
  %global shared_lib_sec_name perconaserverclient
%endif

# Version for compat libs
%if 0%{?rhel} > 6
%global compat_prefix         56
%global compatver             5.6.51
%global percona_compatver     91.0
%global compatlib             18
%global compatsrc             https://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-%{compatver}-%{percona_compatver}/binary/redhat/7/x86_64/Percona-Server-shared-56-%{compatver}-rel%{percona_compatver}.1.el7.x86_64.rpm
%endif

%if 0%{?rhel} == 6
%global compat_prefix         51
%global compatver             5.1.73
%global percona_compatver     14.12
%global compatlib             16
%global compatsrc             https://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.73-rel14.12/RPM/rhel6/x86_64/Percona-Server-shared-51-5.1.73-rel14.12.624.rhel6.x86_64.rpm
%endif

# multiarch
%global multiarchs            ppc %{power64} %{ix86} x86_64 %{sparc}

%ifarch x86_64
%global __isa_bits            64
%endif

%global src_dir               %{src_base}-%{mysql_version}-%{percona_server_version}

# We build debuginfo package so this is not used
%if 0%{?nodebuginfo}
%global _enable_debug_package 0
%global debug_package         %{nil}
%global __os_install_post     /usr/lib/rpm/brp-compress %{nil}
%endif

%global license_files_server  %{src_dir}/README.md
%global license_type          GPLv2

Name:           percona-server
Summary:        Percona-Server: a very fast and reliable SQL database server
Group:          Applications/Databases
Version:        %{mysql_version}
Release:        %{release}
License:        Copyright (c) 2000, 2018, %{mysql_vendor}. All rights reserved. Under %{?license_type} license as shown in the Description field..
Source0:        http://www.percona.com/downloads/Percona-Server-8.0/Percona-Server-%{mysql_version}-%{percona_server_version}/source/%{src_dir}.tar.gz
URL:            http://www.percona.com/
Packager:       Percona MySQL Development Team <mysqldev@percona.com>
Vendor:         %{percona_server_vendor}
Source5:        mysql_config.sh
Source10:       http://jenkins.percona.com/downloads/boost/@@BOOST_PACKAGE_NAME@@.tar.gz
Source90:       filter-provides.sh
Source91:       filter-requires.sh
Patch0:         mysql-5.7-sharedlib-rename.patch
BuildRequires:  cmake >= 2.8.2
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  perl
%{?el7:BuildRequires: perl(Time::HiRes)}
%{?el7:BuildRequires: perl(Env)}
%{?el8:BuildRequires: perl(Env)}
%{?el9:BuildRequires: perl(Env)}
BuildRequires:  perl(Carp)
BuildRequires:  perl(Config)
BuildRequires:  perl(Cwd)
BuildRequires:  perl(Data::Dumper)
BuildRequires:  perl(English)
BuildRequires:  perl(Errno)
BuildRequires:  perl(Exporter)
BuildRequires:  perl(Fcntl)
BuildRequires:  perl(File::Basename)
BuildRequires:  perl(File::Copy)
BuildRequires:  perl(File::Find)
BuildRequires:  perl(File::Path)
BuildRequires:  perl(File::Spec)
BuildRequires:  perl(File::Spec::Functions)
BuildRequires:  perl(File::Temp)
BuildRequires:  perl(Getopt::Long)
BuildRequires:  perl(IO::File)
BuildRequires:  perl(IO::Handle)
BuildRequires:  perl(IO::Pipe)
BuildRequires:  perl(IO::Select)
BuildRequires:  perl(IO::Socket)
BuildRequires:  perl(IO::Socket::INET)
BuildRequires:  perl(JSON)
BuildRequires:  perl(Memoize)
BuildRequires:  perl(POSIX)
BuildRequires:  perl(Sys::Hostname)
BuildRequires:  perl(Time::HiRes)
BuildRequires:  perl(Time::localtime)
BuildRequires:  time
BuildRequires:  libaio-devel
BuildRequires:  ncurses-devel
BuildRequires:  pam-devel
BuildRequires:  readline-devel
BuildRequires:  numactl-devel
BuildRequires:  openssl-devel
BuildRequires:  zlib-devel
BuildRequires:  bison
BuildRequires:  openldap-devel
BuildRequires:  libcurl-devel
%if 0%{?systemd}
BuildRequires:  systemd
BuildRequires:  pkgconfig(systemd)
%endif
BuildRequires:  cyrus-sasl-devel
BuildRequires:  openldap-devel
%if 0%{?rhel} >= 8
BuildRequires:  cmake >= 3.6.1
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  libtirpc-devel
BuildRequires:  rpcgen
%else
BuildRequires:  cmake3 >= 3.6.1
BuildRequires:  devtoolset-8-gcc
BuildRequires:  devtoolset-8-gcc-c++
%endif
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

%if 0%{?rhel} > 6
# For rpm => 4.9 only: https://fedoraproject.org/wiki/Packaging:AutoProvidesAndRequiresFiltering
%global __requires_exclude ^perl\\((GD|hostnames|lib::mtr|lib::v1|mtr_|My::)
%global __provides_exclude_from ^(/usr/share/(mysql|mysql-test)/.*|%{_libdir}/mysql/plugin/.*\\.so)$
%else
# https://fedoraproject.org/wiki/EPEL:Packaging#Generic_Filtering_on_EPEL6
%global __perl_provides %{SOURCE90}
%global __perl_requires %{SOURCE91}
%endif

%description
The Percona Server software delivers a very fast, multi-threaded, multi-user,
and robust SQL (Structured Query Language) database server. Percona Server
is intended for mission-critical, heavy-load production systems.

Percona recommends that all production deployments be protected with a support
contract (http://www.percona.com/mysql-suppport/) to ensure the highest uptime,
be eligible for hot fixes, and boost your team's productivity.

%package -n percona-server-server
Summary:        Percona Server: a very fast and reliable SQL database server
Group:          Applications/Databases
Requires:       coreutils
Requires:       grep
Requires:       procps
Requires:       shadow-utils
Requires:       net-tools
Requires(pre):  percona-server-shared
Requires:       percona-server-client
Requires:       percona-icu-data-files
Requires:       openssl
Obsoletes:     community-mysql-bench
Obsoletes:     mysql-bench
Obsoletes:     mariadb-connector-c-config
Obsoletes:     mariadb-backup
Obsoletes:     mariadb-bench
Obsoletes:     mariadb-server
Obsoletes:     mariadb-server-galera
Obsoletes:     mariadb-server-utils
Obsoletes:     mariadb-galera-server
Obsoletes:     mariadb-gssapi-server
Obsoletes:     mariadb-oqgraph-engine
Provides:       MySQL-server%{?_isa} = %{version}-%{release}
Provides:       mysql-server = %{version}-%{release}
Provides:       mysql-server%{?_isa} = %{version}-%{release}
Conflicts:      Percona-SQL-server-50 Percona-Server-server-51 Percona-Server-server-55 Percona-Server-server-56 Percona-Server-server-57

%if 0%{?systemd}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd
%else
Requires(post):   /sbin/chkconfig
Requires(preun):  /sbin/chkconfig
Requires(preun):  /sbin/service
%endif

%if 0%{?rhel} == 8
Obsoletes:      mariadb-connector-c-config
%endif

%if 0%{?tokudb} == 0
Obsoletes:      percona-server-tokudb
%endif

%description -n percona-server-server
The Percona Server software delivers a very fast, multi-threaded, multi-user,
and robust SQL (Structured Query Language) database server. Percona Server
is intended for mission-critical, heavy-load production systems.

Percona recommends that all production deployments be protected with a support
contract (http://www.percona.com/mysql-suppport/) to ensure the highest uptime,
be eligible for hot fixes, and boost your team's productivity.

This package includes the Percona Server with XtraDB binary
as well as related utilities to run and administer Percona Server.

If you want to access and work with the database, you have to install
package "percona-server-client" as well!


%package -n percona-server-client
Summary:        Percona Server - Client
Group:          Applications/Databases
Requires:       percona-server-shared
Provides:       mysql-client MySQL-client mysql MySQL
Conflicts:      Percona-SQL-client-50 Percona-Server-client-51 Percona-Server-client-55 Percona-Server-client-56 Percona-Server-client-57

%description -n percona-server-client
This package contains the standard Percona Server client and administration tools.

For a description of Percona Server see http://www.percona.com/software/percona-server/

%package -n percona-server-test
Summary:        Test suite for the Percona Server
Group:          Applications/Databases
Requires:       perl(Carp)
Requires:       perl(Config)
Requires:       perl(Cwd)
Requires:       perl(Data::Dumper)
Requires:       perl(English)
Requires:       perl(Errno)
Requires:       perl(Exporter)
Requires:       perl(Fcntl)
Requires:       perl(File::Basename)
Requires:       perl(File::Copy)
Requires:       perl(File::Find)
Requires:       perl(File::Path)
Requires:       perl(File::Spec)
Requires:       perl(File::Spec::Functions)
Requires:       perl(File::Temp)
Requires:       perl(Getopt::Long)
Requires:       perl(IO::File)
Requires:       perl(IO::Handle)
Requires:       perl(IO::Pipe)
Requires:       perl(IO::Select)
Requires:       perl(IO::Socket)
Requires:       perl(IO::Socket::INET)
Requires:       perl(JSON)
Requires:       perl(Memoize)
Requires:       perl(POSIX)
Requires:       perl(Sys::Hostname)
Requires:       perl(Time::HiRes)
Requires:       perl(Time::localtime)
Provides:       MySQL-test%{?_isa} = %{version}-%{release}
Obsoletes:      MySQL-test < %{version}-%{release}
Obsoletes:      mysql-test < %{version}-%{release}
Obsoletes:      mariadb-test
Provides:       mysql-test = %{version}-%{release}
Provides:       mysql-test%{?_isa} = %{version}-%{release}
Conflicts:      Percona-SQL-test-50 Percona-Server-test-51 Percona-Server-test-55 Percona-Server-test-56 Percona-Server-test-57

%description -n percona-server-test
This package contains the Percona Server regression test suite.

For a description of Percona Server see http://www.percona.com/software/percona-server/

%package -n percona-server-devel
Summary:        Percona Server - Development header files and libraries
Group:          Applications/Databases
Obsoletes:     mariadb-devel
Obsoletes:     mariadb-connector-c-devel
Obsoletes:     mysql-connector-c-devel < 6.2
Provides:       mysql-devel = %{version}-%{release}
Provides:       mysql-devel%{?_isa} = %{version}-%{release}
Conflicts:      Percona-SQL-devel-50 Percona-Server-devel-51 Percona-Server-devel-55 Percona-Server-devel-56 Percona-Server-devel-57
Obsoletes:      mariadb-connector-c-devel
%if 0%{?rhel} > 6
Obsoletes:      mariadb-devel
%endif

%description -n percona-server-devel
This package contains the development header files and libraries necessary
to develop Percona Server client applications.

For a description of Percona Server see http://www.percona.com/software/percona-server/

%package -n percona-server-shared
Summary:        Percona Server - Shared libraries
Group:          Applications/Databases
Provides:       mysql-libs = %{version}-%{release}
Provides:       mysql-libs%{?_isa} = %{version}-%{release}
Obsoletes:      mariadb-libs
Obsoletes:      mysql-connector-c-shared < 6.2
Obsoletes:      mysql-libs < %{version}-%{release}
Provides:       mysql-shared
%if 0%{?rhel} < 9
Requires(pre):  percona-server-shared-compat
%endif

%description -n percona-server-shared
This package contains the shared libraries (*.so*) which certain languages
and applications need to dynamically load and use Percona Server.

%if 0%{?compatlib}
%package -n percona-server-shared-compat
Summary:        Shared compat libraries for Percona Server %{compatver}-%{percona_compatver} database client applications
Group:          Applications/Databases
Provides:       mysql-libs-compat = %{version}-%{release}
Provides:       mysql-libs-compat%{?_isa} = %{version}-%{release}
Provides:       MySQL-shared-compat%{?_isa} = %{version}-%{release}
%if 0%{?rhel} > 6
Provides:       libmysqlclient.so.18()(64bit)
Provides:       libmysqlclient.so.18(libmysqlclient_16)(64bit)
Provides:       libmysqlclient.so.18(libmysqlclient_18)(64bit)
Obsoletes:      mariadb-libs
%else
Obsoletes:      mysql-libs
%endif
Conflicts:      Percona-Server-shared-51
Conflicts:      Percona-Server-shared-55
Conflicts:      Percona-Server-shared-55
Conflicts:      Percona-Server-shared-56
Conflicts:      Percona-Server-shared-57

%description -n percona-server-shared-compat
This package contains the shared compat libraries for Percona Server %{compatver}-%{percona_compatver} client
applications.
%endif

%if 0%{?tokudb}
# ----------------------------------------------------------------------------
%package -n percona-server-tokudb
Summary:        Percona Server - TokuDB package
Group:          Applications/Databases
Requires:       percona-server-server = %{version}-%{release}
Requires:       percona-server-shared = %{version}-%{release}
Requires:       percona-server-client = %{version}-%{release}
Requires:       jemalloc >= 3.3.0
Provides:       tokudb-plugin = %{version}-%{release}

%description -n percona-server-tokudb
This package contains the TokuDB plugin for Percona Server %{version}-%{release}
%endif

%if 0%{?rocksdb}
# ----------------------------------------------------------------------------
%package -n percona-server-rocksdb
Summary:        Percona Server - RocksDB package
Group:          Applications/Databases
Requires:       percona-server-server = %{version}-%{release}
Requires:       percona-server-shared = %{version}-%{release}
Requires:       percona-server-client = %{version}-%{release}

%description -n percona-server-rocksdb
This package contains the RocksDB plugin for Percona Server %{version}-%{release}
%endif

%package  -n   percona-mysql-router
Summary:       Percona MySQL Router
Group:         Applications/Databases
Provides:      percona-mysql-router = %{version}-%{release}
Obsoletes:     percona-mysql-router < %{version}-%{release}
Provides:      mysql-router

%description -n percona-mysql-router
The Percona MySQL Router software delivers a fast, multi-threaded way of
routing connections from MySQL Clients to MySQL Servers.

%package   -n   percona-mysql-router-devel
Summary:        Development header files and libraries for Percona MySQL Router
Group:          Applications/Databases
Provides:       percona-mysql-router-devel = %{version}-%{release}
Obsoletes:      mysql-router-devel

%description -n percona-mysql-router-devel
This package contains the development header files and libraries
necessary to develop Percona MySQL Router applications.

%package   -n   percona-icu-data-files
Summary:        MySQL packaging of ICU data files

%description -n percona-icu-data-files
This package contains ICU data files needer by MySQL regular expressions.

%prep
%setup -q -T -a 0 -a 10 -c -n %{src_dir}
pushd %{src_dir}
%patch0 -p0

%build
# Fail quickly and obviously if user tries to build as root
%if 0%{?runselftest}
if [ "x$(id -u)" = "x0" ] ; then
   echo "The MySQL regression tests may fail if run as root."
   echo "If you really need to build the RPM as root, use"
   echo "--define='runselftest 0' to skip the regression tests."
   exit 1
fi
%endif

# Download compat libs
%if 0%{?compatlib}
(
  rm -rf percona-compatlib
  mkdir percona-compatlib
  pushd percona-compatlib
  wget %{compatsrc}
%if 0%{?rhel} > 6
  rpm2cpio Percona-Server-shared-%{compat_prefix}-%{compatver}-rel%{percona_compatver}.1.el7.x86_64.rpm | cpio --extract --make-directories --verbose
%else
  rpm2cpio Percona-Server-shared-%{compat_prefix}-%{compatver}-rel%{percona_compatver}.624.rhel6.x86_64.rpm | cpio --extract --make-directories --verbose
%endif # 0%{?rhel} > 6
  popd
)
%endif # 0%{?compatlib}

# Build debug versions of mysqld and libmysqld.a
mkdir debug
(
  cd debug
  # Attempt to remove any optimisation flags from the debug build
  optflags=$(echo "%{optflags}" | sed -e 's/-O2 / /' -e 's/-Wp,-D_FORTIFY_SOURCE=2/ -Wno-missing-field-initializers -Wno-error /' -e 's/%{_lto_cflags}/ /')
  cmake ../%{src_dir} \
           -DBUILD_CONFIG=mysql_release \
           -DINSTALL_LAYOUT=RPM \
           -DCMAKE_BUILD_TYPE=Debug \
           -DWITH_BOOST=.. \
           -DCMAKE_C_FLAGS="$optflags" \
           -DCMAKE_CXX_FLAGS="$optflags" \
           -DUSE_LD_LLD=0 \
           -DWITH_AUTHENTICATION_CLIENT_PLUGINS=1 \
           -DWITH_CURL=system \
%if 0%{?systemd}
           -DWITH_SYSTEMD=1 \
%endif
           -DWITH_INNODB_MEMCACHED=1 \
           -DINSTALL_LIBDIR="%{_lib}/mysql" \
           -DINSTALL_PLUGINDIR="%{_lib}/mysql/plugin" \
           -DMYSQL_UNIX_ADDR="%{mysqldatadir}/mysql.sock" \
           -DINSTALL_MYSQLSHAREDIR=share/percona-server \
           -DINSTALL_SUPPORTFILESDIR=share/percona-server \
           -DFEATURE_SET="%{feature_set}" \
           -DWITH_PAM=1 \
           -DWITH_ROCKSDB=1 \
           -DROCKSDB_DISABLE_AVX2=1 \
           -DROCKSDB_DISABLE_MARCH_NATIVE=1 \
           -DWITH_INNODB_MEMCACHED=1 \
           -DMYSQL_MAINTAINER_MODE=OFF \
           -DFORCE_INSOURCE_BUILD=1 \
           -DWITH_NUMA=1 \
           -DWITH_LDAP=system \
           -DWITH_PACKAGE_FLAGS=OFF \
           -DWITH_SYSTEM_LIBS=ON \
           -DWITH_PROTOBUF=bundled \
           -DWITH_RAPIDJSON=bundled \
           -DWITH_ICU=bundled \
           -DWITH_LZ4=bundled \
           -DWITH_ZLIB=bundled \
           -DWITH_ZSTD=bundled \
           -DWITH_READLINE=system \
           -DWITH_LIBEVENT=bundled \
           -DWITH_FIDO=bundled \
           -DWITH_ENCRYPTION_UDF=ON \
           -DWITH_KEYRING_VAULT=ON \
           %{?ssl_option} \
           %{?mecab_option} \
           -DCOMPILATION_COMMENT="%{compilation_comment_debug}" %{TOKUDB_FLAGS} %{TOKUDB_DEBUG_OFF} %{ROCKSDB_FLAGS}
  echo BEGIN_DEBUG_CONFIG ; egrep '^#define' include/config.h ; echo END_DEBUG_CONFIG
  make %{?_smp_mflags} VERBOSE=1
)
# Build full release
mkdir release
(
  cd release
  cmake ../%{src_dir} \
           -DBUILD_CONFIG=mysql_release \
           -DINSTALL_LAYOUT=RPM \
           -DCMAKE_BUILD_TYPE=RelWithDebInfo \
           -DWITH_BOOST=.. \
           -DCMAKE_C_FLAGS="%{optflags}" \
           -DCMAKE_CXX_FLAGS="%{optflags}" \
           -DUSE_LD_LLD=0 \
           -DWITH_AUTHENTICATION_CLIENT_PLUGINS=1 \
           -DWITH_CURL=system \
%if 0%{?systemd}
           -DWITH_SYSTEMD=1 \
%endif
           -DWITH_INNODB_MEMCACHED=1 \
           -DINSTALL_LIBDIR="%{_lib}/mysql" \
           -DINSTALL_PLUGINDIR="%{_lib}/mysql/plugin" \
           -DMYSQL_UNIX_ADDR="%{mysqldatadir}/mysql.sock" \
           -DINSTALL_MYSQLSHAREDIR=share/percona-server \
           -DINSTALL_SUPPORTFILESDIR=share/percona-server \
           -DFEATURE_SET="%{feature_set}" \
           -DWITH_PAM=1 \
           -DWITH_ROCKSDB=1 \
           -DROCKSDB_DISABLE_AVX2=1 \
           -DROCKSDB_DISABLE_MARCH_NATIVE=1 \
           -DWITH_INNODB_MEMCACHED=1 \
           -DMYSQL_MAINTAINER_MODE=OFF \
           -DFORCE_INSOURCE_BUILD=1 \
           -DWITH_NUMA=1 \
           -DWITH_LDAP=system \
           -DWITH_PACKAGE_FLAGS=OFF \
           -DWITH_SYSTEM_LIBS=ON \
           -DWITH_LZ4=bundled \
           -DWITH_ZLIB=bundled \
           -DWITH_PROTOBUF=bundled \
           -DWITH_RAPIDJSON=bundled \
           -DWITH_ICU=bundled \
           -DWITH_READLINE=system \
           -DWITH_LIBEVENT=bundled \
           -DWITH_ZSTD=bundled \
           -DWITH_FIDO=bundled \
           -DWITH_ENCRYPTION_UDF=ON \
           -DWITH_KEYRING_VAULT=ON \
           %{?ssl_option} \
           %{?mecab_option} \
           -DCOMPILATION_COMMENT="%{compilation_comment_release}" %{TOKUDB_FLAGS} %{TOKUDB_DEBUG_OFF} %{ROCKSDB_FLAGS}
  echo BEGIN_NORMAL_CONFIG ; egrep '^#define' include/config.h ; echo END_NORMAL_CONFIG
  make %{?_smp_mflags} VERBOSE=1
)

%install
%if 0%{?compatlib}
  # Install compat libs
  %if 0%{?rhel} > 6
    install -D -m 0755 percona-compatlib/usr/lib64/libmysqlclient.so.18.1.0 %{buildroot}%{_libdir}/mysql/libmysqlclient.so.18.1.0
    install -D -m 0755 percona-compatlib/usr/lib64/libmysqlclient_r.so.18.1.0 %{buildroot}%{_libdir}/mysql/libmysqlclient_r.so.18.1.0
  %else
    install -D -m 0755 percona-compatlib/usr/lib64/libmysqlclient.so.16.0.0 %{buildroot}%{_libdir}/mysql/libmysqlclient.so.16.0.0
    install -D -m 0755 percona-compatlib/usr/lib64/libmysqlclient_r.so.16.0.0 %{buildroot}%{_libdir}/mysql/libmysqlclient_r.so.16.0.0
  %endif # 0%{?rhel} > 6
%endif # 0%{?compatlib}

MBD=$RPM_BUILD_DIR/%{src_dir}

# Ensure that needed directories exists
install -d -m 0751 %{buildroot}/var/lib/mysql
install -d -m 0755 %{buildroot}/var/run/mysqld
install -d -m 0750 %{buildroot}/var/lib/mysql-files
install -d -m 0750 %{buildroot}/var/lib/mysql-keyring

# Router directories
install -d -m 0755 %{buildroot}/var/log/mysqlrouter
install -d -m 0755 %{buildroot}/var/run/mysqlrouter

# Install all binaries
cd $MBD/release
make DESTDIR=%{buildroot} install

# Install logrotate and autostart
#install -D -m 0644 packaging/rpm-common/mysql.logrotate %{buildroot}%{_sysconfdir}/logrotate.d/mysql
#investigate this logrotate
install -D -m 0644 $MBD/release/support-files/mysql-log-rotate %{buildroot}%{_sysconfdir}/logrotate.d/mysql
install -D -m 0644 $MBD/%{src_dir}/build-ps/rpm/mysqld.cnf %{buildroot}%{_sysconfdir}/my.cnf
install -d %{buildroot}%{_sysconfdir}/my.cnf.d

#%if 0%{?systemd}
#%else
%if 0%{?rhel} < 7
  install -D -m 0755 $MBD/%{src_dir}/build-ps/rpm/mysql.init %{buildroot}%{_sysconfdir}/init.d/mysql
%endif

# Add libdir to linker
install -d -m 0755 %{buildroot}%{_sysconfdir}/ld.so.conf.d
echo "%{_libdir}/mysql" > %{buildroot}%{_sysconfdir}/ld.so.conf.d/mysql-%{_arch}.conf

# multiarch support
%ifarch %{multiarchs}
  mv %{buildroot}/%{_bindir}/mysql_config %{buildroot}/%{_bindir}/mysql_config-%{__isa_bits}
  install -p -m 0755 %{SOURCE5} %{buildroot}/%{_bindir}/mysql_config
%endif

%if 0%{?systemd}
%else
install -D -p -m 0755 packaging/rpm-common/mysqlrouter.init %{buildroot}%{_sysconfdir}/init.d/mysqlrouter
%endif
install -D -p -m 0644 packaging/rpm-common/mysqlrouter.conf %{buildroot}%{_sysconfdir}/mysqlrouter/mysqlrouter.conf

# set rpath for plugin to use private/libfido2.so
patchelf --debug --set-rpath '$ORIGIN/../private' %{buildroot}/%{_libdir}/mysql/plugin/authentication_fido.so

# Remove files pages we explicitly do not want to package
rm -rf %{buildroot}%{_infodir}/mysql.info*
rm -rf %{buildroot}%{_datadir}/percona-server/mysql.server
rm -rf %{buildroot}%{_datadir}/percona-server/mysqld_multi.server
rm -f %{buildroot}%{_datadir}/percona-server/win_install_firewall.sql
rm -f %{buildroot}%{_datadir}/percona-server/audit_log_filter_win_install.sql
rm -rf %{buildroot}%{_bindir}/mysql_embedded
rm -rf %{buildroot}/usr/cmake/coredumper-relwithdebinfo.cmake
rm -rf %{buildroot}/usr/cmake/coredumper.cmake
rm -rf %{buildroot}/usr/include/kmip.h
rm -rf %{buildroot}/usr/include/kmippp.h
rm -rf %{buildroot}/usr/lib/libkmip.a
rm -rf %{buildroot}/usr/lib/libkmippp.a
%if 0%{?tokudb}
  rm -f %{buildroot}%{_prefix}/README.md
  rm -f %{buildroot}%{_prefix}/COPYING.AGPLv3
  rm -f %{buildroot}%{_prefix}/COPYING.GPLv2
  rm -f %{buildroot}%{_prefix}/PATENTS
%endif

# Remove upcoming man pages, to avoid breakage when they materialize
# Keep this comment as a placeholder for future cases
# rm -f %{buildroot}%{_mandir}/man1/<manpage>.1

# Remove removed manpages here until they are removed from the docs repo

%check
%if 0%{?runselftest}
  pushd release
    make test VERBOSE=1
    export MTR_BUILD_THREAD=auto
  pushd mysql-test
  ./mtr \
    --mem --parallel=auto --force --retry=0 \
    --mysqld=--binlog-format=mixed \
    --suite-timeout=720 --testcase-timeout=30 \
    --clean-vardir
  rm -r $(readlink var) var
%endif

%pretrans -n percona-server-server
if [ -d %{_datadir}/mysql ] && [ ! -L %{_datadir}/mysql ]; then
  MYCNF_PACKAGE=$(rpm -qf /usr/share/mysql --queryformat "%{NAME}")
fi

if [ "$MYCNF_PACKAGE" == "mariadb-libs" -o "$MYCNF_PACKAGE" == "mysql-libs" ]; then
  MODIFIED=$(rpm -Va "$MYCNF_PACKAGE" | grep '/usr/share/mysql' | awk '{print $1}' | grep -c 5)
  if [ "$MODIFIED" == 1 ]; then
    cp -r %{_datadir}/mysql %{_datadir}/mysql.old
  fi
fi

%pre -n percona-server-server
/usr/sbin/groupadd -g 27 -o -r mysql >/dev/null 2>&1 || :
/usr/sbin/useradd -M %{!?el5:-N} -g mysql -o -r -d /var/lib/mysql -s /bin/false \
    -c "Percona Server" -u 27 mysql >/dev/null 2>&1 || :
if [ "$1" = 1 ]; then
  if [ -f %{_sysconfdir}/my.cnf ]; then
    timestamp=$(date '+%Y%m%d-%H%M')
    cp %{_sysconfdir}/my.cnf \
    %{_sysconfdir}/my.cnf.rpmsave-${timestamp}
  fi
fi

%post -n percona-server-server
datadir=$(/usr/bin/my_print_defaults server mysqld | grep '^--datadir=' | sed -n 's/--datadir=//p' | tail -n 1)
/bin/chmod 0751 "$datadir" >/dev/null 2>&1 || :
if [ ! -e /var/log/mysqld.log ]; then
    /usr/bin/install -m0640 -omysql -gmysql /dev/null /var/log/mysqld.log
fi
#/bin/touch /var/log/mysqld.log >/dev/null 2>&1 || :
%if 0%{?systemd}
  %systemd_post mysqld.service
  if [ $1 == 1 ]; then
      /usr/bin/systemctl enable mysqld >/dev/null 2>&1 || :
  fi
%else
  if [ $1 == 1 ]; then
      /sbin/chkconfig --add mysql
  fi
%endif

if [ -d /etc/percona-server.conf.d ]; then
    CONF_EXISTS=$(grep "percona-server.conf.d" /etc/my.cnf | wc -l)
    if [ ${CONF_EXISTS} = 0 ]; then
        echo "!includedir /etc/percona-server.conf.d/" >> /etc/my.cnf
    fi
fi

echo "Percona Server is distributed with several useful UDF (User Defined Function) from Percona Toolkit."
echo "Run the following commands to create these functions:"
echo "mysql -e \"CREATE FUNCTION fnv1a_64 RETURNS INTEGER SONAME 'libfnv1a_udf.so'\""
echo "mysql -e \"CREATE FUNCTION fnv_64 RETURNS INTEGER SONAME 'libfnv_udf.so'\""
echo "mysql -e \"CREATE FUNCTION murmur_hash RETURNS INTEGER SONAME 'libmurmur_udf.so'\""
echo "See http://www.percona.com/doc/percona-server/8.0/management/udf_percona_toolkit.html for more details"

%preun -n percona-server-server
%if 0%{?systemd}
  %systemd_preun mysqld.service
%else
  if [ "$1" = 0 ]; then
    /sbin/service mysql stop >/dev/null 2>&1 || :
    /sbin/chkconfig --del mysql
  fi
%endif
if [ "$1" = 0 ]; then
  if [ -L %{_datadir}/mysql ]; then
      rm %{_datadir}/mysql
  fi
  if [ -f %{_sysconfdir}/my.cnf ]; then
    cp %{_sysconfdir}/my.cnf \
    %{_sysconfdir}/my.cnf.rpmsave
  fi
fi

%postun -n percona-server-server
%if 0%{?systemd}
  %systemd_postun_with_restart mysqld.service
%else
  if [ $1 -ge 1 ]; then
    /sbin/service mysql condrestart >/dev/null 2>&1 || :
  fi
%endif

%posttrans -n percona-server-server
if [ -d %{_datadir}/mysql ] && [ ! -L %{_datadir}/mysql ]; then
  MYCNF_PACKAGE=$(rpm -qf /usr/share/mysql --queryformat "%{NAME}")
  if [ "$MYCNF_PACKAGE" == "file %{_datadir}/mysql is not owned by any package" ]; then
    mv %{_datadir}/mysql %{_datadir}/mysql.old
  fi
fi

if [ ! -d %{_datadir}/mysql ] && [ ! -L %{_datadir}/mysql ]; then
    ln -s %{_datadir}/percona-server %{_datadir}/mysql
fi

%post -n percona-server-shared -p /sbin/ldconfig

%postun -n percona-server-shared -p /sbin/ldconfig

%if 0%{?compatlib}
%if 0%{?rhel} > 6
%post -n percona-server-shared-compat
for lib in libmysqlclient{.so.18.0.0,.so.18,_r.so.18.0.0,_r.so.18}; do
  if [ ! -f %{_libdir}/mysql/${lib} ]; then
    ln -s libmysqlclient.so.18.1.0 %{_libdir}/mysql/${lib};
  fi
done
/sbin/ldconfig

%postun -n percona-server-shared-compat
for lib in libmysqlclient{.so.18.0.0,.so.18,_r.so.18.0.0,_r.so.18}; do
  if [ -h %{_libdir}/mysql/${lib} ]; then
    rm -f %{_libdir}/mysql/${lib};
  fi
done
/sbin/ldconfig
%else
%post -n percona-server-shared-compat
for lib in libmysqlclient{.so.16.0.0,.so.16,_r.so.16.0.0,_r.so.16}; do
  if [ ! -f %{_libdir}/mysql/${lib} ]; then
    ln -s libmysqlclient.so.16.1.0 %{_libdir}/mysql/${lib};
  fi
done
/sbin/ldconfig

%postun -n percona-server-shared-compat
for lib in libmysqlclient{.so.16.0.0,.so.16,_r.so.16.0.0,_r.so.16}; do
  if [ -h %{_libdir}/mysql/${lib} ]; then
    rm -f %{_libdir}/mysql/${lib};
  fi
done
/sbin/ldconfig
%endif
%endif

%if 0%{?tokudb}
%post -n percona-server-tokudb
if [ $1 -eq 1 ] ; then
  echo -e "\n\n * This release of Percona Server is distributed with TokuDB storage engine."
  echo -e " * Run the following script to enable the TokuDB storage engine in Percona Server:\n"
  echo -e "\tps-admin --enable-tokudb -u <mysql_admin_user> -p[mysql_admin_pass] [-S <socket>] [-h <host> -P <port>]\n"
  echo -e " * See http://www.percona.com/doc/percona-server/8.0/tokudb/tokudb_installation.html for more installation details\n"
  echo -e " * See http://www.percona.com/doc/percona-server/8.0/tokudb/tokudb_intro.html for an introduction to TokuDB\n\n"
fi
%endif

%if 0%{?rocksdb}
%post -n percona-server-rocksdb
if [ $1 -eq 1 ] ; then
  echo -e "\n\n * This release of Percona Server is distributed with RocksDB storage engine."
  echo -e " * Run the following script to enable the RocksDB storage engine in Percona Server:\n"
  echo -e "\tps-admin --enable-rocksdb -u <mysql_admin_user> -p[mysql_admin_pass] [-S <socket>] [-h <host> -P <port>]\n"
fi
%endif

%pre -n percona-mysql-router
/usr/sbin/groupadd -r mysqlrouter >/dev/null 2>&1 || :
/usr/sbin/useradd -M -N -g mysqlrouter -r -d /var/lib/mysqlrouter -s /bin/false \
    -c "Percona MySQL Router" mysqlrouter >/dev/null 2>&1 || :

%post -n percona-mysql-router
/sbin/ldconfig
%if 0%{?systemd}
%systemd_post mysqlrouter.service
%else
/sbin/chkconfig --add mysqlrouter
%endif # systemd

%preun -n percona-mysql-router
%if 0%{?systemd}
%systemd_preun mysqlrouter.service
%else
if [ "$1" = 0 ]; then
    /sbin/service mysqlrouter stop >/dev/null 2>&1 || :
    /sbin/chkconfig --del mysqlrouter
fi
%endif # systemd

%postun -n percona-mysql-router
/sbin/ldconfig
%if 0%{?systemd}
%systemd_postun_with_restart mysqlrouter.service
%else
if [ $1 -ge 1 ]; then
    /sbin/service mysqlrouter condrestart >/dev/null 2>&1 || :
fi
%endif # systemd


%files -n percona-server-server
%defattr(-, root, root, -)
%doc %{?license_files_server}
%doc %{src_dir}/Docs/INFO_SRC*
%doc release/Docs/INFO_BIN*
%attr(644, root, root) %{_mandir}/man1/innochecksum.1*
%attr(644, root, root) %{_mandir}/man1/ibd2sdi.1*
%attr(644, root, root) %{_mandir}/man1/my_print_defaults.1*
%attr(644, root, root) %{_mandir}/man1/myisam_ftdump.1*
%attr(644, root, root) %{_mandir}/man1/myisamchk.1*
%attr(644, root, root) %{_mandir}/man1/myisamlog.1*
%attr(644, root, root) %{_mandir}/man1/myisampack.1*
%attr(644, root, root) %{_mandir}/man8/mysqld.8*
%if 0%{?systemd}
%else
%attr(644, root, root) %{_mandir}/man1/mysqld_multi.1*
%attr(644, root, root) %{_mandir}/man1/mysqld_safe.1*
%endif
%attr(644, root, root) %{_mandir}/man1/mysqldumpslow.1*
%attr(644, root, root) %{_mandir}/man1/mysql_secure_installation.1*
%attr(644, root, root) %{_mandir}/man1/mysql_upgrade.1*
%attr(644, root, root) %{_mandir}/man1/mysqlman.1*
%attr(644, root, root) %{_mandir}/man1/mysql_tzinfo_to_sql.1*
%attr(644, root, root) %{_mandir}/man1/perror.1*
%attr(644, root, root) %{_mandir}/man1/mysql_ssl_rsa_setup.1*
%attr(644, root, root) %{_mandir}/man1/lz4_decompress.1*
%attr(644, root, root) %{_mandir}/man1/zlib_decompress.1*
%if 0%{?rhel} < 7
%attr(644, root, root) %{_mandir}/man1/mysql.server.1*
%endif

%config(noreplace) %{_sysconfdir}/my.cnf
%dir %{_sysconfdir}/my.cnf.d

%attr(755, root, root) %{_bindir}/comp_err
%attr(755, root, root) %{_bindir}/innochecksum
%attr(755, root, root) %{_bindir}/ibd2sdi
%attr(755, root, root) %{_bindir}/my_print_defaults
%attr(755, root, root) %{_bindir}/myisam_ftdump
%attr(755, root, root) %{_bindir}/myisamchk
%attr(755, root, root) %{_bindir}/myisamlog
%attr(755, root, root) %{_bindir}/myisampack
%attr(755, root, root) %{_bindir}/mysql_secure_installation
%attr(755, root, root) %{_bindir}/mysql_tzinfo_to_sql
%attr(755, root, root) %{_bindir}/mysql_upgrade
%attr(755, root, root) %{_bindir}/mysqldumpslow
%attr(755, root, root) %{_bindir}/ps_mysqld_helper
%attr(755, root, root) %{_bindir}/perror
%attr(755, root, root) %{_bindir}/mysql_ssl_rsa_setup
%attr(755, root, root) %{_bindir}/lz4_decompress
%attr(755, root, root) %{_bindir}/zlib_decompress
%attr(755, root, root) %{_bindir}/ps-admin
%if 0%{?systemd}
%attr(755, root, root) %{_bindir}/mysqld_pre_systemd
%attr(755, root, root) %{_bindir}/mysqld_safe
%else
%attr(755, root, root) %{_bindir}/mysqld_multi
%attr(755, root, root) %{_bindir}/mysqld_safe
%endif
%attr(755, root, root) %{_sbindir}/mysqld
%attr(755, root, root) %{_sbindir}/mysqld-debug
%dir %{_libdir}/mysql/private
%attr(755, root, root) %{_libdir}/mysql/private/libprotobuf-lite.so.*
%attr(755, root, root) %{_libdir}/mysql/private/libprotobuf.so.*
%attr(755, root, root) %{_libdir}/mysql/private/libfido2.so.*

%dir %{_libdir}/mysql/plugin
%attr(755, root, root) %{_libdir}/mysql/plugin/adt_null.so
%attr(755, root, root) %{_libdir}/mysql/plugin/auth_socket.so
%attr(755, root, root) %{_libdir}/mysql/plugin/authentication_ldap_sasl_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/authentication_kerberos_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/authentication_fido_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/group_replication.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_log_sink_syseventlog.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_log_sink_json.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_log_filter_dragnet.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_mysqlbackup.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_validate_password.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_audit_api_message_emit.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_query_attributes.so
%attr(755, root, root) %{_libdir}/mysql/plugin/connection_control.so
%attr(755, root, root) %{_libdir}/mysql/plugin/ddl_rewriter.so
%attr(755, root, root) %{_libdir}/mysql/plugin/ha_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/ha_mock.so
%attr(755, root, root) %{_libdir}/mysql/plugin/keyring_file.so
%attr(755, root, root) %{_libdir}/mysql/plugin/keyring_udf.so
%attr(755, root, root) %{_libdir}/mysql/plugin/innodb_engine.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libmemcached.so
%attr(755, root, root) %{_libdir}/mysql/plugin/locking_service.so
%attr(755, root, root) %{_libdir}/mysql/plugin/mypluglib.so
%attr(755, root, root) %{_libdir}/mysql/plugin/mysql_clone.so
%attr(755, root, root) %{_libdir}/mysql/plugin/mysql_no_login.so
%attr(755, root, root) %{_libdir}/mysql/plugin/rewrite_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/rewriter.so
%attr(755, root, root) %{_libdir}/mysql/plugin/semisync_master.so
%attr(755, root, root) %{_libdir}/mysql/plugin/semisync_slave.so
%attr(755, root, root) %{_libdir}/mysql/plugin/validate_password.so
%attr(755, root, root) %{_libdir}/mysql/plugin/version_token.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_keyring_file.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_audit_api_message.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_host_application_signal.so
%attr(755, root, root) %{_libdir}/mysql/plugin/test_services_host_application_signal.so
%attr(755, root, root) %{_libdir}/mysql/plugin/data_masking*
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_udf_services.so
%attr(755, root, root) %{_libdir}/mysql/plugin/authentication_ldap_simple.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_component_deinit.so
%attr(755, root, root) %{_libdir}/mysql/plugin/binlog_utils_udf.so
%attr(755, root, root) %{_libdir}/mysql/plugin/test_udf_wrappers.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_reference_cache.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_mysql_system_variable_set.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_table_access.so
%attr(755, root, root) %{_libdir}/mysql/plugin/semisync_replica.so
%attr(755, root, root) %{_libdir}/mysql/plugin/semisync_source.so

%dir %{_libdir}/mysql/plugin/debug
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/data_masking.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/adt_null.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/auth_socket.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/authentication_ldap_simple.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/authentication_ldap_sasl_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/authentication_kerberos_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/authentication_fido_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/group_replication.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_log_sink_syseventlog.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_log_sink_json.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_log_filter_dragnet.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_mysqlbackup.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_validate_password.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_audit_api_message_emit.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_query_attributes.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/connection_control.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/ddl_rewriter.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/ha_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/ha_mock.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/keyring_file.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/keyring_udf.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/innodb_engine.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libmemcached.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/locking_service.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/mypluglib.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/mysql_clone.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/mysql_no_login.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/rewrite_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/rewriter.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/semisync_master.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/semisync_slave.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/validate_password.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/version_token.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_keyring_file.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_audit_api_message.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_host_application_signal.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/test_services_host_application_signal.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_udf_services.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_component_deinit.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/binlog_utils_udf.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/test_udf_wrappers.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_reference_cache.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_mysql_system_variable_set.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_table_access.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/semisync_replica.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/semisync_source.so
%if 0%{?mecab}
%{_libdir}/mysql/mecab
%attr(755, root, root) %{_libdir}/mysql/plugin/libpluginmecab.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libpluginmecab.so
%endif
# Percona plugins
%attr(755, root, root) %{_libdir}/mysql/plugin/audit_log.so
#%attr(644, root, root) %{_datadir}/mysql-*/audit_log_filter_linux_install.sql
#%attr(755, root, root) %{_libdir}/mysql/plugin/authentication_pam.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/authentication_ldap_sasl.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/authentication_ldap_simple.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/keyring_okv.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/keyring_encrypted_file.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/mysql_clone.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/thread_pool.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/openssl_udf.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/firewall.so
#%attr(644, root, root) %{_datadir}/mysql-*/linux_install_firewall.sql
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/audit_log.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/scalability_metrics.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/debug/scalability_metrics.so
%attr(755, root, root) %{_libdir}/mysql/plugin/auth_pam.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/auth_pam.so
%attr(755, root, root) %{_libdir}/mysql/plugin/auth_pam_compat.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/auth_pam_compat.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libfnv1a_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libfnv1a_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/libfnv_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libfnv_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/libmurmur_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libmurmur_udf.*
%attr(755, root, root) %{_libdir}/mysql/plugin/dialog.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/dialog.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/query_response_time.so
#%attr(755, root, root) %{_libdir}/mysql/plugin/debug/query_response_time.so
%attr(755, root, root) %{_libdir}/mysql/plugin/keyring_vault.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/keyring_vault.so
%attr(755, root, root) %{_libdir}/mysql/plugin/procfs.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/procfs.so
%attr(755, root, root) %{_libdir}/mysql/plugin/authentication_fido.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/authentication_fido.so
%attr(755, root, root) %{_libdir}/mysql/plugin/authentication_ldap_sasl.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/authentication_ldap_sasl.so

%if 0%{?rhel} > 6
%attr(755, root, root) %{_libdir}/mysql/plugin/component_encryption_udf.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_encryption_udf.so
%endif
%attr(755, root, root) %{_libdir}/mysql/plugin/component_keyring_kms.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_keyring_kms.so
#
#%attr(644, root, root) %{_datadir}/percona-server/fill_help_tables.sql
#%attr(644, root, root) %{_datadir}/percona-server/mysql_sys_schema.sql
#%attr(644, root, root) %{_datadir}/percona-server/mysql_system_tables.sql
#%attr(644, root, root) %{_datadir}/percona-server/mysql_system_tables_data.sql
#%attr(644, root, root) %{_datadir}/percona-server/mysql_test_data_timezone.sql
%attr(644, root, root) %{_datadir}/percona-server/mysql-log-rotate
#%attr(644, root, root) %{_datadir}/percona-server/mysql_security_commands.sql
%attr(644, root, root) %{_datadir}/percona-server/dictionary.txt
%attr(644, root, root) %{_datadir}/percona-server/innodb_memcached_config.sql
%attr(644, root, root) %{_datadir}/percona-server/install_rewriter.sql
%attr(644, root, root) %{_datadir}/percona-server/uninstall_rewriter.sql
%if 0%{?systemd}
%attr(644, root, root) %{_unitdir}/mysqld.service
%attr(644, root, root) %{_unitdir}/mysqld@.service
%attr(644, root, root) %{_prefix}/lib/tmpfiles.d/mysql.conf
%else
%attr(755, root, root) %{_sysconfdir}/init.d/mysql
%endif
%attr(644, root, root) %config(noreplace,missingok) %{_sysconfdir}/logrotate.d/mysql
%dir %attr(751, mysql, mysql) /var/lib/mysql
%dir %attr(755, mysql, mysql) /var/run/mysqld
%dir %attr(750, mysql, mysql) /var/lib/mysql-files
%dir %attr(750, mysql, mysql) /var/lib/mysql-keyring

%attr(755, root, root) %{_datadir}/percona-server/messages_to_clients.txt
%attr(755, root, root) %{_datadir}/percona-server/messages_to_error_log.txt
%attr(755, root, root) %{_datadir}/percona-server/charsets/
%attr(755, root, root) %{_datadir}/percona-server/bulgarian/
%attr(755, root, root) %{_datadir}/percona-server/czech/
%attr(755, root, root) %{_datadir}/percona-server/danish/
%attr(755, root, root) %{_datadir}/percona-server/dutch/
%attr(755, root, root) %{_datadir}/percona-server/english/
%attr(755, root, root) %{_datadir}/percona-server/estonian/
%attr(755, root, root) %{_datadir}/percona-server/french/
%attr(755, root, root) %{_datadir}/percona-server/german/
%attr(755, root, root) %{_datadir}/percona-server/greek/
%attr(755, root, root) %{_datadir}/percona-server/hungarian/
%attr(755, root, root) %{_datadir}/percona-server/italian/
%attr(755, root, root) %{_datadir}/percona-server/japanese/
%attr(755, root, root) %{_datadir}/percona-server/korean/
%attr(755, root, root) %{_datadir}/percona-server/norwegian-ny/
%attr(755, root, root) %{_datadir}/percona-server/norwegian/
%attr(755, root, root) %{_datadir}/percona-server/polish/
%attr(755, root, root) %{_datadir}/percona-server/portuguese/
%attr(755, root, root) %{_datadir}/percona-server/romanian/
%attr(755, root, root) %{_datadir}/percona-server/russian/
%attr(755, root, root) %{_datadir}/percona-server/serbian/
%attr(755, root, root) %{_datadir}/percona-server/slovak/
%attr(755, root, root) %{_datadir}/percona-server/spanish/
%attr(755, root, root) %{_datadir}/percona-server/swedish/
%attr(755, root, root) %{_datadir}/percona-server/ukrainian/
#%attr(755, root, root) %{_datadir}/percona-server/mysql_system_users.sql
#
%attr(755, root, root) %{_libdir}/mysql/plugin/component_keyring_kmip.so
%attr(755, root, root) %{_libdir}/mysql/plugin/authentication_oci_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_keyring_kmip.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/authentication_oci_client.so


%files -n percona-server-client
%defattr(-, root, root, -)
%doc %{?license_files_server}
%attr(755, root, root) %{_bindir}/mysql
%attr(755, root, root) %{_bindir}/mysqladmin
%attr(755, root, root) %{_bindir}/mysqlbinlog
%attr(755, root, root) %{_bindir}/mysqlcheck
%attr(755, root, root) %{_bindir}/mysqldump
%attr(755, root, root) %{_bindir}/mysqlimport
%attr(755, root, root) %{_bindir}/mysqlpump
%attr(755, root, root) %{_bindir}/mysqlshow
%attr(755, root, root) %{_bindir}/mysqlslap
%attr(755, root, root) %{_bindir}/mysql_config_editor
%attr(755, root, root) %{_bindir}/mysql_migrate_keyring

%attr(644, root, root) %{_mandir}/man1/mysql.1*
%attr(644, root, root) %{_mandir}/man1/mysqladmin.1*
%attr(644, root, root) %{_mandir}/man1/mysqlbinlog.1*
%attr(644, root, root) %{_mandir}/man1/mysqlcheck.1*
%attr(644, root, root) %{_mandir}/man1/mysqldump.1*
%attr(644, root, root) %{_mandir}/man1/mysqlpump.1*
%attr(644, root, root) %{_mandir}/man1/mysqlimport.1*
%attr(644, root, root) %{_mandir}/man1/mysqlshow.1*
%attr(644, root, root) %{_mandir}/man1/mysqlslap.1*
%attr(644, root, root) %{_mandir}/man1/mysql_config_editor.1*

%files -n percona-server-devel
%defattr(-, root, root, -)
%doc %{?license_files_server}
%attr(644, root, root) %{_mandir}/man1/comp_err.1*
%attr(644, root, root) %{_mandir}/man1/mysql_config.1*
%attr(755, root, root) %{_bindir}/mysql_config
%attr(755, root, root) %{_bindir}/mysql_config-%{__isa_bits}
%{_includedir}/mysql
%{_datadir}/aclocal/mysql.m4
%{_libdir}/mysql/lib%{shared_lib_pri_name}.a
%{_libdir}/mysql/libmysqlservices.a
%{_libdir}/mysql/lib%{shared_lib_pri_name}.so
%{_libdir}/pkgconfig/%{shared_lib_pri_name}.pc

%files -n percona-server-shared
%defattr(-, root, root, -)
%doc %{?license_files_server}
%dir %attr(755, root, root) %{_libdir}/mysql
%attr(644, root, root) %{_sysconfdir}/ld.so.conf.d/mysql-%{_arch}.conf
%{_libdir}/mysql/lib%{shared_lib_pri_name}.so.21*
#coredumper
%attr(755, root, root) %{_includedir}/coredumper/coredumper.h
%attr(755, root, root) /usr/lib/libcoredumper.a

%if 0%{?compatlib}
%files -n percona-server-shared-compat
%defattr(-, root, root, -)
%doc %{?license_files_server}
%dir %attr(755, root, root) %{_libdir}/mysql
%attr(644, root, root) %{_sysconfdir}/ld.so.conf.d/mysql-%{_arch}.conf
%{_libdir}/mysql/libmysqlclient.so.%{compatlib}.*
%{_libdir}/mysql/libmysqlclient_r.so.%{compatlib}.*
%endif

%files -n percona-server-test
%defattr(-, root, root, -)
%doc %{?license_files_server}
%attr(-, root, root) %{_datadir}/mysql-test
%attr(755, root, root) %{_bindir}/mysql_client_test
%attr(755, root, root) %{_bindir}/mysqltest
%attr(755, root, root) %{_bindir}/mysqltest_safe_process
%attr(755, root, root) %{_bindir}/mysqlxtest
%attr(755, root, root) %{_bindir}/mysql_keyring_encryption_test

%attr(755, root, root) %{_libdir}/mysql/plugin/auth.so
%attr(755, root, root) %{_libdir}/mysql/plugin/auth_test_plugin.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_example_component1.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_example_component2.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_example_component3.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_log_sink_test.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_backup_lock_service.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_string_service_charset.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_string_service_long.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_string_service.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_pfs_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_pfs_example_component_population.so
%attr(755, root, root) %{_libdir}/mysql/plugin/pfs_example_plugin_employee.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_pfs_notification.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_pfs_resource_group.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_udf_registration.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_mysql_current_thread_reader.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_udf_reg_3_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_udf_reg_avg_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_udf_reg_int_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_udf_reg_int_same_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_udf_reg_only_3_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_udf_reg_real_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_udf_unreg_3_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_udf_unreg_int_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_udf_unreg_real_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_sys_var_service_int.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_sys_var_service.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_sys_var_service_same.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_sys_var_service_str.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_status_var_service.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_status_var_service_int.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_status_var_service_reg_only.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_status_var_service_str.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_status_var_service_unreg_only.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_system_variable_source.so
%attr(644, root, root) %{_libdir}/mysql/plugin/daemon_example.ini
%attr(755, root, root) %{_libdir}/mysql/plugin/libdaemon_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/replication_observers_example_plugin.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_framework.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_services.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_services_threaded.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_session_detach.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_session_attach.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_session_in_thd.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_session_info.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_2_sessions.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_all_col_types.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_cmds_1.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_commit.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_complex.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_errors.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_lock.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_processlist.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_replication.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_shutdown.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_sleep_is_connected.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_stmt.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_sqlmode.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_stored_procedures_functions.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_views_triggers.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_x_sessions_deinit.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_x_sessions_init.so
%attr(755, root, root) %{_libdir}/mysql/plugin/qa_auth_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/qa_auth_interface.so
%attr(755, root, root) %{_libdir}/mysql/plugin/qa_auth_server.so
%attr(755, root, root) %{_libdir}/mysql/plugin/test_security_context.so
%attr(755, root, root) %{_libdir}/mysql/plugin/test_services_plugin_registry.so
%attr(755, root, root) %{_libdir}/mysql/plugin/test_udf_services.so
%attr(755, root, root) %{_libdir}/mysql/plugin/udf_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_mysqlx_global_reset.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_mysql_runtime_error.so
%attr(755, root, root) %{_libdir}/mysql/plugin/libtest_sql_reset_connection.so
%attr(755, root, root) %{_libdir}/mysql/plugin/component_test_sensitive_system_variables.so
%attr(755, root, root) %{_libdir}/mysql/plugin/conflicting_variables.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_mysql_runtime_error.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_reset_connection.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/auth.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/auth_test_plugin.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_example_component1.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_example_component2.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_example_component3.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_log_sink_test.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_backup_lock_service.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_string_service_charset.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_string_service_long.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_string_service.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_pfs_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_pfs_example_component_population.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/pfs_example_plugin_employee.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_pfs_notification.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_pfs_resource_group.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_udf_registration.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_mysql_current_thread_reader.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_udf_reg_3_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_udf_reg_avg_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_udf_reg_int_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_udf_reg_int_same_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_udf_reg_only_3_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_udf_reg_real_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_udf_unreg_3_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_udf_unreg_int_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_udf_unreg_real_func.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_sys_var_service_int.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_sys_var_service.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_sys_var_service_same.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_sys_var_service_str.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_status_var_service.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_status_var_service_int.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_status_var_service_reg_only.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_status_var_service_str.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_status_var_service_unreg_only.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_system_variable_source.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libdaemon_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/replication_observers_example_plugin.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_framework.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_services.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_services_threaded.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_session_detach.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_session_attach.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_session_in_thd.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_session_info.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_2_sessions.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_all_col_types.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_cmds_1.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_commit.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_complex.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_errors.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_lock.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_processlist.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_replication.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_shutdown.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_sleep_is_connected.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_stmt.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_sqlmode.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_stored_procedures_functions.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_sql_views_triggers.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_x_sessions_deinit.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/libtest_x_sessions_init.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/qa_auth_client.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/qa_auth_interface.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/qa_auth_server.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/test_security_context.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/test_services_plugin_registry.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/test_udf_services.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/udf_example.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_mysqlx_global_reset.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/component_test_sensitive_system_variables.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/conflicting_variables.so

%if 0%{?tokudb}
%files -n percona-server-tokudb
%attr(-, root, root)
%{_bindir}/tokuftdump
%{_libdir}/mysql/plugin/ha_tokudb.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/ha_tokudb.so
%attr(755, root, root) %{_bindir}/tokuft_logprint
%attr(755, root, root) %{_libdir}/mysql/plugin/tokudb_backup.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/tokudb_backup.so
%attr(755, root, root) %{_libdir}/mysql/libHotBackup.so
%{_includedir}/backup.h
%endif

%if 0%{?rocksdb}
%files -n percona-server-rocksdb
%attr(-, root, root)
%{_libdir}/mysql/plugin/ha_rocksdb.so
%attr(755, root, root) %{_libdir}/mysql/plugin/debug/ha_rocksdb.so
%attr(755, root, root) %{_bindir}/ldb
%attr(755, root, root) %{_bindir}/mysql_ldb
%attr(755, root, root) %{_bindir}/sst_dump
%endif

%files -n percona-mysql-router
%defattr(-, root, root, -)
%doc %{src_dir}/router/README.router  %{src_dir}/router/LICENSE.router
%dir %{_sysconfdir}/mysqlrouter
%config(noreplace) %{_sysconfdir}/mysqlrouter/mysqlrouter.conf
%attr(644, root, root) %config(noreplace,missingok) %{_sysconfdir}/logrotate.d/mysqlrouter
%{_bindir}/mysqlrouter
%{_bindir}/mysqlrouter_keyring
%{_bindir}/mysqlrouter_passwd
%{_bindir}/mysqlrouter_plugin_info
%attr(644, root, root) %{_mandir}/man1/mysqlrouter.1*
%attr(644, root, root) %{_mandir}/man1/mysqlrouter_passwd.1*
%attr(644, root, root) %{_mandir}/man1/mysqlrouter_plugin_info.1*
%if 0%{?systemd}
%{_unitdir}/mysqlrouter.service
%{_tmpfilesdir}/mysqlrouter.conf
%else
%{_sysconfdir}/init.d/mysqlrouter
%endif
%{_libdir}/mysqlrouter/private/libmysqlharness.so.*
%{_libdir}/mysqlrouter/private/libmysqlharness_stdx.so.*
%{_libdir}/mysqlrouter/private/libmysqlharness_tls.so.*
%{_libdir}/mysqlrouter/private/libmysqlrouter.so.*
%{_libdir}/mysqlrouter/private/libmysqlrouter_connection_pool.so.*
%{_libdir}/mysqlrouter/private/libmysqlrouter_http.so.*
%{_libdir}/mysqlrouter/private/libmysqlrouter_http_auth_backend.so.*
%{_libdir}/mysqlrouter/private/libmysqlrouter_http_auth_realm.so.*
%{_libdir}/mysqlrouter/private/libprotobuf-lite.so.*
%{_libdir}/mysqlrouter/private/libmysqlrouter_io_component.so.1
%{_libdir}/mysqlrouter/private/libmysqlrouter_metadata_cache.so.*
%{_libdir}/mysqlrouter/private/libmysqlrouter_mysqlxmessages.so.*
%{_libdir}/mysqlrouter/private/libmysqlrouter_routing.so.*
%dir %{_libdir}/mysqlrouter
%dir %{_libdir}/mysqlrouter/private
%{_libdir}/mysqlrouter/*.so
%dir %attr(755, mysqlrouter, mysqlrouter) /var/log/mysqlrouter
%dir %attr(755, mysqlrouter, mysqlrouter) /var/run/mysqlrouter

%files -n percona-icu-data-files
%defattr(-, root, root, -)
%doc %{?license_files_server}
%dir %attr(755, root, root) %{_libdir}/mysql/private/icudt69l
%{_libdir}/mysql/private/icudt69l/unames.icu
%{_libdir}/mysql/private/icudt69l/brkitr

%changelog
* Fri Feb 12 2021 Percona Development Team <info@percona.com> - 8.0.22-13
- Release 8.0.22-13

* Wed Aug  2 2017 Evgeniy Patlan <evgeniy.patlan@percona.com>
- Added RocksDB

* Thu Sep  1 2016 Evgeniy Patlan <evgeniy.patlan@percona.com>
- fix license field

* Thu Aug 25 2016 Evgeniy Patlan <evgeniy.patlan@percona.com>
- Provide my.cnf for all systems

* Wed Mar 09 2016 Tomislav Plavcic <tomislav.plavcic@percona.com> - 5.7.11-4
- Include mysql-keyring directory
- Provide keyring_file.so plugin

* Thu Feb 11 2016 Tomislav Plavcic <tomislav.plavcic@percona.com> - 5.7.10-3
- Fix for centos6 to write temp pass into log file instead of stdout (#1541769)

* Tue Feb 02 2016 Tomislav Plavcic <tomislav.plavcic@percona.com> - 5.7.10-2rc2
- Re-added TokuBackup to the packaging (JEN-439)

* Thu Dec 10 2015 Tomislav Plavcic <tomislav.plavcic@percona.com> - 5.7.10-1rc1
- Initial release PS 5.7.10-1rc1

* Tue Sep 29 2015 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.9-1
- Updated for 5.7.9
- Added libtest_* plugins to test package subpackage
- Add mysqlpump man page
- Obsolete mysql-connector-c-shared dependencies

* Mon Jul 06 2015 Murthy Narkedimilli <murthy.narkedimilli@oracle.com> - 5.7.8-0.2.rc
- Bumped the version of libmysqlclient.so and libmysqld.so from 20 -> 21.

* Thu Jun 25 2015 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.8-0.2.rc
- Add support for pkg-config

* Wed May 20 2015 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.8-0.2.rc
- Added libtest_framework.so, libtest_services.so, libtest_services_threaded.so plugins
- Build and ship mecab plugin

* Tue Feb 3 2015 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.6-0.2.m16
- Include boost sources
- Fix cmake buildrequires
- Fix build on el5 with gcc44
- Add license info in each subpackage
- Soname bump, more compat packages
- Updated default shell for mysql user
- Added mysql_ssl_rsa_setup
- Include mysql-files directory

* Thu Sep 18 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.6-0.2.m16
- Provide replication_observers_example_plugin.so plugin

* Tue Sep 2 2014 Bjorn Munch <bjorn.munch@oracle.com> - 5.7.6-0.1.m16
- Updated for 5.7.6

* Fri Aug 08 2014 Erlend Dahl <erlend.dahl@oracle.com> - 5.7.5-0.6.m15
- Provide mysql_no_login.so plugin

* Wed Aug 06 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.5-0.5.m15
- Provide mysql-compat-server dependencies

* Wed Jul 09 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.5-0.4.m15
- Remove perl(GD) and dtrace dependencies

* Thu Jun 26 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.5-0.3.m15
- Resolve embedded-devel conflict issue

* Wed Jun 25 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.5-0.2.m15
- Add bench package
- Enable dtrace

* Thu Apr 24 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.5-0.1.m15
- Updated for 5.7.5

* Mon Apr 07 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.4-0.5.m14
- Fix Cflags for el7

* Mon Mar 31 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.4-0.4.m14
- Support for enterprise packages
- Upgrade from MySQL-* packages

* Wed Mar 12 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.4-0.3.m14
- Resolve conflict with mysql-libs-compat

* Thu Mar 06 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.4-0.2.m14
- Resolve conflict issues during upgrade
- Add ha_example.so plugin which is now included

* Fri Feb 07 2014 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.4-0.1.m14
- 5.7.4
- Enable shared libmysqld by cmake option
- Move mysqltest and test plugins to test subpackage

* Mon Nov 18 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.3-0.3.m13
- Fixed isa_bits error

* Fri Oct 25 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.7.3-0.1.m13
- Initial 5.7 port

* Fri Oct 25 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.15-1
- Fixed uln advanced rpm libyassl.a error
- Updated to 5.6.15

* Wed Oct 16 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.14-3
- Fixed mysql_install_db usage
- Improved handling of plugin directory

* Fri Sep 27 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.14-2
- Refresh mysql-install patch and service renaming

* Mon Sep 16 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.14-1
- Updated to 5.6.14

* Wed Sep 04 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.13-5
- Support upgrade from 5.5 ULN packages to 5.6

* Tue Aug 27 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.13-4
- Enhanced perl filtering
- Added openssl-devel to buildreq

* Wed Aug 21 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.13-3
- Removed mysql_embedded binary to resolve multilib conflict issue

* Fri Aug 16 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.13-2
- Fixed Provides and Obsoletes issues in server, test packages

* Wed Aug 14 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.13-1
- Updated to 5.6.13

* Mon Aug 05 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.12-9
- Added files list to embedded packages

* Thu Aug 01 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.12-8
- Updated libmysqld.a with libmysqld.so in embedded package

* Mon Jul 29 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.12-7
- Updated test package dependency from client to server

* Wed Jul 24 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.12-6
- Added libs-compat dependency under libs package to resolve server
  installation conflicts issue.

* Wed Jul 17 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.12-5
- Removed libmysqlclient.so.16 from libs package

* Fri Jul 05 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.12-4
- Adjusted to work on OEL6

* Wed Jun 26 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.12-3
- Move libs to mysql/
- Basic multi arch support
- Fix changelog dates

* Thu Jun 20 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.12-2
- Major cleanup

* Tue Jun 04 2013 Balasubramanian Kandasamy <balasubramanian.kandasamy@oracle.com> - 5.6.12-1
- Updated to 5.6.12

* Mon Nov 05 2012 Joerg Bruehe <joerg.bruehe@oracle.com>

- Allow to override the default to use the bundled yaSSL by an option like
      --define="with_ssl /path/to/ssl"

* Wed Oct 10 2012 Bjorn Munch <bjorn.munch@oracle.com>

- Replace old my-*.cnf config file examples with template my-default.cnf

* Fri Oct 05 2012 Joerg Bruehe <joerg.bruehe@oracle.com>

- Let the installation use the new option "--random-passwords" of "mysql_install_db".
  (Bug# 12794345 Ensure root password)
- Fix an inconsistency: "new install" vs "upgrade" are told from the (non)existence
  of "$mysql_datadir/mysql" (holding table "mysql.user" and other system stuff).

* Tue Jul 24 2012 Joerg Bruehe <joerg.bruehe@oracle.com>

- Add a macro "runselftest":
  if set to 1 (default), the test suite will be run during the RPM build;
  this can be oveeridden via the command line by adding
      --define "runselftest 0"
  Failures of the test suite will NOT make the RPM build fail!

* Mon Jul 16 2012 Joerg Bruehe <joerg.bruehe@oracle.com>

- Add the man page for the "mysql_config_editor".

* Mon Jun 11 2012 Joerg Bruehe <joerg.bruehe@oracle.com>

- Make sure newly added "SPECIFIC-ULN/" directory does not disturb packaging.

* Wed Feb 29 2012 Brajmohan Saxena <brajmohan.saxena@oracle.com>

- Removal all traces of the readline library from mysql (BUG 13738013)

* Wed Sep 28 2011 Joerg Bruehe <joerg.bruehe@oracle.com>

- Fix duplicate mentioning of "mysql_plugin" and its manual page,
  it is better to keep alphabetic order in the files list (merging!).

* Wed Sep 14 2011 Joerg Bruehe <joerg.bruehe@oracle.com>

- Let the RPM capabilities ("obsoletes" etc) ensure that an upgrade may replace
  the RPMs of any configuration (of the current or the preceding release series)
  by the new ones. This is done by not using the implicitly generated capabilities
  (which include the configuration name) and relying on more generic ones which
  just list the function ("server", "client", ...).
  The implicit generation cannot be prevented, so all these capabilities must be
  explicitly listed in "Obsoletes:"

* Tue Sep 13 2011 Jonathan Perkin <jonathan.perkin@oracle.com>

- Add support for Oracle Linux 6 and Red Hat Enterprise Linux 6.  Due to
  changes in RPM behaviour ($RPM_BUILD_ROOT is removed prior to install)
  this necessitated a move of the libmygcc.a installation to the install
  phase, which is probably where it belonged in the first place.

* Tue Sep 13 2011 Joerg Bruehe <joerg.bruehe@oracle.com>

- "make_win_bin_dist" and its manual are dropped, cmake does it different.

* Thu Sep 08 2011 Daniel Fischer <daniel.fischer@oracle.com>

- Add mysql_plugin man page.

* Tue Aug 30 2011 Tor Didriksen <tor.didriksen@oracle.com>

- Set CXX=g++ by default to add a dependency on libgcc/libstdc++.
  Also, remove the use of the -fno-exceptions and -fno-rtti flags.
  TODO: update distro_buildreq/distro_requires

* Tue Aug 30 2011 Joerg Bruehe <joerg.bruehe@oracle.com>

- Add the manual page for "mysql_plugin" to the server package.

* Fri Aug 19 2011 Joerg Bruehe <joerg.bruehe@oracle.com>

- Null-upmerge the fix of bug#37165: This spec file is not affected.
- Replace "/var/lib/mysql" by the spec file variable "%%{mysqldatadir}".

* Fri Aug 12 2011 Daniel Fischer <daniel.fischer@oracle.com>

- Source plugin library files list from cmake-generated file.

* Mon Jul 25 2011 Chuck Bell <chuck.bell@oracle.com>

- Added the mysql_plugin client - enables or disables plugins.

* Thu Jul 21 2011 Sunanda Menon <sunanda.menon@oracle.com>

- Fix bug#12561297: Added the MySQL embedded binary

* Thu Jul 07 2011 Joerg Bruehe <joerg.bruehe@oracle.com>

- Fix bug#45415: "rpm upgrade recreates test database"
  Let the creation of the "test" database happen only during a new installation,
  not in an RPM upgrade.
  This affects both the "mkdir" and the call of "mysql_install_db".

* Wed Feb 09 2011 Joerg Bruehe <joerg.bruehe@oracle.com>

- Fix bug#56581: If an installation deviates from the default file locations
  ("datadir" and "pid-file"), the mechanism to detect a running server (on upgrade)
  should still work, and use these locations.
  The problem was that the fix for bug#27072 did not check for local settings.

* Mon Jan 31 2011 Joerg Bruehe <joerg.bruehe@oracle.com>

- Install the new "manifest" files: "INFO_SRC" and "INFO_BIN".

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

* Tue Oct 06 2009 Magnus Blaudd <mvensson@mysql.com>

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

- Call "make install" using "benchdir_root=%%{_datadir}",
  because that is affecting the regression test suite as well.

* Thu Nov 16 2006 Joerg Bruehe <joerg@mysql.com>

- Explicitly note that the "MySQL-shared" RPMs (as built by MySQL AB)
  replace "mysql-shared" (as distributed by SuSE) to allow easy upgrading
  (bug#22081).

* Mon Nov 13 2006 Joerg Bruehe <joerg@mysql.com>

- Add "--with-partition" t 2006 Joerg Bruehe <joerg@mysql.com>

- Use the Perl script to run the tests, because it will automatically check
  whether the server is configured with SSL.

* Tue Jun 27 2006 Joerg Bruehe <joerg@mysql.com>

- move "mysqldumpslow" from the client RPM to the server RPM (bug#20216)

- Revert all previous attempts to call "mysql_upgrade" during RPM upgrade,
  there are some more aspects which need to be solved before this is possible.
  For now, just ensure the binary "mysql_upgrade" is delivered and installysql.com>

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

* Tue Mar 07 2006 Kent Boortz <kent@mysql.com>

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
- Fixed %%packager name to "MySQL Production Engineering Team"

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
lenz@mysql.com>

- fixed the removing of the RPM_BUILD_ROOT in the %%clean section (the
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

- old mysqlmanager, mysq* Mon Feb 7 2005 Tomas Ulin <tomas@mysql.com>

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

* Fri Dec 31 2004 Lenz Grimmer <lenz@mysql.com>

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

* Sat Dec 13 2003 Lenz Grimmer <lenz@mysql.com>

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

- disabled MyISAM RAID (--with-raid)- it throws an assertion which
  needs to be investigated first.

* Mon Mar 10 2003 Lenz Grimmer <lenz@mysql.com>

- added missing file mysql_secure_installation to server subpackage
  (BUG 141)

* Tue Feb 11 2003 Lenz Grimmer <lenz@mysql.com>

- re-added missing pre- and post(un)install scripts to server subpackage
- added config file /etc/my.cnf to the file list (just for completeness)
- make sure to create the datadir with 755 permissions

* Mon Jan 27 2003 Lenz Grimmer <lenz@mysql.com>

- removed unusedql.com>

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

* Tue Sep 28 1999 David Axmark <davida@mysql.com>

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
