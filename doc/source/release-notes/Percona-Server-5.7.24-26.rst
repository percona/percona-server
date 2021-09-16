.. rn:: 5.7.23-26

================================================================================
Percona Server 5.7.24-26
================================================================================

Percona announces the release of `Percona Server for MySQL
<https://www.percona.com/software/percona-server>`_ 5.7.24-26 on December 4,
2018 (downloads are available `here
<https://www.percona.com/downloads/Percona-Server-5.7/>`_ and from the `Percona
Software Repositories
<https://www.percona.com/doc/percona-server/5.7/installation.html#installing-from-binaries>`_).
This release merges changes of `MySQL 5.7.24
<https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-24.html>`_, including
all the bug fixes in it. Percona Server for MySQL 5.7.24-26 is now the current
GA release in the 5.7 series. All of Percona’s software is open-source and free.

This release includes fixes to the following upstream CVEs (Common
Vulnerabilities and Exposures): *CVE-2016-9843*, *CVE-2018-3155*, *CVE-2018-3143*,
*CVE-2018-3156*, *CVE-2018-3251*, *CVE-2018-3133*, *CVE-2018-3144*, *CVE-2018-3185*,
*CVE-2018-3247*, *CVE-2018-3187*, *CVE-2018-3174*, *CVE-2018-3171*. For more
information, see `Oracle Critical Patch Update Advisory - October 2018
<https://www.oracle.com/technetwork/security-advisory/cpuoct2018-4428296.html>`.

Improvements
================================================================================

- :psbug:`4790`: Improve user statistics accuracy

Bugs Fixed
================================================================================

- Slave replication could break if upstream bug #74145 (FLUSH LOGS improperly
  disables the logging if the log file cannot be accessed) occurred in
  master. Bug fixed :psbug:`1017` (Upstream :mysqlbug:`83232`).
- Setting the :variable:`tokudb_last_lock_timeout` variable via the command line
  could cause the server to stop working when the actual timeout took place. Bug
  fixed :psbug:`4943`.
- Dropping TokuDB table with non-alphanumeric characters could lead to a
  crash. Bug fixed :psbug:`4979`.
- When using MyRocks storage engine, the server could crash after running
  ``ALTER TABLE DROP INDEX`` on a slave. Bug fixed :psbug:`4744`.
- The audit log could be corrupted when the ``audit_log_rotations`` variable was
  changed at runtime. Bug fixed :psbug:`4950`.

.. rubric:: Other Bugs Fixed

- :psbug:`4781`: sql_yacc.yy uses SQLCOM_SELECT instead of SQLCOM_SHOW_XXXX_STATS
- :psbug:`4881`: Add LLVM/clang 7 to Travis-CI
- :psbug:`4825`: Backport MTR fixes from 8.0
- :psbug:`4998`: Valgrind: compilation fails with: writing to ‘struct buf_buddy_free_t’ with no trivial copy-assignment
- :psbug:`4980`: Valgrind: Syscall param write(buf) points to uninitialised byte(s): Event_encrypter::encrypt_and_write()
- :psbug:`4982`: Valgrind: Syscall param io_submit(PWRITE) points to uninitialised byte(s): buf_dblwr_write_block_to_datafile()
- :psbug:`4983`: Valgrind: Syscall param io_submit(PWRITE) points to uninitialised byte(s): buf_flush_write_block_low()
- :psbug:`4951`: Many libc-related Valgrind errors on CentOS7
- :psbug:`5012`: Valgrind: misused UNIV_MEM_ALLOC after ut_zalloc_nokey
- :psbug:`4908`: UBSan and valgrind errors with encrypted temporary files
- :psbug:`4532`: Replace obsolete HAVE_purify with HAVE_VALGRIND in ha_rocksdb.cc
- :psbug:`4955`: Backport mysqld fixes for valgrind warnings from 8.0
- :psbug:`4529`: MTR: index_merge_rocksdb2 inadvertently tests InnoDB instead of MyRocks
- :psbug:`5056`: handle_fatal_signal (sig=11) in ha_tokudb::write_row
- :psbug:`5084`: innodb_buffer_pool_size is an uninitialized variable
- :psbug:`4836`: Missing PFS signed variable aggregation
- :psbug:`5033`: rocksdb.show_engine: Result content mismatch
- :psbug:`5034`: rocksdb.rocksdb: Result content mismatch
- :psbug:`5035`: rocksdb.show_table_status: 1051: Unknown table 'db_new'

.. 5.7.24-26 replace:: 5.7.24-26
