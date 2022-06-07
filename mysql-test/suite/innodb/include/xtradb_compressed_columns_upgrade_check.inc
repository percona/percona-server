--let $label = [$table_name]
--let $partition_spec =
if($partition_name != '')
{
  --let $label = $label[$partition_name]
  --let $partition_spec = PARTITION ($partition_name) 
}

--disable_query_log
eval SET @a_short = '$a_short';
eval SET @b_short = '$b_short';
eval SET @a_long = '$a_long';
eval SET @b_long = '$b_long';
--enable_query_log

--echo $label checking data consistency just after upgrade
--let $assert_text= $label initial number of records must be 2
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec] = 2
--source include/assert.inc
--let $assert_text= $label initial short record must not be corrupted
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 0 + $part_shift + $subpart_shift AND a = @a_short AND b = @b_short] = 1
--source include/assert.inc
--let $assert_text= $label initial long record must not be corrupted
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 2 + $part_shift + $subpart_shift AND a = @a_long AND b = @b_long] = 1
--source include/assert.inc

--echo $label inserting a new record
eval INSERT INTO $table_name VALUES (4 + $part_shift + $subpart_shift, @a_short, @b_long);
--let $assert_text= $label number of records after insert must be 3
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec] = 3
--source include/assert.inc
--let $assert_text= $label short record after insert must not be corrupted
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 0 + $part_shift + $subpart_shift AND a = @a_short AND b = @b_short] = 1
--source include/assert.inc
--let $assert_text= $label long record after insert must not be corrupted
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 2 + $part_shift + $subpart_shift AND a = @a_long AND b = @b_long] = 1
--source include/assert.inc
--let $assert_text= $label inserted record must not be corrupted
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 4 + $part_shift + $subpart_shift AND a = @a_short AND b = @b_long] = 1
--source include/assert.inc

--echo $label updating existing record
eval UPDATE $table_name SET a = @a_long, b = @b_short WHERE id = 4 + $part_shift + $subpart_shift;
--let $assert_text= $label number of records after update must be 3
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec] = 3
--source include/assert.inc
--let $assert_text= $label short record after update must not be corrupted
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 0 + $part_shift + $subpart_shift AND a = @a_short AND b = @b_short] = 1
--source include/assert.inc
--let $assert_text= $label long record after update must not be corrupted
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 2 + $part_shift + $subpart_shift AND a = @a_long AND b = @b_long] = 1
--source include/assert.inc
--let $assert_text= $label updated record must not be corrupted
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 4 + $part_shift + $subpart_shift AND a = @a_long AND b = @b_short] = 1
--source include/assert.inc

--echo $label deleting existing record
eval DELETE FROM $table_name WHERE id = 4 + $part_shift + $subpart_shift;
--let $assert_text= $label number of records after delete must be 2
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec] = 2
--source include/assert.inc
--let $assert_text= $label short record after delete must not be corrupted
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 0 + $part_shift + $subpart_shift AND a = @a_short AND b = @b_short] = 1
--source include/assert.inc
--let $assert_text= $label long record after delete must not be corrupted
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 2 + $part_shift + $subpart_shift AND a = @a_long AND b = @b_long] = 1
--source include/assert.inc
--let $assert_text= $label deleted record must not exist
--let $assert_cond= [SELECT COUNT(*) FROM $table_name $partition_spec WHERE id = 4] = 0
--source include/assert.inc
