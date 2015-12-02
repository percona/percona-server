# OPTIMIZE TABLE must not change table engine
# This statement will use "ALGORITHM=INPLACE" in both 5.6 and 5.7
eval OPTIMIZE TABLE $enforce_table;

--echo After OPTIMIZE TABLE statement
# Make sure table engine hasn't been changed
# Make sure column definition hasn't been changed
--let $enforce_engine= InnoDB
--let $enforce_column= varchar(1)
--source include/percona_enforce_storage_engine_alter_table_assert.inc

# Modify table definition (do not specify "ENGINE")
# This statement will use "ALGORITHM=COPY" in both 5.6 and 5.7
eval ALTER TABLE $enforce_table MODIFY a INT;

--echo After ALTER TABLE
# Make sure table engine hasn't been changed
# Make sure column definition has been changed
--let $enforce_column= int(11)
--source include/percona_enforce_storage_engine_alter_table_assert.inc

