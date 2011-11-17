#
# percona-shared-compat.spec
#
# Based on MySQL-shared-compat.spec, which is Copyright (C) 2003 MySQL AB
#
# RPM build instructions to create a "meta" package that includes different
# versions of the MySQL shared libraries (for compatibility with
# distributions that ship older versions of MySQL).
#
# In this version we simply repackage mysql-shared-compat from upstream.
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc., 59
# Temple Place, Suite 330, Boston, MA  02111-1307  USA

# For 5.0 and up, this is needed because of "libndbclient".
%define _unpackaged_files_terminate_build 0

#
# Change this to match the version of the shared libs you want to include
#
%define version55 5.5.16
%define version51 5.1.48
%define version50 5.0.91
%define version41 4.1.22
%define version40 4.0.27
#%define version3 3.23.58

%define redhatversion %(lsb_release -rs | awk -F. '{ print $1}')

Name:         Percona-Server-shared-compat
Packager:     Percona MySQL Development Team <mysql-dev@percona.com>
Vendor:       Percona Inc
License:      GPL v2
Group:        Applications/Databases
URL:          http://www.percona.com/percona-lab.html
Autoreqprov:  on
Version:      %{version55}
Release:      %{release}.%{gotrevision}.rhel%{redhatversion}
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Obsoletes:    MySQL-shared mysql-libs
Provides:     MySQL-shared mysql-libs
Summary:      MySQL shared client libraries for MySQL %{version}, %{version50}, %{version41} and %{version40}

# We extract the older libraries from mysql-shared-compat and the newer from our sources
Source0:      MySQL-shared-compat-%{version55}-1.linux2.6.%{_arch}.rpm
#Source1:      MySQL-shared-%{version50}-1.%{_arch}.rpm
#Source2:      MySQL-shared-%{version41}-0.%{_arch}.rpm
#Source3:      MySQL-shared-%{version40}-0.%{_arch}.rpm
#Source3:      MySQL-shared-%{version3}-1.%{_arch}.rpm
# No need to include the RPMs once more - they can be downloaded seperately
# if you want to rebuild this package
NoSource:     0
#NoSource:     1
#NoSource:     2
#NoSource:     3
BuildRoot:    %{_tmppath}/%{name}-%{version}-build

%description
This package includes the shared libraries for MySQL 4.0, 4.1, 5.0 and 5.1.
Install this package instead of "MySQL-shared", if you have applications
installed that are dynamically linked against older versions of the MySQL
client library but you want to upgrade to MySQL %{version} without breaking the
library dependencies.

%install
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT;
mkdir -p $RPM_BUILD_ROOT
cd $RPM_BUILD_ROOT
rpm2cpio %{SOURCE0} | cpio -iv --make-directories
#rpm2cpio %{SOURCE1} | cpio -iv --make-directories
#rpm2cpio %{SOURCE2} | cpio -iv --make-directories
#rpm2cpio %{SOURCE3} | cpio -iv --make-directories
/sbin/ldconfig -n $RPM_BUILD_ROOT%{_libdir}

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT;

%files
%defattr(-, root, root)
%{_libdir}/libmysqlclient*

