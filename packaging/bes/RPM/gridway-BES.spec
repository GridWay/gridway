%define _name gridway
%define _release RC1

Name:		gridway-BES
Version:	5.12
Release:	RC1%{dist}
Summary:	OGSA-BES MAD for GridWay

Group:		System Environment/Libraries
License:	Apache License
URL:		http://www.gridway.org/
Vendor:         Initiative for Globus in Europe (IGE)
Source:		%{_name}-%{version}.%{_release}.tar.gz
# svn co http://svn.gridway.org/gridway/tags/GW_5_12_RC1 gridway-5.12.RC1
# cp -p gridway-5.12.RC1/packaging/bes/RPM/README gridway-5.12.RC1/README
# cp -p gridway-5.12.RC1/packaging/bes/RPM/LICENSE.ThirdParty gridway-5.12.RC1/src/em_mad/bes/
## BUILD and RUNTIME Dependencies at src/em_mad/bes/lib ##
# wget -U NoSuchBrowser/1.0 http://repo1.maven.org/maven2/org/apache/xmlbeans/xmlbeans/2.5.0/xmlbeans-2.5.0.jar
# wget http://sourceforge.net/projects/gridsam/files/gridsam/2.3.0/gridsam.war/download
# jar xvf gridsam.war WEB-INF/lib/gridsam-schema-2.3.0.jar
# tar -czf gridway-5.12.RC1.tar.gz gridway-5.12.RC1/

BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)

BuildRequires:	java-devel
BuildRequires:	axis
BuildRequires:  xalan-j2
#BuildRequires:	xmlbeans
#BuildRequires:	gridsam-schema
Requires:	gridway-core
Requires:	java
Requires:       axis
Requires:       wsdl4j
Requires:       log4j
Requires:	xalan-j2
#Requires:	xmlbeans
#Requires:	gridsam-schema

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
echo "# MADs for OGSA-BES (GridSAM)
IM_MAD = static_gridsam:gw_im_mad_static:-l etc/GridSAM_hosts.list:dummy_gridsam:bes_gridsam
EM_MAD = bes_gridsam:GW_em_mad_bes:-t gridsam:jsdl
TM_MAD = dummy_gridsam:gw_tm_mad_dummy:-u gsiftp\://<hostname>
# MADs for OGSA-BES (UNICORE)
#IM_MAD = static_unicore:gw_im_mad_static:-l etc/UNICORE_hosts.list:dummy_unicore:bes_unicore
#EM_MAD = bes_unicore:GW_em_mad_bes:-t unicore:jsdl
#TM_MAD = dummy_unicore:gw_tm_mad_dummy:-i" >> /usr/etc/gwd.conf
touch /usr/etc/GridSAM_hosts.list
touch /usr/etc/UNICORE_hosts.list

%postun 
sed '/# MADs for OGSA-BES/d' -i /usr/etc/gwd.conf
sed '/static_gridsam/d' -i /usr/etc/gwd.conf
sed '/static_unicore/d' -i /usr/etc/gwd.conf
sed '/bes/d' -i /usr/etc/gwd.conf
sed '/dummy/d' -i /usr/etc/gwd.conf
if [ ! -s "/usr/etc/GridSAM_hosts.list" ]; then
 rm -f /usr/etc/GridSAM_hosts.list;
fi
if [ ! -s "/usr/etc/UNICORE_hosts.list" ]; then
 rm -f /usr/etc/UNICORE_hosts.list;
fi

%build
cd src/em_mad/bes/lib
ln -s xmlbeans-2.5.0.jar xmlbeans.jar
cd ../
make
cd ../../tm_mad/dummy/
make 

%install
rm -rf $RPM_BUILD_ROOT
export GW_LOCATION=$RPM_BUILD_ROOT/usr/
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib
mkdir -p $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/lib
mkdir -p $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/endorsed
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/gridway-bes-5.12.RC1/
mkdir -p $RPM_BUILD_ROOT/usr/etc
cp README $RPM_BUILD_ROOT/usr/share/doc/gridway-bes-5.12.RC1/
cd src/em_mad/bes/lib
cp xmlbeans.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/lib/
cp gridsam-schema-2.3.0.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/lib/
cp /usr/share/java/xalan-j2.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/endorsed
cp /usr/share/java/xalan-j2-serializer.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/endorsed
cp /usr/share/java/axis/saaj.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/endorsed
cd ../
cp LICENSE.ThirdParty $RPM_BUILD_ROOT/usr/share/doc/gridway-bes-5.12.RC1/
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
/usr/lib/java-ext/gridway-bes/lib/xmlbeans.jar
/usr/lib/java-ext/gridway-bes/lib/gridsam-schema-2.3.0.jar
/usr/lib/java-ext/gridway-bes/endorsed/xalan-j2.jar
/usr/lib/java-ext/gridway-bes/endorsed/xalan-j2-serializer.jar
/usr/lib/java-ext/gridway-bes/endorsed/saaj.jar
/usr/share/doc/gridway-bes-5.12.RC1/LICENSE.ThirdParty
/usr/share/doc/gridway-bes-5.12.RC1/README
/usr/etc/client-config.wsdd
/usr/etc/crypto.properties
/usr/bin/gw_tm_mad_dummy
/usr/bin/gw_tm_mad_dummy.bin

%changelog
* Fri Aug 31 2012 GridWay Project Leads <contact@gridway.org> - 5.12-RC1
- Update to 5.12-RC1

* Thu May 17 2012 GridWay Project Leads <contact@gridway.org> - 5.10-2
- Bug fixing

* Mon Apr 09 2012 GridWay Project Leads <contact@gridway.org> - 5.10-1
- Fixing RPM dependencies

* Wed Mar 28 2012 GridWay Project Leads <contact@gridway.org> - 5.10-0
- Update to 5.10-0

* Mon Feb 13 2012 GridWay Project Leads <contact@gridway.org> - 5.10-RC1
- Initial version
