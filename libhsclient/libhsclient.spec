Summary: handlersocket client library
Name: libhsclient
Version: 1.0.4
Release: 1%{?dist}
Group: System Environment/Libraries
License: BSD
Source: libhsclient.tar.gz
Packager: Akira Higuchi <higuchi dot akira at dena dot jp>
BuildRoot: /var/tmp/%{name}-%{version}-root

%description

%prep
%setup -n %{name}

%define _use_internal_dependency_generator 0

%build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/include/handlersocket
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
mkdir -p $RPM_BUILD_ROOT/%{_libdir}
install -m 755 libhsclient.a $RPM_BUILD_ROOT/%{_libdir}
install -m 644 *.hpp $RPM_BUILD_ROOT/usr/include/handlersocket/

%files
%defattr(-, root, root)
/usr/include/*
%{_libdir}/*.a

