.. rn:: 5.7.23-23

========================
Percona Server 5.7.23-23
========================

Percona is glad to announce the release of Percona Server 5.7.23-23 on
September 12, 2018. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.23-23/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.23
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-23.html>`_
and includes all the bug fixes in it. |Percona Server| 5.7.23-23 is
now the current GA (Generally Available) release in the 5.7 series.

All software developed by Percona is open-source and free.

New Features
============

* The :variable:`max_binlog_files` variable is deprecated and replaced with
  the :variable:`binlog_space_limit` variable. The behavior of
  :variable:`binlog_space_limit` is consistent with the variable
  :variable:`relay-log-space-limit` used for relay logs; both variables have the
  same semantics. For more information, see :psbug:`275`.
* Starting with 5.7.23-23, it is possible to encrypt all data in the InnoDB
  system tablespace and in the parallel double write buffer. A new variable
  :variable:`innodb_sys_tablespace_encrypt` is introduced to encrypt the system
  tablespace. This feature is considered **ALPHA** quality. The encryption of
  the parallel double write buffer file is controlled by the variable
  :variable:`innodb_parallel_dblwr_encrypt`. Both variables are ``OFF`` by
  default. For more information, see :psbug:`3822`.
* Changing ``rocksdb_update_cf_options`` returns any warnings and errors to the
  client instead of printing them to the server error log. For more information,
  see :psbug:`4258`.
* :variable:`rocksdb_number_stat_computers` and 
  :variable:`rocksdb_rate_limit_delay_millis` variables have been removed. For
  more information, see :psbug:`4780`.
* A number of new variables were introduced for MyRocks: 
  :variable:`rocksdb_rows_filtered` to show the number of rows filtered out for
  TTL in MyRocks tables, :variable:`rocksdb_bulk_load_allow_sk` to allow adding
  secondary keys using the bulk loading feature,
  :variable:`rocksdb_error_on_suboptimal_collation` toggling warning or error
  in case of an index creation on a char field where the table has a sub-optimal
  collation, :variable:`rocksdb_stats_recalc_rate` specifying the number of
  indexes to recalculate per second,
  :variable:`rocksdb_commit_time_batch_for_recovery` toggler of writing the
  commit time write batch into the database,
  and :variable:`rocksdb_write_policy` specifying when two-phase commit data are
  actually written into the database.
	
Bugs Fixed
==========

* The statement ``SELECT...ORDER BY`` produced inconsistent results with the
  ``euckr`` charset or ``euckr_bin`` collation. Bug fixed :psbug:`4513`
  (upstream :mysqlbug:`91091`).
* InnoDB statistics could incorrectly report zeros in the :ref:`slow query log
  <slow_extended>`. Bug fixed :psbug:`3828`.
* With the FIPS mode enabled and performance_schema=off, the instance crashed
  when running the ``CREATE VIEW`` command. Bug fixed :psbug:`3840`.
* The soft limit of the core file size was set incorrectly starting with PS
  :rn:`5.7.21-20`. Bug fixed :psbug:`4479`.
* The option ``innodb-optimize-keys`` could fail when a dumped table has two
  columns such that the name of one of them contains the other as as a prefix and
  is defined with the AUTO_INCREMENT attribute. Bug fixed :psbug:`4524`.
* When :variable:`innodb_temp_tablespace_encrypt` was set to ``ON`` the ``CREATE TABLE``
  command could ignore the value of the ``ENCRYPTION`` option. Bug fixed
  :psbug:`4565`.
* If ``FLUSH STATUS`` was run from a different session, a statement could be
  counted twice in ``GLOBAL STATUS``. Bug fixed :psbug:`4570` (upstream
  :mysqlbug:`91541`).
* In some cases, it was not possible to set the :variable:`flush_caches`
  variable on systems that use systemd. Bug fixed :psbug:`3796`.
* A message in the MyRocks log file did not clearly inform whether fast CRC32
  was supported. Bug fixed :psbug:`3988`.
* ``mysqld`` could not be started on Ubuntu if the database recovery had taken
  longer than ten minutes. Bug fixed :psbug:`4546` (upstream :mysqlbug:`91423`).
* The ALTER TABLE command was slow when the number of dirty pages was high. Bug fixed
  :psbug:`3702`.
* Setting the global variable :variable:`version_suffix` to NULL could
  lead to a server crash. Bug fixed :psbug:`4785`.
* When more space was added to the data partition after the error that the disk
  partition was full, MyRocks could ignore data update commands. Bug fixed
  :psbug:`4706`.

Other Bugs Fixed
================

* :psbug:`4620` \"Enable encryption of temporary tablespace from foreground thread\"
* :psbug:`4727` \"intrinsic temp table behaviour shouldn\'t depend on innodb_encrypt_tables\"
* :psbug:`4046` \"Ship assert failure: \'res == 0\' (bulk loader)\"
* :psbug:`3851` \"Percona Ver 5.6.39-83.1 Failing assertion: sym_node->table != NULL\"
* :psbug:`4533` \"audit_log MTR tests should refer to include files without parent directories\"
* :psbug:`4619` \"main.flush_read_lock fails with timeout in wait_condition.inc.\" 
* :psbug:`4561` \"Read after free at Binlog_crypt_data::load_latest_binlog_key()\"
* :psbug:`4587` \"ROCKSDB_INCLUDE_RFR macro in wrong file\"
  
.. 5.7.23-23 replace:: 5.7.23-23
