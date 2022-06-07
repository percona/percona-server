#
# SET column type
#

# $extra_col_opts is set by a calling test
# $extra_col_opts_ddl_err is set by a calling test
# $extra_col_opts_dml_err is set by a calling test

# Valid values.

if ($extra_col_opts_ddl_err == 0)
{
  eval CREATE TABLE t1 (
    a SET('') $extra_col_opts,
    b SET('test1','test2','test3','test4','test5') $extra_col_opts,
    c SET('01','02','03','04','05','06','07','08','09','10','11','12','13','14','15','16','17','18','19','20','21','22','23','24','25','26','27','28','29','30','31','32','33','34','35','36','37','38','39','40','41','42','43','44','45','46','47','48','49','50''51','52','53','54','55','56','57','58','59','60','61','62','63','64') $extra_col_opts,
    PRIMARY KEY (c)
  ) ENGINE=rocksdb;
}
if ($extra_col_opts_ddl_err == 1)
{
  # 5.7 uses strict mode by default, so this will now return an error
  --error ER_PRIMARY_CANT_HAVE_NULL
  eval CREATE TABLE t1 (
    a SET('') $extra_col_opts,
    b SET('test1','test2','test3','test4','test5') $extra_col_opts,
    c SET('01','02','03','04','05','06','07','08','09','10','11','12','13','14','15','16','17','18','19','20','21','22','23','24','25','26','27','28','29','30','31','32','33','34','35','36','37','38','39','40','41','42','43','44','45','46','47','48','49','50''51','52','53','54','55','56','57','58','59','60','61','62','63','64') $extra_col_opts,
    PRIMARY KEY (c)
  ) ENGINE=rocksdb;

  # 5.7 _requires_ a not NULL PK and can't mimick 5.6 behavior, so we are
  # adding in a fake key to satisfy and allow the test to resume
  eval CREATE TABLE t1 (
    a SET('') $extra_col_opts,
    b SET('test1','test2','test3','test4','test5') $extra_col_opts,
    c SET('01','02','03','04','05','06','07','08','09','10','11','12','13','14','15','16','17','18','19','20','21','22','23','24','25','26','27','28','29','30','31','32','33','34','35','36','37','38','39','40','41','42','43','44','45','46','47','48','49','50''51','52','53','54','55','56','57','58','59','60','61','62','63','64') $extra_col_opts,
    fake_key INT NOT NULL AUTO_INCREMENT,
    PRIMARY KEY (fake_key)
  ) ENGINE=rocksdb;
}
SHOW COLUMNS IN t1;

# 5.7 uses strict mode by default, so this will now return an error
--error 1265 # WARN_DATA_TRUNCATE not recognized by mtr
INSERT INTO t1 (a,b,c) VALUES
('','test2,test3','01,34,44,,23'),
('',5,2),
(',','test4,test2','');

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 (a,b,c) VALUES
('','test2,test3','01,34,44,,23'),
('',5,2),
(',','test4,test2','');
set session sql_mode=@orig_sql_mode;

--sorted_result
SELECT a,b,c FROM t1;

# Out of range values
# (should produce warnings)

# 5.7 uses strict mode by default, so this will now return an error
--error 1265 # WARN_DATA_TRUNCATE not recognized by mtr
INSERT INTO t1 (a,b,c) VALUES (0,'test6',-1);

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 (a,b,c) VALUES (0,'test6',-1);
set session sql_mode=@orig_sql_mode;

--sorted_result
SELECT a,b,c FROM t1;

# Non-unique values in set
# (should produce a warning)

# 5.7 uses strict mode by default, so this will now return an error
--error ER_DUPLICATED_VALUE_IN_TYPE
eval ALTER TABLE t1 ADD COLUMN e SET('a','A') $extra_col_opts;

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
eval ALTER TABLE t1 ADD COLUMN e SET('a','A') $extra_col_opts;
set session sql_mode=@orig_sql_mode;

SHOW COLUMNS IN t1;

# 5.7 uses strict mode by default, so this will now return an error
--error ER_DUPLICATED_VALUE_IN_TYPE
eval ALTER TABLE t1 ADD COLUMN f SET('1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',' ','11','12','13','14','15','16','17','18','19','1a','1b','1c','1d','1e','1f','1g','1h','1i','1j','1k','1l','1m','1n','1o','1p','1q','1r','1s','1t','1u','1v','1w','1x','1y','1z','20','21','22','23','24','25','26','27','28','29','2a','2b','2c','2d','2e','2f','2g','2h','2i','2j','2k','2l','2m','2n','2o','2p','2q','2r','2s','2t','2u','2v','2w','2x','2y','2z','30','31','32','33','34','35','36','37','38','39','3a','3b','3c','3d','3e','3f','3g','3h','3i') $extra_col_opts;

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
--error ER_TOO_BIG_SET
eval ALTER TABLE t1 ADD COLUMN f SET('1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',' ','11','12','13','14','15','16','17','18','19','1a','1b','1c','1d','1e','1f','1g','1h','1i','1j','1k','1l','1m','1n','1o','1p','1q','1r','1s','1t','1u','1v','1w','1x','1y','1z','20','21','22','23','24','25','26','27','28','29','2a','2b','2c','2d','2e','2f','2g','2h','2i','2j','2k','2l','2m','2n','2o','2p','2q','2r','2s','2t','2u','2v','2w','2x','2y','2z','30','31','32','33','34','35','36','37','38','39','3a','3b','3c','3d','3e','3f','3g','3h','3i') $extra_col_opts;
set session sql_mode=@orig_sql_mode;

# Simple comparison

--sorted_result
SELECT a,b,c,e FROM t1 WHERE FIND_IN_SET('test2',b)>0 OR a != '';

DROP TABLE t1;
