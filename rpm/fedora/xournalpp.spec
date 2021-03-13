# This spec file is intended for a daily git snapshot

# Force out of source build
%undefine __cmake_in_source_build

#This spec file is intended for daily development snapshot release
%global	build_repo https://github.com/xournalpp/xournalpp/
%global	build_branch master
%global	version_string 1.1.0
%define	build_commit %(git ls-remote %{build_repo} | grep "refs/heads/%{build_branch}" | cut -c1-41)
%define	build_shortcommit %(c=%{build_commit}; echo ${c:0:7})
%global	build_timestamp %(date +"%Y%m%d")
%global	rel_build %{build_timestamp}git%{build_shortcommit}%{?dist}
%bcond_without  cppunit


Name:           xournalpp
Version:        %{version_string}
Release:        0.1.%{rel_build}
Summary:        Handwriting note-taking software with PDF annotation support

License:        GPLv2+
URL:            https://github.com/%{name}/%{name}
Source:         %{url}/archive/%{build_branch}.tar.gz

BuildRequires:  cmake >= 3.10
BuildRequires:  desktop-file-utils
BuildRequires:  gcc-c++
BuildRequires:  gettext
BuildRequires:  git
BuildRequires:  libappstream-glib
%if %{with cppunit}
BuildRequires:  pkgconfig(cppunit) >= 1.12-0
%endif
BuildRequires:  pkgconfig(glib-2.0) >= 2.32.0
BuildRequires:  pkgconfig(gtk+-3.0) >= 3.18.9
BuildRequires:  pkgconfig(librsvg-2.0)
BuildRequires:  pkgconfig(libxml-2.0) >= 2.0.0
BuildRequires:  pkgconfig(libzip) >= 1.0.1
BuildRequires:  pkgconfig(lua) >= 5.3
BuildRequires:  pkgconfig(poppler-glib) >= 0.41.0
BuildRequires:  pkgconfig(portaudiocpp) >= 12
BuildRequires:  pkgconfig(sndfile) >= 1.0.25
BuildRequires:	pkgconfig(zlib)
Recommends:     texlive-scheme-basic
Recommends:     texlive-dvipng
Recommends:     texlive-standalone
Requires:       hicolor-icon-theme
Requires:       %{name}-plugins = %{version}-%{release}
Requires:       %{name}-ui = %{version}-%{release}

%description
Xournal++ is a handwriting note-taking software with PDF annotation support.
Supports Pen input like Wacom Tablets

%package	plugins
Summary:        Default plugin for %{name}
BuildArch:      noarch

%description	plugins
The %{name}-plugins package contains sample plugins for  %{name}.

%package	ui
Summary:        User interface for %{name}
BuildArch:      noarch

%description	ui
The %{name}-ui package contains a graphical user interface for  %{name}.


%prep
%autosetup -n %{name}-%{build_branch}

#Fix tlh aka klingon language
mv po/tlh_AA.po po/tlh.po
sed -i 's|tlh-AA|tlh|g' po/tlh.po

%build
%cmake \
        %if %{with cppunit}
         -DENABLE_CPPUNIT=ON
        %endif
%cmake_build

%install
%cmake_install

#Remove depreciated key from desktop file
desktop-file-install \
 --remove-key="Encoding" \
  %{buildroot}%{_datadir}/applications/com.github.%{name}.%{name}.desktop

%find_lang %{name}

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/com.github.%{name}.%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/com.github.%{name}.%{name}.appdata.xml

%files -f %{name}.lang
%license LICENSE
%doc README.md AUTHORS
%{_bindir}/xournalpp-thumbnailer
%{_bindir}/%{name}
%{_datadir}/applications/com.github.%{name}.%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/com.github.%{name}.%{name}.svg
%{_datadir}/icons/hicolor/scalable/mimetypes/*
%{_datadir}/mime/packages/com.github.%{name}.%{name}.xml
%exclude %{_datadir}/mimelnk/application/*
%{_datadir}/thumbnailers/com.github.%{name}.%{name}.thumbnailer
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/resources/default_template.tex
%{_metainfodir}/com.github.%{name}.%{name}.appdata.xml

%files plugins
%{_datadir}/%{name}/plugins

%files ui
%{_datadir}/%{name}/ui

%changelog
* Sat Feb 20 2021 Luya Tshimbalanga <luya@fedoraproject.org>
- Add librsvg2 dependencies
- Add notice about daily git snapshot

* Mon Dec 16 2019 Luya Tshimbalanga <luya@fedoraproject.org>
- Implement some version autodetection to reduce maintenance work.
