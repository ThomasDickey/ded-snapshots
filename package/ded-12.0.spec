Summary: DED directory editor
%define AppVersion 20100624
%define LibVersion 20100624
# $Header: /users/source/archives/ded.vcs/package/RCS/ded-12.0.spec,v 1.6 2010/06/24 09:53:45 tom Exp $
Name: ded
Version: 12.x
# Base version is 12.x; rpm version corresponds to "Source1" directory name.
Release: %{AppVersion}
License: MIT-X11
Group: Applications/Editors
URL: ftp://invisible-island.net/ded
Source0: td_lib-%{LibVersion}.tgz
Source1: ded-%{AppVersion}.tgz
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
rm -rf ded-12.x
mkdir ded-12.x
%setup -q -D -T -a 1
mv ded-%{AppVersion}/* .
%setup -q -D -T -a 0

%build

cd td_lib-%{LibVersion}

./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--libdir=%{_libdir} \
		--mandir=%{_mandir} \
		--datadir=%{_datadir} \
		--with-ncursesw \
		--with-screen=ncursesw
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

* Thu Jun 24 2010 Thomas Dickey
- move data files to data directory

* Tue May 26 2010 Thomas Dickey
- code cleanup with clang --analyze

* Sun May 02 2010 Thomas Dickey
- correct option for specifying ncursesw library

* Tue Mar 23 2010 Thomas Dickey
- initial version

