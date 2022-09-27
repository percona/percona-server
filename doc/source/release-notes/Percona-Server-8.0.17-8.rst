.. _8.0.17-8:

================================================================================
*Percona Server for MySQL* 8.0.17-8
================================================================================

|Percona| announces the release of |Percona Server| |release| on |date|
(downloads are available `here
<https://www.percona.com/downloads/Percona-Server-8.0/>`__ and from the
`Percona Software Repositories
<https://www.percona.com/doc/percona-server/8.0/installation.html#installing-from-binaries>`__).

This release includes fixes to bugs found in previous releases of |Percona
Server| 8.0.

|Percona Server| |release| is now the current GA release in the 8.0 series. All
of |Percona|’s software is open-source and free.

Percona Server for MySQL 8.0 includes all the `features available in MySQL
8.0.17 Community Edition
<https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-17.html>`__ in
addition to enterprise-grade features developed by Percona.

New Features
================================================================================

|Percona Server| has implemented the ability to have a |MySQL|
:ref:`psaas_utility_user` who has system access to do administrative tasks but limited
access to user schemas. The user is invisible to other users. This feature is
especially useful to those who are operating |MySQL| as a Service. This feature
has the same functionality as the utility user in earlier versions and has been
delay-ported to version 8.0.

|Percona Server| has implemented `data masking <https://www.percona.com/doc/percona-server/8.0/security/data-masking.html>`__ . 

Bugs Fixed
================================================================================

- Changed the default of :ref:`innodb_empty_free_list_algorithm` to
  ``backoff``. Bugs fixed :psbug:`5881`

- When the Adaptive Hash Index (AHI) was enabled or disabled, there was an AHI
  overhead during DDL operations. Bugs fixed :psbug:`5747`.

- An upgrade to :ref:`8.0.16-7` with encrypted tablespace fails on
  :ref:`innodb_dynamic_metadata`. Bugs fixed :psbug:`5874`.

- The ``rocksdb.ttl_primary`` test case sometimes fails. Bugs fixed
  :psbug:`5722` (Louis Hust)

- The ``rocksdb.ns_snapshot_read_committed`` test case sometimes fails. Bugs
  fixed :psbug:`5798` (Louis Hust).

- During a binlogging replication event, if the master crashes after the
  multi-threaded slave has begun copying to the slave's relay log and before the
  process has completed, a ``STOP SLAVE`` on the slave takes longer than expected.
  Bugs fixed :psbug:`5824`.

- The purpose of the `sql_require_primary_key
  <https://dev.mysql.com/doc/refman/8.0/en/server-system-variables.html#sysvar_sql_require_primary_key>`__
  option is to avoid replication performance issues. Temporary tables are not
  replicated. The option cannot be used with temporary tables. Bugs fixed
  :psbug:`5931`.

- When using ``skip-innodb_doublewrite`` in my.cnf, a parallel doublewrite
  buffer is still created. Bugs fixed :psbug:`3411`.

- The metadata for every InnoDB table contains encryption information, either a
  'Y' or an 'N' value based on the ENCRYPTION clause or the
  :ref:`default_table_encryption` value. You are unable to switch the storage
  engine from InnoDB to MyRocks because MyRocks does not support the ENCRYPTION
  clause. Bugs fixed :psbug:`5865`.

- MyRocks does not allow index condition pushdown optimization for specific data
  types, such as ``varchar``.  Bugs fixed :psbug:`5024`.

Other bugs fixed: :psbug:`5880`, :psbug:`5838`, :psbug:`5682`,
:psbug:`5979`, :psbug:`5793`, :psbug:`6020`, :psbug:`5327`,
:psbug:`5839`, :psbug:`5933`, :psbug:`5939`, :psbug:`5659`, :psbug:`5924`,
:psbug:`5926`, :psbug:`5925`, :psbug:`5875`, :psbug:`5533`,
:psbug:`5867`, :psbug:`5864`, :psbug:`5760`, :psbug:`5909`, :psbug:`5985`,
:psbug:`5941`, :psbug:`5954`, :psbug:`5790`, and :psbug:`5593`.

.. |release| replace:: 8.0.17-8
.. |date| replace:: October 30, 2019
