.. _upstream_bug_fixes:

=============================================================
 List of upstream |MySQL| bugs fixed in |Percona Server| 5.7
=============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`90264` - Some file operations in mf_iocache2.c are not instrumented                |
|:JIRA bug: :psbug:`3937`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.21-21`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`90238` - Comparison of uninitailized memory in log_in_use                          |
|:JIRA bug: :psbug:`3925`                                                                                     |
|:Upstream state: No Feedback (checked on 2018-05-24)                                                         |
|:Fix Released: :rn:`5.7.21-21`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`89916` - hp_test1/hp_test2 are not built unless WITH_EMBEDDED_SERVER is defined    |
|:JIRA bug: :psbug:`3845`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.21-21`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`89822` - InnoDB retries open on EINTR error only if innodb_use_native_aio is ...   |
|:JIRA bug: :psbug:`3843`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.21-21`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`89646` - Clang warnings in 5.7.21                                                  |
|:JIRA bug: :psbug:`3814`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.21-21`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`89598` - plugin_mecab.cc:54:19: warning: unused variable 'bundle_mecab'            |
|:JIRA bug: :psbug:`3804`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.21-20`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`89422` - Dangerous enum-ulong casts in sql_formatter_options                       |
|:JIRA bug: :psbug:`3780`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.21-20`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`89421` - Missing mutex_unlock in Slave_reporting_capability::va_report             |
|:JIRA bug: :psbug:`3780`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.21-20`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`89420` - Enforcing C++03 mode in non debug builds                                  |
|:JIRA bug: :psbug:`3780`                                                                                     |
|:Upstream state: Open (checked on 2018-05-24)                                                                |
|:Fix Released: :rn:`5.7.21-20`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`89205` - gap locks on READ COMMITTED cause by page split                           |
|:JIRA bug: :psbug:`1130`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.22-22`                                                                               |
|:Upstream fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`88057` - Intermediary slave does not log master changes with...                    |
|:JIRA bug: :psbug:`1119`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.20-19`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`87065` - Release lock on table statistics after query plan created                 |
|:JIRA bug: :psbug:`2503`                                                                                     |
|:Upstream state: Open (checked on 2018-05-24)                                                                |
|:Fix Released: :rn:`5.7.20-18`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`86260` - Assert on KILL'ing a stored routine invocation                            |
|:JIRA bug: :psbug:`1091`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream fix: 5.7.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`86209` - audit plugin + MB collation connection + PREPARE stmt parse error crash...|
|:JIRA bug: :psbug:`1089`                                                                                     |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`86164` - Fulltext search can not find word which contains punctuation marks        |
|:JIRA bug: :psbug:`2501`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.21-20`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`86016` - Make MTR show core dump stacktraces from unit tests too                   |
|:JIRA bug: :psbug:`2499`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85838` - rpl_diff.inc in 5.7 does not compare data from different servers          |
|:JIRA bug: :psbug:`2257`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85835` - server crash n-gram full text searching                                   |
|:JIRA bug: :psbug:`237`                                                                                      |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.18-15`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85678` - field-t deletes Fake_TABLE objects through base TABLE pointer w/o ...     |
|:JIRA bug: :psbug:`2253`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: 5.7.19                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85671` - segfault-t failing under recent AddressSanitizer                          |
|:JIRA bug: :psbug:`2252`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85258` - DROP TEMPORARY TABLE creates a transaction in binary log on read only...  |
|:JIRA bug: :psbug:`1785`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85158` - heartbeats/fakerotate cause a forced sync_master_info                     |
|:JIRA bug: :psbug:`1812`                                                                                     |
|:Upstream state: Open (checked on 2018-05-24)                                                                |
|:Fix Released: :rn:`5.7.20-19`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85141` - Write/fsync amplification w/ duplicate GTIDs                              |
|:JIRA bug: :psbug:`1786`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84736` - 5.7 range optimizer crash                                                 |
|:JIRA bug: :psbug:`1055`                                                                                     |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84437` - super-read-only does not allow FLUSH LOGS on 5.7                          |
|:JIRA bug: :psbug:`1772`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: 5.7.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84420` - stopwords and ngram indexes                                               |
|:JIRA bug: :psbug:`1802`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.20-18`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84415` - slave don't report Seconds_Behind_Master when running ...                 |
|:JIRA bug: :psbug:`1770`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: 5.7.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84366` - InnoDB index dives do not detect concurrent tree changes, return bogus... |
|:JIRA bug: :psbug:`1089`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.17-11`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84350` - Error 1290 executing flush logs in read-only slave                        |
|:JIRA bug: :psbug:`1044`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: 5.7.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83814` - Add support for OpenSSL 1.1                                               |
|:JIRA bug: :psbug:`1105`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83648` - Assertion failure in thread x in file fts0que.cc line 3659                |
|:JIRA bug: :psbug:`1023`                                                                                     |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83124` - Bug 81657 fix merge to 5.6 broken                                         |
|:JIRA bug: :psbug:`1750`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: 5.7.17                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83073` - GCC 5 and 6 miscompile mach_parse_compressed                              |
|:JIRA bug: :psbug:`1745`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: 5.7.17                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83003` - Using temporary tables on slaves increases GTID sequence number           |
|:JIRA bug: :psbug:`964`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.17-11`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82980` - Multi-threaded slave leaks worker threads in case of thread create ...    |
|:JIRA bug: :psbug:`2193`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82935` - Cipher ECDHE-RSA-AES128-GCM-SHA256 listed in man/Ssl_cipher_list, not...  |
|:JIRA bug: :psbug:`1737`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82886` - Server may crash due to a glibc bug in handling short-lived detached ...  |
|:JIRA bug: :psbug:`1006`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: 5.7.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82307` - Memory leaks in unit tests                                                |
|:JIRA bug: :psbug:`2157`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream fix: 5.7.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82283` - main.mysqlbinlog_debug fails with a LeakSanitizer error                   |
|:JIRA bug: :psbug:`2156`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream fix: 5.7.19                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82026` - Stack buffer overflow with --ssl-cipher=<more than 4K characters>         |
|:JIRA bug: :psbug:`2155`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82019` - Is client library supposed to retry EINTR indefinitely or not             |
|:JIRA bug: :psbug:`1720`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream fix: 5.7.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81814` - InnoDB adaptive hash index uses a bad partitioning algorithm for the ...  |
|:JIRA bug: :psbug:`2498`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81810` - Inconsistent sort order for blob/text between InnoDB and filesort         |
|:JIRA bug: :psbug:`1799`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81714` - mysqldump get_view_structure does not free MYSQL_RES in one error path    |
|:JIRA bug: :psbug:`2152`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.13-6`                                                                                |
|:Upstream fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81675` - mysqlbinlog does not free the existing connection before opening new ...  |
|:JIRA bug: :psbug:`1718`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-6`                                                                                |
|:Upstream fix: 5.7.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81657` - DBUG_PRINT in THD::decide_logging_format prints incorrectly, access ...   |
|:JIRA bug: :psbug:`2150`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-6`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81467` - innodb_fts.sync_block test unstable due to slow query log nondeterminism  |
|:JIRA bug: :psbug:`2232`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80962` - Replication does not work when @@GLOBAL.SERVER_UUID is missing on the...  |
|:JIRA bug: :psbug:`1684`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream fix: 5.7.13                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80607` - main.log_tables-big unstable on loaded hosts                              |
|:JIRA bug: :psbug:`2141`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: 5.7.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80606` - my_write, my_pwrite no longer safe to call from THD-less server utility...|
|:JIRA bug: :psbug:`970`                                                                                      |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80496` - buf_dblwr_init_or_load_pages now returns an error code, but caller not... |
|:JIRA bug: :psbug:`3384`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80288` - missing innodb_numa_interleave                                            |
|:JIRA bug: :psbug:`974`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream fix: 5.7.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80053` - Assertion in binlog coordinator on slave with 2 2pc handler log_slave ... |
|:JIRA bug: :psbug:`3361`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79894` - Page cleaner worker threads are not instrumented for performance schema   |
|:JIRA bug: :psbug:`3356`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79703` - Spin rounds per wait will be negative in InnoDB status if spin waits >... |
|:JIRA bug: :psbug:`1684`                                                                                     |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79610` - Failed DROP DATABASE due FK constraint on master breaks slave             |
|:JIRA bug: :psbug:`1683`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79569` - Some --big-test tests were forgotten to update in 5.7.10                  |
|:JIRA bug: :psbug:`3339`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream fix: 5.7.11                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79117` - "change_user" command should be aware of preceding "error" command        |
|:JIRA bug: :psbug:`659`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: 5.7.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`78894` - buf_pool_resize can lock less in checking whether AHI is on or off        |
|:JIRA bug: :psbug:`3340`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77684` - DROP TABLE IF EXISTS may brake replication if slave has replication ...   |
|:JIRA bug: :psbug:`1639`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: 5.7.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77591` - ALTER TABLE does not allow to change NULL/NOT NULL if foreign key exists  |
|:JIRA bug: :psbug:`1635`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77399` - Deadlocks missed by INFORMATION_SCHEMA.INNODB_METRICS lock_deadlocks ...  |
|:JIRA bug: :psbug:`1635`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`76418` - Server crashes when querying partitioning table MySQL_5.7.14              |
|:JIRA bug: :psbug:`1050`                                                                                     |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.18-15`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`76142` - InnoDB tablespace import fails when importing table w/ different data ... |
|:JIRA bug: :psbug:`1697`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.13-6`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75534` - Solve buffer pool mutex contention by splitting it                        |
|:JIRA bug: :ref:`innodb_split_buf_pool_mutex`                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75504` - btr_search_guess_on_hash makes found block young twice?                   |
|:JIRA bug: :psbug:`2454`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75480` - Selecting wrong pos with SHOW BINLOG EVENTS causes a potentially ...      |
|:JIRA bug: :psbug:`1600`                                                                                     |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75311` - Error for SSL cipher is unhelpful                                         |
|:JIRA bug: :psbug:`1779`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75189` - engines suite tests depending on InnoDB implementation details            |
|:JIRA bug: :psbug:`2103`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74637` - make dirty page flushing more adaptive                                    |
|:JIRA bug: :ref:`Multi-threaded asynchronous LRU flusher <lru_manager_threads>`                              |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-3`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73418` - Add --manual-lldb option to mysql-test-run.pl                             |
|:JIRA bug: :psbug:`2448`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72615` - MTR --mysqld=--default-storage-engine=foo incompatible w/ dynamically...  |
|:JIRA bug: :psbug:`2071`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72475` - Binlog events with binlog_format=MIXED are unconditionally logged in ...  |
|:JIRA bug: :psbug:`151`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72466` - More memory overhead per page in the InnoDB buffer pool                   |
|:JIRA bug: :psbug:`1689`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72123` - Spurious lock_wait_timeout_thread wakeup in lock_wait_suspend_thread()    |
|:JIRA bug: :psbug:`2504`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72108` - Hard to read history file                                                 |
|:JIRA bug: :psbug:`2066`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71761` - ANALYZE TABLE should remove its table from background stat processing...  |
|:JIRA bug: :psbug:`1749`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71759` - memory leak with string thread variable that set memalloc flag            |
|:JIRA bug: :psbug:`1004`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71411` - buf_flush_LRU() does not return correct number in case of compressed ...  |
|:JIRA bug: :psbug:`1461`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71270` - Failures to end bulk insert for partitioned tables handled incorrectly    |
|:JIRA bug: :psbug:`700`                                                                                      |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71217` - Threadpool - add thd_wait_begin/thd_wait_end to the network IO functions  |
|:JIRA bug: :psbug:`1343`                                                                                     |
|:Upstream state: Open (checked on 2018-05-24)                                                                |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71183` - os_file_fsync() should handle fsync() returning EINTR                     |
|:JIRA bug: :psbug:`1461`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71091` - CSV engine does not properly process "", in quotes                        |
|:JIRA bug: :psbug:`153`                                                                                      |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70500` - Page cleaner should perform LRU flushing regardless of server activity    |
|:JIRA bug: :psbug:`1428`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70490` - Suppression is too strict on some systems                                 |
|:JIRA bug: :psbug:`2038`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69991` - MySQL client is broken without readline                                   |
|:JIRA bug: :psbug:`1467`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69639` - mysql failed to build with dtrace Sun D 1.11                              |
|:JIRA bug: :psbug:`1392`                                                                                     |
|:Upstream state: Unsupported                                                                                 |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69258` - does buf_LRU_buf_pool_running_out need to lock buffer pool mutexes        |
|:JIRA bug: :psbug:`1414`                                                                                     |
|:Upstream state: Not a Bug                                                                                   |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69232` - buf_dblwr->mutex can be splited into two                                  |
|:JIRA bug: :ref:`parallel_doublewrite_buffer`                                                                |
|:Upstream state: No Feedback (checked on 2018-05-24)                                                         |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69170` - buf_flush_LRU is lazy                                                     |
|:JIRA bug: :psbug:`2430`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69146` - Needless log flush order mutex acquisition in buf_pool_get_oldest_mod...  |
|:JIRA bug: :psbug:`2418`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68714` - Remove literal statement digest values from perfschema tests              |
|:JIRA bug: :psbug:`1340`                                                                                     |
|:Upstream state: Not a Bug                                                                                   |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68481` - InnoDB LRU flushing for MySQL 5.6 needs work                              |
|:JIRA bug: :psbug:`2432`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68052` - SSL Certificate Subject ALT Names with IPs not respected with --ssl-ver...|
|:JIRA bug: :psbug:`1076`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream fix: N/A                                                                                           | 
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67808` - in innodb engine, double write and multi-buffer pool instance reduce ...  |
|:JIRA bug: :ref:`parallel_doublewrite_buffer`                                                                |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63130` - CMake-based check for the presence of a system readline library is not... |
|:JIRA bug: :psbug:`1467`                                                                                     |
|:Upstream state: Can't Repeat (checked on 2018-05-24)                                                        |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`57583` - fast index create not used during "alter table foo engine=innodb"         |
|:JIRA bug: :psbug:`2113`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53645` - SHOW GRANTS not displaying all the applicable grants                      |
|:JIRA bug: :psbug:`191`                                                                                      |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53588` - Blackhole : Specified key was too long; max key length is 1000 bytes      |
|:JIRA bug: :psbug:`1126`                                                                                     |
|:Upstream state: No Feedback (checked on 2018-05-24)                                                         |
|:Fix Released: :rn:`5.7.20-19`                                                                               |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49120` - mysqldump should have flag to delay creating indexes for innodb plugin... |
|:JIRA bug: :psbug:`2619`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`42415` - UPDATE/DELETE with LIMIT clause unsafe for SBL even with ORDER BY PK ...  |
|:JIRA bug: :psbug:`44`                                                                                       |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`39833` - CREATE INDEX does full table copy on TEMPORARY table                      |
|:JIRA bug: N/A                                                                                               |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`35125` - Allow the ability to set the server_id for a connection for logging to... |
|:Launchpad BP: `Blueprint <https://blueprints.launchpad.net/percona-server/+spec/per-session-server-id>`_    |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`25007` - memory tables with dynamic rows format                                    |
|:JIRA bug: :psbug:`2407`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`20001` - Support for temp-tables in INFORMATION_SCHEMA                             |
|:JIRA bug: :ref:`temp_tables`                                                                                |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+

