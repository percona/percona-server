.. rn:: 5.6.36-82.1

============================
|Percona Server| 5.6.36-82.1
============================

Percona is glad to announce the release of |Percona Server| 5.6.36-82.1 on
August 1, 2017 (Downloads are available `here
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

|Percona Server| can now be built with support of OpenSSL 1.1.

|Percona Server| is now available on Debian 9 (stretch). The support only covers
the ``amd64`` architecture.

|TokuDB| enables to kill a query that is awaiting an FT locktree lock.


Bugs Fixed
==========

 Row counts in |TokuDB| could be lost intermittently after restarts. Bug fixed
 :tdbbug:`2`.

 In |TokuDB|, two races in the fractal tree lock manager could significantly
 affect transactional throughput for some applications that used a small number
 of concurrent transactions.  These races manifested as transactions
 unnecessarily waiting for an available lock. Bug fixed :tdbbug:`3`.

 |TokuDB| could assert when opening a dictionary with no useful information
 to error log. Bug fixed :tdbbug:`23`.

 |TokuDB| could assert for various reasons deserializing nodes with no useful
 error output. Bug fixed :tdbbug:`24`.

 |Percona Server| could crash when running a query over a partitioned table that
 uses an index to read a range of rows if this range was not covered by any
 existing partition. Bug fixed :bug:`1657941` (upstream :mysqlbug:`76418`).

 With two client connections to a server (debug server build), the server could
 crash after one of the clients set the global option ``userstat`` and flushed
 the client statistics (``FLUSH CLIENT_STATISTICS``) and then both clients were
 closed. Bug fixed :bug:`1661488`.

 |TokuDB| did not pass cmake flags on to snappy cmake. Bug fixed
 :tdbbug:`41`.  The progress status for partitioned TokuDB table ALTERs was
 misleading. Bug fixed :tdbbug:`42`.

 When a client application connecting to the Aurora cluster end point
 using SSL (``--ssl-verify-server-cert`` or
 ``--ssl-mode=VERIFY_IDENTITY`` option), wildcard and :abbr:`SAN
 (Subject Alternative Name)` enabled SSL certificates were ignored. See
 also :ref:`compatibility-matrix`.  Note that the
 ``--ssl-verify-server-cert`` option is deprecated in |Percona Server|
 5.7. Bug fixed :bug:`1673656` (upstream :mysqlbug:`68052`).

 Killing a stored procedure execution could result in an assert failure on a
 debug server build. Bug fixed :bug:`1689736` (upstream :mysqlbug:`86260`).

 It was not possible to build |Percona Server| on Debian 9 (stretch) due to
 issues with OpenSSL 1.1. Bug fixed :bug:`1702903` (upstream :mysqlbug:`83814`). 

 The ``SET STATEMENT .. FOR`` statement changed the global instead of the session
 value of a variable if the statement occurred immediately after the ``SET
 GLOBAL`` or ``SHOW GLOBAL STATUS`` command. Bug fixed :bug:`1385352`.

 The synchronization between the LRU manager and page cleaner threads was not
 done at shutdown. Bug fixed :bug:`1689552`.

Other bugs fixed: 
:tdbbug:`6`,
:tdbbug:`44`,
:tdbbug:`65`,
:bug:`1160986`,
:bug:`1676740`,
:bug:`1689989`,
:bug:`1689998`,
:bug:`1690012`,
:bug:`1699788`, and
:bug:`1684601` (upstream :mysqlbug:`86016`).

.. _compatibility-matrix:

Compatibility Matrix
====================

=======================  =======  ==================  ====================
Feature                  YaSSL    OpenSSL < 1.0.2     OpenSSL >= 1.0.2
=======================  =======  ==================  ====================
'commonName' validation  Yes      Yes                 Yes       
SAN validation           No       Yes                 Yes       
Wildcards support        No       No                  Yes         
=======================  =======  ==================  ====================
