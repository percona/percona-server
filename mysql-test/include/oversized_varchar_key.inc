eval CREATE TABLE t1 ( pk VARCHAR(16) NOT NULL PRIMARY KEY ) CHARSET=$charset COLLATE=$collation;

INSERT INTO t1 VALUES ('abc'), ('def'), ('ghi'), ('jkl'), ('mno');
ANALYZE TABLE t1;

EXPLAIN
SELECT * FROM t1 WHERE pk IN ('abc', 'this_value_is_>16', 'def');
SELECT * FROM t1 WHERE pk IN ('abc', 'this_value_is_>16', 'def');

EXPLAIN
SELECT * FROM t1 WHERE pk IN ('abc', 'this_value_is_>16', 'def');
SELECT * FROM t1 WHERE pk IN ('abc', 'this_value_is_>16', 'def');

EXPLAIN
SELECT * FROM t1 WHERE pk IN ('this_value_is_>16');
SELECT * FROM t1 WHERE pk IN ('this_value_is_>16');

EXPLAIN
SELECT * FROM t1 WHERE pk IN ('this_value_is_>16', 'and_so_is_this_one');
SELECT * FROM t1 WHERE pk IN ('this_value_is_>16', 'and_so_is_this_one');


eval CREATE TABLE t2 ( a VARCHAR(1), KEY (a) ) CHARSET=$charset COLLATE=$collation;

INSERT INTO t2 VALUES (NULL), ('a'), ('b'), ('æ'), ('ß'), ('s');
INSERT INTO t2 SELECT 'x' FROM t2;
INSERT INTO t2 SELECT 'x' FROM t2;

ANALYZE TABLE t2;

--echo # Strings containing soft hyphen (U+00AD) will not match for UTF-16

EXPLAIN
SELECT * FROM t2 WHERE a IN ('ae', concat(_utf8mb4 x'c2ad', 'b'), 'ss');
SELECT * FROM t2 WHERE a IN ('ae', concat(_utf8mb4 x'c2ad', 'b'), 'ss');

EXPLAIN
SELECT * FROM t2 WHERE a IN (concat(_utf8mb4 x'c2ad', 'ae'), 'ss');
SELECT * FROM t2 WHERE a IN (concat(_utf8mb4 x'c2ad', 'ae'), 'ss');


EXPLAIN
SELECT * FROM t2 WHERE a IN (concat(_utf8mb4 x'c2ad', 'ae', _utf8mb4 x'c2ad'), 'ss');
SELECT * FROM t2 WHERE a IN (concat(_utf8mb4 x'c2ad', 'ae', _utf8mb4 x'c2ad'), 'ss');

EXPLAIN
SELECT * FROM t2 WHERE a = 'ae' OR a = concat(_utf8mb4 x'c2ad', 'b') OR a = 'ss';
SELECT * FROM t2 WHERE a = 'ae' OR a = concat(_utf8mb4 x'c2ad', 'b') OR a = 'ss';

--echo # The plan is unstable in this case, however it always uses an index, and results are consistent.
--replace_column 5 X 10 X
EXPLAIN
SELECT * FROM t2 WHERE a IN ('ae', concat(repeat(_utf8mb4 x'c2ad', 2), 'b', repeat(_utf8mb4 x'c2ad', 2)));
--echo # Adding handler statistics to show that execution is also consistent.
FLUSH STATUS;
SELECT * FROM t2 WHERE a IN ('ae', concat(repeat(_utf8mb4 x'c2ad', 2), 'b', repeat(_utf8mb4 x'c2ad', 2)));
SHOW STATUS LIKE 'Handler_read_%';


eval CREATE TABLE t3 ( a VARCHAR(768), KEY(a)) CHARSET=$charset COLLATE=$collation;

INSERT INTO t3 VALUES( repeat('æ', 767) ), ( repeat('æ', 768) );

ANALYZE TABLE t3;

--replace_regex /'(.\uC3A6){767}'/'æ...'/
EXPLAIN
SELECT 1 FROM t3 WHERE a IN ( 'a', repeat('æ', 767) );
SELECT 1 FROM t3 WHERE a IN ( 'a', repeat('æ', 767) );

--replace_regex /'(ae){768}'/'ae..'/
EXPLAIN
SELECT 1 FROM t3 WHERE a IN ( 'a', repeat('ae', 768) );
SELECT 1 FROM t3 WHERE a IN ( 'a', repeat('ae', 768) );

--replace_regex /'(ae){769}'/'ae...'/
EXPLAIN
SELECT 1 FROM t3 WHERE a IN ( 'a', repeat('ae', 769) );
SELECT 1 FROM t3 WHERE a IN ( 'a', repeat('ae', 769) );


eval CREATE TABLE t4 ( a VARCHAR(100), KEY(a(50))) CHARSET=$charset COLLATE=$collation;

