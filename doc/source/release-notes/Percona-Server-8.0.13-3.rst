.. _8.0.13-3:

================================================================================
*Percona Server for MySQL* 8.0.13-3
================================================================================

|Percona| announces the GA release of |Percona Server| |release| on |date|
(downloads are available `here
<https://www.percona.com/downloads/Percona-Server-8.0/>`__ and from the `Percona
SoftwareRepositories
<https://www.percona.com/doc/percona-server/8.0/installation.html#installing-from-binaries>`__).
This release merges changes of |MySQL| 8.0.13, including all the bug fixes in
it. |Percona Server| |release| is now the current GA release in the 8.0
series. All of |Percona|’s software is open-source and free.

Percona Server for MySQL 8.0 includes all the `features available in MySQL 8.0
Community Edition
<https://dev.mysql.com/doc/refman/8.0/en/mysql-nutshell.html>`__ in addition to
enterprise-grade features developed by Percona.  For a list of
highlighted features from both MySQL 8.0 and Percona Server for MySQL 8.0,
please see the `GA release announcement
<https://www.percona.com/blog/2018/12/21/announcing-general-availability-of-percona-server-for-mysql-8-0/>`__.

.. note::

   If you are upgrading from 5.7 to 8.0, please ensure that you read the
   `upgrade guide
   <https://www.percona.com/doc/percona-server/8.0/upgrading_guide.html>`__ and the
   document `Changed in Percona Server for MySQL 8.0
   <https://www.percona.com/doc/percona-server/8.0/changed_in_version.html>`__.

Features Removed in Percona Server for MySQL 8.0
================================================================================

- Slow Query Log Rotation and Expiration: Not widely used, can be accomplished
  using ``logrotate``
- CSV engine mode for standard-compliant quote and comma parsing
- Expanded program option modifiers
- The ``ALL_O_DIRECT`` InnoDB flush method: it is not compatible with the
  new redo logging implementation
- ``XTRADB_RSEG`` table from ``INFORMATION_SCHEMA``
- InnoDB memory size information from ``SHOW ENGINE INNODB STATUS;`` the
  same information is available from Performance Schema memory summary
  tables
- Query cache enhancements: The query cache is no longer present in
  MySQL 8.0

Features Being Deprecated in Percona Server for MySQL 8.0
================================================================================

- |TokuDB| Storage Engine: |TokuDB| will be supported throughout the |Percona
  Server| 8.0 release series, but will not be available in the next major
  release. |Percona| encourages |TokuDB| users to explore the |MyRocks| Storage
  Engine which provides similar benefits for the majority of workloads and has
  better optimized support for modern hardware.

Issues Resolved in |Percona Server| |release|
================================================================================

Improvements
--------------------------------------------------------------------------------

-  :psbug:`5014`: Update Percona Backup Locks feature to use the new ``BACKUP_ADMIN``
   privilege in MySQL 8.0
-  :psbug:`4805`: Re-Implemented Compressed Columns with Dictionaries feature in PS 8.0
-  :psbug:`4790`: Improved accuracy of User Statistics feature

Bugs Fixed Since 8.0.12-rc1
--------------------------------------------------------------------------------

-  Fixed a crash in ``mysqldump`` in the ``--innodb-optimize-keys``
   functionality :psbug:`4972`
-  Fixed a crash that can occur when system tables are locked by the
   user due to a ``lock_wait_timeout`` :psbug:`5134`
-  Fixed a crash that can occur when system tables are locked by the
   user from a ``SELECT FOR UPDATE`` statement :psbug:`5027`
-  Fixed a bug that caused ``innodb_buffer_pool_size`` to be
   uninitialized after a restart if it was set using ``SET PERSIST`` :psbug:`5069`
-  Fixed a crash in TokuDB that can occur when a temporary table
   experiences an autoincrement rollover :psbug:`5056`
-  Fixed a bug where marking an index as invisible would cause a table
   rebuild in TokuDB and also in MyRocks :psbug:`5031`
-  Fixed a bug where audit logs could get corrupted if the
   ``audit_log_rotations`` was changed during runtime. :psbug:`4950`
-  Fixed a bug where ``LOCK INSTANCE FOR BACKUP`` and
   ``STOP SLAVE SQL_THREAD`` would cause replication to be blocked and
   unable to be restarted. :psbug:`4758` (Upstream :mysqlbug:`93649`)

Other Bugs Fixed:

:psbug:`5155`, :psbug:`5139`, :psbug:`5057`, :psbug:`5049`, :psbug:`4999`, :psbug:`4971`,
:psbug:`4943`, :psbug:`4918`, :psbug:`4917`, :psbug:`4898`, and :psbug:`4744`.

Known Issues
================================================================================

We have a few features and issues outstanding that should be resolved in the
next release.

Pending Feature Re-Implementations and Improvements
--------------------------------------------------------------------------------

-  :psbug:`4892`: Re-Implement Expanded Fast Index Creation feature.
-  :psbug:`5216`: Re-Implement Utility User feature.
-  :psbug:`5143`: Identify Percona features which can make use of dynamic privileges instead of ``SUPER``

Notable Issues in Features
--------------------------------------------------------------------------------

-  :psbug:`5148`: Regression in Compressed Columns Feature when using ``innodb-force-recovery``
-  :psbug:`4996`: Regression in User Statistics feature where ``TOTAL_CONNECTIONS`` field report incorrect data
-  :psbug:`4933`: Regression in  Slow Query Logging Extensions feature where incorrect transaction idaccounting can cause an assert during certain DDLs.
-  :psbug:`5206`: TokuDB: A crash can occur in TokuDB when using Native Partioning and the optimizer has ``index_merge_union`` enabled. Workaround by using ``SET SESSION optimizer_switch="index_merge_union=off";``
-  :psbug:`5174`: MyRocks: Attempting to use unsupported features against MyRocks can lead to a crash rather than an error.
-  :psbug:`5024`: MyRocks: Queries can return the wrong results on tables with no primary key, non-unique ``CHAR``/``VARCHAR`` rows, and ``UTF8MB4`` charset.
-  :psbug:`5045`: MyRocks: Altering a column or table comment cause the table to be rebuilt

Find the release notes for Percona Server for MySQL 8.0.13-3 in our online documentation. Report bugs in the Jira bug tracker.

.. |release| replace:: 8.0.13-3
.. |date| replace:: December 21, 2018
		       
