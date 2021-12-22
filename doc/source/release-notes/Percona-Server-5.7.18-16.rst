.. rn:: 5.7.18-16

==========================
|Percona Server| 5.7.18-16
==========================

Percona is glad to announce the GA (Generally Available) release of |Percona
Server| 5.7.18-16 on July 28, 2017 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.18-16/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.18
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-18.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.18-16 is the current GA release
in the |Percona Server| 5.7 series. All of Percona's software is open-source
and free, all the details of the release can be found in the `5.7.18-16
milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.7.18-16>`_

Please note that RHEL 5, CentOS 5 and Ubuntu versions 12.04 and older are not
supported in future releases of |Percona Server| and no further packages are
added for these distributions.

New Feature
===========

|Percona Server| is now available on Debian 9 (stretch). The support only
covers the ``amd64`` architecture.

|Percona Server| can now be built with support of OpenSSL 1.1.

MyRocks storage engine has been merged into |Percona Server|.

TokuDB enables to kill a query that is awaiting an FT locktree lock.

TokuDB enables using the ``MySQL DEBUG_SYNC`` facility within |Percona FT|.

Bugs Fixed
==========

Row counts in TokuDB could be lost intermittently after restarts. Bug fixed
:tdbbug:`2`.

In TokuDB, two races in the fractal tree lock manager could significantly
affect transactional throughput for some applications that used a small number
of concurrent transactions.  These races manifested as transactions
unnecessarily waiting for an available lock. Bug fixed :tdbbug:`3`.

|Percona FT| could assert when opening a dictionary with no useful information
to error log. Bug fixed :tdbbug:`23`.

|Percona FT| could assert for various reasons deserializing nodes with no
useful error output. Bug fixed :tdbbug:`24`.

It was not possible to build |Percona Server| on Debian 9 (stretch) due to
issues with OpenSSL 1.1. Bug fixed :bug:`1702903` (upstream :mysqlbug:`83814`).

Packaging was using the ``dpkg --verify`` command which is not available on
wheezy/precise. Bug fixed :bug:`1694907`.

Enabling and disabling the slow query log rotation spuriously added the version
suffix to the next slow query log file name. Bug fixed :bug:`1704056`.

With two client connections to a server (debug server build), the server could
crash after one of the clients set the global option ``userstat`` and flushed
the client statistics (``FLUSH CLIENT_STATISTICS``) and then both clients were
closed. Bug fixed :bug:`1661488`.

|Percona FT| did not pass cmake flags on to snappy cmake. Bug fixed
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

The ``SET STATEMENT .. FOR`` statement changed the global instead of the
session value of a variable if the statement occurred immediately after the
``SET GLOBAL`` or ``SHOW GLOBAL STATUS`` command. Bug fixed :bug:`1385352`.

When running ``SHOW ENGINE INNODB STATUS``, the ``Buffer pool size, bytes``
entry contained **0**. BUg fixed :bug:`1586262`.

The synchronization between the LRU manager and page cleaner threads was not
done at shutdown. Bug fixed :bug:`1689552`.

Removed spurious ``lock_wait_timeout_thread`` wakeups, potentially reducing
``lock_sys_wait_mutex`` contention. Patch by Inaam Rama merged from
``WebScaleSQL``. Bug fixed :bug:`1704267` (upstream :mysqlbug:`72123`).

Other bugs fixed:
:bug:`1686603`,
:tdbbug:`6`,
:tdbbug:`44`,
:tdbbug:`65`,
:bug:`1160986`,
:bug:`1686934`,
:bug:`1688319`,
:bug:`1689989`,
:bug:`1690012`,
:bug:`1691682`,
:bug:`1697700`,
:bug:`1699788`,
:bug:`1121072`, and
:bug:`1684601` (upstream :mysqlbug:`86016`).


.. note:: Due to new package dependency,
   Ubuntu/Debian users should use ``apt-get dist-upgrade``
   or ``apt-get install percona-server-server-5.7`` to upgrade.

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
