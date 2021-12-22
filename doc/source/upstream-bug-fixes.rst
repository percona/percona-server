.. _upstream_bug_fixes:

============================================================
List of upstream MySQL bugs fixed in |Percona Server|  5.7
============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`93917` - Wrong binlog entry for BLOB on a blackhole intermediary master            |
|:JIRA bug: :psbug:`5353`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.26-29`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`93708` - Page Cleaner will sleep for long time if clock changes                    |
|:JIRA bug: :psbug:`5221`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.26-29`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`92850` - Bad select+order by+limit performance in 5.7                              |
|:JIRA bug: :psbug:`4949`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.25-28`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`92809` - Inconsistent ResultSet for different Execution Plans                      |
|:JIRA bug: :psbug:`4907`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.25-28`                                                                               |
|:Upstream Fix: 5.7.27                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`92108` - Deadlock by concurrent show binlogs, pfs session_variables table ...      |
|:JIRA bug: :psbug:`4716`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.25-28`                                                                               |
|:Upstream Fix: 5.7.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`91541` - Flush status statement adds twice to global values                        |
|:JIRA bug: :psbug:`4570`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.23-23`                                                                               |
|:Upstream Fix: 5.7.26                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`91423` - Can\'t run mysql on Ubuntu systems with long recovery time                |
|:JIRA bug: :psbug:`4546`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.23-23`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`91091` - A simple SELECT on a table with CHARSET=euckr COLLATE=euckr_bin ...       |
|:JIRA bug: :psbug:`4513`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.23-23`                                                                               |
|:Upstream Fix: 5.7.24                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`90264` - Some file operations in mf_iocache2.c are not instrumented                |
|:JIRA bug: :psbug:`3937`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.21-21`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`90238` - Comparison of uninitailized memory in log_in_use                          |
|:JIRA bug: :psbug:`3925`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.21-21`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89916` - hp_test1/hp_test2 are not built unless WITH_EMBEDDED_SERVER is defined    |
|:JIRA bug: :psbug:`3845`                                                                                     |
|:Upstream State: Won't fix                                                                                   |
|:Fix Released: :rn:`5.7.21-21`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89822` - InnoDB retries open on EINTR error only if innodb_use_native_aio is ...   |
|:JIRA bug: :psbug:`3843`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.21-21`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89646` - Clang warnings in 5.7.21                                                  |
|:JIRA bug: :psbug:`3814`                                                                                     |
|:Upstream State: Won't fix                                                                                   |
|:Fix Released: :rn:`5.7.21-21`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89598` - plugin_mecab.cc:54:19: warning: unused variable 'bundle_mecab'            |
|:JIRA bug: :psbug:`3804`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.21-20`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89422` - Dangerous enum-ulong casts in sql_formatter_options                       |
|:JIRA bug: :psbug:`3780`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.21-20`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89421` - Missing mutex_unlock in Slave_reporting_capability::va_report             |
|:JIRA bug: :psbug:`3780`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.21-20`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89420` - Enforcing C++03 mode in non debug builds                                  |
|:JIRA bug: :psbug:`3780`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.21-20`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89205` - gap locks on READ COMMITTED cause by page split                           |
|:JIRA bug: :psbug:`1130`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.22-22`                                                                               |
|:Upstream Fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`88720` -  Inconsistent and unsafe FLUSH behavior in terms of replication           |
|:JIRA bug: :psbug:`1827`                                                                                     |
|:Upstream State: Verified (checked on 2019-02-11)                                                            |
|:Fix Released: :rn:`5.7.25-28`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`88057` - Intermediary slave does not log master changes with...                    |
|:JIRA bug: :psbug:`1119`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.20-19`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`87065` - Release lock on table statistics after query plan created                 |
|:JIRA bug: :psbug:`2503`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.20-18`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`86260` - Assert on KILL'ing a stored routine invocation                            |
|:JIRA bug: :psbug:`1091`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream Fix: 5.7.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`86209` - audit plugin + MB collation connection + PREPARE stmt parse error crash...|
|:JIRA bug: :psbug:`1089`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`86164` - Fulltext search can not find word which contains punctuation marks        |
|:JIRA bug: :psbug:`2501`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.21-20`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`86016` - Make MTR show core dump stacktraces from unit tests too                   |
|:JIRA bug: :psbug:`2499`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85838` - rpl_diff.inc in 5.7 does not compare data from different servers          |
|:JIRA bug: :psbug:`2257`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85835` - server crash n-gram full text searching                                   |
|:JIRA bug: :psbug:`237`                                                                                      |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.7.18-15`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85678` - field-t deletes Fake_TABLE objects through base TABLE pointer w/o ...     |
|:JIRA bug: :psbug:`2253`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream Fix: 5.7.19                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85671` - segfault-t failing under recent AddressSanitizer                          |
|:JIRA bug: :psbug:`2252`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85258` - DROP TEMPORARY TABLE creates a transaction in binary log on read only...  |
|:JIRA bug: :psbug:`1785`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85158` - heartbeats/fakerotate cause a forced sync_master_info                     |
|:JIRA bug: :psbug:`1812`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.20-19`                                                                               |
|:Upstream Fix: 5.7.26                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85141` - Write/fsync amplification w/ duplicate GTIDs                              |
|:JIRA bug: :psbug:`1786`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`84958` -  InnoDB's MVCC has O(N^2) behaviors                                       |
|:JIRA bug: :psbug:`4712`                                                                                     |
|:JIRA bug: :psbug:`5450`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.26-29`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`84736` - 5.7 range optimizer crash                                                 |
|:JIRA bug: :psbug:`1055`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`84437` - super-read-only does not allow FLUSH LOGS on 5.7                          |
|:JIRA bug: :psbug:`1772`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream Fix: 5.7.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`84420` - stopwords and ngram indexes                                               |
|:JIRA bug: :psbug:`1802`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.20-18`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`84415` - slave don't report Seconds_Behind_Master when running ...                 |
|:JIRA bug: :psbug:`1770`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream Fix: 5.7.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`84366` - InnoDB index dives do not detect concurrent tree changes, return bogus... |
|:JIRA bug: :psbug:`1089`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.17-11`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`84350` - Error 1290 executing flush logs in read-only slave                        |
|:JIRA bug: :psbug:`1044`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream Fix: 5.7.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83814` - Add support for OpenSSL 1.1                                               |
|:JIRA bug: :psbug:`1105`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83648` - Assertion failure in thread x in file fts0que.cc line 3659                |
|:JIRA bug: :psbug:`1023`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83232` -  replication breaks after bug #74145 happens in master                    |
|:JIRA bug: :psbug:`1017`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.24-26`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83124` - Bug 81657 fix merge to 5.6 broken                                         |
|:JIRA bug: :psbug:`1750`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream Fix: 5.7.17                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83073` - GCC 5 and 6 miscompile mach_parse_compressed                              |
|:JIRA bug: :psbug:`1745`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream Fix: 5.7.17                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83003` - Using temporary tables on slaves increases GTID sequence number           |
|:JIRA bug: :psbug:`964`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.17-11`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82980` - Multi-threaded slave leaks worker threads in case of thread create ...    |
|:JIRA bug: :psbug:`2193`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream Fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82940` - mysqld crashes itself when creating index                                 |
|:JIRA bug: :psbug:`3410`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.26-29`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82935` - Cipher ECDHE-RSA-AES128-GCM-SHA256 listed in man/Ssl_cipher_list, not...  |
|:JIRA bug: :psbug:`1737`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82886` - Server may crash due to a glibc bug in handling short-lived detached ...  |
|:JIRA bug: :psbug:`1006`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream Fix: 5.7.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82307` - Memory leaks in unit tests                                                |
|:JIRA bug: :psbug:`2157`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream Fix: 5.7.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82283` - main.mysqlbinlog_debug fails with a LeakSanitizer error                   |
|:JIRA bug: :psbug:`2156`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream Fix: 5.7.19                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82026` - Stack buffer overflow with --ssl-cipher=<more than 4K characters>         |
|:JIRA bug: :psbug:`2155`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82019` - Is client library supposed to retry EINTR indefinitely or not             |
|:JIRA bug: :psbug:`1720`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream Fix: 5.7.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81814` - InnoDB adaptive hash index uses a bad partitioning algorithm for the ...  |
|:JIRA bug: :psbug:`2498`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81810` - Inconsistent sort order for blob/text between InnoDB and filesort         |
|:JIRA bug: :psbug:`1799`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-14`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81714` - mysqldump get_view_structure does not free MYSQL_RES in one error path    |
|:JIRA bug: :psbug:`2152`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.13-6`                                                                                |
|:Upstream Fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81675` - mysqlbinlog does not free the existing connection before opening new ...  |
|:JIRA bug: :psbug:`1718`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-6`                                                                                |
|:Upstream Fix: 5.7.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81657` - DBUG_PRINT in THD::decide_logging_format prints incorrectly, access ...   |
|:JIRA bug: :psbug:`2150`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-6`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81467` - innodb_fts.sync_block test unstable due to slow query log nondeterminism  |
|:JIRA bug: :psbug:`2232`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`80962` - Replication does not work when @@GLOBAL.SERVER_UUID is missing on the...  |
|:JIRA bug: :psbug:`1684`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream Fix: 5.7.13                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`80607` - main.log_tables-big unstable on loaded hosts                              |
|:JIRA bug: :psbug:`2141`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream Fix: 5.7.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`80606` - my_write, my_pwrite no longer safe to call from THD-less server utility...|
|:JIRA bug: :psbug:`970`                                                                                      |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`80496` - buf_dblwr_init_or_load_pages now returns an error code, but caller not... |
|:JIRA bug: :psbug:`3384`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`80288` - missing innodb_numa_interleave                                            |
|:JIRA bug: :psbug:`974`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream Fix: 5.7.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`80053` - Assertion in binlog coordinator on slave with 2 2pc handler log_slave ... |
|:JIRA bug: :psbug:`3361`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`79894` - Page cleaner worker threads are not instrumented for performance schema   |
|:JIRA bug: :psbug:`3356`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`79703` - Spin rounds per wait will be negative in InnoDB status if spin waits >... |
|:JIRA bug: :psbug:`1684`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`79610` - Failed DROP DATABASE due FK constraint on master breaks slave             |
|:JIRA bug: :psbug:`1683`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.14-7`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`79569` - Some --big-test tests were forgotten to update in 5.7.10                  |
|:JIRA bug: :psbug:`3339`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-2`                                                                                |
|:Upstream Fix: 5.7.11                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`79117` - "change_user" command should be aware of preceding "error" command        |
|:JIRA bug: :psbug:`659`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: 5.7.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`78894` - buf_pool_resize can lock less in checking whether AHI is on or off        |
|:JIRA bug: :psbug:`3340`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`77684` - DROP TABLE IF EXISTS may brake replication if slave has replication ...   |
|:JIRA bug: :psbug:`1639`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: 5.7.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`77591` - ALTER TABLE does not allow to change NULL/NOT NULL if foreign key exists  |
|:JIRA bug: :psbug:`1635`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`77399` - Deadlocks missed by INFORMATION_SCHEMA.INNODB_METRICS lock_deadlocks ...  |
|:JIRA bug: :psbug:`1635`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`76418` - Server crashes when querying partitioning table MySQL_5.7.14              |
|:JIRA bug: :psbug:`1050`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.7.18-15`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`76142` - InnoDB tablespace import fails when importing table w/ different data ... |
|:JIRA bug: :psbug:`1697`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.13-6`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75534` - Solve buffer pool mutex contention by splitting it                        |
|:JIRA bug: :ref:`innodb_split_buf_pool_mutex`                                                                |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75504` - btr_search_guess_on_hash makes found block young twice?                   |
|:JIRA bug: :psbug:`2454`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75480` - Selecting wrong pos with SHOW BINLOG EVENTS causes a potentially ...      |
|:JIRA bug: :psbug:`1600`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75311` - Error for SSL cipher is unhelpful                                         |
|:JIRA bug: :psbug:`1779`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.17-12`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75189` - engines suite tests depending on InnoDB implementation details            |
|:JIRA bug: :psbug:`2103`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`74637` - make dirty page flushing more adaptive                                    |
|:JIRA bug: :ref:`Multi-threaded asynchronous LRU flusher <lru_manager_threads>`                              |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-3`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`73418` - Add --manual-lldb option to mysql-test-run.pl                             |
|:JIRA bug: :psbug:`2448`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72615` - MTR --mysqld=--default-storage-engine=foo incompatible w/ dynamically...  |
|:JIRA bug: :psbug:`2071`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72475` - Binlog events with binlog_format=MIXED are unconditionally logged in ...  |
|:JIRA bug: :psbug:`151`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72466` - More memory overhead per page in the InnoDB buffer pool                   |
|:JIRA bug: :psbug:`1689`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.12-5`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72123` - Spurious lock_wait_timeout_thread wakeup in lock_wait_suspend_thread()    |
|:JIRA bug: :psbug:`2504`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72108` - Hard to read history file                                                 |
|:JIRA bug: :psbug:`2066`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71761` - ANALYZE TABLE should remove its table from background stat processing...  |
|:JIRA bug: :psbug:`1749`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71759` - memory leak with string thread variable that set memalloc flag            |
|:JIRA bug: :psbug:`1004`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.15-9`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71411` - buf_flush_LRU() does not return correct number in case of compressed ...  |
|:JIRA bug: :psbug:`1461`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71270` - Failures to end bulk insert for partitioned tables handled incorrectly    |
|:JIRA bug: :psbug:`700`                                                                                      |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71217` - Threadpool - add thd_wait_begin/thd_wait_end to the network IO functions  |
|:JIRA bug: :psbug:`1343`                                                                                     |
|:Upstream State: Open (checked on 2019-05-21)                                                                |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71183` - os_file_fsync() should handle fsync() returning EINTR                     |
|:JIRA bug: :psbug:`1461`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71091` - CSV engine does not properly process "", in quotes                        |
|:JIRA bug: :psbug:`153`                                                                                      |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70500` - Page cleaner should perform LRU flushing regardless of server activity    |
|:JIRA bug: :psbug:`1428`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70490` - Suppression is too strict on some systems                                 |
|:JIRA bug: :psbug:`2038`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: 5.7.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69991` - MySQL client is broken without readline                                   |
|:JIRA bug: :psbug:`1467`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69639` - mysql failed to build with dtrace Sun D 1.11                              |
|:JIRA bug: :psbug:`1392`                                                                                     |
|:Upstream State: Unsupported                                                                                 |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69258` - does buf_LRU_buf_pool_running_out need to lock buffer pool mutexes        |
|:JIRA bug: :psbug:`1414`                                                                                     |
|:Upstream State: Not a bug                                                                                   |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69232` - buf_dblwr->mutex can be splited into two                                  |
|:JIRA bug: :ref:`parallel_doublewrite_buffer`                                                                |
|:Upstream State: No Feedback (checked on 2019-05-21)                                                         |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69170` - buf_flush_LRU is lazy                                                     |
|:JIRA bug: :psbug:`2430`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69146` - Needless log flush order mutex acquisition in buf_pool_get_oldest_mod...  |
|:JIRA bug: :psbug:`2418`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68714` - Remove literal statement digest values from perfschema tests              |
|:JIRA bug: :psbug:`1340`                                                                                     |
|:Upstream State: Not a bug                                                                                   |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68481` - InnoDB LRU flushing for MySQL 5.6 needs work                              |
|:JIRA bug: :psbug:`2432`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68052` - SSL Certificate Subject ALT Names with IPs not respected with --ssl-ver...|
|:JIRA bug: :psbug:`1076`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.7.18-16`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`67808` - in innodb engine, double write and multi-buffer pool instance reduce ...  |
|:JIRA bug: :ref:`parallel_doublewrite_buffer`                                                                |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.11-4`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`63130` - CMake-based check for the presence of a system readline library is not... |
|:JIRA bug: :psbug:`1467`                                                                                     |
|:Upstream State: Can't Repeat                                                                                |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`57583` - fast index create not used during "alter table foo engine=innodb"         |
|:JIRA bug: :psbug:`2113`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`53645` - SHOW GRANTS not displaying all the applicable grants                      |
|:JIRA bug: :psbug:`191`                                                                                      |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`53588` - Blackhole : Specified key was too long; max key length is 1000 bytes      |
|:JIRA bug: :psbug:`1126`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.20-19`                                                                               |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`49120` - mysqldump should have flag to delay creating indexes for innodb plugin... |
|:JIRA bug: :psbug:`2619`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`42415` - UPDATE/DELETE with LIMIT clause unsafe for SBL even with ORDER BY PK ...  |
|:JIRA bug: :psbug:`44`                                                                                       |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`39833` - CREATE INDEX does full table copy on TEMPORARY table                      |
|:JIRA bug: N/A                                                                                               |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`35125` - Allow the ability to set the server_id for a connection for logging to... |
|:Launchpad BP: `Blueprint <https://blueprints.launchpad.net/percona-server/+spec/per-session-server-id>`_    |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: 5.7.10-1                                                                                      |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`25007` - memory tables with dynamic rows format                                    |
|:JIRA bug: :psbug:`2407`                                                                                     |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`20001` - Support for temp-tables in INFORMATION_SCHEMA                             |
|:JIRA bug: :ref:`temp_tables`                                                                                |
|:Upstream State: Verified (checked on 2019-05-21)                                                            |
|:Fix Released: :rn:`5.7.10-1`                                                                                |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
