.. rn:: 5.6.44-86.0

================================================================================
|Percona Server| 5.6.44-86.0
================================================================================

Percona is glad to announce the release of |Percona Server| 5.6.44-86.0 on
May 24, 2019 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.44-86.0/>`_
and from the :doc:`Percona Software Repositories </installation>`).

This release merges changes of `MySQL 5.6.44
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-44.html>`_, including
all the bug fixes in it. Percona Server for MySQL 5.6.44-86.0 is now the current
GA release in the 5.6 series. All of Percona’s software is open-source and free.

Bugs Fixed
================================================================================

 Percona Server 5.6.44-85.0-1 Debian/Ubuntu packages would reset the root
 password to an empty string during the upgrade on Debian/Ubuntu. This fixes
 the `CVE-2019-12301 <https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2019-12301>`_
 issue. Bug fixed :psbug:`5640`.

 As mentioned in the
 `blogpost <https://www.percona.com/blog/2019/05/24/critical-update-for-percona-server-for-mysql-5-6-44-85-0/>`_
 packages were replaced in the repositories after the issue was discovered. While
 CentOS/RHEL wasn't affected by the CVE, some users got the 
 ``[Errno -1]  Package does not match intended download.`` message when trying to install
 the packages  once they were replaced. 

Report bugs in the `Jira bug tracker <https://jira.percona.com/projects/PS>`_.
