# ==== Purpose ====
#
# Generate set of supported audit events.
#
# ==== Usage ====
#
# --source generate_audit_events.inc

--disable_result_log
--disable_query_log

# AuditRecordGeneral,
# AuditRecordTableAccess,
# AuditRecordCommand,
# AuditRecordQuery,
CREATE TABLE t1 (c1 INT);
CREATE TABLE t2 (c1 INT);
CREATE TABLE t3 (c1 INT);
INSERT INTO t1 VALUES (1);
INSERT INTO t2 VALUES (1);
INSERT INTO t3 VALUES (1);
INSERT INTO t1 VALUES (2);
UPDATE t1 SET c1 = 3 WHERE c1 = 2;
DELETE FROM t1 WHERE c1 = 3;

# AuditRecordStoredProgram
DELIMITER //;
CREATE PROCEDURE p1()
BEGIN
  SELECT * FROM t1;
END//
DELIMITER ;//
CALL p1();

DROP PROCEDURE p1;
DROP TABLE t1, t2, t3;

# AuditRecordGlobalVariable
SELECT @@global.audit_log_filter.file;

# AuditRecordMessage
INSTALL COMPONENT "file://component_audit_api_message_emit";
SELECT audit_api_message_emit_udf('component_text', 'producer_text', 'message_text', 'key', 'value') AS 'Message';
UNINSTALL COMPONENT "file://component_audit_api_message_emit";

# AuditRecordConnection
# AuditRecordAuthentication
CREATE USER 'user1'@'%' IDENTIFIED BY '111';
GRANT SUPER ON *.* to 'user1'@'%';
connect (conn1,localhost,user1,111,test);
disconnect conn1;
connection default;
DROP USER user1;

# AuditRecordServerShutdown
#--source include/restart_mysqld.inc

--enable_query_log
--enable_result_log
