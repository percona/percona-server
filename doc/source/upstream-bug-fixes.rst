.. _upstream_bug_fixes:

==============================================================
List of upstream |MySQL| bugs fixed in |Percona Server|    5.6
==============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`90351` - GLOBAL STATUS variables drift after rollback                              |
|:JIRA bug: :psbug:`3951`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.40-84.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`90264` - Some file operations in mf_iocache2.c are not instrumented                |
|:JIRA bug: :psbug:`3937`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.40-84.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`90238` - Comparison of uninitailized memory in log_in_use                          |
|:JIRA bug: :psbug:`3925`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.40-84.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`90111` - Incorrect enum comparisons                                                |
|:JIRA bug: :psbug:`3893`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.40-84.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89766` - a typo in `cmake/plugin.cmake` prevents `MYSQL_SERVER` to be defined ...  |
|:JIRA bug: :psbug:`3871`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.40-84.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`88057` - Intermediary slave does not log master changes with...                    |
|:JIRA bug: :psbug:`1119`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.39-83.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`87065` - Release lock on table statistics after query plan created                 |
|:JIRA bug: :psbug:`2503`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.38-83.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`86260` - Assert on KILL'ing a stored routine invocation                            |
|:JIRA bug: :psbug:`1091`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.1`                                                                             |
|:Upstream Fix: 5.6.40                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`86209` - audit plugin + MB collation connection + PREPARE stmt parse error crash...|
|:JIRA bug: :psbug:`1089`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85838` - rpl_diff.inc in 5.7 does not compare data from different servers          |
|:JIRA bug: :psbug:`2257`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85678` - field-t deletes Fake_TABLE objects through base TABLE pointer w/o ...     |
|:JIRA bug: :psbug:`2253`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream Fix: 5.6.37                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85671` - segfault-t failing under recent AddressSanitizer                          |
|:JIRA bug: :psbug:`2252`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream Fix: 5.6.37                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85258` - DROP TEMPORARY TABLE creates a transaction in binary log on read only...  |
|:JIRA bug: :psbug:`1785`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`84415` - slave don't report Seconds_Behind_Master when running ...                 |
|:JIRA bug: :psbug:`1770`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`84366` - InnoDB index dives do not detect concurrent tree changes, return bogus... |
|:JIRA bug: :psbug:`1743`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.35-80.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83814` - Add support for OpenSSL 1.1                                               |
|:JIRA bug: :psbug:`1105`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83648` - Assertion failure in thread x in file fts0que.cc line 3659                |
|:JIRA bug: :psbug:`1023`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.35-80.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83232` - replication breaks after bug :mysqlbug:`74145` happens in master          |
|:JIRA bug: :psbug:`1017`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.42-84.2`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83124` - Bug 81657 fix merge to 5.6 broken                                         |
|:JIRA bug: :psbug:`1750`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: 5.6.35                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83073` - GCC 5 and 6 miscompile mach_parse_compressed                              |
|:JIRA bug: :psbug:`1745`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: 5.6.35                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83003` - Using temporary tables on slaves increases GTID sequence number           |
|:JIRA bug: :psbug:`964`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.35-80.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82980` - Multi-threaded slave leaks worker threads in case of thread create ...    |
|:JIRA bug: :psbug:`2193`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: 5.6.38                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82935` - Cipher ECDHE-RSA-AES128-GCM-SHA256 listed in man/Ssl_cipher_list, not ... |
|:JIRA bug: :psbug:`1737`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82886` - Server may crash due to a glibc bug in handling short-lived detached ...  |
|:JIRA bug: :psbug:`1006`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: 5.6.35                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82798` - Small buffer pools might be too small for rseg init during crash recovery |
|:JIRA bug: :psbug:`3525`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82019` - Is client library supposed to retry EINTR indefinitely or not             |
|:JIRA bug: :psbug:`1720`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.32-78.0`                                                                             |
|:Upstream Fix: 5.6.33                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81714` - mysqldump get_view_structure does not free MYSQL_RES in one error path    |
|:JIRA bug: :psbug:`2152`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream Fix: 5.6.38                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81675` - mysqlbinlog does not free the existing connection before opening new ...  |
|:JIRA bug: :psbug:`1718`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream Fix: 5.6.33                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81674` - LeakSanitizer-enabled build fails to bootstrap server for MTR             |
|:JIRA bug: :psbug:`3486`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.32-78.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81657` - DBUG_PRINT in THD::decide_logging_format prints incorrectly, access ...   |
|:JIRA bug: :psbug:`2150`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`81467` - innodb_fts.sync_block test unstable due to slow query log nondeterminism  |
|:JIRA bug: :psbug:`2232`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.35-80.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`80014` - mysql build fails, memory leak in gen_lex_hash, clang address sanitizer   |
|:JIRA bug: :psbug:`3433`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.30-76.3`                                                                             |
|:Upstream Fix: 5.6.35                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`79703` - Spin rounds per wait will be negative in InnoDB status if spin waits ...  |
|:JIRA bug: :psbug:`1684`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.28-76.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`79610` - Failed DROP DATABASE due FK constraint on master breaks slave             |
|:JIRA bug: :psbug:`1683`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.32-78.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`79185` - Innodb freeze running REPLACE statements                                  |
|:JIRA bug: :psbug:`945`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.27-76.0`                                                                             |
|:Upstream Fix: 5.6.30                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`79117` - "change_user" command should be aware of preceding "error" command        |
|:JIRA bug: :psbug:`659`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`78223` - memory leak in mysqlbinlog                                                |
|:JIRA bug: :psbug:`3440`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`78050` - Crash on when XA functions activated by a storage engine                  |
|:JIRA bug: :psbug:`742`                                                                                      |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`77684` - DROP TABLE IF EXISTS may brake replication if slave has replication filter|
|:JIRA bug: :psbug:`1639`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.26-74.0`                                                                             |
|:Upstream Fix: 5.6.30                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`77637` - mysql 5.6.25 compiled warning                                             |
|:JIRA bug: :psbug:`3632`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.39-83.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`77591` - ALTER TABLE does not allow to change NULL/NOT NULL if foreign key exists  |
|:JIRA bug: :psbug:`1635`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.26-74.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`77399` - Deadlocks missed by INFORMATION_SCHEMA.INNODB_METRICS lock_deadlocks ...  |
|:JIRA bug: :psbug:`1632`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`77275` - Newest RHEL/CentOS openssl update breaks mysql DHE ciphers                |
|:JIRA bug: :psbug:`906`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.25-73.0`                                                                             |
|:Upstream Fix: 5.6.26                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`76927` - Duplicate UK values in READ-COMMITTED (again)                             |
|:JIRA bug: :psbug:`1494`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.25-73.0`                                                                             |
|:Upstream Fix: 5.6.27                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`76418` - Server crashes when querying partitioning table MySQL_5.7.14              |
|:JIRA bug: :psbug:`1050`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.36-82.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`76349` - memory leak in add_derived_key()                                          |
|:JIRA bug: :psbug:`826`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.24-72.2`                                                                             |
|:Upstream Fix: 5.6.27                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`76142` - InnoDB tablespace import fails when importing table w/ different datadir  |
|:JIRA bug: :psbug:`1697`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75642` - Extend valid range of dummy certificates ni mysql-test/std_data           |
|:JIRA bug: :psbug:`1605`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.22-72.0`                                                                             |
|:Upstream Fix: 5.6.23                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75595` - Compute InnoDB redo log block checksums faster                            |
|:Launchpad BP: `<https://blueprints.launchpad.net/percona-server/+spec/more-efficient-log-block-checksums>`_ |
|:Upstream State: Closed                                                                                      |
|:Fix Released: 5.6.14-62.0                                                                                   |
|:Upstream Fix: 5.6.25                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75534` - Solve buffer pool mutex contention by splitting it                        |
|:JIRA bug: :ref:`innodb_split_buf_pool_mutex`                                                                |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75480` - Selecting wrong pos with SHOW BINLOG EVENTS causes a potentially ...      |
|:JIRA bug: :psbug:`1600`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.25-73.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75311` - Error for SSL cipher is unhelpful                                         |
|:JIRA bug: :psbug:`1779`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.35-80.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75235` - Optimize ibuf merge when reading a page from disk                         |
|:JIRA bug: :psbug:`2484`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75189` - engines suite tests depending on InnoDB implementation details            |
|:JIRA bug: :psbug:`2103`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.22-71.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`74987` - mtr failure on Ubuntu Utopic, mysqlhotcopy fails with wrong error(255)    |
|:JIRA bug: :psbug:`2102`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.22-71.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`74842` - Incorrect attribute((nonnull)) for btr_cur_ins_lock_and_undo callees      |
|:JIRA bug: :psbug:`385`                                                                                      |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.21-70.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`74644` - A query on empty table with BLOBs may crash server                        |
|:JIRA bug: :psbug:`176`                                                                                      |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.22-71.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`74637` - make dirty page flushing more adaptive                                    |
|:Launchpad BP: `Split LRU ...   <https://blueprints.launchpad.net/percona-server/+spec/lru-manager-thread>`_ |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: 5.6.16-64.0                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`74440` - mysql_install_db not handling mysqld startup failure                      |
|:JIRA bug: :psbug:`1553`                                                                                     |
|:Upstream State: Won't fix                                                                                   |
|:Fix Released: :rn:`5.6.21-70.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`73979` - wrong stack size calculation leads to stack overflow in pinbox allocator  |
|:JIRA bug: :psbug:`807`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.22-71.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`73736` - Missing testcase sync in rpl_err_ignoredtable                             |
|:JIRA bug: :psbug:`2081`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.21-69.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`73689` - Zero can be a valid InnoDB checksum, but validation will fail later       |
|:JIRA bug: :psbug:`PS-909`                                                                                   |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.25-73.0`                                                                             |
|:Upstream Fix: 5.6.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`73418` - Add --manual-lldb option to mysql-test-run.pl                             |
|:JIRA bug: :psbug:`2448`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.20-68.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`73066` - Replication stall with multi-threaded replication                         |
|:JIRA bug: :psbug:`1511`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.21-70.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72615` - MTR --mysqld=--default-storage-engine=foo incompatible w/ dynamically...  |
|:JIRA bug: :psbug:`2071`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.17-66.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72475` - Binlog events with binlog_format=MIXED are unconditionally logged in ROW..|
|:JIRA bug: :psbug:`151`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.21-70.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72466` - More memory overhead per page in the InnoDB buffer pool                   |
|:JIRA bug: :psbug:`1689`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.30-76.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72457` - Replication with no tmpdir space can break replication                    |
|:JIRA bug: :psbug:`1107`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.42-84.2`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72163` - Rev 5774 broke rpl_plugin_load                                            |
|:JIRA bug: :psbug:`2068`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.17-65.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`72108` - Hard to read history file                                                 |
|:JIRA bug: :psbug:`2066`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.24-72.2`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71988` - page_cleaner: aggressive background flushing                              |
|:JIRA bug: :psbug:`1437`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71761` - ANALYZE TABLE should remove its table from background stat processing ... |
|:JIRA bug: :psbug:`1749`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71759` - memory leak with string thread variable that set memalloc flag            |
|:JIRA bug: :psbug:`1006`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71708` - 70768 fix perf regression: high rate of RW lock creation and destruction  |
|:JIRA bug: :psbug:`1474`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream Fix: 5.6.19                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71624` - printf size_t results in a fatal warning in 32-bit debug builds           |
|:JIRA bug: :psbug:`760`                                                                                      |
|:Upstream State: Can't Repeat (checked on 2018-11-28)                                                        |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71603` - file name is not escaped in binlog for LOAD DATA INFILE statement         |
|:JIRA bug: :psbug:`3092`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71411` - buf_flush_LRU() does not return correct number in case of compressed pages|
|:JIRA bug: :psbug:`2430`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71374` - Slave IO thread won't attempt auto reconnect to the master/error-code 1159|
|:JIRA bug: :psbug:`1470`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.16-64.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71270` - Failures to end bulk insert for partitioned tables handled incorrectly    |
|:JIRA bug: :psbug:`700`                                                                                      |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71250` - Bison 3 breaks mysql build                                                |
|:JIRA bug: :psbug:`376`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.17-65.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71217` - Threadpool - add thd_wait_begin/thd_wait_end to the network IO functions  |
|:JIRA bug: :psbug:`1343`                                                                                     |
|:Upstream State: Open (checked on 2018-11-28)                                                                |
|:Fix Released: :rn:`5.6.15-63.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71183` - os_file_fsync() should handle fsync() returning EINTR                     |
|:JIRA bug: :psbug:`1461`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71094` - ssl.cmake related warnings                                                |
|:JIRA bug: :psbug:`2058`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71092` - InnoDB FTS introduced new mutex sync level in 5.6.15, broke UNIV_SYNC ... |
|:JIRA bug: :psbug:`1393`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.15-63.0`                                                                             |
|:Upstream Fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71091` - CSV engine does not properly process ``""``, in quotes                    |
|:JIRA bug: :psbug:`153`                                                                                      |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.21-70.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71089` - CMake warning when generating Makefile                                    |
|:JIRA bug: :psbug:`2059`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream Fix: 5.6.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70860` - --tc-heuristic-recover option values are broken                           |
|:JIRA bug: :psbug:`1514`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.20-68.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70854` - Tc_log_page_size should be unflushable or server crashes if 2 XA SEs ...  |
|:JIRA bug: :psbug:`743`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70500` - Page cleaner should perform LRU flushing regardless of server activity    |
|:JIRA bug: :psbug:`1428`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70490` - Suppression is too strict on some systems                                 |
|:JIRA bug: :psbug:`2038`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70489` - Crash when using AES_ENCRYPT on empty string                              |
|:JIRA bug: :psbug:`689`                                                                                      |
|:Upstream State: Unsupported (checked on 2018-11-28)                                                         |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70453` - Add hard timeouts to page cleaner flushes                                 |
|:JIRA bug: :psbug:`2431`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70417` - rw_lock_x_lock_func_nowait() calls os_thread_get_curr_id() mostly ...     |
|:JIRA bug: :psbug:`2429`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: 5.6.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70277` - last argument of LOAD DATA ... SET ... statement repeated twice in binlog |
|:JIRA bug: :psbug:`3020`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70228` - Is buf_LRU_free_page() really supposed to make non-zip block sticky at ...|
|:JIRA bug: :psbug:`1415`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`70216` - Unnecessary overhead from persistent adaptive hash index latches          |
|:JIRA bug: :psbug:`715`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69991` - MySQL client is broken without readline                                   |
|:JIRA bug: :psbug:`1467`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.24-72.2`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69856` - mysql_install_db does not function properly in 5.6 for debug builds       |
|:JIRA bug: :psbug:`359`                                                                                      |
|:Upstream State: Won't fix                                                                                   |
|:Fix Released: :rn:`5.6.12-60.4`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69639` - mysql failed to build with dtrace Sun D 1.11                              |
|:JIRA bug: :psbug:`1392`                                                                                     |
|:Upstream State: Unsupported (checked on 2018-11-28)                                                         |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69617` - 5.6.12 removed UNIV_SYNC_DEBUG from UNIV_DEBUG                            |
|:JIRA bug: :psbug:`1411`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream Fix: 5.6.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69524` - Some tests for table cache variables fail if open files limit is too low  |
|:JIRA bug: :psbug:`96`                                                                                       |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.12-60.4`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69396` - Can't set query_cache_type to 0 when it is already 0                      |
|:JIRA bug: :psbug:`3563`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69265` - -DBUILD_CONFIG=mysql_release -DWITH_DEBUG=ON fails 4 and skips 27 MTR ... |
|:JIRA bug: :psbug:`1345`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69258` - does buf_LRU_buf_pool_running_out need to lock buffer pool mutexes        |
|:JIRA bug: :psbug:`1414`                                                                                     |
|:Upstream State: Not a bug                                                                                   |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69252` - All the parts.partition_max* tests are broken with MTR --parallel         |
|:JIRA bug: :psbug:`1364`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69179` - accessing information_schema.partitions causes plans to change            |
|:JIRA bug: :psbug:`680`                                                                                      |
|:Upstream State: Duplicate                                                                                   |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream Fix: 5.6.14                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69170` - buf_flush_LRU is lazy                                                     |
|:JIRA bug: :psbug:`2430`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69146` - Optimization in buf_pool_get_oldest_modification if srv_buf_pool_instances|
|:JIRA bug: :psbug:`2418`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69124` - Incorrect truncation of long SET expression in LOAD DATA can cause SQL ...|
|:JIRA bug: :psbug:`663`                                                                                      |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`69059` - GTIDs lack a reasonable deployment strategy                               |
|:Launchpad BP: `GTID deploy... <https://blueprints.launchpad.net/percona-server/+spec/gtid-deployment-step>`_|
|:Upstream State: Closed                                                                                      |
|:Fix Released: 5.6.22-72.0                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68999` - SSL_OP_NO_COMPRESSION not defined                                         |
|:JIRA bug: :psbug:`362`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.25                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68970` - fsp_reserve_free_extents switches from small to big tblspace handling ... |
|:JIRA bug: :psbug:`656`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68845` - Unnecessary log_sys->mutex reacquisition in mtr_log_reserve_and_write()   |
|:JIRA bug: :psbug:`1347`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68800` - client doesn't read plugin-dir from my.cnf set by MYSQL_READ_DEFAULT_FILE |
|:JIRA bug: :psbug:`82`                                                                                       |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68714` - Remove literal statement digest values from perfschema tests              |
|:JIRA bug: :psbug:`1340`                                                                                     |
|:Upstream State: Not a bug                                                                                   |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68713` - create_duplicate_weedout_tmp_table() leaves key_part_flag uninitialized   |
|:JIRA bug: :psbug:`644`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68659` - InnoDB Linux native aio should submit more i/o requests at once           |
|:JIRA bug: :ref:`aio_page_requests`                                                                          |
|:Upstream State: Analyzing (checked on 2018-11-28)                                                           |
|:Fix Released: :rn:`5.6.38-83.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68635` - Doc: Multiple issues with performance_schema_max_statement_classes        |
|:JIRA bug: :psbug:`1339`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68555` - thread convoys from log_checkpoint_margin with innodb_buffer_pool_inst... |
|:JIRA bug: :psbug:`2434`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68490` - slave_max_allowed_packet Not Honored on Slave IO Connect                  |
|:JIRA bug: :psbug:`49`                                                                                       |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68481` - InnoDB LRU flushing for MySQL 5.6 needs work                              |
|:JIRA bug: :psbug:`2432`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68477` - Suboptimal code in skip_trailing_space()                                  |
|:JIRA bug: :psbug:`1321`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68476` - Suboptimal code in my_strnxfrm_simple()                                   |
|:JIRA bug: :psbug:`1320`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68354` - Server crashes on update/join FEDERATED + local table when only 1 local...|
|:JIRA bug: :psbug:`96`                                                                                       |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.12-60.4`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68116` - InnoDB monitor may hit an assertion error in buf_page_get_gen in debug ...|
|:JIRA bug: :psbug:`616`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.10-60.2`                                                                             |
|:Upstream Fix: 5.6.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68052` - SSL Certificate Subject ALT Names with IPs not respected with --ssl-ver...|
|:JIRA bug: :psbug:`1076`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`68045` - security vulnerability CVE-2012-4414                                      |
|:JIRA bug: :psbug:`348`                                                                                      |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`67974` - Server crashes in add_identifier on concurrent ALTER TABLE and SHOW ENGINE|
|:JIRA bug: :psbug:`344`                                                                                      |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`67879` - Slave deadlock caused by stop slave, show slave status and global read... |
|:Launchpad BP: :ref:`show_slave_status_nolock`                                                               |
|:Upstream State: Closed                                                                                      |
|:Fix Released: 5.6.11-60.3                                                                                   |
|:Upstream Fix: 5.6.23                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`67806` - Multiple user level lock per connection                                   |
|:JIRA bug: :ref:`multiple_user_level_locks`                                                                  |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.19-67.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`67685` - security vulnerability CVE-2012-5611                                      |
|:JIRA bug: :psbug:`350`                                                                                      |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`67504` - Duplicate error in replication with slave triggers and auto increment     |
|:JIRA bug: :psbug:`34`                                                                                       |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`66779` - innochecksum does not work with compressed tables                         |
|:JIRA bug: :psbug:`1302`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.25-73.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`66550` - security vulnerability CVE-2012-4414                                      |
|:JIRA bug: :psbug:`348`                                                                                      |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`66301` - INSERT ... ON DUPLICATE KEY UPDATE + innodb_autoinc_lock_mode=1 is broken |
|:JIRA bug: :psbug:`576`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`66237` - Temporary files created by binary log cache are not purged after transa...|
|:JIRA bug: :psbug:`599`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`65946` - Sid_map::Sid_map calls DBUG which may have unitialized THR_KEY_mysys and..|
|:JIRA bug: :psbug:`585`                                                                                      |
|:Upstream State: Duplicate                                                                                   |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream Fix: 5.6.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`64800` - mysqldump with --include-master-host-port putting quotes around port no.  |
|:JIRA bug: :psbug:`1923`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`64663` - Segfault when adding indexes to InnoDB temporary tables                   |
|:JIRA bug: :psbug:`557`                                                                                      |
|:Upstream State: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`64556` - Interrupting a query inside InnoDB causes an unrelated warning to be ...  |
|:JIRA bug: :psbug:`1967`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: 5.6.14                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`64432` - Bug :mysqlbug:`54330` (Broken fast index creation) was never fixed in 5.5 |
|:JIRA bug: :psbug:`544`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`63451` - atomic/x86-gcc.h:make_atomic_cas_body64 potential miscompilation bug      |
|:JIRA bug: :psbug:`508`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`63144` - CREATE TABLE IF NOT EXISTS metadata lock is too restrictive               |
|:JIRA bug: :psbug:`40`                                                                                       |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.13                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`63130` - CMake-based check for the presence of a system readline library is not... |
|:JIRA bug: :psbug:`1467`                                                                                     |
|:Upstream State: Can't Repeat (checked on 2018-11-28)                                                        |
|:Fix Released: :rn:`5.6.24-72.2`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`62856` - Check for "stack overrun" doesn't work with gcc-4.6, server crashes       |
|:JIRA bug: :psbug:`2795`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`62578` - mysql client aborts connection on terminal resize                         |
|:JIRA bug: :psbug:`84`                                                                                       |
|:Upstream State: Won't fix                                                                                   |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`62018` - innodb adaptive hash index mutex contention                               |
|:JIRA bug: :psbug:`1410`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`61595` - mysql-test/include/wait_for_slave_param.inc timeout logic is incorrect    |
|:JIRA bug: :psbug:`485`                                                                                      |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`61180` - korr/store macros in my_global.h assume the argument to be a char pointer |
|:JIRA bug: :psbug:`2795`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`61178` - Incorrect implementation of intersect(ulonglong) in non-optimized Bitmap..|
|:JIRA bug: :psbug:`2795`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`60782` - Audit plugin API: no MYSQL_AUDIT_GENERAL_LOG notifications with general...|
|:JIRA bug: :psbug:`1369`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.17-65.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`60743` - typo in cmake/dtrace.cmake                                                |
|:JIRA bug: :psbug:`1924`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: 5.6.13                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`60682` - deadlock from thd_security_context                                        |
|:JIRA bug: :psbug:`1310`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`57583` - fast index create not used during "alter table foo engine=innodb"         |
|:JIRA bug: :psbug:`2619`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`57430` - query optimizer does not pick covering index for some "order by" queries  |
|:JIRA bug: :psbug:`1587`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.22-71.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`56676` - 'show slave status' ,'show global status' hang when 'stop slave' takes... |
|:Launchpad BP: :ref:`show_slave_status_nolock`                                                               |
|:Upstream State: Closed                                                                                      |
|:Fix Released: 5.6.11-60.3                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`54814` - make BUF_READ_AHEAD_AREA a constant                                       |
|:JIRA bug: :psbug:`668`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`54430` - innodb should retry partial reads/writes where errno was 0                |
|:JIRA bug: :psbug:`1948`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`54430` - innodb should retry partial reads/writes where errno was 0                |
|:JIRA bug: :psbug:`1948`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`54160` - InnoDB should retry on failed read or write, not immediately panic        |
|:JIRA bug: :psbug:`2628`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`54127` - mysqld segfaults when built using --with-max-indexes=128                  |
|:JIRA bug: :psbug:`2795`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`53645` - SHOW GRANTS not displaying all the applicable grants                      |
|:JIRA bug: :psbug:`1467`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.23-72.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`53588` - Blackhole: Specified key was too long; max key length is 1000 bytes       |
|:JIRA bug: :psbug:`1126`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.39-83.1`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`49169` - read_view_open_now is inefficient with many concurrent sessions           |
|:JIRA bug: :psbug:`636` and :psbug:`637`                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`49120` - mysqldump should have flag to delay creating indexes for innodb plugin    |
|:JIRA bug: :psbug:`2619`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`47134` - Crash on startup when XA support functions activated by a second engine   |
|:JIRA bug: :psbug:`742`                                                                                      |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`45679` - KILL QUERY not behaving consistently and will hang in some cases          |
|:JIRA bug: :psbug:`3551`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`42415` - UPDATE/DELETE with LIMIT clause unsafe for SBL even with ORDER BY PK ...  |
|:JIRA bug: :psbug:`44`                                                                                       |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`41975` - Support for SSL options not included in mysqlbinlog                       |
|:JIRA bug: :psbug:`1393`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: :rn:`5.6.15-63.0`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`39833` - CREATE INDEX does full table copy on TEMPORARY table                      |
|:JIRA bug: N/A                                                                                               |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.10-60.2`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`35125` - Allow the ability to set the server_id for a connection for logging to... |
|:Launchpad Bug: `Blueprint <https://blueprints.launchpad.net/percona-server/+spec/per-session-server-id>`_   |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: 5.6.26-74.0                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`25007` - memory tables with dynamic rows format                                    |
|:JIRA bug: :psbug:`2407`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`20001` - Support for temp-tables in INFORMATION_SCHEMA                             |
|:JIRA bug: :ref:`temp_tables`                                                                                |
|:Upstream State: Verified (checked on 2018-11-28)                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`1118` - Allow multiple concurrent locks with GET_LOCK()                            |
|:Launchpad BP: :ref:`multiple_user_level_locks`                                                              |
|:Upstream State: Closed                                                                                      |
|:Fix Released: 5.6.19-67.0                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
