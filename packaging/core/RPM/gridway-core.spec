%define _name gridway
%define _release RC1

Name:		gridway-core
Version:	5.14
Release:	RC1%{dist}
Summary:	GridWay Core

Group:		System Environment/Libraries
License:	Apache License
URL:		http://www.gridway.org/
Vendor:         Initiative for Globus in Europe (IGE)
# svn co http://svn.gridway.org/gridway/tags/GW_%{version}_%{_release} gridway-%{version}.%{_release}
# cp -p gridway-%{version}.%{_release}/packaging/core/RPM/README gridway-%{version}.%{_release}/README
# cp -p gridway-%{version}.%{_release}/packaging/core/RPM/gwd gridway-%{version}.%{_release}/etc/
# chmod 755 gridway-%{version}.%{_release}/etc/gwd
# tar -czf gridway-%{version}.%{_release}.tar.gz gridway-%{version}.%{_release}/

Source:		%{_name}-%{version}.%{_release}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)

BuildRequires:	gcc
BuildRequires:	java-devel
Requires(post): chkconfig
Requires(preun): chkconfig
Requires(preun): initscripts
Requires(postun): initscripts
Requires: db4
Requires: db4-devel
Requires: db4-utils
Requires: globus-proxy-utils

%description
The GridWay Metascheduler enables large-scale, reliable and efficient sharing 
of computing resources: clusters, supercomputers, stand-alone servers... It 
supports different LRM systems (PBS, SGE, LSF, Condor...) within a single 
organization or scattered across several administrative domains. GridWay 
provides a single point of access to all resources in your organization, from
 in-house systems to Grid infrastructures and Cloud providers. GridWay supports
 most of existing Grid middle-wares, can be used on main production Grid 
infrastructures and can dynamically access to Cloud resources.

The %{_name}-core package contains:
GridWay Core

%prep
%setup -q -n %{_name}-%{version}.%{_release}

%post
/usr/sbin/groupadd gwusers > /dev/null 2> /dev/null
/usr/sbin/useradd -d /var/lib/%{_name} -g gwusers -s /bin/sh gwadmin > /dev/null 2> /dev/null
chown -R gwadmin:gwusers /var/lib/%{_name}/
/sbin/chkconfig --add gwd
/sbin/ldconfig

%preun
if [ $1 -eq 0 ] ; then
 /sbin/service gwd stop >/dev/null 2>&1
 /sbin/chkconfig --del gwd
fi

%postun
/usr/sbin/userdel gwadmin > /dev/null 2> /dev/null
/usr/sbin/groupdel gwusers > /dev/null 2> /dev/null
if [ "$1" -ge "1" ] ; then
 /sbin/service gwd condrestart >/dev/null 2>&1 || :
fi
/sbin/ldconfig

%build
JAVA_HOME=/usr/lib/jvm/java ./configure --prefix=/usr/ --localstatedir=/var/lib/%{_name}/ --datarootdir=/usr/share/doc/%{_name}-%{version}.%{_release} --with-db=yes --with-syslog=LOG_USER --enable-drmaa-java
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mv $RPM_BUILD_ROOT/usr/xml_schema/ $RPM_BUILD_ROOT/var/lib/%{_name}/
%if "%{?rhel}" <="5" || "%{?fedora}" <= "9"
mkdir -p $RPM_BUILD_ROOT/%{_initrddir}
cp etc/gwd $RPM_BUILD_ROOT/%{_initrddir}
%else
mkdir -p $RPM_BUILD_ROOT/%{_initddir}
cp etc/gwd $RPM_BUILD_ROOT/%{_initddir}
%endif
rm -rf $RPM_BUILD_ROOT/usr/bin/gw_flood_scheduler

%clean
rm -rf $RPM_BUILD_ROOT

%check

