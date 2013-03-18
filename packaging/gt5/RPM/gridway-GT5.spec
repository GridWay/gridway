%define _name gridway
%define _release 0

%ifarch alpha ia64 ppc64 s390x sparc64 x86_64
%global flavor gcc64
%else
%global flavor gcc32
%endif

Name:		gridway-GT5
Version:	5.14
Release:	0%{dist}
Summary:	GT5 MADs for GridWay

Group:		System Environment/Libraries
License:	Apache License
URL:		http://www.gridway.org/
Vendor:         Initiative for Globus in Europe (IGE)
Source:		%{_name}-%{version}.%{_release}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)

BuildRequires:	globus-common-devel
BuildRequires:	globus-gram-client
BuildRequires:	globus-gram-client-devel
BuildRequires:	globus-gass-copy
BuildRequires:	globus-ftp-client-devel
BuildRequires:	globus-gass-copy-devel
BuildRequires:	grid-packaging-tools
BuildRequires:  globus-common-progs
Requires:	gridway-core
Requires:	globus-gram-client
Requires:	globus-gass-copy
Requires:	globus-gass-copy-progs
Requires:	globus-proxy-utils
# svn co http://svn.gridway.org/gridway/tags/GW_%{version}_%{_release} gridway-%{version}.%{_release}
# cp -p gridway-%{version}.%{_release}/packaging/gt5/RPM/README gridway-%{version}.%{_release}/README.gt5
# tar -czf gridway-%{version}.%{_release}.tar.gz gridway-%{version}.%{_release}/

%description
GridWay uses several Middleware Access Drivers (MAD) to interface with 
different Grid services. The following MADs are part of the GridWay 
distribution: Information MADS to interface with different information 
services, Execution MADs to interface with different execution services, and 
Transfer MADs to interface with different transfer services.

The %{_name}-GT5 package contains:
GridWay MADs to interface with Globus Toolkit 5 services. These are an 
Execution MAD to interface with GRAM5 services and a Transfer MAD to interface 
with GridFTP services.

%prep
%setup -q -n %{_name}-%{version}.%{_release}

%post 
echo "# MADs for GT5
IM_MAD = static:gw_im_mad_static:-l etc/gram5_hosts.list:gridFTP:gram5
EM_MAD = gram5:gw_em_mad_gram5::rsl
TM_MAD = gridFTP:gw_tm_mad_ftp:" >> /usr/etc/gwd.conf
touch /usr/etc/gram5_hosts.list

%postun 
sed '/# MADs for GT5/d' -i /usr/etc/gwd.conf
sed '/gram5/d' -i /usr/etc/gwd.conf
sed '/gridFTP/d' -i /usr/etc/gwd.conf
if [ ! -s "/usr/etc/gram5_hosts.list" ]; then
 rm -f /usr/etc/gram5_hosts.list;
fi

%build
cd src/em_mad/gram5/
globus-makefile-header --flavor=%{flavor} globus_gram_client > makefile-header
make
cd ../../tm_mad/gridftp/
globus-makefile-header --flavor=%{flavor} globus_gass_copy > makefile-header
make 

%install
rm -rf $RPM_BUILD_ROOT
export GW_LOCATION=$RPM_BUILD_ROOT/usr/
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/gridway-gt5-5.14.0/
cp README.gt5 $RPM_BUILD_ROOT/usr/share/doc/gridway-gt5-5.14.0/README
cd src/em_mad/gram5/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../../tm_mad/gridftp/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../..

%clean
rm -rf $RPM_BUILD_ROOT

%check

%files
%defattr(-,root,root,-)
/usr/bin/gw_em_mad_gram5
/usr/bin/gw_em_mad_gram5.bin
/usr/bin/gw_tm_mad_ftp
/usr/bin/gw_tm_mad_ftp.bin
/usr/share/doc/gridway-gt5-5.14.0/README

%changelog
* Mon Apr 01 2013 GridWay Project Leads <contact@gridway.org> - 5.14-0
- Update to 5.14-0

* Sun Sep 30 2012 GridWay Project Leads <contact@gridway.org> - 5.12-0
- Update to 5.12-0

* Fri Aug 31 2012 GridWay Project Leads <contact@gridway.org> - 5.12-RC1
- Update to 5.12-RC1

* Thu May 17 2012 GridWay Project Leads <contact@gridway.org> - 5.10-2
- Bug fixing

* Mon Apr 09 2012 GridWay Project Leads <contact@gridway.org> - 5.10-1
- Update to 5.10-1

* Wed Mar 28 2012 GridWay Project Leads <contact@gridway.org> - 5.10-0
- Update to 5.10-0

* Thu Feb 09 2012 GridWay Project Leads <contact@gridway.org> - 5.10-RC1
- Update to 5.10-RC1

* Fri Jan 27 2012 Mattias Ellert <mattias.ellert@fysat.uu.se> - 5.8-2
- Recompiled for Globus Toolkit 5.2

* Thu Sep 30 2011 GridWay Project Leads <contact@gridway.org> - 5.8-0
- Update to 5.8-0

* Wed Aug 24 2011 GridWay Project Leads <contact@gridway.org> - 5.8-RC1
- Update to 5.8-RC1

* Wed Jun 29 2011 GridWay Project Leads <contact@gridway.org> - 5.7-1
- Fixed some recommendations for FHS compilant

* Thu Jun 09 2011 GridWay Project Leads <contact@gridway.org> - 5.7-1
- Update to 5.7-1 

* Mon Jun 06 2011 GridWay Project Leads <contact@gridway.org> - 5.7-0
- Initial version
