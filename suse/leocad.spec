# spec file for package lpub4
#
# Copyright (c) 2014 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

Name:           leocad
Version:	__VERSION__
Release:	0
License:	GPL-2.0
Summary:	a CAD program for creating virtual LEGO models usingthe LDraw library
Url:		http://www.leocad.org/
Group:		Productivity/Graphics/CAD
Source:		leocad.tar.bz2
BuildRequires:	libqt4-devel 
Requires:	ldraw-library 
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
LeoCAD is a CAD program for creating virtual LEGO models. It has an easy to use
interface and currently includes over 6000 different pieces created by LDraw
community. 

LDraw is an open standard for LEGO CAD programs that allow the user to create
virtual LEGO models and scenes.

%prep
%setup -q -n leocad

%build
qmake QMAKE_CXXFLAGS='-D LIBPATH_DEFAULT=\"/usr/share/ldraw/\"'
make

%install
install -d %{buildroot}/%{_bindir}
install -m 755 build/release/leocad  %{buildroot}/%{_bindir}/leocad

%files
%defattr(-,root,root)
%{_bindir}/leocad

%changelog
