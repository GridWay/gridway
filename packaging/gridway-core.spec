%define _name gridway

Name:		gridway-core
Version:	5.7
Release:	1%{?dist}
Summary:	GridWay Core
Packager:	GridWay Project Leads <http://gridway.org/>

Group:		System Environment/Libraries
License:	Apache License
URL:		http://www.gridway.org/
Vendor:		GridWay Project Leads
# wget http://dev.gridway.org/attachments/download/50/gridway-5.7.tar.gz
# wget http://dev.gridway.org/projects/gridway/repository/raw/trunk/packaging/README
# wget http://dev.gridway.org/projects/gridway/repository/raw/trunk/packaging/gwd
# wget http://dev.gridway.org/projects/gridway/repository/revisions/415/raw/trunk/src/Makefile.am
# wget http://dev.gridway.org/projects/gridway/repository/revisions/415/raw/trunk/src/Makefile.in
# tar -xzf gridway-5.7.tar.gz
# cp -p README gridway-5.7/src/share/README
# cp -p gwd gridway-5.7/src/etc/gwd
# cp -p Makefile.am gridway-5.7/src/
# cp -p Makefile.in gridway-5.7/src/
# tar -czf gridway-5.7.tar.gz gridway-5.7/

Source:		%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	gcc
Requires(post): chkconfig
Requires(preun): chkconfig
Requires(preun): initscripts
Requires(postun): initscripts

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
%setup -q -n %{_name}-%{version}

%post
/usr/sbin/groupadd gwusers > /dev/null 2> /dev/null
/usr/sbin/useradd -d /opt/%{_name}/%{version} -g gwusers -s /bin/sh gwadmin > /dev/null 2> /dev/null
chown -R gwadmin:gwusers /opt/%{_name}/%{version}
%if "%{?rhel}" <="5" || "%{?fedora}" <= "9"
mv /opt/%{_name}/%{version}/etc/gwd %{_initrddir}
%else
mv /opt/%{_name}/%{version}/etc/gwd %{_initddir}
%endif
/sbin/chkconfig --add gwd

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
%if "%{?rhel}" <="5" || "%{?fedora}" <= "9"
rm %{_initrddir}/gwd
%else
rm %{_initddir}/gwd
%endif

%build
./configure --prefix=/opt/%{_name}/%{version}
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
cp etc/gwd $RPM_BUILD_ROOT/opt/%{_name}/%{version}/etc

%clean
rm -rf $RPM_BUILD_ROOT

%check

%files
%defattr(-,root,root,-)
/opt/gridway/5.7/etc/gwd.conf
/opt/gridway/5.7/etc/gwrc
/opt/gridway/5.7/etc/job_template.default
/opt/gridway/5.7/etc/sched.conf
/opt/gridway/5.7/etc/gwd
/opt/gridway/5.7/bin/gw_flood_scheduler
/opt/gridway/5.7/bin/gw_im_mad_common.sh
/opt/gridway/5.7/bin/gw_im_mad_static
/opt/gridway/5.7/bin/gw_mad_common.sh
/opt/gridway/5.7/bin/gw_sched
/opt/gridway/5.7/bin/gwacct
/opt/gridway/5.7/bin/gwd
/opt/gridway/5.7/bin/gwdagman
/opt/gridway/5.7/bin/gwhistory
/opt/gridway/5.7/bin/gwhost
/opt/gridway/5.7/bin/gwkill
/opt/gridway/5.7/bin/gwps
/opt/gridway/5.7/bin/gwsubmit
/opt/gridway/5.7/bin/gwuser
/opt/gridway/5.7/bin/gwwait
/opt/gridway/5.7/lib/libdrmaa.a
/opt/gridway/5.7/lib/libdrmaa.la
/opt/gridway/5.7/lib/libdrmaa.so
/opt/gridway/5.7/lib/libdrmaa.so.0
/opt/gridway/5.7/lib/libdrmaa.so.0.0.0
/opt/gridway/5.7/libexec/gw_monitor.sh
/opt/gridway/5.7/libexec/gw_wrapper.sh
/opt/gridway/5.7/libexec/ruby/dagman/gridway.rb
/opt/gridway/5.7/libexec/ruby/dagman/node.rb
/opt/gridway/5.7/libexec/ruby/dagman/parse.rb
/opt/gridway/5.7/libexec/ruby/dagman/runner.rb
/opt/gridway/5.7/share/LICENSE
/opt/gridway/5.7/share/NOTICE
/opt/gridway/5.7/share/README
/opt/gridway/5.7/share/RELEASE_NOTES
/opt/gridway/5.7/include/drmaa.h
/opt/gridway/5.7/include/gw_acct.h
/opt/gridway/5.7/include/gw_action.h
/opt/gridway/5.7/include/gw_array.h
/opt/gridway/5.7/include/gw_array_pool.h
/opt/gridway/5.7/include/gw_client.h
/opt/gridway/5.7/include/gw_cmds_common.h
/opt/gridway/5.7/include/gw_common.h
/opt/gridway/5.7/include/gw_conf.h
/opt/gridway/5.7/include/gw_dm.h
/opt/gridway/5.7/include/gw_dm_mad.h
/opt/gridway/5.7/include/gw_drmaa_jt.h
/opt/gridway/5.7/include/gw_em.h
/opt/gridway/5.7/include/gw_em_mad.h
/opt/gridway/5.7/include/gw_em_rsl.h
/opt/gridway/5.7/include/gw_file_parser.h
/opt/gridway/5.7/include/gw_history.h
/opt/gridway/5.7/include/gw_host.h
/opt/gridway/5.7/include/gw_host_pool.h
/opt/gridway/5.7/include/gw_im.h
/opt/gridway/5.7/include/gw_im_mad.h
/opt/gridway/5.7/include/gw_job.h
/opt/gridway/5.7/include/gw_job_pool.h
/opt/gridway/5.7/include/gw_job_template.h
/opt/gridway/5.7/include/gw_log.h
/opt/gridway/5.7/include/gw_rm.h
/opt/gridway/5.7/include/gw_rm_connection_list.h
/opt/gridway/5.7/include/gw_rm_msg.h
/opt/gridway/5.7/include/gw_sch_conf.h
/opt/gridway/5.7/include/gw_scheduler.h
/opt/gridway/5.7/include/gw_template.h
/opt/gridway/5.7/include/gw_tm.h
/opt/gridway/5.7/include/gw_tm_mad.h
/opt/gridway/5.7/include/gw_um.h
/opt/gridway/5.7/include/gw_user.h
/opt/gridway/5.7/include/gw_user_pool.h
/opt/gridway/5.7/include/gw_xfr_files.h
/opt/gridway/5.7/xml_schema/gridway.xsd
/opt/gridway/5.7/var
/opt/gridway/5.7/var/acct

%changelog
* Wed Jun 29 2011 GridWay Project Leads <http://gridway.org/> - 5.7-1.el5
- Fixed some recommendations for FHS compilant

* Thu Jun 9 2011 GridWay Project Leads <http://gridway.org/> - 5.7-1.el5
- Update to GridWay 5.7-1

* Fri Jun 3 2011 GridWay Project Leads <http://gridway.org/> - 5.7-0.el5
- Initial version
