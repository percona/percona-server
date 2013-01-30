.. rn:: 5.5.29-29.4

===============================
 |Percona Server| 5.5.29-29.4
===============================

Percona is glad to announce the release of |Percona Server| 5.5.29-29.4 on January 23rd, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.29-29.4/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.29 <http://dev.mysql.com/doc/refman/5.5/en/news-5.5.29.html>`_, including all the bug fixes in it, |Percona Server| 5.5.29-29.4 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.29-29.4 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.29-29.4>`_. 

Bug Fixes
=========

  Fixed the upstream bug :mysqlbug:`68045` and ported a fix for the security vulnerability `CVE-2012-4414 <http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2012-4414>`_ from the |Percona Server| :rn:`5.5.28-29.3`. This bug fix replaces the upstream fix for the |MySQL| bug :mysqlbug:`66550`. More details about this can be found in Stewart's `blogpost <http://www.mysqlperformanceblog.com/2013/01/13/cve-2012-4414-in-mysql-5-5-29-and-percona-server-5-5-29/>`_. Bug fixed :bug:`1049871` (*Vlad Lesin*).
