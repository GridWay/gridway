%define _name gridway
%define _release 0

Name:		gridway-EMI
Version:	5.14
Release:	0%{dist}
Summary:	EMI MADs for GridWay

Group:		System Environment/Libraries
License:	Apache License
URL:		http://www.gridway.org/
Vendor:         Initiative for Globus in Europe (IGE)
Source:		%{_name}-%{version}.%{_release}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)

BuildRequires:	nordugrid-arc-debuginfo >= 3.0
BuildRequires:	nordugrid-arc-devel >= 3.0
BuildRequires:  nordugrid-arc-plugins-needed >= 3.0
Requires:	gridway-core
Requires:	nordugrid-arc-plugins-needed >= 3.0
# svn co http://svn.gridway.org/gridway/tags/GW_%{version}_%{_release} gridway-%{version}.%{_release}
# cp -p gridway-%{version}.%{_release}/packaging/emi/RPM/README gridway-%{version}.%{_release}/README.emi
# tar -czf gridway-%{version}.%{_release}.tar.gz gridway-%{version}.%{_release}/

%description
GridWay uses several Middleware Access Drivers (MAD) to interface with 
different Grid services. The following MADs are part of the GridWay 
distribution: Information MADS to interface with different information 
services, Execution MADs to interface with different execution services, and 
Transfer MADs to interface with different transfer services.

The %{_name}-EMI package contains:
GridWay MADs to interface with the EMI-ES. These are an Execution MAD to 
interface with EMI Execution Service and a dummy Transfer MAD.

%prep
%setup -q -n %{_name}-%{version}.%{_release}

%post 
echo "# MADs for EMI
IM_MAD = static:gw_im_mad_static:-l etc/emi_hosts.list:dummy:emi-es
EM_MAD = emi-es:gw_em_mad_emies::adl
TM_MAD = dummy:gw_tm_mad_dummy:-u gsiftp\://<hostname>" >> /usr/etc/gwd.conf

%postun 
sed '/# MADs for EMI/d' -i /usr/etc/gwd.conf
sed '/emi-es/d' -i /usr/etc/gwd.conf
sed '/dummy/d' -i /usr/etc/gwd.conf

%build
cd src/em_mad/emi-es/
make
cd ../../tm_mad/dummy/
make 

%install
rm -rf $RPM_BUILD_ROOT
export GW_LOCATION=$RPM_BUILD_ROOT/usr/
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/gridway-emi-5.14.0/
cp README.emi $RPM_BUILD_ROOT/usr/share/doc/gridway-emi-5.14.0/README
cd src/em_mad/emi-es/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../../tm_mad/dummy/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../..

%clean
rm -rf $RPM_BUILD_ROOT

%check

%files
%defattr(-,root,root,-)
/usr/bin/gw_em_mad_emies
/usr/bin/gw_em_mad_emies.bin
/usr/bin/gw_tm_mad_dummy
/usr/bin/gw_tm_mad_dummy.bin
/usr/share/doc/gridway-emi-5.14.0/README

%changelog
* Mon Apr 01 2013 GridWay Project Leads <contact@gridway.org> - 5.14-0
- Update to 5.14-0

* Sun Sep 30 2012 GridWay Project Leads <contact@gridway.org> - 5.12-0
- Update to 5.12-0

* Fri Sep 07 2012 GridWay Project Leads <contact@gridway.org> - 5.12-RC2
- Update to UMD-2

* Fri Aug 31 2012 GridWay Project Leads <contact@gridway.org> - 5.12-RC1
- Update to 5.12-RC1

* Thu May 17 2012 GridWay Project Leads <contact@gridway.org> - 5.10-2
- Bug fixing

* Mon Apr 09 2012 GridWay Project Leads <contact@gridway.org> - 5.10-1
- Update to 5.10-1

* Wed Mar 28 2012 GridWay Project Leads <contact@gridway.org> - 5.10-0
- Update to 5.10-0

* Fri Feb 10 2012 GridWay Project Leads <contact@gridway.org> - 5.10-RC1
- Initial version
