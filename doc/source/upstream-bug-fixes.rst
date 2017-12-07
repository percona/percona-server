.. _upstream_bug_fixes:

=============================================================
 List of upstream |MySQL| bugs fixed in |Percona Server| 5.6
=============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`87065` - Release lock on table statistics after query plan created                 |
|:Launchpad bug: :bug:`1704195`                                                                               |
|:Upstream state: Open (checked on 2017-12-08)                                                                |
|:Fix Released: :rn:`5.6.38-83.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68659` - InnoDB Linux native aio should submit more i/o requests at once           |  
|:Launchpad bug: :ref:`aio_page_requests`                                                                     |
|:Upstream state: Analyzing (checked on 2017-12-08)                                                           |
|:Fix Released: :rn:`5.6.38-83.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83814` - Add support for OpenSSL 1.1                                               |
|:Launchpad bug: :bug:`1702903`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.36-82.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`86260` - Assert on KILL'ing a stored routine invocation                            |
|:Launchpad bug: :bug:`1689736`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.36-82.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68052` - SSL Certificate Subject ALT Names with IPs not respected with --ssl-ver...|
|:Launchpad bug: :bug:`1673656`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.36-82.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85838` - rpl_diff.inc in 5.7 does not compare data from different servers          |
|:Launchpad bug: :bug:`1680510`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85678` - field-t deletes Fake_TABLE objects through base TABLE pointer w/o ...     |
|:Launchpad bug: :bug:`1677130`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream fix: 5.6.37                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85671` - segfault-t failing under recent AddressSanitizer                          |
|:Launchpad bug: :bug:`1676847`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream fix: 5.6.37                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85258` - DROP TEMPORARY TABLE creates a transaction in binary log on read only...  |
|:Launchpad bug: :bug:`1668602`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84415` - slave don't report Seconds_Behind_Master when running ...                 |
|:Launchpad bug: :bug:`1654091`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`86209` - audit plugin + MB collation connection + PREPARE stmt parse error crash...|
|:Launchpad bug: :bug:`1688698`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.36-82.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81467` - innodb_fts.sync_block test unstable due to slow query log nondeterminism  |
|:Launchpad bug: :bug:`1662163`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.35-80.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75311` - Error for SSL cipher is unhelpful                                         |
|:Launchpad bug: :bug:`1660339`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.35-80.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83648` - Assertion failure in thread x in file fts0que.cc line 3659                |
|:Launchpad bug: :bug:`1634932`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.35-80.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`84366` - InnoDB index dives do not detect concurrent tree changes, return bogus... |
|:Launchpad bug: :bug:`1625151`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.35-80.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83003` - Using temporary tables on slaves increases GTID sequence number           |
|:Launchpad bug: :bug:`1539504`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.35-80.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71759` - memory leak with string thread variable that set memalloc flag            |
|:Launchpad bug: :bug:`1621012`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82886` - Server may crash due to a glibc bug in handling short-lived detached ...  |
|:Launchpad bug: :bug:`1621012`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: 5.6.35                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`45679` - KILL QUERY not behaving consistently and will hang in some cases          |
|:Launchpad bug: :bug:`1621046`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82935` - Cipher ECDHE-RSA-AES128-GCM-SHA256 listed in man/Ssl_cipher_list, not ... |
|:Launchpad bug: :bug:`1622034`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`76418` - Server crashes when querying partitioning table MySQL_5.7.14              |
|:Launchpad bug: :bug:`1657941`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.36-82.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69396` - Can't set query_cache_type to 0 when it is already 0                      |
|:Launchpad bug: :bug:`1625501`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71761` - ANALYZE TABLE should remove its table from background stat processing ... |
|:Launchpad bug: :bug:`1626441`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83124` - Bug 81657 fix merge to 5.6 broken                                         |
|:Launchpad bug: :bug:`1626936`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: 5.6.35                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82980` - Multi-threaded slave leaks worker threads in case of thread create ...    |
|:Launchpad bug: :bug:`1619622`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: 5.6.38                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75235` - Optimize ibuf merge when reading a page from disk                         |
|:Launchpad bug: :bug:`1618393`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83073` - GCC 5 and 6 miscompile mach_parse_compressed                              |
|:Launchpad bug: :bug:`1626002`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: 5.6.35                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82798` - Small buffer pools might be too small for rseg init during crash recovery |
|:Launchpad bug: :bug:`1616392`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.33-79.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81674` - LeakSanitizer-enabled build fails to bootstrap server for MTR             |
|:Launchpad bug: :bug:`1603978`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.32-78.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82019` - Is client library supposed to retry EINTR indefinitely or not             |
|:Launchpad bug: :bug:`1591202`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.32-78.0`                                                                             |
|:Upstream fix: 5.6.33                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79610` - Failed DROP DATABASE due FK constraint on master breaks slave             |
|:Launchpad bug: :bug:`1525407`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.32-78.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77399` - Deadlocks missed by INFORMATION_SCHEMA.INNODB_METRICS lock_deadlocks ...  |
|:Launchpad bug: :bug:`1466414`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`76142` - InnoDB tablespace import fails when importing table w/ different datadir  |
|:Launchpad bug: :bug:`1548597`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81675` - mysqlbinlog does not free the existing connection before opening new ...  |
|:Launchpad bug: :bug:`1587840`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream fix: 5.6.33                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`78223` - memory leak in mysqlbinlog                                                |
|:Launchpad bug: :bug:`1582761`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81714` - mysqldump get_view_structure does not free MYSQL_RES in one error path    |
|:Launchpad bug: :bug:`1588845`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream fix: 5.6.38                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81657` - DBUG_PRINT in THD::decide_logging_format prints incorrectly, access ...   |
|:Launchpad bug: :bug:`1587426`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.31-77.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80014` - mysql build fails, memory leak in gen_lex_hash, clang address sanitizer   |
|:Launchpad bug: :bug:`1580993`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.30-76.3`                                                                             |
|:Upstream fix: 5.6.35                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72466` - More memory overhead per page in the InnoDB buffer pool                   |
|:Launchpad bug: :bug:`1536693`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.30-76.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79703` - Spin rounds per wait will be negative in InnoDB status if spin waits ...  |
|:Launchpad bug: :bug:`1527160`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.28-76.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79185` - Innodb freeze running REPLACE statements                                  |
|:Launchpad bug: :bug:`1519094`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.27-76.0`                                                                             |
|:Upstream fix: 5.6.30                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77684` - DROP TABLE IF EXISTS may brake replication if slave has replication filter|
|:Launchpad bug: :bug:`1475107`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.26-74.0`                                                                             |
|:Upstream fix: 5.6.30                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77591` - ALTER TABLE does not allow to change NULL/NOT NULL if foreign key exists  |
|:Launchpad bug: :bug:`1470677`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.26-74.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`35125` - Allow the ability to set the server_id for a connection for logging to... |
|:Launchpad bug: `Blueprint <https://blueprints.launchpad.net/percona-server/+spec/per-session-server-id>`_   |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.26-74.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75480` - Selecting wrong pos with SHOW BINLOG EVENTS causes a potentially ...      |
|:Launchpad bug: :bug:`1409652`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.25-73.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`76927` - Duplicate UK values in READ-COMMITTED (again)                             |
|:Launchpad bug: :bug:`1308016`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.25-73.0`                                                                             |
|:Upstream fix: 5.6.27                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66779` - innochecksum does not work with compressed tables                         |
|:Launchpad bug: :bug:`1100652`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.25-73.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73689` - Zero can be a valid InnoDB checksum, but validation will fail later       |
|:Launchpad bug: :bug:`1467760`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.25-73.0`                                                                             |
|:Upstream fix: 5.6.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77275` - Newest RHEL/CentOS openssl update breaks mysql DHE ciphers                |
|:Launchpad bug: :bug:`1462856`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.25-73.0`                                                                             |
|:Upstream fix: 5.6.26                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`76349` - memory leak in add_derived_key()                                          |
|:Launchpad bug: :bug:`1380985`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.24-72.2`                                                                             |
|:Upstream fix: 5.6.27                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72108` - Hard to read history file                                                 |
|:Launchpad bug: :bug:`1296192`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.24-72.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69991` - MySQL client is broken without readline                                   |
|:Launchpad bug: :bug:`1266386`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.24-72.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63130` - CMake-based check for the presence of a system readline library is not... |
|:Launchpad bug: :bug:`1266386`                                                                               |
|:Upstream state: Can't repeat (checked on 2017-12-08)                                                        |
|:Fix Released: :rn:`5.6.24-72.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53645` - SHOW GRANTS not displaying all the applicable grants                      |
|:Launchpad bug: :bug:`1354988`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.23-72.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69059` - GTIDs lack a reasonable deployment strategy                               |
|:Launchpad BP: `GTID deploy... <https://blueprints.launchpad.net/percona-server/+spec/gtid-deployment-step>`_|     
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.22-72.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75642` - Extend valid range of dummy certificates ni mysql-test/std_data           |
|:Launchpad bug: :bug:`1415843`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.22-72.0`                                                                             |
|:Upstream fix: 5.6.23                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73979` - wrong stack size calculation leads to stack overflow in pinbox allocator  |
|:Launchpad bug: :bug:`1351148`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.22-71.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74644` - A query on empty table with BLOBs may crash server                        |
|:Launchpad bug: :bug:`1384568`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.22-71.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`57430` - query optimizer does not pick covering index for some "order by" queries  |
|:Launchpad bug: :bug:`1394967`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.22-71.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74987` - mtr failure on Ubuntu Utopic, mysqlhotcopy fails with wrong error(255)    |
|:Launchpad bug: :bug:`1396330`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.22-71.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75189` - engines suite tests depending on InnoDB implementation details            |
|:Launchpad bug: :bug:`1401776`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.22-71.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72475` - Binlog events with binlog_format=MIXED are unconditionally logged in ROW..|
|:Launchpad bug: :bug:`1313901`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.21-70.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74842` - Incorrect attribute((nonnull)) for btr_cur_ins_lock_and_undo callees      |
|:Launchpad bug: :bug:`1390695`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.21-70.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74440` - mysql_install_db not handling mysqld startup failure                      |
|:Launchpad bug: :bug:`1382782`                                                                               |
|:Upstream state: Won't Fix                                                                                   |
|:Fix Released: :rn:`5.6.21-70.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73066` - Replication stall with multi-threaded replication                         |
|:Launchpad bug: :bug:`1331586`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.21-70.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71091` - CSV engine does not properly process ``""``, in quotes                    |
|:Launchpad bug: :bug:`1316042`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.21-70.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73736` - Missing testcase sync in rpl_err_ignoredtable                             |
|:Launchpad bug: :bug:`1361568`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.21-69.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70860` - --tc-heuristic-recover option values are broken                           |
|:Launchpad bug: :bug:`1334330`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.20-68.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73418` - Add --manual-lldb option to mysql-test-run.pl                             |
|:Launchpad bug: :bug:`1328482`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.20-68.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67806` - Multiple user level lock per connection                                   |
|:Launchpad bug: :ref:`multiple_user_level_locks`                                                             |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.19-67.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`1118` - Allow multiple concurrent locks with GET_LOCK()                            |
|:Launchpad BP: :ref:`multiple_user_level_locks`                                                              |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.19-67.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72615` - MTR --mysqld=--default-storage-engine=foo incompatible w/ dynamically...  |
|:Launchpad bug: :bug:`1318537`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.17-66.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72163` - Rev 5774 broke rpl_plugin_load                                            |
|:Launchpad bug: :bug:`1299688`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.17-65.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60782` - Audit plugin API: no MYSQL_AUDIT_GENERAL_LOG notifications with general...|
|:Launchpad bug: :bug:`1182535`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.17-65.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71250` - Bison 3 breaks mysql build                                                |
|:Launchpad bug: :bug:`1262439`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.17-65.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71374` - Slave IO thread won't attempt auto reconnect to the master/error-code 1159|
|:Launchpad bug: :bug:`1268729`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.16-64.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74637` - make dirty page flushing more adaptive                                    |
|:Launchpad BP: `Split LRU ...   <https://blueprints.launchpad.net/percona-server/+spec/lru-manager-thread>`_ |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71988` - page_cleaner: aggressive background flushing                              |
|:Launchpad bug: :bug:`1238039`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71624` - printf size_t results in a fatal warning in 32-bit debug builds           |
|:Launchpad bug: :bug:`1277505`                                                                               |
|:Upstream state: Can't repeat (checked on 2017-12-08)                                                        |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71094` - ssl.cmake related warnings                                                |
|:Launchpad bug: :bug:`1274411`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71089` - CMake warning when generating Makefile                                    |
|:Launchpad bug: :bug:`1274827`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: 5.6.18                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71708` - 70768 fix perf regression: high rate of RW lock creation and destruction  |
|:Launchpad bug: :bug:`1279671`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: 5.6.19                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54430` - innodb should retry partial reads/writes where errno was 0                |
|:Launchpad bug: :bug:`1262500`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70854` - Tc_log_page_size should be unflushable or server crashes if 2 XA SEs ...  |
|:Launchpad bug: :bug:`1255551`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`78050` - Crash on when XA functions activated by a storage engine                  |
|:Launchpad bug: :bug:`1255549`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`47134` - Crash on startup when XA support functions activated by a second engine   |
|:Launchpad bug: :bug:`1255549`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71270` - Failures to end bulk insert for partitioned tables handled incorrectly    |
|:Launchpad bug: :bug:`1204871`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.16-64.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71217` - Threadpool - add thd_wait_begin/thd_wait_end to the network IO functions  |
|:Launchpad bug: :bug:`1159743`                                                                               |
|:Upstream state: Open (checked on 2017-12-08)                                                                |
|:Fix Released: :rn:`5.6.15-63.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`41975` - Support for SSL options not included in mysqlbinlog                       |
|:Launchpad bug: :bug:`1258154`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.15-63.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71092` - InnoDB FTS introduced new mutex sync level in 5.6.15, broke UNIV_SYNC ... |
|:Launchpad bug: :bug:`1258154`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.15-63.0`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75595` - Compute InnoDB redo log block checksums faster                            |
|:Launchpad BP: `<https://blueprints.launchpad.net/percona-server/+spec/more-efficient-log-block-checksums>`_ |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.14-62.0`                                                                             |
|:Upstream fix: 5.6.25                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71411` - buf_flush_LRU() does not return correct number in case of compressed pages|
|:Launchpad bug: :bug:`1231918`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70417` - rw_lock_x_lock_func_nowait() calls os_thread_get_curr_id() mostly ...     |
|:Launchpad bug: :bug:`1230220`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: 5.6.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70490` - Suppression is too strict on some systems                                 |
|:Launchpad bug: :bug:`1205196`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64556` - Interrupting a query inside InnoDB causes an unrelated warning to be ...  |
|:Launchpad bug: :bug:`1115158`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: 5.6.14                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70500` - Page cleaner should perform LRU flushing regardless of server activity    |
|:Launchpad bug: :bug:`1234562`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60682` - deadlock from thd_security_context                                        |
|:Launchpad bug: :bug:`1115048`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70489` - Crash when using AES_ENCRYPT on empty string                              |
|:Launchpad bug: :bug:`1201033`                                                                               |
|:Upstream state: Unsupported                                                                                 |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68481` - InnoDB LRU flushing for MySQL 5.6 needs work                              |
|:Launchpad bug: :bug:`1232406`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70453` - Add hard timeouts to page cleaner flushes                                 |
|:Launchpad bug: :bug:`1232101`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69170` - buf_flush_LRU is lazy                                                     |
|:Launchpad bug: :bug:`1231918`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68555` - thread convoys from log_checkpoint_margin with innodb_buffer_pool_inst... |
|:Launchpad bug: :bug:`1236884`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75534` - Solve buffer pool mutex contention by splitting it                        |
|:Launchpad bug: :ref:`innodb_split_buf_pool_mutex`                                                           |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70228` - Is buf_LRU_free_page() really supposed to make non-zip block sticky at ...|
|:Launchpad bug: :bug:`1220544`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69617` - 5.6.12 removed UNIV_SYNC_DEBUG from UNIV_DEBUG                            |
|:Launchpad bug: :bug:`1216815`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: 5.6.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69258` - does buf_LRU_buf_pool_running_out need to lock buffer pool mutexes        |
|:Launchpad bug: :bug:`1219842`                                                                               |
|:Upstream state: Not a Bug                                                                                   |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70216` - Unnecessary overhead from persistent adaptive hash index latches          |
|:Launchpad bug: :bug:`1218347`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62018` - innodb adaptive hash index mutex contention                               |
|:Launchpad bug: :bug:`1216804`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54814` - make BUF_READ_AHEAD_AREA a constant                                       |
|:Launchpad bug: :bug:`1186974`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69179` - accessing information_schema.partitions causes plans to change            |
|:Launchpad bug: :bug:`1192354`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream fix: 5.6.14                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`42415` - UPDATE/DELETE with LIMIT clause unsafe for SBL even with ORDER BY PK ...  |
|:Launchpad bug: :bug:`1132194`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69639` - mysql failed to build with dtrace Sun D 1.11                              |
|:Launchpad bug: :bug:`1196460`                                                                               |
|:Upstream state: Open (checked on 2017-12-08)                                                                |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69524` - Some tests for table cache variables fail if open files limit is too low  |
|:Launchpad bug: :bug:`1182572`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.12-60.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68354` - Server crashes on update/join FEDERATED + local table when only 1 local...|
|:Launchpad bug: :bug:`1182572`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.12-60.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69856` - mysql_install_db does not function properly in 5.6 for debug builds       |
|:Launchpad bug: :bug:`1179359`                                                                               |
|:Upstream state: Won't Fix                                                                                   |
|:Fix Released: :rn:`5.6.12-60.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67879` - Slave deadlock caused by stop slave, show slave status and global read... |
|:Launchpad BP: :ref:`show_slave_status_nolock`                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.23                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79117` - "change_user" command should be aware of preceding "error" command        |
|:Launchpad bug: :bug:`1172090`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`56676` - 'show slave status' ,'show global status' hang when 'stop slave' takes... |
|:Launchpad BP: :ref:`show_slave_status_nolock`                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71603` - file name is not escaped in binlog for LOAD DATA INFILE statement         |
|:Launchpad bug: :bug:`1277351`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71183` - os_file_fsync() should handle fsync() returning EINTR                     |
|:Launchpad bug: :bug:`1262651`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63451` - atomic/x86-gcc.h:make_atomic_cas_body64 potential miscompilation bug      |
|:Launchpad bug: :bug:`878022`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70277` - last argument of LOAD DATA ... SET ... statement repeated twice in binlog |
|:Launchpad bug: :bug:`1223196`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69252` - All the parts.partition_max* tests are broken with MTR --parallel         |
|:Launchpad bug: :bug:`1180481`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69265` - -DBUILD_CONFIG=mysql_release -DWITH_DEBUG=ON fails 4 and skips 27 MTR ... |
|:Launchpad bug: :bug:`1163135`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68714` - Remove literal statement digest values from perfschema tests              |
|:Launchpad bug: :bug:`1157078`                                                                               |
|:Upstream state: Not a Bug                                                                                   |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68635` - Doc: Multiple issues with performance_schema_max_statement_classes        |
|:Launchpad bug: :bug:`1157075`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68800` - client doesn't read plugin-dir from my.cnf set by MYSQL_READ_DEFAULT_FILE |
|:Launchpad bug: :bug:`1155859`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69124` - Incorrect truncation of long SET expression in LOAD DATA can cause SQL ...|
|:Launchpad bug: :bug:`1175519`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68970` - fsp_reserve_free_extents switches from small to big tblspace handling ... |
|:Launchpad bug: :bug:`1169494`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68713` - create_duplicate_weedout_tmp_table() leaves key_part_flag uninitialized   |
|:Launchpad bug: :bug:`1157037`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68490` - slave_max_allowed_packet Not Honored on Slave IO Connect                  |
|:Launchpad bug: :bug:`1135097`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68999` - SSL_OP_NO_COMPRESSION not defined                                         |
|:Launchpad bug: :bug:`1183610`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.25                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68845` - Unnecessary log_sys->mutex reacquisition in mtr_log_reserve_and_write()   |
|:Launchpad bug: :bug:`1163439`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62578` - mysql client aborts connection on terminal resize                         |
|:Launchpad bug: :bug:`925343`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49169` - read_view_open_now is inefficient with many concurrent sessions           |
|:Launchpad bug: :bug:`1131187` and :bug:`1131189`                                                            |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63144` - CREATE TABLE IF NOT EXISTS metadata lock is too restrictive               |
|:Launchpad bug: :bug:`1127008`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.13                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68477` - Suboptimal code in skip_trailing_space()                                  |
|:Launchpad bug: :bug:`1132351`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68476` - Suboptimal code in my_strnxfrm_simple()                                   |
|:Launchpad bug: :bug:`1132350`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67504` - Duplicate error in replication with slave triggers and auto increment     |
|:Launchpad bug: :bug:`1068210`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67974` - Server crashes in add_identifier on concurrent ALTER TABLE and SHOW ENGINE|
|:Launchpad bug: :bug:`1017192`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68045` - security vulnerability CVE-2012-4414                                      |
|:Launchpad bug: :bug:`1049871`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66550` - security vulnerability CVE-2012-4414                                      |
|:Launchpad bug: :bug:`1049871`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67685` - security vulnerability CVE-2012-5611                                      |
|:Launchpad bug: :bug:`1083377`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66237` - Temporary files created by binary log cache are not purged after transa...|
|:Launchpad bug: :bug:`1070856`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54430` - innodb should retry partial reads/writes where errno was 0                |
|:Launchpad bug: :bug:`1079596`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62856` - Check for "stack overrun" doesn't work with gcc-4.6, server crashes       |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61180` - korr/store macros in my_global.h assume the argument to be a char pointer |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61178` - Incorrect implementation of intersect(ulonglong) in non-optimized Bitmap..|
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54127` - mysqld segfaults when built using --with-max-indexes=128                  |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64800` - mysqldump with --include-master-host-port putting quotes around port no.  | 
|:Launchpad bug: :bug:`1013432`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66301` - INSERT ... ON DUPLICATE KEY UPDATE + innodb_autoinc_lock_mode=1 is broken |
|:Launchpad bug: :bug:`1035225`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60743` - typo in cmake/dtrace.cmake                                                |
|:Launchpad bug: :bug:`1013455`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.13                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64663` - Segfault when adding indexes to InnoDB temporary tables                   |
|:Launchpad bug: :bug:`999147`                                                                                |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64432` - Bug :mysqlbug:`54330` (Broken fast index creation) was never fixed in 5.5 |
|:Launchpad bug: :bug:`939485`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`25007` - memory tables with dynamic rows format                                    |
|:Launchpad bug: :bug:`1148822`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61595` - mysql-test/include/wait_for_slave_param.inc timeout logic is incorrect    |
|:Launchpad bug: :bug:`800035`                                                                                |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54160` - InnoDB should retry on failed read or write, not immediately panic        |
|:Launchpad bug: :bug:`764395`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`39833` - CREATE INDEX does full table copy on TEMPORARY table                      |
|:Launchpad bug: N/A                                                                                          |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.10-60.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68116` - InnoDB monitor may hit an assertion error in buf_page_get_gen in debug ...|
|:Launchpad bug: :bug:`1100178`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.10-60.2`                                                                             |
|:Upstream fix: 5.6.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`65946` - Sid_map::Sid_map calls DBUG which may have unitialized THR_KEY_mysys and..|
|:Launchpad bug: :bug:`1050758`                                                                               |
|:Upstream state: Duplicate/Closed                                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream fix: 5.6.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`20001` - Support for temp-tables in INFORMATION_SCHEMA                             |
|:Launchpad bug: :ref:`temp_tables`                                                                           |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49120` - mysqldump should have flag to delay creating indexes for innodb plugin    |
|:Launchpad bug: :bug:`744103`                                                                                |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`57583` - fast index create not used during "alter table foo engine=innodb"         |
|:Launchpad bug: :bug:`744103`                                                                                |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69146` - Optimization in buf_pool_get_oldest_modification if srv_buf_pool_instances|
|:Launchpad bug: :bug:`1176496`                                                                               |
|:Upstream state: Verified (checked on 2017-12-08)                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
