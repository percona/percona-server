.. _upstream_bug_fixes:

=============================================================
 List of upstream |MySQL| bugs fixed in |Percona Server| 5.7
=============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`86260` - Assert on KILL'ing a stored routine invocation                            |
|:Launchpad bug: :bug:`1689736`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`86209` - audit plugin + MB collation connection + PREPARE stmt parse error crash...|
|:Launchpad bug: :bug:`1688698`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`86016` - Make MTR show core dump stacktraces from unit tests too                   |
|:Launchpad bug: :bug:`1684601`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85838` - rpl_diff.inc in 5.7 does not compare data from different servers          |
|:Launchpad bug: :bug:`1680510`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85835` - server crash n-gram full text searching                                   |
|:Launchpad bug: :bug:`1679025`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.18-15`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85678` - field-t deletes Fake_TABLE objects through base TABLE pointer w/o ...     |
|:Launchpad bug: :bug:`1677130`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: 5.7.19                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85671` - segfault-t failing under recent AddressSanitizer                          |
|:Launchpad bug: :bug:`1676847`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85258` - DROP TEMPORARY TABLE creates a transaction in binary log on read only...  |
|:Launchpad bug: :bug:`1668602`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85141` - Write/fsync amplification w/ duplicate GTIDs                              |
|:Launchpad bug: :bug:`1669928`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84736` - 5.7 range optimizer crash                                                 |
|:Launchpad bug: :bug:`1660591`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84437` - super-read-only does not allow FLUSH LOGS on 5.7                          |
|:Launchpad bug: :bug:`1654682`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84415` - slave don't report Seconds_Behind_Master when running ...                 |
|:Launchpad bug: :bug:`1654091`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84366` - InnoDB index dives do not detect concurrent tree changes, return bogus... |
|:Launchpad bug: :bug:`1625151`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.17-11`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84350` - Error 1290 executing flush logs in read-only slave                        |
|:Launchpad bug: :bug:`1652852`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83814` - Add support for OpenSSL 1.1                                               |
|:Launchpad bug: :bug:`1702903`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83648` - Assertion failure in thread x in file fts0que.cc line 3659                |
|:Launchpad bug: :bug:`1634932`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83124` - Bug 81657 fix merge to 5.6 broken                                         |
|:Launchpad bug: :bug:`1626936`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: 5.7.17                                                                                        | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83073` - GCC 5 and 6 miscompile mach_parse_compressed                              |
|:Launchpad bug: :bug:`1626002`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: 5.7.17                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83003` - Using temporary tables on slaves increases GTID sequence number           |
|:Launchpad bug: :bug:`1539504`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.17-11`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82980` - Multi-threaded slave leaks worker threads in case of thread create ...    |
|:Launchpad bug: :bug:`1619622`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82935` - Cipher ECDHE-RSA-AES128-GCM-SHA256 listed in man/Ssl_cipher_list, not...  |
|:Launchpad bug: :bug:`1622034`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82886` - Server may crash due to a glibc bug in handling short-lived detached ...  |
|:Launchpad bug: :bug:`1621012`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: 5.7.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82307` - Memory leaks in unit tests                                                |
|:Launchpad bug: :bug:`1604774`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream fix: 5.7.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82283` - main.mysqlbinlog_debug fails with a LeakSanitizer error                   |
|:Launchpad bug: :bug:`1604462`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream fix: 5.7.19                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82026` - Stack buffer overflow with --ssl-cipher=<more than 4K characters>         |
|:Launchpad bug: :bug:`1596845`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82019` - Is client library supposed to retry EINTR indefinitely or not             |
|:Launchpad bug: :bug:`1591202`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream fix: 5.7.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81814` - InnoDB adaptive hash index uses a bad partitioning algorithm for the ...  |
|:Launchpad bug: :bug:`1679155`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81810` - Inconsistent sort order for blob/text between InnoDB and filesort         |
|:Launchpad bug: :bug:`1674867`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81714` - mysqldump get_view_structure does not free MYSQL_RES in one error path    |
|:Launchpad bug: :bug:`1588845`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.13-6`                                                                                |
|:Upstream fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81675` - mysqlbinlog does not free the existing connection before opening new ...  |
|:Launchpad bug: :bug:`1587840`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-6`                                                                                |
|:Upstream fix: 5.7.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81657` - DBUG_PRINT in THD::decide_logging_format prints incorrectly, access ...   |
|:Launchpad bug: :bug:`1587426`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-6`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81467` - innodb_fts.sync_block test unstable due to slow query log nondeterminism  |
|:Launchpad bug: :bug:`1662163`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80962` - Replication does not work when @@GLOBAL.SERVER_UUID is missing on the...  |
|:Launchpad bug: :bug:`1566642`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream fix: 5.7.13                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80607` - main.log_tables-big unstable on loaded hosts                              |
|:Launchpad bug: :bug:`1554043`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: 5.7.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80606` - my_write, my_pwrite no longer safe to call from THD-less server utility...|
|:Launchpad bug: :bug:`1552682`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80496` - buf_dblwr_init_or_load_pages now returns an error code, but caller not... |
|:Launchpad bug: :bug:`1549301`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80288` - missing innodb_numa_interleave                                            |
|:Launchpad bug: :bug:`1561091`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream fix: 5.7.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80053` - Assertion in binlog coordinator on slave with 2 2pc handler log_slave ... |
|:Launchpad bug: :bug:`1534249`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79894` - Page cleaner worker threads are not instrumented for performance schema   |
|:Launchpad bug: :bug:`1532747`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79703` - Spin rounds per wait will be negative in InnoDB status if spin waits >... |
|:Launchpad bug: :bug:`1527160`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79610` - Failed DROP DATABASE due FK constraint on master breaks slave             |
|:Launchpad bug: :bug:`1525407`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
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
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
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
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77399` - Deadlocks missed by INFORMATION_SCHEMA.INNODB_METRICS lock_deadlocks ...  |
|:Launchpad bug: :bug:`1466414`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`76418` - Server crashes when querying partitioning table MySQL_5.7.14              |
|:Launchpad bug: :bug:`1657941`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.18-15`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`76142` - InnoDB tablespace import fails when importing table w/ different data ... |
|:Launchpad bug: :bug:`1548597`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.13-6`                                                                                |
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
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75480` - Selecting wrong pos with SHOW BINLOG EVENTS causes a potentially ...      |
|:Launchpad bug: :bug:`1409652`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75311` - Error for SSL cipher is unhelpful                                         |
|:Launchpad bug: :bug:`1660339`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75189` - engines suite tests depending on InnoDB implementation details            |
|:Launchpad bug: :bug:`1401776`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74637` - make dirty page flushing more adaptive                                    |
|:Launchpad bug: :ref:`Multi-threaded asynchronous LRU flusher <lru_manager_threads>`                         |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-3`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73418` - Add --manual-lldb option to mysql-test-run.pl                             |
|:Launchpad bug: :bug:`1328482`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72615` - MTR --mysqld=--default-storage-engine=foo incompatible w/ dynamically...  |
|:Launchpad bug: :bug:`1318537`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72475` - Binlog events with binlog_format=MIXED are unconditionally logged in ...  |
|:Launchpad bug: :bug:`1313901`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72466` - More memory overhead per page in the InnoDB buffer pool                   |
|:Launchpad bug: :bug:`1536693`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72123` - Spurious lock_wait_timeout_thread wakeup in lock_wait_suspend_thread()    |
|:Launchpad bug: :bug:`1704267`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72108` - Hard to read history file                                                 |
|:Launchpad bug: :bug:`1296192`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71761` - ANALYZE TABLE should remove its table from background stat processing...  |
|:Launchpad bug: :bug:`1626441`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71759` - memory leak with string thread variable that set memalloc flag            |
|:Launchpad bug: :bug:`1620152`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71411` - buf_flush_LRU() does not return correct number in case of compressed ...  |
|:Launchpad bug: :bug:`1262651`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71270` - Failures to end bulk insert for partitioned tables handled incorrectly    |
|:Launchpad bug: :bug:`1204871`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71217` - Threadpool - add thd_wait_begin/thd_wait_end to the network IO functions  |
|:Launchpad bug: :bug:`1159743`                                                                               |
|:Upstream state: Open (checked on 2017-08-31)                                                                |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71183` - os_file_fsync() should handle fsync() returning EINTR                     |
|:Launchpad bug: :bug:`1262651`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71091` - CSV engine does not properly process "", in quotes                        |
|:Launchpad bug: :bug:`1316042`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70500` - Page cleaner should perform LRU flushing regardless of server activity    |
|:Launchpad bug: :bug:`1234562`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70490` - Suppression is too strict on some systems                                 |
|:Launchpad bug: :bug:`1205196`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69991` - MySQL client is broken without readline                                   |
|:Launchpad bug: :bug:`1266386`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69639` - mysql failed to build with dtrace Sun D 1.11                              |
|:Launchpad bug: :bug:`1196460`                                                                               |
|:Upstream state: Open (checked on 2017-08-31)                                                                |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69258` - does buf_LRU_buf_pool_running_out need to lock buffer pool mutexes        |
|:Launchpad bug: :bug:`1219842`                                                                               |
|:Upstream state: Not a Bug                                                                                   |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69232` - buf_dblwr->mutex can be splited into two                                  |
|:Launchpad bug: :ref:`parallel_doublewrite_buffer`                                                           |
|:Upstream state: Need Feedback (checked on 2017-08-31)                                                       |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69170` - buf_flush_LRU is lazy                                                     |
|:Launchpad bug: :bug:`1231918`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69146` - Needless log flush order mutex acquisition in buf_pool_get_oldest_mod...  |
|:Launchpad bug: :bug:`1176496`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68714` - Remove literal statement digest values from perfschema tests              |
|:Launchpad bug: :bug:`1157078`                                                                               |
|:Upstream state: Not a Bug                                                                                   |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68481` - InnoDB LRU flushing for MySQL 5.6 needs work                              |
|:Launchpad bug: :bug:`1232406`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68052` - SSL Certificate Subject ALT Names with IPs not respected with --ssl-ver...|
|:Launchpad bug: :bug:`1673656`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67808` - in innodb engine, double write and multi-buffer pool instance reduce ...  |
|:Launchpad bug: :ref:`parallel_doublewrite_buffer`                                                           |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63130` - CMake-based check for the presence of a system readline library is not... |
|:Launchpad bug: :bug:`1266386`                                                                               |
|:Upstream state: Can't Repeat (checked on 2017-08-31)                                                        |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`57583` - fast index create not used during "alter table foo engine=innodb"         |
|:Launchpad bug: :bug:`1451351`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53645` - SHOW GRANTS not displaying all the applicable grants                      |
|:Launchpad bug: :bug:`1354988`                                                                               |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49120` - mysqldump should have flag to delay creating indexes for innodb plugin... |
|:Launchpad bug: :bug:`744103`                                                                                |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`42415` - UPDATE/DELETE with LIMIT clause unsafe for SBL even with ORDER BY PK ...  |
|:Launchpad bug: N/A                                                                                          |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`39833` - CREATE INDEX does full table copy on TEMPORARY table                      |
|:Launchpad bug: N/A                                                                                          |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`35125` - Allow the ability to set the server_id for a connection for logging to... |
|:Launchpad BP: `Blueprint <https://blueprints.launchpad.net/percona-server/+spec/per-session-server-id>`_    |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`25007` - memory tables with dynamic rows format                                    |
|:Launchpad BP: :bug:`1148822`                                                                                |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`20001` - Support for temp-tables in INFORMATION_SCHEMA                             |
|:Launchpad bug: :ref:`temp_tables`                                                                           |
|:Upstream state: Verified (checked on 2017-08-31)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+

