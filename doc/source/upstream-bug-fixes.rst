.. _upstream_bug_fixes:

=============================================================
 List of upstream |MySQL| bugs fixed in |Percona Server| 5.5
=============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53588` - Blackhole : Specified key was too long; max key length is 1000 bytes      |
|:JIRA bug: :psbug:`1126`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.59-38.11`                                                                            |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83264` - uint3korr should stop reading four instead of three bytes on x86          |
|:JIRA bug: :psbug:`3567`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.52-38.3`                                                                             |
|:Upstream fix: 5.5.57                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68052` - SSL Certificate Subject ALT Names with IPs not respected with --ssl-ver...|
|:JIRA bug: :psbug:`1076`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.57-38.9`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83814` - Add support for OpenSSL 1.1                                               |
|:JIRA bug: :psbug:`1105`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.57-38.9`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`86016` - Make MTR show core dump stacktraces from unit tests too                   |
|:JIRA bug: :psbug:`2499`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.57-38.9`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`83073` - GCC 5 and 6 miscompile mach_parse_compressed                              |
|:JIRA bug: :psbug:`1745`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.52-38.2`                                                                             |
|:Upstream fix: 5.5.54                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`56155` - 'You cannot 'ALTER' a log table if logging is enabled' even if I log to...|
|:JIRA bug: :psbug:`595`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.52-38.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79249` - main.group_min_max fails under Valgrind                                   |
|:JIRA bug: :psbug:`1668`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.51-38.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`82019` - Is client library supposed to retry EINTR indefinitely or not             |
|:JIRA bug: :psbug:`1720`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.51-38.1`                                                                             |
|:Upstream fix: 5.5.52                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79610` - Failed DROP DATABASE due FK constraint on master breaks slave             |
|:JIRA bug: :psbug:`1683`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.51-38.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81657` - DBUG_PRINT in THD::decide_logging_format prints incorrectly, access ...   |
|:JIRA bug: :psbug:`2150`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81714` - mysqldump get_view_structure does not free MYSQL_RES in one error path    |
|:JIRA bug: :psbug:`2152`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`78223` - memory leak in mysqlbinlog                                                |
|:JIRA bug: :psbug:`3440`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81675` - mysqlbinlog does not free the existing connection before opening new...   |
|:JIRA bug: :psbug:`1718`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: 5.5.52                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61619` - ssl.cmake file is broken when using custom OpenSSL build                  |
|:JIRA bug: :psbug:`3437`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`80014` - mysql build fails, memory leak in gen_lex_hash, clang address sanitizer   |
|:JIRA bug: :psbug:`3433`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75239` - Support for TLSv1.1 and TLSv1.2                                           |
|:JIRA bug: :psbug:`926`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.50-38.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81324` - "rpl.rpl_start_stop_slave" fail sporadically on 5.5                       |
|:JIRA bug: :psbug:`2145`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.49-37.9`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`81295` - main.bigint/rpl.rpl_stm_user_variables fail on Ubuntu 15.10 Wily in ...   |
|:JIRA bug: :psbug:`3427`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.49-37.9`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79185` - Innodb freeze running REPLACE statements                                  |
|:JIRA bug: :psbug:`945`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.46-37.6`                                                                             |
|:Upstream fix: 5.5.49                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`77275` - Newest RHEL/CentOS openssl update breaks mysql DHE ciphers                |
|:JIRA bug: :psbug:`906`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.44-37.3`                                                                             |
|:Upstream fix: 5.5.45                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66779` - innochecksum does not work with compressed tables                         |
|:JIRA bug: :psbug:`1302`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.44-37.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75480` - Selecting wrong pos with SHOW BINLOG EVENTS causes a potentially ...      |
|:JIRA bug: :psbug:`1600`                                                                                     |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.44-37.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69991` - MySQL client is broken without readline                                   |
|:JIRA bug: :psbug:`1467`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.43-37.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63130` - CMake-based check for the presence of a system readline library is ...    |
|:JIRA bug: :psbug:`1467`                                                                                     |
|:Upstream state: Can't repeat (checked on 2018-05-24)                                                        |
|:Fix Released: :rn:`5.5.43-37.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75868` - main.error_simulation fails on Mac OS X since 5.5.42                      |
|:JIRA bug: :psbug:`3266`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.42-37.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75642` - Extend valid range of dummy certificates ni mysql-test/std_data           |
|:JIRA bug: :psbug:`1605`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.42-37.1`                                                                             |
|:Upstream fix: 5.5.42                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53645` - SHOW GRANTS not displaying all the applicable grants                      |
|:JIRA bug: :psbug:`191`                                                                                      |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.42-37.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74987` - mtr failure on Ubuntu Utopic, mysqlhotcopy fails with wrong error(255)    |
|:JIRA bug: :psbug:`2102`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73281` - openssl_1 tries to test a removed cipher on CentOS 7                      |
|:JIRA bug: :psbug:`3242`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`74440` - mysql_install_db not handling mysqld startup failure                      |
|:JIRA bug: :psbug:`1553`                                                                                     |
|:Upstream state: Won't fix                                                                                   |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`72475` - Binlog events with binlog_format=MIXED are unconditionally logged in ROW..|
|:JIRA bug: :psbug:`151`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75000` - 5.5 fails to compile with debug on Ubuntu Utopic                          |
|:JIRA bug: :psbug:`3236`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: 5.5.42                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73979` - wrong stack size calculation leads to stack overflow in pinbox allocator  |
|:JIRA bug: :psbug:`807`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.41-37.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73126` - Numerous Valgrind errors in OpenSSL                                       |
|:JIRA bug: :psbug:`3160`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.39-36.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73111` - Suppression typo causing spurious MTR Valgrind failures                   |
|:JIRA bug: :psbug:`3159`                                                                                     |
|:Upstream state: Open (checked on 2018-05-24)                                                                |
|:Fix Released: :rn:`5.5.39-36.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`73418` - Add --manual-lldb option to mysql-test-run.pl                             |
|:JIRA bug: :psbug:`2448`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.39-36.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60782` - Audit plugin API: no MYSQL_AUDIT_GENERAL_LOG notifications with general...|
|:JIRA bug: :psbug:`1369`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.37-35.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67352` - table_id is defined differently in sql/table.h vs sql/log_event.h         |
|:JIRA bug: :psbug:`142`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.37-35.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71250` - Bison 3 breaks mysql build                                                |
|:JIRA bug: :psbug:`376`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.37-35.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71375` - Slave IO thread won't attempt auto reconnect to the master/error-code 1593|
|:JIRA bug: :psbug:`3086`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.36-34.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71374` - Slave IO thread won't attempt auto reconnect to the master/error-code 1159|
|:JIRA bug: :psbug:`1470`                                                                                     |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.36-34.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71089` - CMake warning when generating Makefile                                    |
|:JIRA bug: :psbug:`2059`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.36-34.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54430` - innodb should retry partial reads/writes where errno was 0                |
|:JIRA bug: :psbug:`1460`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.36-34.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62311` - segfault in mysqld during early SIGHUP handling                           |
|:JIRA bug: :psbug:`3068`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.36-34.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`41975` - Support for SSL options not included in mysqlbinlog                       |
|:JIRA bug: :psbug:`1393`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.35-33.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69639` - mysql failed to build with dtrace Sun D 1.11                              |
|:JIRA bug: :psbug:`1392`                                                                                     |
|:Upstream state: Unsupported                                                                                 |
|:Fix Released: :rn:`5.5.33-31.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68354` - Server crashes on update/join FEDERATED + local table when only 1 local...|
|:JIRA bug: :psbug:`96`                                                                                       |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.32-31.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`42415` - UPDATE/DELETE with LIMIT clause unsafe for SBL even with ORDER BY PK ...  |
|:JIRA bug: :psbug:`44`                                                                                       |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.32-31.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69179` - accessing information_schema.partitions causes plans to change            |
|:JIRA bug: :psbug:`680`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.32-31.0`                                                                             |
|:Upstream fix: 5.5.34                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68970` - fsp_reserve_free_extents switches from small to big tblspace handling ... |
|:JIRA bug: :psbug:`656`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.32-31.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`79117` - "change_user" command should be aware of preceding "error" command        |
|:JIRA bug: :psbug:`659`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.31-30.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`65077` - internal temporary tables are contended on THR_LOCK_myisam                |
|:JIRA bug: :psbug:`1362`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.31-30.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68999` - SSL_OP_NO_COMPRESSION not defined                                         |
|:JIRA bug: :psbug:`362`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.31-30.3`                                                                             |
|:Upstream fix: 5.5.44                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68197` - InnoDB reports that it's going to wait for I/O but the I/O is async       |
|:JIRA bug: :psbug:`362`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.30-30.2`                                                                             |
|:Upstream fix: 5.5.31                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68845` - Unnecessary log_sys->mutex reacquisition in mtr_log_reserve_and_write()   |
|:JIRA bug: :psbug:`1347`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.30-30.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62578` - mysql client aborts connection on terminal resize                         |
|:JIRA bug: :psbug:`84`                                                                                       |
|:Upstream state: Won't Fix                                                                                   |
|:Fix Released: :rn:`5.5.30-30.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49169` - read_view_open_now is inefficient with many concurrent sessions           |
|:JIRA bug: :psbug:`636` and :psbug:`637`                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.30-30.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63144` - CREATE TABLE IF NOT EXISTS metadata lock is too restrictive               |
|:JIRA bug: :psbug:`40`                                                                                       |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.30-30.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68477` - Suboptimal code in skip_trailing_space()                                  |
|:JIRA bug: :psbug:`1321`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.30-30.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68476` - Suboptimal code in my_strnxfrm_simple()                                   |
|:JIRA bug: :psbug:`1320`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.30-30.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68116` - InnoDB monitor may hit an assertion error in buf_page_get_gen in debug ...|
|:JIRA bug: :psbug:`616`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67504` - Duplicate error in replication with slave triggers and auto increment     |
|:JIRA bug: :psbug:`34`                                                                                       |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67983` - Memory leak on filtered slave                                             |
|:JIRA bug: :psbug:`581`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: 5.5.31                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67974` - Server crashes in add_identifier on concurrent ALTER TABLE and SHOW ENGINE|
|:JIRA bug: :psbug:`344`                                                                                      |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68045` - security vulnerability CVE-2012-4414                                      |
|:JIRA bug: :psbug:`348`                                                                                      |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.29-29.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70277` - last argument of LOAD DATA ... SET ... statement repeated twice in binlog |
|:JIRA bug: :psbug:`3020`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: 5.5.35                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69380` - Incomplete fix for security vulnerability CVE-2012-5611                   |
|:JIRA bug: :psbug:`666`                                                                                      |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66550` - security vulnerability CVE-2012-4414                                      |
|:JIRA bug: :psbug:`348`                                                                                      |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67685` - security vulnerability CVE-2012-5611                                      |
|:JIRA bug: :psbug:`350`                                                                                      |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66237` - Temporary files created by binary log cache are not purged after transa...|
|:JIRA bug: :psbug:`599`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69124` - Incorrect truncation of long SET expression in LOAD DATA can cause SQL ...|
|:JIRA bug: :psbug:`663`                                                                                      |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71603` - file name is not escaped in binlog for LOAD DATA INFILE statement         |
|:JIRA bug: :psbug:`3092`                                                                                     |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67606` - MySQL crashes with segmentation fault when disk quota is reached          |
|:JIRA bug: :psbug:`1948`                                                                                     |
|:Upstream state: Duplicate                                                                                   |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67737` - mysqldump test sometimes fails due to mixing stdout and stderr            |
|:JIRA bug: :psbug:`547`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.28-29.2`                                                                             |
|:Upstream fix: 5.5.29                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66890` - Slave server crash after a START SLAVE                                    |
|:JIRA bug: :psbug:`587`                                                                                      |
|:Upstream state: Duplicate                                                                                   |
|:Fix Released: :rn:`5.5.28-29.1`                                                                             |
|:Upstream fix: 5.5.29                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62856` - Check for "stack overrun" doesn't work with gcc-4.6, server crashes       |
|:JIRA bug: :psbug:`2795`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.28-29.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61180` - korr/store macros in my_global.h assume the argument to be a char pointer |
|:JIRA bug: :psbug:`2795`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.27-29.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61178` - Incorrect implementation of intersect(ulonglong) in non-optimized Bitmap..|
|:JIRA bug: :psbug:`2795`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.27-29.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54127` - mysqld segfaults when built using --with-max-indexes=128                  |
|:JIRA bug: :psbug:`2795`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.27-29.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64800` - mysqldump with --include-master-host-port putting quotes around port no.  |
|:JIRA bug: :psbug:`1923`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.27-28.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66301` - INSERT ... ON DUPLICATE KEY UPDATE + innodb_autoinc_lock_mode=1 is broken |
|:JIRA bug: :psbug:`576`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.27-28.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60743` - typo in cmake/dtrace.cmake                                                |
|:JIRA bug: :psbug:`1924`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.25a-27.1`                                                                            |
|:Upstream fix: 5.5.33                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64663` - Segfault when adding indexes to InnoDB temporary tables                   |
|:JIRA bug: :psbug:`557`                                                                                      |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.24-26.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64624` - Mysql is crashing during replication                                      |
|:JIRA bug: :psbug:`535`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.24-26.0`                                                                             |
|:Upstream fix: 5.5.26                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64160` - page size 1024 but the only supported page size in this release is=16384  |
|:JIRA bug: :psbug:`549`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.21-25.1`                                                                             |
|:Upstream fix: 5.5.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64432` - Bug :mysqlbug:`54330` (Broken fast index creation) was never fixed in 5.5 |
|:JIRA bug: :psbug:`544`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.21-25.0`                                                                             |
|:Upstream fix: 5.5.30                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49336` - mysqlbinlog does not accept input from stdin when stdin is a pipe         |
|:JIRA bug: :psbug:`541`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.21-25.0`                                                                             |
|:Upstream fix: 5.5.28                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71183` - os_file_fsync() should handle fsync() returning EINTR                     |
|:JIRA bug: :psbug:`1461`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.20-24.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63451` - atomic/x86-gcc.h:make_atomic_cas_body64 potential miscompilation bug      |
|:JIRA bug: :psbug:`508`                                                                                      |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.18-23.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62557` - SHOW SLAVE STATUS gives wrong output with master-master and using SET...  |
|:JIRA bug: :psbug:`2692`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.17-22.1`                                                                             |
|:Upstream fix: 5.5.28                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`45702` - Impossible to specify myisam_sort_buffer > 4GB on 64 bit machines         |
|:JIRA bug: :psbug:`2700`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.17-22.1`                                                                             |
|:Upstream fix: 5.5.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62516` - Fast index creation does not update index statistics                      |
|:JIRA bug: :psbug:`2686`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.16-22.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`25007` - memory tables with dynamic rows format                                    |
|:JIRA bug: :psbug:`2407`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.15-21.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`51196` - Slave SQL: Got an error writing communication packets, Error_code: 1160   |
|:JIRA bug: :psbug:`2666`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.14-20.5`                                                                             |
|:Upstream fix: 5.5.21                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`43593` - dump/backup/restore/upgrade tools fails because of utf8_general_ci        |
|:JIRA bug: N/A                                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.14-20.5`                                                                             |
|:Upstream fix: 5.5.21                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61595` - mysql-test/include/wait_for_slave_param.inc timeout logic is incorrect    |
|:JIRA bug: :psbug:`485`                                                                                      |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.13-20.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`39833` - CREATE INDEX does full table copy on TEMPORARY table                      |
|:JIRA bug: N/A                                                                                               |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.11-20.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`57583` - fast index create not used during "alter table foo engine=innodb"         |
|:JIRA bug: :psbug:`2619`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.11-20.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49120` - mysqldump should have flag to delay creating indexes for innodb plugin... |
|:JIRA bug: :psbug:`2619`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.11-20.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54160` - InnoDB should retry on failed read or write, not immediately panic        |
|:JIRA bug: :psbug:`2628`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.11-20.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`51325` - Dropping an empty innodb table takes a long time with large buffer pool   |
|:JIRA bug: none                                                                                              |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.10-20.1`                                                                             |
|:Upstream fix: 5.5.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`56433` - Auto-extension of InnoDB files                                            |
|:JIRA bug: none                                                                                              |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.10-20.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`20001` - Support for temp-tables in INFORMATION_SCHEMA                             |
|:JIRA bug: none                                                                                              |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69146` - Optimization in buf_pool_get_oldest_modification if srv_buf_pool_instances|
|:JIRA bug: :psbug:`2418`                                                                                     |
|:Upstream state: Verified (checked on 2018-05-24)                                                            |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54814` - make BUF_READ_AHEAD_AREA a constant                                       |
|:JIRA bug: :psbug:`1148`                                                                                     |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`75534` - Solve buffer pool mutex contention by splitting it                        |
|:JIRA bug: :ref:`innodb_split_buf_pool_mutex`                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.8-20.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54790` - Use of non-blocking mode for sockets limits performance                   |
|:JIRA bug: :psbug:`1147`                                                                                     |
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

