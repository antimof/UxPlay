Name:           uxplay
Version:        1.65.2
Release:        1%{?dist}

%define gittag  v%{version}

Summary:        AirPlay-Mirror and AirPlay-Audio server
License:        GPLv3+
URL:            https://github.com/FDH2/UxPlay
Source0:        https://github.com/FDH2/UxPlay/archive/%{gittag}/%{name}-%{version}.tar.gz

Packager:       UxPlay maintainer
 
BuildRequires:  cmake >= 3.4.1
BuildRequires:  make
BuildRequires:  gcc
BuildRequires:  gcc-c++
Requires:       avahi

#RedHat and clones 
%if %{defined fedora} || %{defined rhel}
BuildRequires:  pkgconf
BuildRequires:  openssl-devel >= 3.0
BuildRequires:  libplist-devel >= 2.0
BuildRequires:  avahi-compat-libdns_sd-devel
BuildRequires:  gstreamer1-devel
BuildRequires:  gstreamer1-plugins-base-devel
Requires:       gstreamer1-plugins-base
Requires:       gstreamer1-plugins-good
Requires:       gstreamer1-plugins-bad-free
Requires:       gstreamer1-libav
%endif

#Mageia
%if %{defined mkrel}
BuildRequires:  pkgconf
BuildRequires:  openssl-devel >= 3.0
BuildRequires:  libplist-devel >= 2.0
BuildRequires:  avahi-compat-libdns_sd-devel
BuildRequires:  gstreamer1.0-devel
BuildRequires:  gstreamer1.0-plugins-base-devel
Requires:       gstreamer1.0-plugins-base
Requires:       gstreamer1.0-plugins-good
Requires:       gstreamer1.0-plugins-bad
Requires:       gstreamer1.0-libav
%endif

#SUSE
%if %{defined suse_version}
BuildRequires:  pkg-config
BuildRequires:  libopenssl-3-devel
BuildRequires:  libplist-2_0-devel
BuildRequires:  avahi-compat-mDNSResponder-devel
BuildRequires:  gstreamer-devel
BuildRequires:  gstreamer-plugins-base-devel
Requires:       gstreamer-plugins-base
Requires:       gstreamer-plugins-good
Requires:       gstreamer-plugins-bad
Requires:       gstreamer-plugins-libav
%endif

%description
An AirPlay2 Mirror and AirPlay2 Audio (but not Video) server that provides
screen-mirroring (with audio) of iOS/MacOS clients in a display window on
the server host (which can be shared using a screen-sharing application);
Apple Lossless Audio (ALAC) (e.g.,iTunes) can be streamed from client to
server in non-mirror mode

%prep

%autosetup -n UxPlay-%{version}

%cmake -DCMAKE_INSTALL_DOCDIR=%{_docdir}/%{name}
%cmake_build

%if %{defined suse_version}
#suse macro cmake_install installs from _topdir/build (misses docs in _topdir)
cd ..   
%endif

%cmake_install 

%files
%{_bindir}/uxplay
%{_mandir}/man1/uxplay.1*

%doc
%{_docdir}/%{name}/README.txt
%{_docdir}/%{name}/README.html
%{_docdir}/%{name}/README.md

%license
%{_docdir}/%{name}/LICENSE
%{_docdir}/%{name}/llhttp/LICENSE-MIT


%changelog
* Thu Jul 20 2023 UxPlay maintainer  <https://github.com/FDH2/UxPlay>
  Initial uxplay.spec: tested on Fedora 38, Rocky Linux 9.2, OpenSUSE
  Leap 15.5, Mageia 9.
- 
