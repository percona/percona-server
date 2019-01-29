.. rn:: 5.6.40-84.0

============================
|Percona Server| 5.6.40-84.0
============================

Percona is glad to announce the release of |Percona Server| 5.6.40-84.0 on
May 30, 2018 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.40-84.0/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.40
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-40.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.40-84.0 is the current GA release
in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source
and free.

New Features
============

* A new string variable :variable:`version_suffix` allows to change suffix
  for the |Percona Server| version string returned by the read-only
  :variable:`version` variable.  This allows to append the version number for
  the server with a custom suffix to reflect some build or configuration
  specifics. Also :variable:`version_comment` (default value of which is taken
  from the CMake ``COMPILATION_COMMENT`` option) is converted from a global
  read-only to a global read-write variable and thereby it is now cutomizable.

* Query response time plugin now can be disabled at session level with use
  of a new variable :variable:`query_response_time_session_stats`.

Bugs Fixed
==========

* Compilation warning was fixed for ``-DWITH_QUERY_RESPONSE_TIME=ON`` CMake
  compilation option, which makes QRT to be linked statically. Bug fixed
  :psbug:`3841`.

* A code clean-up was done to fix clang 6 specific compilation
  warnings and errors (bug fixed :psbug:`3893`, upstream :mysqlbug:`90111`).

* Using ``-DWITHOUT_<PLUGIN>=ON`` CMake variable to exclude a plugin from the
  build didn't work for some plugins, including a number of storage engines.
  Bug fixed :psbug:`3901`.

* A clean-up in |Percona Server| binlog-related code was made to avoid
  uninitialized memory comparison. Bug fixed :psbug:`3925` (upstream
  :mysqlbug:`90238`).

* Temporary file I/O was not instrumented for Performance Schema. Bug fixed
  :psbug:`3937` (upstream :mysqlbug:`90264`).

* A ``key_block_size`` value was set automatically by the Improved MEMORY
  Storage Engine, which resulted in warnings when changing the engine type to
  |InnoDB|, and constantly growing ``key_block_size`` during alter operations.
  Bugs fixed :psbug:`3936`, :psbug:`3940`, and :psbug:`3943`.

* |Percona Server| Debian packages description included reference to
  ``/etc/mysql/my.cnf`` file, which is not actually present in these packages.
  Bug fixed :psbug:`2046`.

* Fixes were introduced to remove GCC 8 compilation warnings for the
  |Percona Server| build, retaining compatibility with old compiler versions,
  including GCC 4.4. Bugs fixed :psbug:`3950` and :psbug:`4471`.

* A typo in ``plugin.cmake`` file prevented to compile plugins statically
  into the server. Bug fixed :psbug:`3871` (upstream :mysqlbug:`89766`).

* ``-DWITH_NUMA=ON`` build option was silently ignored by CMake when
  NUMA development package was not installed, instead of exiting by error.
  Bug fixed :psbug:`4487`.

* Variables :variable:`innodb_buffer_pool_populate` and
  :variable:`numa_interleave` mapped to the upstream
  `innodb_numa_interleave
  <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_numa_interleave>`_
  variable in :rn:`5.6.27-75.0` were reverted to their original
  implementation due to upstream variant being less effective in
  memory allocation. Now buffer pool is allocated with MAP_POPULATE,
  forcing NUMA interleaved allocation at the buffer pool
  initialization time. Bug fixed :psbug:`3967`.

* :variable:`audit_log_include_accounts` variable did not take effect if
  placed in ``my.cnf`` configuration file, while still working as intended if
  set dynamically. Bug fixed :psbug:`3867`.

* Synchronization between between :variable:`innodb_kill_idle_transaction` and
  :variable:`kill_idle_transaction` system variables was broken because of the
  regression in |Percona Server| :rn:`5.6.40-83.2`. Bug fixed :psbug:`3955`.

* Executing the ``SHOW GLOBAL STATUS`` expression could cause "data drift" on
  global status variables in case of a query rollback: the variable, being by
  its nature a counter and allowing only an increase, could return to its
  previous value. Bug fixed :psbug:`3951` (upstream :mysqlbug:`90351`).

* ``ALTER TABLE ... COMMENT = ...`` statement caused |TokuDB| to rebuild the
  whole table, which is not needed, as only FRM metadata should be changed. The
  fix was provided as a contribution by `Fungo Wang <https://github.com/fungo>`_.
  Bugs fixed :psbug:`4280` and :psbug:`4292`.

* A number of |Percona Server| 8.0 |TokuDB| fixes have been backported to
  |Percona Server| 5.6 in preparation for using MySQL 8.0. Bugs fixed 
  :psbug:`4379`, :psbug:`4380`, :psbug:`4387`, :psbug:`4378`, :psbug:`4383`,
  :psbug:`4384`, :psbug:`4386`,  :psbug:`4382`, :psbug:`4391`,
  :psbug:`4390`, :psbug:`4392`, and :psbug:`4381`.

TokuDB Changes and Fixes
========================

* Two new variables,   :variable:`tokudb_enable_fast_update` and
  :variable:`tokudb_enable_fast_upsert`, were introduced to facilitate the
  |TokuDB| fast updates feature, which involves queries optimization to avoid
  random reads during their execution. Bug fixed :psbug:`4365`.

* A data race was fixed in minicron utility of the PerconaFT, as a contribution
  by Rik Prohaska. Bug fixed :psbug:`4281`.

* Row count and cardinality decrease to zero took place after long-running
  ``REPLACE`` load, ending up with full table scans for any action. Bug fixed
  :psbug:`4296`.

Other Bugs Fixed
================

* :psbug:`3818` "Orphaned file mysql-test/suite/innodb/r/percona_innodb_kill_idle_trx.result"

* :psbug:`3926` "Potentially truncated bitmap file name in
  log_online_open_bitmap_file_read_only() (storage/innobase/log/log0online.cc)"

* :psbug:`2204` "Test main.audit_log_default_db is unstable"

* :psbug:`3767` "Fix compilation warnings/errors with clang"

* :psbug:`3773` "Incorrect key file for table frequently for tokudb"

* :psbug:`3794` "MTR test main.percona_show_temp_tables_stress does not wait
  for events to start"

* :psbug:`3798` "MTR test innodb.percona_extended_innodb_status fails if InnoDB
  status contains unquoted special characters"

* :psbug:`3887` "TokuDB does not compile with
  -DWITH_PERFSCHEMA_STORAGE_ENGINE=OFF"

* :psbug:`4388` "5.7 code still has TOKU_INCLUDE_OPTION_STRUCTS which is a
  MariaDB specific construct"

* :psbug:`4265` "TDB-114 (Change use of MySQL HASH to unordered_map) introduces
  memory leak"

* :psbug:`4277` "memory leaks in TDB-2 and TDB-89 tests"

* :psbug:`4276` "Data race on cache table attributes detected by the thread
  sanitizer"

* :psbug:`4451` "Implement better compression algo testing"

* :psbug:`4469` "variable use out of scope bug in get_last_key test detected by
  ASAN in clang 6"

* :psbug:`4470` "the cachetable-simple-pin-nonblocking-cheap test occasionally
  fails due to a locking conflict with the cachetable evictor"

* :psbug:`1131` "User_var_log_event::User_var_log_event(const char*, uint,
  const Format_description_log_event*): Assertion \`(bytes_read == (data_written
  - ((old_pre_checksum_fd || (description_event->checksum_alg ==
  BINLOG_CHECKSUM_ALG_OFF)) ? 0 : 4))) || ((".
