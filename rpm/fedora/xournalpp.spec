%global	build_repo https://github.com/xournalpp/xournalpp/
%global	build_branch master
%global	version_string 1.0.17
%define	build_commit %(git ls-remote %{build_repo} | grep "refs/heads/%{build_branch}" | cut -c1-41)
%define	build_shortcommit %(c=%{build_commit}; echo ${c:0:7})
%global	build_timestamp %(date +"%Y%m%d")
%global	rel_build %{build_timestamp}git%{build_shortcommit}%{?dist}
%global         _with_cppunit 1


Name:           xournalpp
Version:        %{version_string}
Release:        0.1.%{rel_build}
Summary:        Handwriting note-taking software with PDF annotation support

License:        GPLv2+
URL:            https://github.com/%{name}/%{name}
Source:         %{url}/archive/%{build_branch}.tar.gz

BuildRequires:  cmake3 >= 3.10
BuildRequires:  desktop-file-utils
BuildRequires:  gcc-c++
BuildRequires:  gettext
BuildRequires:  git
BuildRequires:  libappstream-glib
%{?_with_cppunit:
BuildRequires:  pkgconfig(cppunit) >= 1.12-0
}
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gtk+-3.0)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(libzip)
BuildRequires:  pkgconfig(lua)
BuildRequires:  pkgconfig(poppler-glib)
BuildRequires:  pkgconfig(portaudiocpp) >= 12
BuildRequires:  pkgconfig(sndfile)
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
%cmake3 \
        %{_with_cppunit: -DENABLE_CPPUNIT=ON} \
        .
%make_build translations

%install
%make_install

#Remove depreciated key from desktop file
desktop-file-install \
 --remove-key="Encoding" \
  %{buildroot}%{_datadir}/applications/com.github.%{name}.%{name}.desktop

%find_lang %{name}

#Remove scripts from icons interface
rm -r %{buildroot}%{_datadir}/%{name}/ui/icons/hicolor/update-icon-cache.sh
rm -r %{buildroot}%{_datadir}/%{name}/ui/iconsDark/hicolor/update-icon-cache.sh

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
%{_metainfodir}/com.github.%{name}.%{name}.appdata.xml

%files plugins
%{_datadir}/%{name}/plugins

%files ui
%{_datadir}/%{name}/ui

%changelog
* Mon Dec 16 2019 Luya Tshimbalanga <luya@fedoraproject.org>
- Implement some version autodetection to reduce maintenance work.

* Mon Dec 16 2019 Luya Tshimbalanga <luya@fedoraproject.org> - 1.0.16-7
- Remove architecture requirement for plugins and ui

* Mon Dec 16 2019 Luya Tshimbalanga <luya@fedoraproject.org> - 1.0.16-6
- Fix typos

* Mon Dec 16 2019 Luya Tshimbalanga <luya@fedoraproject.org> - 1.0.16-5
- Fix architecture requirement for ui

* Wed Dec 11 2019 Luya Tshimbalanga <luya@fedoraproject.org> - 1.0.16-4
- Review fixes

* Wed Dec 11 2019 Luya Tshimbalanga <luya@fedoraproject.org> - 1.0.16-3
- Add hicolor-icon-theme to requirement
- Use desktop file validation
- Split xournal data share into subpackages
- Review fixes

* Sun Nov 17 2019 Luya Tshimbalanga <luya@fedoraproject.org> 1.0.16-2
- Remove scripts from ui icons directory
- Relocate tlh locale directory

* Sun Nov 17 2019 Luya Tshimbalanga <luya@fedoraproject.org> 1.0.16-1
- Release 1.0.16
- Enable cppunit

* Sun Nov 10 2019 Luya Tshimbalanga <luya@fedoraproject.org> 1.0.15-2
- Update spec file based on review
- Include appstream data

* Sun Nov 10 2019 Luya Tshimbalanga <luya@fedoraproject.org> 1.0.15-1
- Release 1.0.15

* Tue Aug 13 2019 dfas <d.dfas@moens.cc> - 1.0.13-2.git7349762
- Release 1.0.13-current

* Tue Jun 25 2019 dfas <d.dfas@moens.cc> - 1.0.13-1.gita7f0275
- Release 1.0.13-current

* Fri May 3 2019 Francisco Gonzalez <gzmorell@gmail.com> - 1.0.10-1
- First attempt at packaging xournalpp.
