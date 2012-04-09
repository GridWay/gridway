%define _name gridway
%define _release 1

Name:		gridway-BES
Version:	5.10
Release:	1%{dist}
Summary:	OGSA-BES MAD for GridWay

Group:		System Environment/Libraries
License:	Apache License
URL:		http://www.gridway.org/
Vendor:         Initiative for Globus in Europe (IGE)
Source:		%{_name}-%{version}.%{_release}.tar.gz
# wget http://svn.gridway.org/gridway/tags/GW_5_10_1/packaging/bes/RPM/LICENSE.ThirdParty
## BUILD Dependencies ##
# wget -U NoSuchBrowser/1.0 http://repo1.maven.org/maven2/org/apache/xmlbeans/xmlbeans/2.5.0/xmlbeans-2.5.0.jar
# wget http://sourceforge.net/projects/gridsam/files/gridsam/2.3.0/gridsam.war/download
# jar xvf gridsam.war WEB-INF/lib/gridsam-schema-2.3.0.jar

## RUNTIME Dependencies ##
# wget http://repo1.maven.org/maven2/xml-security/xmlsec/1.3.0/xmlsec-1.3.0.jar
# wget http://repo1.maven.org/maven2/wss4j/wss4j/1.5.1/wss4j-1.5.1.jar
# wget http://maven.omii.ac.uk/maven2/repository/omii/omii-security-utils/1.3/omii-security-utils-1.3.jar 

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
#Requires:	xmlbeans
#Requires:	gridsam-schema
#Requires:	xml-security

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
TM_MAD = dummy:gw_tm_mad_dummy:-u gsiftp\://<hostname>" >> /usr/etc/gwd.conf
touch /usr/etc/BES_hosts.list

%postun 
sed '/# MADs for OGSA-BES/d' -i /usr/etc/gwd.conf
sed '/bes/d' -i /usr/etc/gwd.conf
sed '/dummy/d' -i /usr/etc/gwd.conf
if [ ! -s "/usr/etc/BES_hosts.list" ]; then
 rm -f /usr/etc/BES_hosts.list;
fi

%build
cd src/em_mad/bes/lib
ln -s xmlbeans-2.5.0.jar xmlbeans.jar
ln -s wss4j-1.5.1.jar wss4j.jar
ln -s xmlsec-1.3.0.jar xmlsec.jar
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
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/gridway-bes/
mkdir -p $RPM_BUILD_ROOT/usr/etc
cd src/em_mad/bes/lib
cp xmlbeans.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/lib/
cp gridsam-schema-2.3.0.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/lib/
cp omii-security-utils-1.3.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/lib/
cp wss4j.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/lib/
cp xmlsec.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/lib/
cp /usr/share/java/xalan-j2.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/endorsed
cp /usr/share/java/xalan-j2-serializer.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/endorsed
cp /usr/share/java/axis/saaj.jar $RPM_BUILD_ROOT/usr/lib/java-ext/gridway-bes/endorsed
cd ../
cp LICENSE.ThirdParty $RPM_BUILD_ROOT/usr/share/doc/gridway-bes/
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
/usr/lib/java-ext/gridway-bes/lib/omii-security-utils-1.3.jar
/usr/lib/java-ext/gridway-bes/lib/wss4j.jar
/usr/lib/java-ext/gridway-bes/lib/xmlsec.jar
/usr/lib/java-ext/gridway-bes/endorsed/xalan-j2.jar
/usr/lib/java-ext/gridway-bes/endorsed/xalan-j2-serializer.jar
/usr/lib/java-ext/gridway-bes/endorsed/saaj.jar
/usr/share/doc/gridway-bes/LICENSE.ThirdParty
/usr/etc/client-config.wsdd
/usr/etc/crypto.properties
/usr/bin/gw_tm_mad_dummy
/usr/bin/gw_tm_mad_dummy.bin

%changelog
* Mon Apr 09 2012 GridWay Project Leads <contact@gridway.org> - 5.10-1
- Fixing RPM dependencies

* Wed Mar 28 2012 GridWay Project Leads <contact@gridway.org> - 5.10-0
- Update to 5.10-0

* Mon Feb 13 2012 GridWay Project Leads <contact@gridway.org> - 5.10-RC1
- Initial version
