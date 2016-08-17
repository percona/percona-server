.. rn:: 5.5.51-38.1

==============================
 |Percona Server| 5.5.51-38.1
==============================

Percona is glad to announce the release of |Percona Server| 5.5.51-38.1 on
August 19th, 2016. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.51-38.1/>`_
and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.51
<http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-51.html>`_, including
all the bug fixes in it, |Percona Server| 5.5.51-38.1 is now the current stable
release in the 5.5 series. All of |Percona|'s software is open-source and free,
all the details of the release can be found in the `5.5.51-38.1 milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.5.51-38.1>`_.

Bugs Fixed
==========

 :ref:`pam_plugin` would abort authentication while checking UNIX user group
 membership if there were more than a thousand members. Bug fixed
 :bug:`1608902`.

 :ref:`pam_plugin` didn't support spaces in the UNIX user group names. Bug
 fixed :bug:`1544443`.

 If ``DROP DATABASE`` would fail to delete some of the tables in the database,
 the partially-executed command is logged in the binlog as ``DROP TABLE t1, t2,
 ...``  for the tables for which drop succeeded. A slave might fail to
 replicate such ``DROP TABLE`` statement if there exist foreign key
 relationships to any of the dropped tables and the slave has a different
 schema from master. Fix by checking, on the master, whether any of the
 database to be dropped tables participate in a Foreign Key relationship, and
 fail the ``DROP DATABASE`` statement immediately. Bug fixed :bug:`1525407`
 (upstream :mysqlbug:`79610`).

 |Percona Server| 5.5 could not be built with the
 ``-DMYSQL_MAINTAINER_MODE=ON`` option. Bug fixed :bug:`1590454`.

 In the client library, any EINTR received during network I/O was not handled
 correctly. Bug fixed :bug:`1591202` (upstream :mysqlbug:`82019`).

 The included :file:`.gitignore` in the percona-server source distribution had
 a line ``*.spec``, which means someone trying to check in a copy of the
 percona-server source would be missing the spec file required to build the
 RPMs. Bug fixed :bug:`1600051`.

 The fix for bug :bug:`1341067` added a call to free some of the heap memory
 allocated by OpenSSL. This is not safe for repeated calls if OpenSSL is
 linked twice through different libraries and each is trying to free the same.
 Bug fixed :bug:`1604676`.

 If the changed page bitmap redo log tracking thread stops due to any reason,
 then shutdown will wait for a long time for the log tracker thread to quit,
 which it never does. Bug fixed :bug:`1606821`.

 Performing slow |InnoDB| shutdown (:variable:`innodb_fast_shutdown` set to
 ``0``) could result in incomplete purge, if a separate purge thread is running
 (which is a default in |Percona Server|). Bug fixed :bug:`1609364`.

Other bugs fixed: :bug:`1515591` (upstream :mysqlbug:`79249`), :bug:`1612551`,
:bug:`1609523`, :bug:`756387`, :bug:`1097870`, :bug:`1603073`, :bug:`1606478`,
:bug:`1606572`, :bug:`1606782`, :bug:`1607224`, :bug:`1607359`, :bug:`1607606`,
:bug:`1607607`, :bug:`1607671`, :bug:`1608385`, :bug:`1608424`, :bug:`1608437`,
:bug:`1608515`, :bug:`1608845`, :bug:`1609422`, :bug:`1610858`, :bug:`1612084`,
:bug:`1612118`, and :bug:`1613641`.
