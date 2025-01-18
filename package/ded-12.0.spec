Summary: directory editor
%define AppProgram ded
%define AppLibrary td_lib
%define AppVersion 12.x
%define AppRelease 20250117
%define LibRelease 20250117
# $Id: ded-12.0.spec,v 1.40 2025/01/18 00:54:29 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: %{AppRelease}
License: MIT-X11
Group: System Environment/Shells
URL: https://invisible-island.net/ded
Source0: https://invisible-island.net/archives/ded/%{AppProgram}-%{AppRelease}.tgz
BuildRequires: td_lib <= %{AppRelease}
Vendor: Thomas Dickey <dickey@invisible-island.net>

%description
A directory editor for UNIX systems, DED has special functions to manage RCS
and SCCS files.

%prep

# no need for debugging symbols...
%define debug_package %{nil}

%setup -q -n %{AppProgram}-%{AppRelease}

%build

./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--libdir=%{_libdir} \
		--mandir=%{_mandir} \
		--datadir=%{_datadir}
make

%install

[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install DESTDIR=$RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/ded
%{_mandir}/man1/ded.*
%{_datadir}/ded/*.hlp
%{_datadir}/ded/*.rc

%changelog
# each patch should add its ChangeLog entries here

* Thu Jan 19 2023 Thomas Dickey
- build against td_lib package rather than side-by-side configuration

* Sat Mar 24 2018 Thomas Dickey
- disable debug-package

* Fri Jul 09 2010 Thomas Dickey
- add "-m" option

* Sat Jul 03 2010 Thomas Dickey
- code cleanup

* Thu Jun 24 2010 Thomas Dickey
- move data files to data directory

* Tue May 26 2010 Thomas Dickey
- code cleanup with clang --analyze

* Sun May 02 2010 Thomas Dickey
- correct option for specifying ncursesw library

* Tue Mar 23 2010 Thomas Dickey
- initial version

