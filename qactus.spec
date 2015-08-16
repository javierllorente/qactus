#
# spec file for package qactus
#
# Copyright (c) 2015 SUSE LINUX Products GmbH, Nuernberg, Germany.
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


Name:           qactus
Version:        0.7.0
Release:        0
Summary:        An OBS notifier application
License:        GPL-2.0 or GPL-3.0
Group:          Utility/Development/Other
Url:            http://www.javierllorente.com/
Source:         %{name}-%{version}.tar.bz2
BuildRequires:  hicolor-icon-theme
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  qtkeychain-qt5-devel
BuildRequires:  update-desktop-files
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
A Qt-based OBS notifier application

%prep
%setup -q -n %{name}

%build
cat > .qmake.cache <<EOF
PREFIX=%{_prefix}
QMAKE_CXXFLAGS_RELEASE += "%{optflags}"
EOF
qmake-qt5
make %{?_smp_mflags}

%install
make INSTALL_ROOT=%{buildroot} install
%suse_update_desktop_file %{name}

%post
%desktop_database_post

%postun
%desktop_database_postun

%files
%defattr(-,root,root)
%doc COPYING README.md
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/128x128/apps/%{name}.png

%changelog
