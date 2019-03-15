.. rn:: 8.0.15-5

================================================================================
|Percona Server| |release|
================================================================================

|Percona| announces the release of |Percona Server| |release| on |date|
(downloads are available `here
<https://www.percona.com/downloads/Percona-Server-8.0/>`__ and from the `Percona
Software Repositories
<https://www.percona.com/doc/percona-server/8.0/installation.html#installing-from-binaries>`__).

This release includes fixes to bugs found in previous releases of |Percona
Server| 8.0.

.. admonition:: Incompatible changes

   In previous releases, the audit log used to produce time stamps
   inconsistent with the ISO 8601 standard. Release |release| of
   |Percona Server| solves this problem. This change, however, may
   break programs that rely on the old time stamp format.

.. include:: ../.res/text/encrypt_binlog.removing.txt

This release is based on |MySQL| 8.0.14 and 8.0.15. It includes all
bug fixes in these releases. |Percona Server| :rn:`8.0.14` was skipped.

|Percona Server| |release| is now the current GA release in the 8.0
series. All of |Percona|’s software is open-source and free.

Percona Server for MySQL 8.0 includes all the `features available in MySQL 8.0
Community Edition
<https://dev.mysql.com/doc/refman/8.0/en/mysql-nutshell.html>`__ in addition to
enterprise-grade features developed by Percona.  For a list of highlighted
features from both MySQL 8.0 and Percona Server for MySQL 8.0, please see the
`GA release announcement
<https://www.percona.com/blog/2018/12/21/announcing-general-availability-of-percona-server-for-mysql-8-0/>`__.

.. note::

   If you are upgrading from 5.7 to 8.0, please ensure that you read the
   `upgrade guide
   <https://www.percona.com/doc/percona-server/8.0/upgrading_guide.html>`__ and
   the document `Changed in Percona Server for MySQL 8.0
   <https://www.percona.com/doc/percona-server/8.0/changed_in_version.html>`__.

Bugs Fixed
================================================================================

- The audit log produced time stamps inconsistent with the ISO8601 standard. Bug
  fixed :psbug:`226`.
- FLUSH commands written to the binary log could cause errors in case of
  replication. Bug fixed :psbug:`1827` (upstream :mysqlbug:`88720`).
- When `audit_plugin` was enabled, the server could use a lot of memory when
  handling large queries.  Bug fixed :psbug:`5395`.
- The page cleaner could sleep for long time when the system clock was adjusted
  to an earlier point in time. Bug fixed :psbug:`5221` (upstream :mysqlbug:`93708`).
- In some cases, the MyRocks storage engine could crash without triggering
  the crash recovery. Bug fixed :psbug:`5366`.
- In some cases, when it failed to read from a file, InnoDB did not inform the
  name of the file in the related error message. Bug fixed :psbug:`2455`
  (upstream :mysqlbug:`76020`).
- The ``ACCESS_DENIED`` field of the ``information_schema.user_statistics``
  table was not updated correctly. Bugs fixed :psbug:`3956`, :psbug:`4996`.
- ``MyRocks`` could crash while running ``START TRANSACTION WITH
  CONSISTENT SNAPSHOT`` if other transactions were in specific states. Bug fixed
  :psbug:`4705`.
- In some cases, the server using the the ``MyRocks`` storage engine could
  crash when TTL (Time to Live) was defined on a table. Bug fixed :psbug:`4911`.
- MyRocks incorrectly processed transactions in which multiple statements
  had to be rolled back. Bug fixed :psbug:`5219`.
- A stack buffer overrun could happen if the redo log encryption with
  key rotation was enabled. Bug fixed :psbug:`5305`.
- The TokuDB storage engine would assert on load when used with jemalloc 5.x. Bug fixed :psbug:`5406`.

Other bugs fixed:
:psbug:`4106`,
:psbug:`4107`,
:psbug:`4108`,
:psbug:`4121`,
:psbug:`4474`,
:psbug:`4640`,
:psbug:`5055`,
:psbug:`5218`,
:psbug:`5263`,
:psbug:`5328`,
:psbug:`5369`.


.. |release| replace:: 8.0.15-5
.. |date| replace:: March 15, 2019
