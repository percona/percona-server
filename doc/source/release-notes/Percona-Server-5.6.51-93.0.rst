.. _PS-5.6.51-93.0:

================================================================================
*Percona Server for MySQL* 5.6.51-93.0
================================================================================

:Date: December 8, 2022

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.6.51-93.0
includes all the features and bug fixes available in
`MySQL 5.6.51 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-51.html>`_
in addition to enterprise-grade features developed by Percona. 


Bugs Fixed
================================================================================

* :jirabug:`PS-7856`: A server exit when an auto-incremented column was updated in a partitioned table.

* `Bug #28142052 <https://github.com/percona/percona-server/pull/4881/commits/f7b16daaeb604bab0ca25db9beaf713e074b8d43>`__: A `THD::m_query_string` could be allocated on the wrong MEM_ROOT, which allowed the prepared statement to make the string invalid by deleting `Prepared_statement::main_mem_root`.

* `Bug #31933415 <https://github.com/percona/percona-server/pull/4881/commits/a9deb747b31c71fb94afb390d908073817d96d05>`__: Fixed an out of bounds read in ER().

* `Bug#32803050 <https://github.com/percona/percona-server/pull/4426/commits/44728b4063f8bffb4ff0ad287184cab3d73f16cf>`__: When an Update or Delete was executed with indexes on a table with a row-based replication setup and a federated table on the replica, the replica exited.

* :jirabug:`PS-8129`: Fixed a mutex hang in `threadpool_unix`.

* :jirabug:`PS-7563`: Fixed a Read buffer overflow in the `DBUG_PRINT` macro in mysqltest.

* :jirabug:`PS-7769`: Fixed a use-after-return error in `audit_log_exclude_account_validate`.

* :jirabug:`PS-8204`: The wrong length was specified for some XML escape rules and the terminating null symbol from the replacement rule was copied into the resulting string. This operation caused text truncation in the audit log file. 



