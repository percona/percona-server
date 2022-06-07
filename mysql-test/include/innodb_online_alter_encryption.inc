# Restart the server with keyring loaded
--let $restart_parameters="$keyring_restart_param"
--let $do_not_echo_parameters=1
--source include/restart_mysqld.inc

CREATE TABLE t1 (id INT NOT NULL PRIMARY KEY, a VARCHAR(255)) ENGINE=InnoDB ENCRYPTION='y';
CREATE TABLE t2 (id INT NOT NULL PRIMARY KEY, a VARCHAR(255)) ENGINE=InnoDB;
CREATE TABLE t3 (id INT, a VARCHAR(255)) ENGINE=InnoDB ENCRYPTION='y';
CREATE TABLE t4 (id INT, a VARCHAR(255)) ENGINE=InnoDB;
CREATE TABLE t5 (id INT NOT NULL PRIMARY KEY, a TEXT(500), b VARCHAR(255), FULLTEXT(b)) ENGINE=InnoDB ENCRYPTION='y';
CREATE TABLE t6 (id INT, a TEXT(500), b VARCHAR(255), FULLTEXT(b)) ENGINE=InnoDB;
CREATE TABLE t7 (id INT NOT NULL PRIMARY KEY, a VARCHAR(255)) ENGINE=InnoDB ROW_FORMAT=COMPRESSED ENCRYPTION='y';

DELIMITER //;
CREATE PROCEDURE innodb_insert_proc (repeat_count INT)
BEGIN
  DECLARE current_num INT;
  SET current_num = 0;
  WHILE current_num < repeat_count DO
    INSERT INTO t1 VALUES (current_num, REPEAT('foobar', 42));
    INSERT INTO t2 VALUES (current_num, REPEAT('temp', 42));
    INSERT INTO t3 VALUES (current_num, REPEAT('barfoo', 42));
    INSERT INTO t4 VALUES (current_num, REPEAT('repeat', 42));
    INSERT INTO t5 VALUES (current_num, SUBSTRING('A BC DEF GHIJ KLM NOPQRS TUV WXYZ 012 3456789', RAND() * 36 + 1, 100), REPEAT('author new', 22));
    INSERT INTO t6 VALUES (current_num, SUBSTRING('A BC DEF GHIJ KLM NOPQRS TUV WXYZ 012 3456789', RAND() * 36 + 1, 100), REPEAT('mangled old', 22));
    INSERT INTO t7 VALUES (current_num, REPEAT('mysql', 42));
    SET current_num = current_num + 1;
  END WHILE;
END//
DELIMITER ;//
COMMIT;

SET autocommit=0;
CALL innodb_insert_proc(15000);
COMMIT;
SET autocommit=1;

ALTER TABLE t1 ADD COLUMN b INT DEFAULT 2;
ALTER TABLE t2 ADD COLUMN b INT DEFAULT 2;
ALTER TABLE t7 ADD COLUMN b INT DEFAULT 2;
ALTER TABLE t1 ADD KEY a(a), ADD KEY b(b);
ALTER TABLE t2 ADD KEY a(a), ADD KEY b(b);
ALTER TABLE t3 ADD COLUMN c INT DEFAULT 5;
ALTER TABLE t4 ADD COLUMN c INT DEFAULT 5;
ALTER TABLE t3 ADD KEY (a), ADD KEY c(c);
ALTER TABLE t4 ADD KEY (a), ADD KEY c(c);
ALTER TABLE t5 ADD FULLTEXT(a);
ALTER TABLE t6 ADD FULLTEXT(a);
ALTER TABLE t7 ADD KEY a(a), ADD KEY b(b);

SHOW CREATE TABLE t1;
SHOW CREATE TABLE t2;
SHOW CREATE TABLE t3;
SHOW CREATE TABLE t4;
SHOW CREATE TABLE t5;
SHOW CREATE TABLE t6;
SHOW CREATE TABLE t7;

--echo # Restarting server
let $restart_parameters = restart: $KEYRING_PLUGIN_OPT $KEYRING_PLUGIN_LOAD --keyring_file_data=$MYSQL_TMP_DIR/mysecret_keyring
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN keyring_file.so
--source include/restart_mysqld.inc
--echo # Done restarting server

SELECT COUNT(1) FROM t1;
SELECT COUNT(1) FROM t2;
SELECT COUNT(1) FROM t3;
SELECT COUNT(1) FROM t4;
SELECT COUNT(1) FROM t5;
SELECT COUNT(1) FROM t6;
SELECT COUNT(1) FROM t7;

DROP PROCEDURE innodb_insert_proc;
DROP TABLE t1, t2, t3, t4, t5, t6, t7;
