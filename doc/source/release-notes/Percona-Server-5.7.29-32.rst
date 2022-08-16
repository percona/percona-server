.. _5.7.29-32:

================================================================================
*Percona Server for MySQL* 5.7.29-32
================================================================================

:Date: February 5, 2020

:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.7/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.29-32
includes all the features and bug fixes available in
`MySQL 5.7.29 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-29.html>`_
in addition to enterprise-grade features developed by Percona.

Bugs Fixed
================================================================================

* :jirabug:`PS-1469`: The Memory storage engine detected an incorrect "is full" condition when the space contained reusable memory chunks that could be reused.
* :jirabug:`PS-6113`: If ANALYZE TABLE with persistent statistics ran more than 600 seconds the execution of a diagnostic query may cause a server exit. (Upstream :mysqlbug:`97828`)
* :jirabug:`PS-5813`: To set the :variable:`slow_query_log_use_global_control` to "none" could cause an error.
* :jirabug:`PS-6150`: The execution of SHOW ENGINE INNODB STATUS to show locked mutexes could cause a server exit.
* :jirabug:`PS-6750`: The installation of client packages could cause a file conflict in Red Hat Enterprise Linux 8.
* :jirabug:`PS-5940`: When a temporary table was dropped, the server exited. (Upstream :mysqlbug:`96766`)
* :jirabug:`PS-5675`: Concurrent INSERT ... ON DUPLICATE KEY UPDATE statements could cause a failure with a unique index violation. (Upstream :mysqlbug:`96578`)
* :jirabug:`PS-5421`: MyRocks: Corrected documentation for :variable:`rocksdb_db_write_buffer_size`.
* :jirabug:`PS-4794`: Documented that using ps-admin to enable MyRocks does not disable Transparent Huge Pages.
* :jirabug:`PS-6093`: The execution of SHOW ENGINE INNODB STATUS to show locked mutexes with simultaneous access to a compressed table could cause a server exit.
* :jirabug:`PS-6148`: If ANALYZE TABLE with transient statistics ran more than 600 seconds the execution of a diagnostic query may cause a server exit. (Upstream :mysqlbug:`97828`)
* :jirabug:`PS-6125`: MyRocks: To set :variable:`rocksdb_update_cf_options` with a non-existant column family created a partially-defined column family which could cause a server exit.
* :jirabug:`PS-6123`: A Debian/Ubuntu init script used an incorrect comparison which could cause the service command to return before the server start.
* :jirabug:`PS-5956`: Root session could kill :ref:`psaas_utility_user` session.
* :jirabug:`PS-5952`: :ref:`psaas_utility_user` was visible in performance_schema.threads.
* :jirabug:`PS-5843`: A memory leak could occur after "group_replication.gr_majority_loss_restart". (Upstream :mysqlbug:`96471`)
* :jirabug:`PS-5325`: Conditional jump or move depended on uninitialized value on innodb_zip.wl5522_zip or innodb.alter_missing_tablespace.


