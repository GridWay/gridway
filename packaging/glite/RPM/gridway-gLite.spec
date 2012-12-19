%define _name gridway
%define _release RC1

Name:		gridway-gLite
Version:	5.14
Release:	RC1%{dist}
Summary:	gLite MADs for GridWay

Group:		System Environment/Libraries
License:	Apache License
URL:		http://www.gridway.org/
Vendor:         Initiative for Globus in Europe (IGE)
Source:		%{_name}-%{version}.%{_release}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)

BuildRequires:	glite-ce-cream-client-devel
Requires:	gridway-core
Requires:	globus-gass-server-ez-progs
Requires:	glite-ce-cream-client-api-c
Requires:	glite-lbjp-common-gsoap-plugin
# svn co http://svn.gridway.org/gridway/tags/GW_%{version}_%{_release} gridway-%{version}.%{_release}
# cp -p gridway-%{version}.%{_release}/packaging/glite/RPM/README gridway-%{version}.%{_release}/README.glite
# tar -czf gridway-%{version}.%{_release}.tar.gz gridway-%{version}.%{_release}/

%description
GridWay uses several Middleware Access Drivers (MAD) to interface with 
different Grid services. The following MADs are part of the GridWay 
distribution: Information MADS to interface with different information 
services, Execution MADs to interface with different execution services, and 
Transfer MADs to interface with different transfer services.

The %{_name}-gLite package contains:
GridWay MADs to interface with gLite 3.2 services. These are an 
Information MAD to interface with BDII services, an Execution MAD 
to interface with CREAM services and a dummy Transfer MAD.

%prep
%setup -q -n %{_name}-%{version}.%{_release}

%post 
echo "# MADs for gLite
IM_MAD = bdii:gw_im_mad_bdii:-s <bdii-server> -q (GlueCEAccessControlBaseRule=VO\:<VO>)(GlueCEImplementationName=CREAM):dummy:cream
EM_MAD = cream:gw_em_mad_cream::jdl
TM_MAD = dummy:gw_tm_mad_dummy:-g" >> /usr/etc/gwd.conf

%postun 
sed '/# MADs for gLite/d' -i /usr/etc/gwd.conf
sed '/cream/d' -i /usr/etc/gwd.conf
sed '/dummy/d' -i /usr/etc/gwd.conf

%build
cd src/em_mad/cream/
make
cd ../../tm_mad/dummy/
make 

%install
rm -rf $RPM_BUILD_ROOT
export GW_LOCATION=$RPM_BUILD_ROOT/usr/
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/gridway-glite-5.14.RC1/
cp README.glite $RPM_BUILD_ROOT/usr/share/doc/gridway-glite-5.14.RC1/README
cd src/im_mad/bdii/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../../em_mad/cream/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../../tm_mad/dummy/
make install DESTDIR=$RPM_BUILD_ROOT
cd ../..

%clean
rm -rf $RPM_BUILD_ROOT

%check

%files
%defattr(-,root,root,-)
/usr/bin/gw_im_mad_bdii
/usr/libexec/perl/gw_im_mad_bdii.pl
/usr/bin/gw_em_mad_cream
/usr/bin/gw_em_mad_cream.bin
/usr/bin/gw_tm_mad_dummy
/usr/bin/gw_tm_mad_dummy.bin
/usr/share/doc/gridway-glite-5.14.RC1/README

%changelog
* Thu Feb 28 2013 GridWay Project Leads <contact@gridway.org> - 5.14-RC1
- Update to 5.14-RC1

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
