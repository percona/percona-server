--echo #
--echo # PS-3851: Crash when SELECT using a fulltext search index with special character
--echo #

--eval CREATE TABLE tbtest (col1 int(11) NOT NULL AUTO_INCREMENT, col2 varchar(100) COLLATE utf8mb4_bin DEFAULT NULL, col3 text COLLATE utf8mb4_bin, col4 varchar(300) COLLATE utf8mb4_bin DEFAULT NULL, PRIMARY KEY (col1), FULLTEXT KEY FTX_01 (col2, col3, col4) $PARSER, FULLTEXT KEY FTX_02 (col4) $PARSER ) ENGINE=InnoDB AUTO_INCREMENT=100001 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin

INSERT INTO tbtest (col2, col3, col4)
SELECT concat(a.column_name,a.column_name) as c1,
       concat(a.column_name,a.column_name) as c2,
       concat(a.column_name,a.column_name) as c3
FROM information_schema.columns a, information_schema.columns b LIMIT 10000;

OPTIMIZE TABLE tbtest;

--echo # Should not crash
--disable_result_log
SELECT * FROM tbtest WHERE MATCH(col4) AGAINST('1some_inexistent_file_with_long_name\0.jpg');
--enable_result_log

DROP TABLE tbtest;
