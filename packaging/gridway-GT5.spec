%define _name gridway
%ifarch alpha ia64 ppc64 s390x sparc64 x86_64
%global flavor gcc64pthr
%else
%global flavor gcc32pthr
%endif

Name:		gridway-GT5
Version:	5.7
Release:	1%{?dist}
Summary:	GT5 MADs for GridWay
Packager:	GridWay Project Leads <http://gridway.org/>

Group:		System Environment/Libraries
License:	Apache License
URL:		http://www.gridway.org/
Vendor:		GridWay Project Leads
Source:		%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	globus-common-devel
BuildRequires:	globus-gram-client
BuildRequires:	globus-gram-client-devel
BuildRequires:	globus-gass-copy
BuildRequires:	globus-ftp-client-devel
BuildRequires:	globus-gass-copy-devel
BuildRequires:	grid-packaging-tools
Requires:	gridway-core
Requires:	globus-gram-client
Requires:	globus-gass-copy
Requires:	globus-gass-copy-progs
Requires:	globus-proxy-utils

%description
GridWay uses several Middleware Access Drivers (MAD) to interface with 
different Grid services. The following MADs are part of the GridWay 
distribution: Information MADS to interface with different information 
services, Execution MADs to interface with different execution services, and 
Transfer MADs to interface with different transfer services.

The %{_name}-GT5 package contains:
GridWay MADs to interface with Globus Toolkit 5 services. These are an 
Execution MAD to interface with GRAM2 services and a Transfer MAD to interface 
with GridFTP services.

%prep
%setup -q -n %{_name}-%{version}

%post 
chown -R gwadmin:gwusers /opt/%{_name}
echo "IM_MAD = static:gw_im_mad_static:-l etc/gram2_hosts.list:gridFTP:gram2
EM_MAD = gram2:gw_em_mad_gram2::rsl
TM_MAD = gridFTP:gw_tm_mad_ftp:" >> /opt/%{_name}/%{version}/etc/gwd.conf
touch /opt/%{_name}/%{version}/etc/gram2_hosts.list

%postun 
sed '/gram2/d' -i /opt/%{_name}/%{version}/etc/gwd.conf
sed '/gridFTP/d' -i /opt/%{_name}/%{version}/etc/gwd.conf
if [ ! -s "/opt/%{_name}/%{version}/etc/gram2_hosts.list" ]; then
 rm -f /opt/%{_name}/%{version}/etc/gram2_hosts.list;
fi

%build
export GLOBUS_LOCATION=/usr
cd src/em_mad/gram2/
globus-makefile-header --flavor=%{flavor} globus_gram_client > makefile-header
make
cd ../../tm_mad/gridftp/
globus-makefile-header --flavor=%{flavor} globus_gass_copy > makefile-header
make 

%install
rm -rf $RPM_BUILD_ROOT
export GW_LOCATION=$RPM_BUILD_ROOT/opt/%{_name}/%{version}/
mkdir -p $RPM_BUILD_ROOT/opt/%{_name}/%{version}/bin
cd src/em_mad/gram2/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../../tm_mad/gridftp/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../..

%clean
rm -rf $RPM_BUILD_ROOT

%check

%files
%defattr(-,root,root,-)
/opt/gridway/5.7/bin/gw_em_mad_gram2
/opt/gridway/5.7/bin/gw_em_mad_gram2.bin
/opt/gridway/5.7/bin/gw_tm_mad_ftp
/opt/gridway/5.7/bin/gw_tm_mad_ftp.bin

%changelog
* Wed Jun 29 2011 GridWay Project Leads <http://gridway.org/> - 5.7-1.el5
- Fixed some recommendations for FHS compilant

* Thu Jun 9 2011 GridWay Project Leads <http://gridway.org/> - 5.7-1.el5
- Update to GridWay 5.7-1 

* Mon Jun 6 2011 GridWay Project Leads <http://gridway.org/> - 5.7-0.el5
- Initial version
