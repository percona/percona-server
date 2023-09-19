--let $audit_filter_log_path = `SELECT @@global.datadir`
--let $audit_filter_log_name = `SELECT @@global.audit_log_filter.file`

CREATE TABLE t1 (c1 INT);

let $filter = {
  "filter": {
    "class": {
      "name": "table_access",
      "event": {"name": "insert", "log": true}
    }
  }
};

eval SELECT audit_log_filter_set_filter('log_table_insert', '$filter');
SELECT audit_log_filter_set_user('%', 'log_table_insert');

--source clean_all_audit_logs.inc

SET GLOBAL DEBUG='+d,audit_log_filter_debug_timestamp';

# Generate three rotated logs with some events and add some events into currently active log
INSERT INTO t1 VALUES (1);
SET @file1_start_bookmark := audit_log_read_bookmark();
INSERT INTO t1 VALUES (2);
INSERT INTO t1 VALUES (3);

--replace_column 1 <file_name>
SELECT audit_log_rotate();

INSERT INTO t1 VALUES (4);
SET @file2_start_bookmark := audit_log_read_bookmark();
INSERT INTO t1 VALUES (5);
INSERT INTO t1 VALUES (6);

--replace_column 1 <file_name>
SELECT audit_log_rotate();

INSERT INTO t1 VALUES (7);
SET @file3_start_bookmark := audit_log_read_bookmark();
INSERT INTO t1 VALUES (8);
INSERT INTO t1 VALUES (9);

--replace_column 1 <file_name>
SELECT audit_log_rotate();

INSERT INTO t1 VALUES (10);
SET @current_file_start_bookmark := audit_log_read_bookmark();
INSERT INTO t1 VALUES (11);
INSERT INTO t1 VALUES (12);

SELECT @file1_start_bookmark;
SELECT @file2_start_bookmark;
SELECT @file3_start_bookmark;
SELECT @current_file_start_bookmark;

--echo #
--echo # Read all from two rotated logs in one request
--replace_regex /"connection_id": \d*/"connection_id": <id>/
SELECT audit_log_read(@file2_start_bookmark);
SELECT audit_log_read('null');

--echo #
--echo # Read all from two rotated logs by chunks ("max_array_length": 2)
SET @file2_start_with_limit_bookmark := JSON_SET(@file2_start_bookmark, '$.max_array_length', 2);
SELECT @file2_start_with_limit_bookmark;
--replace_regex /"connection_id": \d*/"connection_id": <id>/
SELECT audit_log_read(@file2_start_with_limit_bookmark);
--replace_regex /"connection_id": \d*/"connection_id": <id>/
SELECT audit_log_read('{"max_array_length": 2}');
--replace_regex /"connection_id": \d*/"connection_id": <id>/
SELECT audit_log_read('{"max_array_length": 2}');
SELECT audit_log_read('null');

--echo #
--echo # Cleanup
DROP TABLE t1;

SET GLOBAL DEBUG='-d,audit_log_filter_debug_timestamp';
