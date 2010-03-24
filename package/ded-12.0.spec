Summary: DED directory editor
%define AppVersion 20100323
%define LibVersion 20100323
# $Header: /users/source/archives/ded.vcs/package/RCS/ded-12.0.spec,v 1.3 2010/03/24 00:48:04 tom Exp $
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
		--with-screen=ncursesw
make

cd ..
./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--libdir=%{_libdir} \
		--mandir=%{_mandir}
make

%install

[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install                    DESTDIR=$RPM_BUILD_ROOT

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/ded
%{_bindir}/ded.hlp
%{_mandir}/man1/ded.*

%changelog
# each patch should add its ChangeLog entries here

* Tue Mar 23 2010 Thomas Dickey
- initial version