INSERT INTO t4 VALUES ( repeat('æ', 98) ), ( repeat('æ', 99) ), ( repeat('æ', 100) );
INSERT INTO t4 VALUES ( concat(repeat( 'a', 50 ), repeat( 'æ', 50 )) );

ANALYZE TABLE t4;

--replace_regex /'(.\uC3A6){98}'/'æ...'/
EXPLAIN
SELECT 1 FROM t4 WHERE a IN ( 'a', repeat('æ', 98) );
SELECT 1 FROM t4 WHERE a IN ( 'a', repeat('æ', 98) );


eval CREATE TABLE t5 ( a VARCHAR(16383), KEY(a(768))) CHARSET=$charset COLLATE=$collation;

INSERT INTO t5 VALUES( repeat('æ', 16383) ), ( repeat('æ', 16382) ), ( repeat('æ', 16381) );

ANALYZE TABLE t5;

--replace_regex /'(.\uC3A6){16381}'/'æ...'/
EXPLAIN
SELECT 1 FROM t5 WHERE a IN ( 'a', repeat('æ', 16381) );
SELECT 1 FROM t5 WHERE a IN ( 'a', repeat('æ', 16381) );

--replace_regex /'(.\uC3A6){16382}'/'æ...'/
EXPLAIN
SELECT 1 FROM t5 WHERE a IN ( 'a', repeat('æ', 16382) );
SELECT 1 FROM t5 WHERE a IN ( 'a', repeat('æ', 16382) );

--replace_regex /'(.\uC3A6){16383}'/'æ...'/
EXPLAIN
SELECT 1 FROM t5 WHERE a IN ( 'a', repeat('æ', 16383) );
SELECT 1 FROM t5 WHERE a IN ( 'a', repeat('æ', 16383) );


eval CREATE TABLE t6 ( a VARCHAR(6),  KEY(a(3)) ) CHARSET=$charset COLLATE=$collation;

INSERT INTO t6 VALUES ('aaaaaa'), ('æaaaaa'), ('ææaaaa'), ('æææaaa'), ('ææææaa'), ('æææææa'), ('ææææææ'),
                      ('😊aaaaa'), ('😊æaaaa'), ('😊😊aaaa'), ('😊😊æaaa'), ('😊😊😊aaa');

ANALYZE TABLE t6;

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', 'aaaaaa' );
SELECT * FROM t6 WHERE a IN ( '', 'aaaaaa' );

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', 'æaaaaa' );
SELECT * FROM t6 WHERE a IN ( '', 'æaaaaa' );

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', 'ææaaaa' );
SELECT * FROM t6 WHERE a IN ( '', 'ææaaaa' );

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', 'æææaaa' );
SELECT * FROM t6 WHERE a IN ( '', 'æææaaa' );

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', 'ææææaa' );
SELECT * FROM t6 WHERE a IN ( '', 'ææææaa' );

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', 'æææææa' );
SELECT * FROM t6 WHERE a IN ( '', 'æææææa' );

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', 'ææææææ' );
SELECT * FROM t6 WHERE a IN ( '', 'ææææææ' );

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', '😊aaaaa');
SELECT * FROM t6 WHERE a IN ( '', '😊aaaaa');

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', '😊æaaaa');
SELECT * FROM t6 WHERE a IN ( '', '😊æaaaa');

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', '😊😊aaaa');
SELECT * FROM t6 WHERE a IN ( '', '😊😊aaaa');

EXPLAIN
SELECT * FROM t6 WHERE a IN ( '', '😊😊æaaa');
SELECT * FROM t6 WHERE a IN ( '', '😊😊😊aaa');

EXPLAIN
SELECT * FROM t6 WHERE a IN ('aaaaaa', 'æaaaaa', 'ææaaaa', 'æææaaa', 'ææææaa', 'æææææa', 'ææææææ');
SELECT * FROM t6 WHERE a IN ('aaaaaa', 'æaaaaa', 'ææaaaa', 'æææaaa', 'ææææaa', 'æææææa', 'ææææææ');

EXPLAIN
SELECT * FROM t6 WHERE a IN
    ('aaaaaa', 'æaaaaa', 'ææaaaa', 'æææaaa', 'ææææaa', 'æææææa', 'ææææææ',
     '😊aaaaa', '😊æaaaa', '😊😊aaaa', '😊😊æaaa', '😊😊😊aaa');
SELECT * FROM t6 WHERE a IN
    ('aaaaaa', 'æaaaaa', 'ææaaaa', 'æææaaa', 'ææææaa', 'æææææa', 'ææææææ',
     '😊aaaaa', '😊æaaaa', '😊😊aaaa', '😊😊æaaa', '😊😊😊aaa');


DROP TABLE t1, t2, t3, t4, t5, t6;
