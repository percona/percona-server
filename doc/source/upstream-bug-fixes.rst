.. _upstream_bug_fixes:

================================================================================
List of upstream |MySQL| bugs fixed in |Percona Server|  8.0
================================================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89840` - 60-80k connections causing empty reply for select                         |
|:JIRA bug: :psbug:`314`                                                                                      |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89607` - MySQL crash in debug, PFS thread not handling singals.                    |
|:JIRA bug: :psbug:`311`                                                                                      |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89598` - plugin_mecab.cc:54:19: warning: unused variable 'bundle_mecab'            |
|:JIRA bug: :psbug:`3804`                                                                                     |
|:Upstream State: Closed                                                                                      |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: 8.0                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89467` - Redundant GTID unsafe mark for CREATE/DROP TEMPORARY TABLE in RBR/MBR     |
|:JIRA bug: :psbug:`1816`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`89313` - 60-80k connections causing empty reply for select                         |
|:JIRA bug: :psbug:`314`                                                                                      |
|:Upstream State: N/A                                                                                         |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`88057` - Intermediary slave does not log master changes with ...                   |
|:JIRA bug: :psbug:`1119`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85208` - A follow-up fix for buffer pool mutex split patch might be suboptimal, ...|
|:JIRA bug: :psbug:`3755`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`85205` - A follow-up fix for buffer pool mutex split patch might be suboptimal, ...|
|:JIRA bug: :psbug:`3754`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`84366` - InnoDB index dives do not detect concurrent tree changes, return bogus ...|
|:JIRA bug: :psbug:`1743`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`83648` - Assertion failure in thread x in file fts0que.cc line 3659                |
|:JIRA bug: :psbug:`1023`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82935` - Cipher ECDHE-RSA-AES128-GCM-SHA256 listed in man/Ssl_cipher_list, not ... |
|:JIRA bug: :psbug:`1737`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`82480` - Incorrect schema mismatch error message when importing mismatched tables  |
|:JIRA bug: :psbug:`1697`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`80496` - buf_dblwr_init_or_load_pages now returns an error code, but caller not ...|
|:JIRA bug: :psbug:`3384`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`77591` - ALTER TABLE does not allow to change NULL/NOT NULL if foreign key exists  |
|:JIRA bug: :psbug:`1635`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`77399` - Deadlocks missed by INFORMATION_SCHEMA.INNODB_METRICS lock_deadlocks ...  |
|:JIRA bug: :psbug:`1632`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`76418` - Server crashes when querying partitioning table MySQL_5.7.14              |
|:JIRA bug: :psbug:`1050`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`76142` - InnoDB tablespace import fails when importing table w/ different data ... |
|:JIRA bug: :psbug:`1697`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75480` - Selecting wrong pos with SHOW BINLOG EVENTS causes a potentially ...      |
|:JIRA bug: :psbug:`1600`                                                                                     |
|:Upstream State: N/A                                                                                         |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`75311` - Error for SSL cipher is unhelpful                                         |
|:JIRA bug: :psbug:`1779`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71761` - ANALYZE TABLE should remove its table from background stat processing ... |
|:JIRA bug: :psbug:`1749`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`71411` - buf_flush_LRU() does not return correct number in case of compressed ...  |
|:JIRA bug: :psbug:`2053`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71217` - Threadpool - add thd_wait_begin/thd_wait_end to the network IO functions  |
|:JIRA bug: :psbug:`1343`                                                                                     |
|:Upstream state: Open (checked on 2018-05-24)                                                                |
|:Fix Released: 8.0.13-3                                                                                      |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream Bug: :mysqlbug:`53588` - Blackhole : Specified key was too long; max key length is 1000 bytes      |
|:JIRA bug: :psbug:`1126`                                                                                     |
|:Upstream State: Verified (checked on 2018-11-20)                                                            |
|:Fix Released: 8.0.12-2rc1                                                                                   |
|:Upstream Fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
