.. rn:: 5.6.36-82.1

============================
|Percona Server| 5.6.36-82.1
============================

Percona is glad to announce the release of |Percona Server| 5.6.36-82.1 on
July 28, 2017 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.36-82.1/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.36
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-36.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.34-79.1 is the current GA release
in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.6.36-82.1
milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.6.36-82.1>`_.

New Features
============

 

Bugs Fixed
==========

 |Percona Server| could crash when running a data lookup on the partition table
 if there was only one partition over the time period following simple test. Bug
 fixed :bug:`1657941` (upstream :mysqlbug:`76418`).

 With two client connections to a server (debug server build), the server could
 crash after one of the clients set the global option ``userstat`` and flushed
 the client statistics (``FLUSH CLIENT_STATICTICS``) and then both clients were
 closed. Bug fixed :bug:`1661488`.

 When connecting to the Aurora cluster end point using SSL, SAN (Subject
 Alternative Name) certificates were ignored.  Bug fixed :bug:`1673656` (upstream
 :mysqlbug:`68052`).

 Killing a stored procedure which contained a query with a view resulted in a
 debug assertion failure. Bug fixed :bug:`1689736` (upstream :mysqlbug:`86260`).

 It was not possible to build |Percona Server| on Debian 9 (stretch) due to
 issues with openssl1.1. Bug fixed :bug:`1702903`.

 The ``SET STATEMENT .. FOR`` statement did not work after ``SET GLOBAL`` or
 ``SHOW GLOBAL STATUS`` commands and affected the global value. Bug fixed
 :bug:`1385352`.

 Killing a stored procedure which contained a query with a view resulted in a
 debug assertion failure. Bug fixed :bug:`1689736` (upstream :mysqlbug:`86260`).


Other bugs fixed: 
:bug:`1160986`,
:bug:`1676740`,
:bug:`1689989`,
:bug:`1689998`,
:bug:`1690012`,
:bug:`1699788`, and
:bug:`1684601` (upstream :mysqlbug:`86016`).
