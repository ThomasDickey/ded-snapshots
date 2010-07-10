Summary: DED directory editor
%define AppProgram ded
%define AppLibrary td_lib
%define AppVersion 12.x
%define AppRelease 20100711
%define LibRelease 20100711
# $Id: ded-12.0.spec,v 1.9 2010/07/09 23:05:25 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: %{AppRelease}
License: MIT-X11
Group: System Environment/Shells
URL: ftp://invisible-island.net/ded
Source0: %{AppLibrary}-%{LibRelease}.tgz
Source1: %{AppProgram}-%{AppRelease}.tgz
Vendor: Thomas Dickey <dickey@invisible-island.net>

%description
A directory editor for UNIX systems, DED has special functions to manage RCS
and SCCS files.

%prep

# -a N (unpack Nth source after cd'ing into build-root)
# -b N (unpack Nth source before cd'ing into build-root)
# -D (do not delete directory before unpacking)
# -q (quiet)
# -T (do not do default unpacking, is used with -a or -b)
rm -rf %{AppProgram}-%{AppVersion}
mkdir %{AppProgram}-%{AppVersion}
%setup -q -D -T -a 1
mv %{AppProgram}-%{AppRelease}/* .
%setup -q -D -T -a 0

%build

cd %{AppLibrary}-%{LibRelease}

./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--libdir=%{_libdir} \
		--mandir=%{_mandir} \
		--datadir=%{_datadir} \
		--disable-echo \
		--with-ncursesw
make

cd ..
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

make install                    DESTDIR=$RPM_BUILD_ROOT

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/ded
%{_mandir}/man1/ded.*
%{_datadir}/ded/*.hlp
%{_datadir}/ded/*.rc

%changelog
# each patch should add its ChangeLog entries here

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

