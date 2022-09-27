.. _8.0.19-10:

================================================================================
*Percona Server for MySQL* 8.0.19-10
================================================================================

:Date: March 23, 2020

:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/8.0/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 8.0.19-10
includes all the features and bug fixes available in
`MySQL 8.0.19 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-19.html>`_
in addition to enterprise-grade features developed by Percona.

New Features
================================================================================

* :jirabug:`PS-5729`: Added server's UUID to Percona system keys.
* :jirabug:`PS-5917`: Added Simplified LDAP authentication plugin.
* :jirabug:`PS-4464`: Exposed the last global transaction identifier (GTID) executed for a CONSISTENT SNAPSHOT.



Improvements
================================================================================

* :jirabug:`PS-6775`: Removed the KEYRING_ON option from :ref:`default_table_encryption`.
* :jirabug:`PS-6733`: Added binary search to the Data masking plugin.



Bugs Fixed
================================================================================

* :jirabug:`PS-6811`: Service failed to start asserting ACL_PROXY_USER::check_validity if --skip-name-resolve=1 and there is a Proxy user. (Upstream :mysqlbug:`98908`)
* :jirabug:`PS-6112`: Inconsistent Binlog_snapshot_gtid when mysqldump was used with --single-transaction.
* :jirabug:`PS-5923`: "SELECT ... INTO var_name FOR UPDATE" was not working in MySQL 8.0. (Upstream :mysqlbug:`96677`)
* :jirabug:`PS-6150`: The execution of SHOW ENGINE INNODB STATUS to show locked mutexes could cause a server exit.
* :jirabug:`PS-5379`: Slow startup after an upgrade from MySQL 5.7 to MySQL 8. (Upstream :mysqlbug:`96340`)
* :jirabug:`PS-6750`: The installation of client packages could cause a file conflict in Red Hat Enterprise Linux 8.
* :jirabug:`PS-5675`: Concurrent INSERT ... ON DUPLICATE KEY UPDATE statements could cause a failure with a unique index violation. (Upstream :mysqlbug:`96578`)
* :jirabug:`PS-6857`: New package naming broke dbdeployer.
* :jirabug:`PS-6767`: The execution of a stored function in a WHERE clause was skipped. (Upstream :mysqlbug:`98160`)
* :jirabug:`PS-5421`: MyRocks: Corrected documentation for :ref:`rocksdb_db_write_buffer_size`.
* :jirabug:`PS-6761`: MacOS error in threadpool_unix.cc: there was no matching member function for call to 'compare_exchange_weak'.
* :jirabug:`PS-6900`: The test big-test required re-recording after explicit_encryption was re-added.
* :jirabug:`PS-6897`: The main.udf_myisam test and main.transactional_acl_tables test failed on trunk.
* :jirabug:`PS-6106`: ALTER TABLE without ENCRYPTION clause caused tables to be encrypted.
* :jirabug:`PS-6093`: The execution of SHOW ENGINE INNODB STATUS to show locked mutexes with simultaneous access to a compressed table could cause a server exit.
* :jirabug:`PS-5552`: Assertion 'm_idx >= 0' failed in plan_idx QEP_share d::idx() const. (Upstream :mysqlbug:`98258`)
* :jirabug:`PS-6899`: The tests, main.events_bugs and main.events_1, failed because 2020-01-01 was considered a future time. (Upstream :mysqlbug:`98860`)
* :jirabug:`PS-6881`: Documented that mysql 8.0 does not require mysql_upgrade.
* :jirabug:`PS-6796`: The test, percona_changed_page_bmp_shutdown_thread, was unstable.
* :jirabug:`PS-6773`: A conditional jump or move depended on uninitialized value(s) in sha256_password_authenticate. (Upstream :mysqlbug:`98223`)
* :jirabug:`PS-6125`: MyRocks: To set :ref:`rocksdb_update_cf_options` with a nonexistent column family created a partially-defined column family which could cause a server exit.
* :jirabug:`PS-6037`: When Extra Packages Enterprise Linux (EPEL) 8 repo was enabled on CentOS/RHEL 8, jemalloc v5 was installed.
* :jirabug:`PS-5956`: Root session could kill :ref:`psaas_utility_user` session.
* :jirabug:`PS-5952`: :ref:`psaas_utility_user` was visible in performance_schema.threads.
* :jirabug:`PS-5843`: A memory leak could occur after "group_replication.gr_majority_loss_restart". (Upstream :mysqlbug:`96471`)
* :jirabug:`PS-5642`: The page tracker thread did not exit if the startup failed.
* :jirabug:`PS-5325`: A conditional jump or move depended on uninitialized value on innodb_zip.wl5522_zip or innodb.alter_missing_tablespace.
* :jirabug:`PS-4678`: MyRocks: Documented the generated columns limitation.
* :jirabug:`PS-4649`: TokuDB: Documented PerconaFT (fractal tree indexing).