%files
%defattr(-,root,root,-)
/usr/etc/gwd.conf
/usr/etc/gwrc
/usr/etc/job_template.default
/usr/etc/sched.conf
%if "%{?rhel}" <="5" || "%{?fedora}" <= "9"
%{_initrddir}/gwd
%else
%{_initddir}/gwd
%endif
/usr/bin/gw_im_mad_common.sh
/usr/bin/gw_im_mad_static
/usr/bin/gw_mad_common.sh
/usr/bin/gw_sched
/usr/bin/gwacct
/usr/bin/gwd
/usr/bin/gwdagman
/usr/bin/gwhistory
/usr/bin/gwhost
/usr/bin/gwkill
/usr/bin/gwps
/usr/bin/gwsubmit
/usr/bin/gwuser
/usr/bin/gwwait
/usr/lib/libdrmaa.a
/usr/lib/libdrmaa.la
/usr/lib/libdrmaa.so
/usr/lib/libdrmaa.so.0
/usr/lib/libdrmaa.so.0.0.0
/usr/libexec/gw_monitor.sh
/usr/libexec/gw_wrapper.sh
/usr/libexec/ruby/dagman/gridway.rb
/usr/libexec/ruby/dagman/node.rb
/usr/libexec/ruby/dagman/parse.rb
/usr/libexec/ruby/dagman/runner.rb
/usr/share/doc/gridway-5.14.RC1/LICENSE
/usr/share/doc/gridway-5.14.RC1/NOTICE
/usr/share/doc/gridway-5.14.RC1/README
/usr/share/doc/gridway-5.14.RC1/RELEASE_NOTES
/usr/include/drmaa.h
/usr/include/gw_acct.h
/usr/include/gw_action.h
/usr/include/gw_array.h
/usr/include/gw_array_pool.h
/usr/include/gw_client.h
/usr/include/gw_cmds_common.h
/usr/include/gw_common.h
/usr/include/gw_conf.h
/usr/include/gw_dm.h
/usr/include/gw_dm_mad.h
/usr/include/gw_drmaa_jt.h
/usr/include/gw_em.h
/usr/include/gw_em_mad.h
/usr/include/gw_em_rsl.h
/usr/include/gw_file_parser.h
/usr/include/gw_history.h
/usr/include/gw_host.h
/usr/include/gw_host_pool.h
/usr/include/gw_im.h
/usr/include/gw_im_mad.h
/usr/include/gw_job.h
/usr/include/gw_job_pool.h
/usr/include/gw_job_template.h
/usr/include/gw_log.h
/usr/include/gw_rm.h
/usr/include/gw_rm_connection_list.h
/usr/include/gw_rm_msg.h
/usr/include/gw_sch_conf.h
/usr/include/gw_scheduler.h
/usr/include/gw_template.h
/usr/include/gw_tm.h
/usr/include/gw_tm_mad.h
/usr/include/gw_um.h
/usr/include/gw_user.h
/usr/include/gw_user_pool.h
/usr/include/gw_xfr_files.h
/var/lib/gridway/xml_schema/gridway.xsd
/var/lib/gridway
/var/lib/gridway/acct
/usr/lib/drmaa.jar
/usr/lib/libDrmaaJNI.a
/usr/lib/libDrmaaJNI.la
/usr/lib/libDrmaaJNI.so
/usr/lib/libDrmaaJNI.so.0
/usr/lib/libDrmaaJNI.so.0.0.0
/usr/include/drmaa2.h
/usr/lib/libdrmaa2.a
/usr/lib/libdrmaa2.la
/usr/lib/libdrmaa2.so
/usr/lib/libdrmaa2.so.0
/usr/lib/libdrmaa2.so.0.0.0
/usr/libexec/ssh_wrapper.sh

%changelog
* Thu Feb 28 2013 GridWay Project Leads <contact@gridway.org> - 5.14-RC1
- Update to 5.14-RC1

* Sun Sep 30 2012 GridWay Project Leads <contact@gridway.org> - 5.12-0
- Update to 5.12-0

* Mon Sep 10 2012 GridWay Project Leads <contact@gridway.org> - 5.12-RC2
- Update to 5.12-RC2

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

* Thu Sep 30 2011 GridWay Project Leads <contact@gridway.org> - 5.8-0
- Update to 5.8-0

* Wed Aug 24 2011 GridWay Project Leads <contact@gridway.org> - 5.8-RC1
- Update to 5.8-RC1

* Wed Jun 29 2011 GridWay Project Leads <contact@gridway.org> - 5.7-1
- Fixed some recommendations for FHS compilant

* Thu Jun 09 2011 GridWay Project Leads <contact@gridway.org> - 5.7-1
- Update to 5.7-1

* Fri Jun 03 2011 GridWay Project Leads <contact@gridway.org> - 5.7-0
- Initial version
