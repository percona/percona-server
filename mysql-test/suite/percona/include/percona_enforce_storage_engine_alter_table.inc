# OPTIMIZE TABLE must not change table engine
# This statement will use "ALGORITHM=INPLACE" in 5.6, 5.7, and 8.0
eval OPTIMIZE TABLE $enforce_table;

--echo After OPTIMIZE TABLE statement
# Make sure table engine and column definition hasn't been changed
--let $enforce_engine= InnoDB
--let $enforce_column= varchar(1)
--source ../include/percona_enforce_storage_engine_alter_table_assert.inc


# ALTER TABLE with ALGORITHM=INSTANT must not change table engine
eval ALTER TABLE $enforce_table ADD COLUMN b ENUM('a', 'b'), ALGORITHM=INSTANT;

--echo After ALTER TABLE with ALGORITHM=INSTANT
# Make sure table engine and column definition hasn't been changed
--source ../include/percona_enforce_storage_engine_alter_table_assert.inc


# ALTER TABLE with ALGORITHM=INPLACE must not change table engine
eval ALTER TABLE $enforce_table DROP COLUMN b, ALGORITHM=INPLACE;

--echo After ALTER TABLE with ALGORITHM=INPLACE
# Make sure table engine and column definition hasn't been changed
--source ../include/percona_enforce_storage_engine_alter_table_assert.inc


# ALTER TABLE with ALGORITHM=COPY must not change table engine
eval ALTER TABLE $enforce_table MODIFY a INT, ALGORITHM=COPY;

--echo After ALTER TABLE with ALGORITHM=COPY
# Make sure table engine hasn't been changed
# Make sure column definition has been changed
--let $enforce_column= int
--source ../include/percona_enforce_storage_engine_alter_table_assert.inc
