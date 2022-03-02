--let $keyring_file=""
--let $error_message=""

--echo $validation_condition

if ($validation_condition == 'missing_key_1') {
--let $keyring_file=$MYSQLTEST_VARDIR/std_data/validation_keyrings/keyring_key_0
--let $error_message=Tablespace id . in file t1\.ibd is encrypted but keyring or used key_id 1 is not available\. Can't continue reading table\. Please provide the correct keyring\.
}
if ($validation_condition == 'missing_ver_1_of_key_1') {
--let $keyring_file=$MYSQLTEST_VARDIR/std_data/validation_keyrings/keyring_key_0_key_1_missing_v1_v2
--let $error_message=Tablespace id . in file t1\.ibd is encrypted\. Although server managed to find key_id=1 in keyring to decrypt it, some versions of this key, that are needed to decrypt this space, are missing\. Maybe you are using an old keyring\? Can\'t continue reading table\. Please provide the correct keyring\.
}
if ($validation_condition == 'missing_ver_2_of_key_1') {
--let $keyring_file=$MYSQLTEST_VARDIR/std_data/validation_keyrings/keyring_key_0_key_1_ver_1
--let $error_message=Tablespace id . in file t1\.ibd is encrypted\. Although server managed to find key_id=1 in keyring to decrypt it, some versions of this key, that are needed to decrypt this space, are missing\. Maybe you are using an old keyring\? Can't continue reading table. Please provide the correct keyring\.
}
if ($validation_condition == 'corrupted_ver_1_of_key_1') {
--let $keyring_file=$MYSQLTEST_VARDIR/std_data/validation_keyrings/keyring_key_0_key_1_v1_corrupted
--let $error_message=Tablespace id . in file t1\.ibd is encrypted\. Although server managed to find all needed versions of key_id=1 in keyring to decrypt it, the provided versions are either incorrect or corrupted\. Can't continue reading table\. Please provide the correct keyring\.
}
if ($validation_condition == 'corrupted_ver_2_of_key_1') {
--let $keyring_file=$MYSQLTEST_VARDIR/std_data/validation_keyrings/keyring_key_0_key_1_v1_v2_corrupted
--let $error_message=Tablespace id . in file t1\.ibd is encrypted\. Although server managed to find all needed versions of key_id=1 in keyring to decrypt it, the provided versions are either incorrect or corrupted\. Can't continue reading table\. Please provide the correct keyring\.
}

--echo # t1 is encrypted with ver1 and ver2 of key 1, now switch to keyring without key 1 -
--echo # it should not be possible to open t1
--replace_result $MYSQLTEST_VARDIR MYSQLTEST_VARDIR $KEYRING_PLUGIN KEYRING_PLUGIN
--let $restart_parameters=restart:--default-table-encryption=OFF --innodb-encryption-threads=0 --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --keyring-file-data=$keyring_file --log-error=$MYSQLD_LOG
--source include/start_mysqld.inc

--error ER_GET_ERRMSG
SELECT COUNT(*) from t1;

--source include/shutdown_mysqld.inc

--let ABORT_ON=NOT_FOUND
--let SEARCH_FILE=$MYSQLD_LOG
--let SEARCH_PATTERN=$error_message
--source include/search_pattern_in_file.inc

--remove_file $MYSQLD_LOG

