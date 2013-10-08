.. rn:: 5.1.62-13.3

=============================
 |Percona Server| 5.1.62-13.3
=============================

Percona is glad to announce the release of |Percona Server| 5.1.62-13.3 on April 24th, 2012 (Downloads are available from `Percona Server 5.1.62-13.3 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.62-13.3/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.1.62 <http://dev.mysql.com/doc/refman/5.1/en/news-5-1-62.html>`_, including all the bug fixes in it, |Percona Server| 5.1.62-13.3 is now the current stable release in the 5.1 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.62-13.3 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.62-13.3>`_.

This release doesn't hold the bug fix for MySQL bug `#48848 <http://bugs.mysql.com/bug.php?id=48848>`_ for 32-bit Percona Server release. 

New Features
============

New option :variable:`rewrite-db` has been added to the mysqlbinlog utility that allows the changing names of the used databases in both Row-Based and Statement-Based replication. This was possible before by using tools like grep, awk and sed but only for SBR, because with RBR database name is encoded within the BINLOG '....' statement.

Bug Fixes
=========

 * Dependency issues for libmysqlclient*-dev package(s) on Debian. Bug fixed :bug:`656933` (*Ignacio Nin*).

 * Fixed MySQL bug `#64469 <http://bugs.mysql.com/bug.php?id=64469>`_ that caused MySQL server hanging (sometimes crashing) when a TRUNCATE TABLE is done together with selecting data over same table from information_schema. Bug fixed :bug:`903617` (*Sergei Glushchenko*).

 * Fixed MySQL bug `#49336 <http://bugs.mysql.com/bug.php?id=49336>`_, mysqlbinlog couldn't handle stdin when "|" used. Bug fixed :bug:`933969` (*Sergei Glushchenko*).

 * Fixed MySQL bug `#64127 <http://bugs.mysql.com/bug.php?id=64127>`_, "mysql-test-run.pl" didn't expect InnoDB errors and warnings in the format "InnoDB: ERROR" and "InnoDB: WARNING" with all-uppercase "ERROR" and "WARNING". Bug fixed :bug:`937859` (*Laurynas Biveinis*).

 * Dependency issue while installing libmysqlclient15-dev on Ubuntu systems. Bug fixed :bug:`803151` (*Ignacio Nin*).

 * Removed auto-generated file in 5.1 branch. Bug fixed :bug:`917290`  (*Ignacio Nin*).

 * MySQL 5.0 does not flag BEGIN/SAVEPOINT/COMMIT/ROLLBACK statements in its binlogs with LOG_EVENT_SUPPRESS_USE_F like 5.1+ does. This causes unnecessary `use` statements around such statements when the binlog is dumped by mysqlbinlog. Fixed by always suppressing the output of `use` for these statements. Bug fixed :bug:`929521` (*Oleg Tsarev*).
