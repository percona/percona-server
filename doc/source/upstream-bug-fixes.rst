.. _upstream_bug_fixes:

=============================================================
 List of upstream |MySQL| bugs fixed in |Percona Server| 5.7
=============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80962` - Replication does not work when @@GLOBAL.SERVER_UUID is missing on the...  |
|:Launchpad bug: :bug:`1566642`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80607` - main.log_tables-big unstable on loaded hosts                              |
|:Launchpad bug: :bug:`1554043`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80606` - my_write, my_pwrite no longer safe to call from THD-less server utility...|
|:Launchpad bug: :bug:`1552682`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80496` - buf_dblwr_init_or_load_pages now returns an error code, but caller not... |
|:Launchpad bug: :bug:`1549301`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80288` - missing innodb_numa_interleave                                            |
|:Launchpad bug: :bug:`1561091`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80053` - Assertion in binlog coordinator on slave with 2 2pc handler log_slave ... |
|:Launchpad bug: :bug:`1534249`                                                                               |
|:Upstream state: Open (checked on 2016-06-03)                                                                |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79894` - Page cleaner worker threads are not instrumented for performance schema   |
|:Launchpad bug: :bug:`1532747`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79703` - Spin rounds per wait will be negative in InnoDB status if spin waits >... |
|:Launchpad bug: :bug:`1527160`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79569` - Some --big-test tests were forgotten to update in 5.7.10                  |
|:Launchpad bug: :bug:`1525109`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: 5.7.11                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79117` - "change_user" command should be aware of preceding "error" command        |
|:Launchpad bug: :bug:`1172090`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: 5.7.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`78894` - buf_pool_resize can lock less in checking whether AHI is on or off        |
|:Launchpad bug: :bug:`1525215`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77684` - DROP TABLE IF EXISTS may brake replication if slave has replication ...   |
|:Launchpad bug: :bug:`1475107`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: 5.7.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77591` - ALTER TABLE does not allow to change NULL/NOT NULL if foreign key exists  |
|:Launchpad bug: :bug:`1466414`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77399` - Deadlocks missed by INFORMATION_SCHEMA.INNODB_METRICS lock_deadlocks ...  |
|:Launchpad bug: :bug:`1466414`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75534` - Solve buffer pool mutex contention by splitting it                        |
|:Launchpad bug: :ref:`innodb_split_buf_pool_mutex`                                                           |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75504` - btr_search_guess_on_hash makes found block young twice?                   |
|:Launchpad bug: :bug:`1411694`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75480` - Selecting wrong pos with SHOW BINLOG EVENTS causes a potentially ...      |
|:Launchpad bug: :bug:`1409652`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74637` - make dirty page flushing more adaptive                                    |
|:Launchpad bug: :ref:`Multi-threaded asynchronous LRU flusher <lru_manager_threads>`                         |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-3`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73418` - Add --manual-lldb option to mysql-test-run.pl                             |
|:Launchpad bug: :bug:`1328482`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72466` - More memory overhead per page in the InnoDB buffer pool                   |
|:Launchpad bug: :bug:`1536693`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72108` - Hard to read history file                                                 |
|:Launchpad bug: :bug:`1296192`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71411` - buf_flush_LRU() does not return correct number in case of compressed ...  |
|:Launchpad bug: :bug:`1262651`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71183` - os_file_fsync() should handle fsync() returning EINTR                     |
|:Launchpad bug: :bug:`1262651`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70500` - Page cleaner should perform LRU flushing regardless of server activity    |
|:Launchpad bug: :bug:`1234562`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70490` - Suppression is too strict on some systems                                 |
|:Launchpad bug: :bug:`1205196`                                                                               |
|:Upstream state: Open (checked on 2016-06-03)                                                                |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69991` - MySQL client is broken without readline                                   |
|:Launchpad bug: :bug:`1266386`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69232` - buf_dblwr->mutex can be splited into two                                  |
|:Launchpad bug: :ref:`parallel_doublewrite_buffer`                                                           |
|:Upstream state: Open (checked on 2016-06-03)                                                                |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69146` - Needless log flush order mutex acquisition in buf_pool_get_oldest_mod...  |
|:Launchpad bug: :bug:`1176496`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68714` - Remove literal statement digest values from perfschema tests              |
|:Launchpad bug: :bug:`1157078`                                                                               |
|:Upstream state: Not a Bug                                                                                   |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67808` - in innodb engine, double write and multi-buffer pool instance reduce ...  |
|:Launchpad bug: :ref:`parallel_doublewrite_buffer`                                                           |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63130` - CMake-based check for the presence of a system readline library is not... |
|:Launchpad bug: :bug:`1266386`                                                                               |
|:Upstream state: Can't Repeat (checked on 2016-06-03)                                                        |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`57583` - fast index create not used during "alter table foo engine=innodb"         |
|:Launchpad bug: :bug:`1451351`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53645` - SHOW GRANTS not displaying all the applicable grants                      |
|:Launchpad bug: :bug:`1354988`                                                                               |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49120` - mysqldump should have flag to delay creating indexes for innodb plugin... |
|:Launchpad bug: :bug:`744103`                                                                                |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`39833` - CREATE INDEX does full table copy on TEMPORARY table                      |
|:Launchpad bug: N/A                                                                                          |
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`35125` - Allow the ability to set the server_id for a connection for logging to... |
|:Launchpad BP: `Blueprint <https://blueprints.launchpad.net/percona-server/+spec/per-session-server-id>`_    |                                                                               
|:Upstream state: Verified (checked on 2016-06-03)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+


