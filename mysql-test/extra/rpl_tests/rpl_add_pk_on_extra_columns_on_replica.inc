# This is an auxiliary file called by
# `rpl_add_pk_on_extra_columns_on_replica.test` to test the scenarios where a
# table on replica server has extra columns in addition to the source's table.
#
# Parameters:
# $_CREATE_TABLE_QUERY: CREATE TABLE query to be executed on source server.
# $_DATA_INSERT_QUERY: INSERT query to be executed on source server.
# $_ALTER_TABLE_QUERY: ALTER TABLE query to be executed on replica server.
# $_TEST_QUERY: Query to be executed on source server whose effects need to be checked on replica server.
# $_DISPLAY_QUERY: Query to be executed to display the contents after validation.
# $_CLEANUP_ITER_QUERY: Cleanup query to be executed before every iteration.
# $_CLEANUP_QUERY: Cleanup query to be executed in the end.
# $table_name: Table name for which consistency check to be performed.
# $columns_to_be_masked: List of columns that needs to be masked for checking the consistency.
# $expected_row_count: Expected number of rows for consistency check.
#
# How this test works?
# --------------------
#
# 1. Execute $_CREATE_TABLE_QUERY on source server.
# 2. Sync the replica server with source server.
# 3. Execute ALTER TABLE on replica server.
# 4. Replicate and validate the data
#    4.1. Execute $_DATA_INSERT_QUERY on source server.
#    4.2. Sync the replica server with source server.
#    4.3. Execute $_TEST_QUERY on source server.
#    4.4. Sync replica server with source server.
#    4.5. Assert that replica table has $expected_row_count rows.
#    4.6. Verify that both source and replica tables have same rows except the
#         rows listed in $columns_to_be_masked.
#    4.7. Execute $_CLEANUP_ITER_QUERY for cleanup before next iteration.
# 5. Execute $_CLEANUP_QUERY for cleanup.
#
# References:
# PS-7578: Replication failure with UPDATE when replica server has a PK and
#          source not

--echo
--echo ##############################################
--echo # Testing with:
--echo # CREATE_QUERY: $_CREATE_TABLE_QUERY
--echo # INSERT_QUERY: $_DATA_INSERT_QUERY
--echo # ALTER_QUERY: $_ALTER_TABLE_QUERY
--echo # columns to be mased: $columns_to_be_masked
--echo ##############################################

--echo #
--echo # 1. Execute $_CREATE_TABLE_QUERY on source server.
--echo #
--source include/rpl_connection_master.inc
--eval $_CREATE_TABLE_QUERY

--echo #
--echo # 2. Sync the replica server with source server.
--echo #
--source include/sync_slave_sql_with_master.inc

--echo #
--echo # 3. Execute ALTER TABLE on replica server.
--echo #
--eval $_ALTER_TABLE_QUERY

--echo #
--echo # 4. Replicate and validate the data.
--echo #

# Execute $_DATA_INSERT_QUERY on source server.
--source include/rpl_connection_master.inc
--eval $_DATA_INSERT_QUERY

# Sync replica server with source server.
--source include/sync_slave_sql_with_master.inc

# Execute $_TEST_QUERY on source server.
--source include/rpl_connection_master.inc
--eval $_TEST_QUERY

# Sync replica server with source server.
--source include/sync_slave_sql_with_master.inc

# Assert that replica table has $expected_row_count rows.
--let $assert_text= There are exactly $expected_row_count rows on replica server
--let $assert_cond= COUNT(*) = $expected_row_count FROM $table_name
--source include/assert.inc

# Verify that both source and replica tables have same rows except the rows
# listed in $columns_to_be_masked.
--let $diff_tables= master:$table_name, slave:$table_name
--let $mask_column_list= $columns_to_be_masked
--source include/diff_tables.inc

# Execute $_DISPLAY_QUERY query to display the contents after validation.
--source include/rpl_connection_master.inc
--eval $_DISPLAY_QUERY
--source include/rpl_connection_slave.inc
--eval $_DISPLAY_QUERY

# Execute $_CLEANUP_ITER_QUERY before next iteration.
--source include/rpl_connection_master.inc
--eval $_CLEANUP_ITER_QUERY
--source include/sync_slave_sql_with_master.inc

--echo #
--echo # 5. Cleanup
--echo #
--source include/rpl_connection_master.inc
--eval $_CLEANUP_QUERY
