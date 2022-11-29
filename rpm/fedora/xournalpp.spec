# Force out of source build
%global         __cmake_in_source_build 0

#This spec file is intended for daily development snapshot release
%global	build_repo https://github.com/xournalpp/xournalpp/
%global	build_branch master
%global	version_string 1.1.3+dev
%define	build_commit %(git ls-remote %{build_repo} | grep "refs/heads/%{build_branch}" | cut -c1-41)
%define	build_shortcommit %(c=%{build_commit}; echo ${c:0:7})
%global	build_timestamp %(date +"%Y%m%d")
%global	rel_build %{build_timestamp}git%{build_shortcommit}
%global _gtest 1

Name:           xournalpp
# See https://docs.fedoraproject.org/en-US/packaging-guidelines/Versioning/#_examples
Version:        %{version_string}^%{rel_build}
Release:        1%{dist}
Summary:        Handwriting note-taking software with PDF annotation support

License:        GPLv2+
URL:            %{build_repo}
Source:         %{url}/archive/%{build_branch}.tar.gz

BuildRequires:  cmake >= 3.10
BuildRequires:  desktop-file-utils
BuildRequires:  fdupes
BuildRequires:  gcc-c++
BuildRequires:  gettext
BuildRequires:  git
BuildRequires:  help2man
BuildRequires:  libappstream-glib
#This would be the right way to do it xpp downloads from Google nonetheless.
%{?_gtest:
BuildRequires:  pkgconfig(gtest)
}
BuildRequires:  pkgconfig(glib-2.0) >= 2.32.0
BuildRequires:  pkgconfig(gtk+-3.0) >= 3.18.9
BuildRequires:  pkgconfig(librsvg-2.0)
BuildRequires:  pkgconfig(libxml-2.0) >= 2.0.0
BuildRequires:  pkgconfig(libzip) >= 1.0.1
BuildRequires:  pkgconfig(lua) >= 5.3
BuildRequires:  pkgconfig(poppler-glib) >= 0.41.0
BuildRequires:  pkgconfig(portaudiocpp) >= 12
BuildRequires:  pkgconfig(sndfile) >= 1.0.25
BuildRequires:  pkgconfig(zlib)
BuildRequires:  pkgconfig(gtksourceview-4) >= 4.0
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

%build
%cmake \
        -DDISTRO_CODENAME="Fedora Linux" \
        %{?_gtest: -DENABLE_GTEST=ON} \
        -DENABLE_MATHTEX=ON \
        -DGIT_VERSION=%{build_shortcommit} \
        -DMAC_INTEGRATION=OFF

%cmake_build

%install
%cmake_install

#Remove depreciated key from desktop file
desktop-file-install \
 --remove-key="Encoding" \
 --set-key="StartupWMClass" \
 --set-value="xournalpp" \
  %{buildroot}%{_datadir}/applications/com.github.%{name}.%{name}.desktop
%find_lang %{name}

# Remove unnecssary scripts
find %{buildroot}%{_datadir}/%{name} -name update-icon-cache.sh -delete -print
%fdupes %{buildroot}%{_datadir}

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
%{_datadir}/thumbnailers/com.github.%{name}.%{name}.thumbnailer
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/resources/{default,legacy}_template.tex
%{_mandir}/man1/%{name}*.gz
%{_metainfodir}/com.github.%{name}.%{name}.appdata.xml

%files plugins
%{_datadir}/%{name}/plugins

%files ui
%{_datadir}/%{name}/ui

%changelog
* Sun Mar 6 2022 Luya Tshimbalanga <luya@fedoraproject.org>
- Port enhanced spec file from Michael J Gruber version

* Thu Oct 20 2021 Ulrich Huber <ulrich@huberulrich.de>
- See https://github.com/%{name}/%{name}/CHANGELOG.md

* Sat Feb 20 2021 Luya Tshimbalanga <luya@fedoraproject.org>
- Add librsvg2 dependencies
- Add notice about daily git snapshot

* Mon Dec 16 2019 Luya Tshimbalanga <luya@fedoraproject.org>
- Implement some version autodetection to reduce maintenance work.
