SET @old_innodb_optimize_fulltext_only = @@innodb_optimize_fulltext_only;
SET GLOBAL innodb_optimize_fulltext_only = ON;

--eval CREATE TABLE t1 (c TINYTEXT, FULLTEXT KEY (c) $PARSER) ENGINE=InnoDB $CHARSET

INSERT INTO t1 VALUES ('vdf%vdfd%ghdi%opu');

--exec $MYSQL_SLAP --host=127.0.0.1 --user=root --port=$MASTER_MYPORT --create-schema=test --delimiter=";" --number-of-queries=1000 --concurrency=5 --query="OPTIMIZE TABLE t1; UPDATE t1 SET c = CONCAT('foo', RAND()) LIMIT 200;" > $MYSQL_TMP_DIR/mysql_slap_output.txt

DROP TABLE t1;
SET GLOBAL innodb_optimize_fulltext_only = @old_innodb_optimize_fulltext_only;
--remove_file $MYSQL_TMP_DIR/mysql_slap_output.txt
