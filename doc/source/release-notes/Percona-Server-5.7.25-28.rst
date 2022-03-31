.. rn:: 5.7.25-28

================================================================================
|Percona Server| 5.7.25-28
================================================================================

Percona is glad to announce the release of |Percona Server| 5.7.25-28 on
February 18, 2019. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.25-28/>`_
and from the :doc:`Percona Software Repositories </installation>`.
	
This release is based on `MySQL 5.7.25
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-25.html>`_ and includes
all the bug fixes in it. |Percona Server| 5.7.25-28 is now the current GA
(Generally Available) release in the 5.7 series.
	
All software developed by Percona is open-source and free.

In this release, |Percona Server| introduces the variable
:variable:`binlog_skip_flush_commands`. This variable controls whether
or not ``FLUSH`` commands are written to the binary log. Setting this
variable to **ON** can help avoid problems in replication. For more
information, see :ref:`percona-server.binary-log.flush.writing`.

.. note:: 

   If you're currently using |Percona Server| 5.7, Percona recommends upgrading
   to this version of 5.7 prior to upgrading to |Percona Server| 8.0.

Bugs Fixed
================================================================================

- FLUSH commands written to the binary log could cause errors in case
  of replication. Bug fixed :psbug:`1827`: (upstream :mysql:`88720`).
- Running LOCK TABLES FOR BACKUP followed by STOP SLAVE SQL_THREAD
  could block replication preventing it from being restarted
  normally. Bug fixed :psbug:`4758`.
- The ``ACCESS_DENIED`` field of the
  information_schema.user_statistics table was not updated
  correctly. Bug fixed :psbug:`3956`.
- MySQL could report that the maximum number of connections was
  exceeded with too many connections being in the CLOSE_WAIT state. Bug
  fixed :psbug:`4716` (upstream :mysqlbug:`92108`)
- Wrong query results could be received in semi-join sub queries with
  materialization-scan that allowed inner tables of different
  semi-join nests to interleave. Bug fixed :psbug:`4907` (upstream bug
  :mysqlbug:`92809`).
- In some cases, the server using the MyRocks storage engine could crash
  when TTL (Time to Live) was defined on a table. Bug fixed :psbug:`4911`.
- Running the SELECT statement with the ORDER BY and
  LIMIT clauses could result in a less than optimal performance. Bug
  fixed :psbug:`4949` (upstream :mysqlbug:`92850`)
- There was a typo in ``mysqld_safe.sh``: **trottling** was replaced
  with **throttling**. Bug fixed :psbug:`240`. Thanks to Michael
  Coburn for the patch.
- MyRocks could crash while running ``START TRANSACTION WITH
  CONSISTENT SNAPSHOT`` if other transactions were in specific
  states. Bug fixed :psbug:`4705`.
- In some cases, ``mysqld`` could crash when inserting data into a
  database the name of which contained special characters (CVE-2018-20324). Bug fixed
  :psbug:`5158`.
- MyRocks incorrectly processed transactions in which multiple
  statements had to be rolled back.  Bug fixed :psbug:`5219`.
- In some cases, the MyRocks storage engine could crash without triggering the
  crash recovery. Bug fixed :psbug:`5366`.
- When bootstrapped with undo or redo log encryption enabled on a very fast
  storage, the server could fail to start. Bug fixed :psbug:`4958`.
- Some fields in the output of ``SHOW USER_STATISTICS`` command did
  not contain correct information. Bug fixed :psbug:`4996`.

Other bugs fixed:
:psbug:`2455`,
:psbug:`4791`, 
:psbug:`4855`,
:psbug:`5268`.

This release also contains fixes for the following CVE issues:
CVE-2019-2534,
CVE-2019-2529,
CVE-2019-2482,
CVE-2019-2434.

.. February 18, 2019 replace:: February 18, 2019
.. 5.7.25-28 replace:: 5.7.25-28
