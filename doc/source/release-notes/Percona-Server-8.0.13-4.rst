.. _8.0.13-4:

================================================================================
*Percona Server for MySQL* 8.0.13-4
================================================================================

|Percona| announces the release of |Percona Server| |release| on
|date| (downloads are available `here
<https://www.percona.com/downloads/Percona-Server-8.0/>`__ and from
the `Percona Software Repositories
<https://www.percona.com/doc/percona-server/8.0/installation.html#installing-from-binaries>`__).
This release contains a fix for a critical bug that prevented |Percona
Server| 5.7.24-26 from being upgraded to version 8.0.13-3 if there
were more than around 1000 tables, or if the maximum allocated InnoDB
table ID was around 1000. |Percona Server| |release| is now the
current GA release in the 8.0 series. All of |Percona|’s software is
open-source and free.

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


Bugs Fixed
================================================================================

- It was not possible to upgrade from MySQL 5.7.24-26 to 8.0.13-3 if
  there were more than around 1000 tables, or if the maximum allocated
  InnoDB table ID was around 1000. Bug fixed
  :psbug:`5245`.
- ``SHOW BINLOG EVENTS FROM <bad offset>`` is not diagnosed inside
  ``Format_description_log_events``. Bug fixed :psbug:`5126` (Upstream
  :mysqlbug:`93544`).
- There was a typo in `mysqld_safe.sh`: **trottling** was replaced with
  **throttling**. Bug fixed :psbug:`240`. Thanks to Michael Coburn for the patch.
- |Percona Server| 8.0 could crash with the "Assertion failure:
  dict0dict.cc:7451:space_id != SPACE_UNKNOWN" exception during an
  upgrade from |Percona Server| 5.7.23 to |Percona Server| 8.0.13-3
  with ``--innodb_file_per_table=OFF``. Bug fixed :psbug:`5222`.
- On Debian or Ubuntu, a conflict was reported on the
  :file:`/usr/bin/innochecksum` file when attempting to install |Percona Server| 8
  over the |MySQL| 8. Bug fixed :psbug:`5225`.
- An out-of-bound read exception could occur on debug builds in the compressed
  columns with dictionaries feature. Bug fixed :psbug:`5311`:.
- The ``innodb_data_pending_reads`` server status variable contained an
  incorrect value. Bug fixed :psbug:`5264`:. Thanks to Fangxin Lou for the patch.
- A memory leak and needless allocation in compression dictionaries
   could happen in ``mysqldump``. Bug fixed :psbug:`5307`.
- A compression-related memory leak could happen in ``mysqlbinlog``. Bug
  fixed :psbug:`5308`:.

Other bugs fixed: :psbug:`4797`:, :psbug:`5209`, :psbug:`5268`,
:psbug:`5270`:, :psbug:`5306`, :psbug:`5309`:

.. |release| replace:: 8.0.13-4
.. |date| replace:: January 17, 2019
