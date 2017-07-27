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
in the |Percona Server| 5.7 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.7.18-16
milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.7.18-16>`_

New Feature
===========

.. Do we mention that x64 is the only supported platform.

|Percona Server| is now available on Debian 9 (stretch). The support only covers
the ``amd64`` architecture.

|Percona Server| can now be built with support of SSL 1.1.

|TokuDB| enables to kill a query that is awaiting an FT locktree lock.

|TokuDB| eables using the ``MySQL DEBUG_SYNC`` facility within |Percona FT|.

Bugs Fixed
==========

Row counts in |TokuDB| could be lost intermittently after restarts. Bug fixed
:tdbbug:`2`.

In |TokuDB|, two races in the fractal tree lock manager could significantly
affect transactional throughput for some applications that used a small number
of concurrent transactions.  These races manifested as transactions
unnecessarily waiting for an available lock. Bug fixed :tdbbug:`3`.

|Percona FT| could assert when opening a dictionary with no useful information
to error log. Bug fixed :tdbbug:`23`.

|Percona FT| could assert for various reasons deserializing nodes with no useful
error output. Bug fixed :tdbbug:`24`.

It was not possible to build |Percona Server| on Debian 9 (stretch) due to
issues with openssl1.1. Bug fixed :bug:`1702903`.

Packaging was using the ``dpkg --verify`` command which is not available on
wheezy/precise. Bug fixed :bug:`1694907`.

With two client connections to a server (debug server build), the server could
crash after one of the clients set the global option ``userstat`` and flushed
the client statistics (``FLUSH CLIENT_STATICTICS``) and then both clients were
closed. Bug fixed :bug:`1661488`.

|Percona FT| did not pass cmake flags on to snappy cmake. Bug fixed
:tdbbug:`41`.  The progress status for partitioned TokuDB table ALTERs was
misleading. Bug fixed :tdbbug:`42`.

When connecting to the Aurora cluster end point using SSL, SAN (Subject
Alternative Name) certificates were ignored.  Bug fixed :bug:`1673656` (upstream
:mysqlbug:`68052`).

Killing a stored procedure which contained a query with a view resulted in a
debug assertion failure. Bug fixed :bug:`1689736` (upstream :mysqlbug:`86260`).

The ``SET STATEMENT .. FOR`` statement did not work after ``SET GLOBAL`` or
``SHOW GLOBAL STATUS`` commands and affected the global value. Bug fixed
:bug:`1385352`.

When running ``SHOW ENGINE INNODB STATUS``, the ``Buffer pool size, bytes``
entry contained **0**. BUg fixed :bug:`1586262`.
     
The synchronization between the LRU manager and page cleaner threads was not
done at shutdown. Bug fixed :bug:`1689552`.

Spurious ``lock_wait_timeout_thread wakeup`` in ``lock_wait_suspend_thread()``
could occur. Bug fixed :bug:`1704267` (upstream :mysqlbug:`72123`).

Other bugs fixed:
:bug:`1686603`,
:tdbbug:`6`,
:tdbbug:`44`,
:tdbbug:`65`,
:bug:`1704056`,     
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


