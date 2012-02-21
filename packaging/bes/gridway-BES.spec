%define _name gridway
%define _release 0

Name:		gridway-BES
Version:	5.10
Release:	0%{dist}
Summary:	OGSA-BES MAD for GridWay

Group:		System Environment/Libraries
License:	Apache License
URL:		http://www.gridway.org/
Vendor:         Initiative for Globus in Europe (IGE)
Source:		%{_name}-%{version}.%{_release}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)

BuildRequires:	java-devel
BuildRequires:	dom4j
BuildRequires:	axis
BuildRequires:	xmlbeans
#BuildRequires:	gridsam-client
Requires:	gridway-core
Requires:	xmlbeans
Requires:	axis
#Requires:	gridsam-client
Requires:	apache-commons-logging
Requires:	apache-commons-discovery
Requires:	wsdl4j
Requires:	xml-security
Requires:	dom4j
Requires:	log4j

%description
GridWay uses several Middleware Access Drivers (MAD) to interface with 
different Grid services. The following MADs are part of the GridWay 
distribution: Information MADS to interface with different information 
services, Execution MADs to interface with different execution services, and 
Transfer MADs to interface with different transfer services.

The %{_name}-BES package contains:
An Execution MAD to interface with the OGSA Basic Execution Service (BES). 
This also includes an static Information MAD and a dummy Transfer MAD.

%prep
%setup -q -n %{_name}-%{version}.%{_release}

%post 
echo "# MADs for OGSA-BES
IM_MAD = static:gw_im_mad_static:-l etc/BES_hosts.list:dummy:bes
EM_MAD = bes:GW_em_mad_bes::jsdl
TM_MAD = dummy:gw_tm_mad_dummy:" >> /usr/etc/gwd.conf
touch /usr/etc/BES_hosts.list

%postun 
sed '/# MADs for OGSA-BES/d' -i /usr/etc/gwd.conf
sed '/bes/d' -i /usr/etc/gwd.conf
sed '/dummy/d' -i /usr/etc/gwd.conf
if [ ! -s "/usr/etc/BES_hosts.list" ]; then
 rm -f /usr/etc/BES_hosts.list;
fi

%build
cd src/em_mad/bes/
make
cd ../../tm_mad/dummy/
make 

%install
rm -rf $RPM_BUILD_ROOT
export GW_LOCATION=$RPM_BUILD_ROOT/usr/
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib
cd src/em_mad/bes/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../../tm_mad/dummy/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../..

%clean
rm -rf $RPM_BUILD_ROOT

%check

%files
%defattr(-,root,root,-)
/usr/bin/GW_em_mad_bes
/usr/lib/gw_em_mad_bes.jar
/usr/bin/gw_tm_mad_dummy
/usr/bin/gw_tm_mad_dummy.bin

%changelog
* Mon Feb 13 2012 GridWay Project Leads <contact@gridway.org> - 5.10-0
- Initial version
