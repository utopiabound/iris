Name:           iris
Version:        0.0.1
Release:        1
Summary:        IRIS messanging daemon
URL:            https://github.org/utopiabound/iris
Source:         https://www.fvwm.org/generated/icon_download/fvwm_icons.tar.bz2
License:        GPL?
BuildRequires:	glib2-devel
BuildRequires:	libpurple-devel

%if 0%{?fedora} >= 20 || 0%{?rhel} >= 7 || 0%{?suse_version} >= 1210
BuildRequires:          systemd
Requires(post):         systemd
Requires(preun):        systemd
Requires(postun):       systemd
%else
Requires(post):         /sbin/chkconfig
Requires(preun):        /sbin/chkconfig
Requires(preun):        /sbin/service
Requires(postun):       /sbin/service
%endif


%description
IRIS Messanging deamon listens on system dbus, and will send
notifications to all Bonjour iChat clients.

%prep
%{setup}

%build
%{make_build}

%install
%{make_install}

%clean
rm -rf %{buildroot}

%pre
useradd -M -r -d %{_sharedstatedir}/iris iris

%if 0%{?fedora} >= 20 || 0%{?rhel} >= 7 || 0%{?suse_version} >= 1210

%post
%systemd_post %{name}.service

%preun
if [ $1 -eq 0 ]; then
    # Package removal, not upgrade
    /bin/systemctl --no-reload disable %{name}.service >/dev/null 2>&1 || :
fi

%systemd_preun %{name}.service

%postun
%systemd_postun %{name}.service

%else

%post
useradd -M -r -d %{_sharedstatedir}/iris iris
# enable on initial install
[ $1 -lt 2 ] && /sbin/chkconfig --add dkms_autoinstaller >/dev/null 2>&1 ||:
[ $1 -lt 2 ] && /sbin/chkconfig dkms_autoinstaller on >/dev/null 2>&1 ||:

%preun
# remove on uninstall
[ $1 -lt 1 ] && /sbin/chkconfig dkms_autoinstaller off >/dev/null 2>&1 ||:
[ $1 -lt 1 ] && /sbin/chkconfig --del dkms_autoinstaller >/dev/null 2>&1 ||:

%endif

%files
%defattr(-,root,root)
%{_bindir}/*
%{_sbindir}/*
%{_unitdir}/*
%if 0%{?fedora} >= 20 || 0%{?rhel} >= 7 || 0%{?suse_version} >= 1210
%{_unitdir}/*
%else
%{_initrddir}/*
%endif
%attr(755,iris,iris) %{_sharedstatedir}/iris

%changelog
