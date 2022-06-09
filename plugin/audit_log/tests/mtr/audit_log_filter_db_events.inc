CREATE TABLE db1.t (a INT);
CREATE TABLE db1.trig (a INT);
CREATE TABLE db2.t (a INT);
CREATE TABLE ```db3"`.t (a INT);
CREATE TABLE `some_very_long,database_na'me``some_very_long_database_n"ame____q`.t (a INT);

CREATE TABLE db1.words (id INT, word TEXT);

USE db1;

CREATE VIEW vmat AS SELECT SUM(a) AS s FROM db1.t;
CREATE VIEW vup AS SELECT * FROM db2.t;
CREATE VIEW vjoin AS SELECT * FROM vmat JOIN vup ON vmat.s=vup.a;

DELIMITER //;
CREATE EVENT ev1 ON SCHEDULE AT CURRENT_TIMESTAMP DO BEGIN INSERT INTO t VALUES (77); INSERT INTO t VALUES (7); END//
DELIMITER ;//
let $wait_condition=SELECT count(*)=1 FROM t WHERE a=7;
let $wait_timeout=600;
--source include/wait_condition.inc
CREATE TRIGGER ins_tring BEFORE INSERT ON db1.trig FOR EACH ROW INSERT INTO db2.t VALUES (new.a + 100);
INSERT INTO t VALUES (1), (2), (3);
DELIMITER //;
CREATE PROCEDURE p1()
BEGIN
  INSERT INTO db2.t VALUES (207);
END//
DELIMITER ;//
CALL p1();
DELIMITER //;
CREATE PROCEDURE p2(a INT)
BEGIN
  INSERT INTO db2.t VALUES (200 + a);
  IF a = 0 THEN
    CALL p2(a - 1);
  END IF;
END//
DELIMITER ;//
SET max_sp_recursion_depth = 20;
CALL p2(10);
INSERT INTO trig VALUES (1), (2), (3);
INSERT INTO words VALUES (0, 'one'), (2, 'two'), (3, 'three');

let $i=0;
while ($i < 20)
{
eval CREATE TABLE a$i (a INT);
inc $i;
}

let $i=1;
let $j=0;
while ($i < 20)
{
eval CREATE TRIGGER tr$i BEFORE INSERT ON a$i FOR EACH ROW INSERT INTO a$j VALUES (new.a);
inc $i;
inc $j;
}

CREATE TABLE b0 (a INT);
DELIMITER //;
CREATE TRIGGER tr_b_0 BEFORE INSERT ON b0 FOR EACH ROW BEGIN SET @tmp=1; SET @abc='cba'; END//
DELIMITER ;//

INSERT INTO a19 VALUES (1);

INSERT INTO b0 VALUES (1);

SELECT * FROM t LEFT JOIN words ON t.a = words.id LEFT JOIN t q ON t.a = q.a / 2;
DELETE t, q, words FROM t LEFT JOIN words ON t.a = words.id LEFT JOIN t q ON t.a = q.a / 2 WHERE t.a > 2;
DELETE t, q, words FROM t LEFT JOIN words ON t.a = words.id LEFT JOIN db2.t q ON t.a = q.a / 2 WHERE t.a > 2;

USE db2;
INSERT INTO t VALUES (1), (2), (3), (6);
INSERT INTO t SELECT * FROM db1.t;

USE ```db3"`;
INSERT INTO t VALUES (1), (2), (3);

USE `some_very_long,database_na'me``some_very_long_database_n"ame____q`;
INSERT INTO t VALUES (1), (2), (3);

use db1;
INSERT INTO vup VALUES (1);
SELECT * FROM vjoin;
SELECT * FROM vmat JOIN vup ON vmat.s=vup.a;
SELECT * FROM vmat;
SELECT a FROM vjoin;
SELECT s FROM vjoin;
SELECT a,s FROM vjoin;
SELECT s FROM vmat;
SELECT a FROM vup;
UPDATE vup SET a=a+1;
UPDATE vjoin SET a=a+1;

USE test;

SELECT * FROM db1.t WHERE a IN (SELECT a FROM `some_very_long,database_na'me``some_very_long_database_n"ame____q`.t);

SELECT 1;

DROP TABLE db1.t;
DROP TABLE db1.words;
DROP TABLE db1.trig;
DROP TABLE db2.t;
DROP TABLE ```db3"`.t;
DROP TABLE `some_very_long,database_na'me``some_very_long_database_n"ame____q`.t;
DROP PROCEDURE db1.p1;
DROP PROCEDURE db1.p2;
DROP VIEW db1.vmat;
DROP VIEW db1.vup;
DROP VIEW db1.vjoin;
let $i=0;
while ($i < 20)
{
eval DROP TABLE db1.a$i;
inc $i;
}
DROP TABLE db1.b0;
