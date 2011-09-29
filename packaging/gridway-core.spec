%define _name gridway
%define _release 0

Name:		gridway-core
Version:	5.8
Release:	0%{dist}
Summary:	GridWay Core
Packager:	GridWay Project Leads <http://gridway.org/>

Group:		System Environment/Libraries
License:	Apache License
URL:		http://www.gridway.org/
Vendor:		GridWay Project Leads
# wget http://dev.gridway.org/attachments/download/55/gridway-5.8.0.tar.gz
# wget http://dev.gridway.org/projects/gridway/repository/raw/branches/gw_5_8_branch/packaging/README
# wget http://dev.gridway.org/projects/gridway/repository/raw/branches/gw_5_8_branch/packaging/gwd
# tar -xzf gridway-5.8.0.tar.gz
# cp -p README gridway-5.8.0/
# cp -p gwd gridway-5.8.0/etc/
# chmod 755 gridway-5.8.0/etc/gwd
# tar -czf gridway-5.8.0.tar.gz gridway-5.8.0/

Source:		%{_name}-%{version}.%{_release}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)

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
%setup -q -n %{_name}-%{version}.%{_release}

%post
/usr/sbin/groupadd gwusers > /dev/null 2> /dev/null
/usr/sbin/useradd -d /usr/share/%{_name} -g gwusers -s /bin/sh gwadmin > /dev/null 2> /dev/null
chown -R gwadmin:gwusers /usr/share/%{_name}/%{version}.%{_release}
%if "%{?rhel}" <="5" || "%{?fedora}" <= "9"
mv /usr/share/%{_name}/%{version}.%{_release}/etc/gwd %{_initrddir}
%else
mv /usr/share/%{_name}/%{version}.%{_release}/etc/gwd %{_initddir}
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
./configure --prefix=/usr/share/%{_name}/%{version}.%{_release}
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
cp etc/gwd $RPM_BUILD_ROOT/usr/share/%{_name}/%{version}.%{_release}/etc

%clean
rm -rf $RPM_BUILD_ROOT

%check

