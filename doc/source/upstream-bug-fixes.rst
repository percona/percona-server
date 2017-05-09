.. _upstream_bug_fixes:

=============================================================
 List of upstream |MySQL| bugs fixed in |Percona Server| 5.5
=============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`85838` - rpl_diff.inc in 5.7 does not compare data from different servers          |
|:Launchpad bug: :bug:`1680510`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.55-38.8`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83073` - GCC 5 and 6 miscompile mach_parse_compressed                              |
|:Launchpad bug: :bug:`1626002`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.52-38.2`                                                                             |
|:Upstream fix: 5.5.54                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`56155` - 'You cannot 'ALTER' a log table if logging is enabled' even if I log to...|
|:Launchpad bug: :bug:`1065841`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.52-38.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79249` - main.group_min_max fails under Valgrind                                   |
|:Launchpad bug: :bug:`1515591`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.51-38.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82019` - Is client library supposed to retry EINTR indefinitely or not             |
|:Launchpad bug: :bug:`1591202`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.51-38.1`                                                                             |
|:Upstream fix: 5.5.52                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79610` - Failed DROP DATABASE due FK constraint on master breaks slave             |
|:Launchpad bug: :bug:`1525407`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.51-38.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81657` - DBUG_PRINT in THD::decide_logging_format prints incorrectly, access ...   |
|:Launchpad bug: :bug:`1587426`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81714` - mysqldump get_view_structure does not free MYSQL_RES in one error path    |
|:Launchpad bug: :bug:`1588845`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`78223` - memory leak in mysqlbinlog                                                |
|:Launchpad bug: :bug:`1582761`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81675` - mysqlbinlog does not free the existing connection before opening new...   |
|:Launchpad bug: :bug:`1587840`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: 5.5.52                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61619` - ssl.cmake file is broken when using custom OpenSSL build                  |
|:Launchpad bug: :bug:`1582639`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80014` - mysql build fails, memory leak in gen_lex_hash, clang address sanitizer   |
|:Launchpad bug: :bug:`1580993`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75239` - Support for TLSv1.1 and TLSv1.2                                           |
|:Launchpad bug: :bug:`1501089`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81324` - "rpl.rpl_start_stop_slave" fail sporadically on 5.5                       |
|:Launchpad bug: :bug:`1578303`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.49-37.9`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81295` - main.bigint/rpl.rpl_stm_user_variables fail on Ubuntu 15.10 Wily in ...   |
|:Launchpad bug: :bug:`1578625`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.49-37.9`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79185` - Innodb freeze running REPLACE statements                                  |
|:Launchpad bug: :bug:`1519094`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.46-37.6`                                                                             |
|:Upstream fix: 5.5.49                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77275` - Newest RHEL/CentOS openssl update breaks mysql DHE ciphers                |
|:Launchpad bug: :bug:`1462856`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.44-37.3`                                                                             |
|:Upstream fix: 5.5.45                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66779` - innochecksum does not work with compressed tables                         |
|:Launchpad bug: :bug:`1100652`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.44-37.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75480` - Selecting wrong pos with SHOW BINLOG EVENTS causes a potentially ...      |
|:Launchpad bug: :bug:`1409652`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.44-37.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69991` - MySQL client is broken without readline                                   |
|:Launchpad bug: :bug:`1266386`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.43-37.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63130` - CMake-based check for the presence of a system readline library is ...    |
|:Launchpad bug: :bug:`1266386`                                                                               |
|:Upstream state: Can't repeat (checked on 2017-05-09)                                                        |
|:Fix Released: :rn:`5.5.43-37.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75868` - main.error_simulation fails on Mac OS X since 5.5.42                      |
|:Launchpad bug: :bug:`1424568`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.42-37.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75642` - Extend valid range of dummy certificates ni mysql-test/std_data           |
|:Launchpad bug: :bug:`1415843`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.42-37.1`                                                                             |
|:Upstream fix: 5.5.42                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53645` - SHOW GRANTS not displaying all the applicable grants                      |
|:Launchpad bug: :bug:`1354988`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.42-37.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74987` - mtr failure on Ubuntu Utopic, mysqlhotcopy fails with wrong error(255)    |
|:Launchpad bug: :bug:`1396330`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73281` - openssl_1 tries to test a removed cipher on CentOS 7                      |
|:Launchpad bug: :bug:`1401791`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74440` - mysql_install_db not handling mysqld startup failure                      |
|:Launchpad bug: :bug:`1382782`                                                                               |
|:Upstream state: Won't Fix                                                                                   |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72475` - Binlog events with binlog_format=MIXED are unconditionally logged in ROW..|
|:Launchpad bug: :bug:`1313901`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75000` - 5.5 fails to compile with debug on Ubuntu Utopic                          |
|:Launchpad bug: :bug:`1396358`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: 5.5.42                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73979` - wrong stack size calculation leads to stack overflow in pinbox allocator  |
|:Launchpad bug: :bug:`1351148`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73126` - Numerous Valgrind errors in OpenSSL                                       |
|:Launchpad bug: :bug:`1334743`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.39-36.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73111` - Suppression typo causing spurious MTR Valgrind failures                   |
|:Launchpad bug: :bug:`1334317`                                                                               |
|:Upstream state: Open (checked on 2017-05-09)                                                                |
|:Fix Released: :rn:`5.5.39-36.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73418` - Add --manual-lldb option to mysql-test-run.pl                             |
|:Launchpad bug: :bug:`1328482`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.39-36.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60782` - Audit plugin API: no MYSQL_AUDIT_GENERAL_LOG notifications with general...|
|:Launchpad bug: :bug:`1182535`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.37-35.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67352` - table_id is defined differently in sql/table.h vs sql/log_event.h         |
|:Launchpad bug: :bug:`1070255`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.37-35.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71250` - Bison 3 breaks mysql build                                                |
|:Launchpad bug: :bug:`1262439`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.37-35.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71375` - Slave IO thread won't attempt auto reconnect to the master/error-code 1593|
|:Launchpad bug: :bug:`1268735`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.36-34.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71374` - Slave IO thread won't attempt auto reconnect to the master/error-code 1159|
|:Launchpad bug: :bug:`1268729`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.36-34.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71089` - CMake warning when generating Makefile                                    |
|:Launchpad bug: :bug:`1274827`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.36-34.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54430` - innodb should retry partial reads/writes where errno was 0                |
|:Launchpad bug: :bug:`1262500`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.36-34.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62311` - segfault in mysqld during early SIGHUP handling                           |
|:Launchpad bug: :bug:`1249193`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.36-34.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`41975` - Support for SSL options not included in mysqlbinlog                       |
|:Launchpad bug: :bug:`1197524`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.35-33.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69639` - mysql failed to build with dtrace Sun D 1.11                              |
|:Launchpad bug: :bug:`1196460`                                                                               |
|:Upstream state: Open (checked on 2017-05-09)                                                                |
|:Fix Released: :rn:`5.5.33-31.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68354` - Server crashes on update/join FEDERATED + local table when only 1 local...|
|:Launchpad bug: :bug:`1182572`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.32-31.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`42415` - UPDATE/DELETE with LIMIT clause unsafe for SBL even with ORDER BY PK ...  |
|:Launchpad bug: :bug:`1132194`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.32-31.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69179` - accessing information_schema.partitions causes plans to change            |
|:Launchpad bug: :bug:`1192354`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.32-31.0`                                                                             |
|:Upstream fix: 5.5.34                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68970` - fsp_reserve_free_extents switches from small to big tblspace handling ... |
|:Launchpad bug: :bug:`1169494`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.32-31.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79117` - "change_user" command should be aware of preceding "error" command        |
|:Launchpad bug: :bug:`1172090`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.31-30.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`65077` - internal temporary tables are contended on THR_LOCK_myisam                |
|:Launchpad bug: :bug:`1179978`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.31-30.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68999` - SSL_OP_NO_COMPRESSION not defined                                         |
|:Launchpad bug: :bug:`1183610`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.31-30.3`                                                                             |
|:Upstream fix: 5.5.44                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68197` - InnoDB reports that it's going to wait for I/O but the I/O is async       |
|:Launchpad bug: :bug:`1107539`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.30-30.2`                                                                             |
|:Upstream fix: 5.5.31                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68845` - Unnecessary log_sys->mutex reacquisition in mtr_log_reserve_and_write()   |
|:Launchpad bug: :bug:`1163439`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.30-30.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62578` - mysql client aborts connection on terminal resize                         |
|:Launchpad bug: :bug:`925343`                                                                                |
|:Upstream state: Won't Fix                                                                                   |
|:Fix Released: :rn:`5.5.30-30.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49169` - read_view_open_now is inefficient with many concurrent sessions           |
|:Launchpad bug: :bug:`1131187` and :bug:`1131189`                                                            |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.30-30.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63144` - CREATE TABLE IF NOT EXISTS metadata lock is too restrictive               |
|:Launchpad bug: :bug:`1127008`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.30-30.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68477` - Suboptimal code in skip_trailing_space()                                  |
|:Launchpad bug: :bug:`1132351`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.30-30.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68476` - Suboptimal code in my_strnxfrm_simple()                                   |
|:Launchpad bug: :bug:`1132350`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.30-30.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68116` - InnoDB monitor may hit an assertion error in buf_page_get_gen in debug ...|
|:Launchpad bug: :bug:`1100178`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67504` - Duplicate error in replication with slave triggers and auto increment     |
|:Launchpad bug: :bug:`1068210`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67983` - Memory leak on filtered slave                                             |
|:Launchpad bug: :bug:`1042946`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: 5.5.31                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67974` - Server crashes in add_identifier on concurrent ALTER TABLE and SHOW ENGINE|
|:Launchpad bug: :bug:`1017192`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68045` - security vulnerability CVE-2012-4414                                      |
|:Launchpad bug: :bug:`1049871`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.29-29.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70277` - last argument of LOAD DATA ... SET ... statement repeated twice in binlog |
|:Launchpad bug: :bug:`1223196`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: 5.5.35                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69380` - Incomplete fix for security vulnerability CVE-2012-5611                   |
|:Launchpad bug: :bug:`1186748`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66550` - security vulnerability CVE-2012-4414                                      |
|:Launchpad bug: :bug:`1049871`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67685` - security vulnerability CVE-2012-5611                                      |
|:Launchpad bug: :bug:`1083377`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66237` - Temporary files created by binary log cache are not purged after transa...|
|:Launchpad bug: :bug:`1070856`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69124` - Incorrect truncation of long SET expression in LOAD DATA can cause SQL ...|
|:Launchpad bug: :bug:`1175519`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71603` - file name is not escaped in binlog for LOAD DATA INFILE statement         |
|:Launchpad bug: :bug:`1277351`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67606` - MySQL crashes with segmentation fault when disk quota is reached          |
|:Launchpad bug: :bug:`1079596`                                                                               |
|:Upstream state: Duplicate                                                                                   |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67737` - mysqldump test sometimes fails due to mixing stdout and stderr            |
|:Launchpad bug: :bug:`959198`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.28-29.2`                                                                             |
|:Upstream fix: 5.5.29                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66890` - Slave server crash after a START SLAVE                                    |
|:Launchpad bug: :bug:`1053342`                                                                               |
|:Upstream state: Duplicate                                                                                   |
|:Fix Released: :rn:`5.5.28-29.1`                                                                             |
|:Upstream fix: 5.5.29                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62856` - Check for "stack overrun" doesn't work with gcc-4.6, server crashes       |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.28-29.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61180` - korr/store macros in my_global.h assume the argument to be a char pointer |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.27-29.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61178` - Incorrect implementation of intersect(ulonglong) in non-optimized Bitmap..|
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.27-29.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54127` - mysqld segfaults when built using --with-max-indexes=128                  |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.27-29.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64800` - mysqldump with --include-master-host-port putting quotes around port no.  | 
|:Launchpad bug: :bug:`1013432`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.27-28.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66301` - INSERT ... ON DUPLICATE KEY UPDATE + innodb_autoinc_lock_mode=1 is broken |
|:Launchpad bug: :bug:`1035225`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.27-28.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60743` - typo in cmake/dtrace.cmake                                                |
|:Launchpad bug: :bug:`1013455`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.25a-27.1`                                                                            |
|:Upstream fix: 5.5.33                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64663` - Segfault when adding indexes to InnoDB temporary tables                   |
|:Launchpad bug: :bug:`999147`                                                                                |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.24-26.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64624` - Mysql is crashing during replication                                      |
|:Launchpad bug: :bug:`915814`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.24-26.0`                                                                             |
|:Upstream fix: 5.5.26                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64160` - page size 1024 but the only supported page size in this release is=16384  |
|:Launchpad bug: :bug:`966844`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.21-25.1`                                                                             |
|:Upstream fix: 5.5.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64432` - Bug :mysqlbug:`54330` (Broken fast index creation) was never fixed in 5.5 |
|:Launchpad bug: :bug:`939485`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.21-25.0`                                                                             |
|:Upstream fix: 5.5.30                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49336` - mysqlbinlog does not accept input from stdin when stdin is a pipe         |
|:Launchpad bug: :bug:`933969`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.21-25.0`                                                                             |
|:Upstream fix: 5.5.28                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71183` - os_file_fsync() should handle fsync() returning EINTR                     |
|:Launchpad bug: :bug:`1262651`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.20-24.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63451` - atomic/x86-gcc.h:make_atomic_cas_body64 potential miscompilation bug      |
|:Launchpad bug: :bug:`878022`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.18-23.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62557` - SHOW SLAVE STATUS gives wrong output with master-master and using SET...  |
|:Launchpad bug: :bug:`860910`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.17-22.1`                                                                             |
|:Upstream fix: 5.5.28                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`45702` - Impossible to specify myisam_sort_buffer > 4GB on 64 bit machines         |
|:Launchpad bug: :bug:`878404`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.17-22.1`                                                                             |
|:Upstream fix: 5.5.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62516` - Fast index creation does not update index statistics                      |
|:Launchpad bug: :bug:`857590`                                                                                |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.16-22.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`25007` - memory tables with dynamic rows format                                    |
|:Launchpad bug: :bug:`1148822`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.15-21.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`51196` - Slave SQL: Got an error writing communication packets, Error_code: 1160   |
|:Launchpad bug: :bug:`813587`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.14-20.5`                                                                             |
|:Upstream fix: 5.5.21                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`43593` - dump/backup/restore/upgrade tools fails because of utf8_general_ci        |
|:Launchpad bug: N/A                                                                                          |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.14-20.5`                                                                             |
|:Upstream fix: 5.5.21                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61595` - mysql-test/include/wait_for_slave_param.inc timeout logic is incorrect    |
|:Launchpad bug: :bug:`800035`                                                                                |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.13-20.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`39833` - CREATE INDEX does full table copy on TEMPORARY table                      |
|:Launchpad bug: N/A                                                                                          |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.11-20.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`57583` - fast index create not used during "alter table foo engine=innodb"         |
|:Launchpad bug: :bug:`744103`                                                                                |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.11-20.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49120` - mysqldump should have flag to delay creating indexes for innodb plugin... |
|:Launchpad bug: :bug:`744103`                                                                                |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.11-20.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54160` - InnoDB should retry on failed read or write, not immediately panic        |
|:Launchpad bug: :bug:`764395`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.11-20.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`51325` - Dropping an empty innodb table takes a long time with large buffer pool   |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.10-20.1`                                                                             |
|:Upstream fix: 5.5.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`56433` - Auto-extension of InnoDB files                                            |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.10-20.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`20001` - Support for temp-tables in INFORMATION_SCHEMA                             |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69146` - Optimization in buf_pool_get_oldest_modification if srv_buf_pool_instances|
|:Launchpad bug: :bug:`1176496`                                                                               |
|:Upstream state: Verified (checked on 2017-05-09)                                                            |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54814` - make BUF_READ_AHEAD_AREA a constant                                       |
|:Launchpad bug: :bug:`606811`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75534` - Solve buffer pool mutex contention by splitting it                        |
|:Launchpad bug: :ref:`innodb_split_buf_pool_mutex`                                                           |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54790` - Use of non-blocking mode for sockets limits performance                   |
|:Launchpad bug: :bug:`606810`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67879` - Slave deadlock caused by stop slave, show slave status and global read... |
|:Launchpad BP: :ref:`show_slave_status_nolock`                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`56676` - 'show slave status' ,'show global status' hang when 'stop slave' takes... |
|:Launchpad BP: :ref:`show_slave_status_nolock`                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+