%files
%defattr(-,root,root,-)
/usr/share/gridway/5.8.0/etc/gwd.conf
/usr/share/gridway/5.8.0/etc/gwrc
/usr/share/gridway/5.8.0/etc/job_template.default
/usr/share/gridway/5.8.0/etc/sched.conf
/usr/share/gridway/5.8.0/etc/gwd
/usr/share/gridway/5.8.0/bin/gw_flood_scheduler
/usr/share/gridway/5.8.0/bin/gw_im_mad_common.sh
/usr/share/gridway/5.8.0/bin/gw_im_mad_static
/usr/share/gridway/5.8.0/bin/gw_mad_common.sh
/usr/share/gridway/5.8.0/bin/gw_sched
/usr/share/gridway/5.8.0/bin/gwacct
/usr/share/gridway/5.8.0/bin/gwd
/usr/share/gridway/5.8.0/bin/gwdagman
/usr/share/gridway/5.8.0/bin/gwhistory
/usr/share/gridway/5.8.0/bin/gwhost
/usr/share/gridway/5.8.0/bin/gwkill
/usr/share/gridway/5.8.0/bin/gwps
/usr/share/gridway/5.8.0/bin/gwsubmit
/usr/share/gridway/5.8.0/bin/gwuser
/usr/share/gridway/5.8.0/bin/gwwait
/usr/share/gridway/5.8.0/lib/libdrmaa.a
/usr/share/gridway/5.8.0/lib/libdrmaa.la
/usr/share/gridway/5.8.0/lib/libdrmaa.so
/usr/share/gridway/5.8.0/lib/libdrmaa.so.0
/usr/share/gridway/5.8.0/lib/libdrmaa.so.0.0.0
/usr/share/gridway/5.8.0/libexec/gw_monitor.sh
/usr/share/gridway/5.8.0/libexec/gw_wrapper.sh
/usr/share/gridway/5.8.0/libexec/ruby/dagman/gridway.rb
/usr/share/gridway/5.8.0/libexec/ruby/dagman/node.rb
/usr/share/gridway/5.8.0/libexec/ruby/dagman/parse.rb
/usr/share/gridway/5.8.0/libexec/ruby/dagman/runner.rb
/usr/share/gridway/5.8.0/share/LICENSE
/usr/share/gridway/5.8.0/share/NOTICE
/usr/share/gridway/5.8.0/share/README
/usr/share/gridway/5.8.0/share/RELEASE_NOTES
/usr/share/gridway/5.8.0/include/drmaa.h
/usr/share/gridway/5.8.0/include/gw_acct.h
/usr/share/gridway/5.8.0/include/gw_action.h
/usr/share/gridway/5.8.0/include/gw_array.h
/usr/share/gridway/5.8.0/include/gw_array_pool.h
/usr/share/gridway/5.8.0/include/gw_client.h
/usr/share/gridway/5.8.0/include/gw_cmds_common.h
/usr/share/gridway/5.8.0/include/gw_common.h
/usr/share/gridway/5.8.0/include/gw_conf.h
/usr/share/gridway/5.8.0/include/gw_dm.h
/usr/share/gridway/5.8.0/include/gw_dm_mad.h
/usr/share/gridway/5.8.0/include/gw_drmaa_jt.h
/usr/share/gridway/5.8.0/include/gw_em.h
/usr/share/gridway/5.8.0/include/gw_em_mad.h
/usr/share/gridway/5.8.0/include/gw_em_rsl.h
/usr/share/gridway/5.8.0/include/gw_file_parser.h
/usr/share/gridway/5.8.0/include/gw_history.h
/usr/share/gridway/5.8.0/include/gw_host.h
/usr/share/gridway/5.8.0/include/gw_host_pool.h
/usr/share/gridway/5.8.0/include/gw_im.h
/usr/share/gridway/5.8.0/include/gw_im_mad.h
/usr/share/gridway/5.8.0/include/gw_job.h
/usr/share/gridway/5.8.0/include/gw_job_pool.h
/usr/share/gridway/5.8.0/include/gw_job_template.h
/usr/share/gridway/5.8.0/include/gw_log.h
/usr/share/gridway/5.8.0/include/gw_rm.h
/usr/share/gridway/5.8.0/include/gw_rm_connection_list.h
/usr/share/gridway/5.8.0/include/gw_rm_msg.h
/usr/share/gridway/5.8.0/include/gw_sch_conf.h
/usr/share/gridway/5.8.0/include/gw_scheduler.h
/usr/share/gridway/5.8.0/include/gw_template.h
/usr/share/gridway/5.8.0/include/gw_tm.h
/usr/share/gridway/5.8.0/include/gw_tm_mad.h
/usr/share/gridway/5.8.0/include/gw_um.h
/usr/share/gridway/5.8.0/include/gw_user.h
/usr/share/gridway/5.8.0/include/gw_user_pool.h
/usr/share/gridway/5.8.0/include/gw_xfr_files.h
/usr/share/gridway/5.8.0/xml_schema/gridway.xsd
/usr/share/gridway/5.8.0/var
/usr/share/gridway/5.8.0/var/acct

%changelog
* Thu Sep 30 2011 GridWay Project Leads <http://gridway.org/> - 5.8-0
- Update to GridWay 5.8-0

* Wed Aug 24 2011 GridWay Project Leads <http://gridway.org/> - 5.8-RC1
- Update to GridWay 5.8-RC1

* Wed Jun 29 2011 GridWay Project Leads <http://gridway.org/> - 5.7-1
- Fixed some recommendations for FHS compilant

* Thu Jun 9 2011 GridWay Project Leads <http://gridway.org/> - 5.7-1
- Update to GridWay 5.7-1

* Fri Jun 3 2011 GridWay Project Leads <http://gridway.org/> - 5.7-0
- Initial version
