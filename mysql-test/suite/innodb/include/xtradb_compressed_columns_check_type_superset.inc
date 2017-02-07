# set $file_format to either 'Barracuda' or 'Antelope'
# ('Barracuda' is required to use DYNAMIC and COMPRESSED row formats)
# set $compressed_attribute to either 'COLUMN_FORMAT COMPRESSED' or an empty string
# set $row_format to one of the following values:
# - DEFAULT (equivalent to COMPACT)
# - DYNAMIC (long fields stored externally)
# - FIXED (not supported, will be silently transformed into COMPACT)
# - COMPRESSED (row level compression)
# - REDUNDANT (old format)
# - COMPACT (default, primary format)

# suppressing warnings caused by changing 'innodb-log-file-size'
call mtr.add_suppression("InnoDB: Resizing redo log");
call mtr.add_suppression("InnoDB: Starting to delete and rewrite log files");
call mtr.add_suppression("InnoDB: New log files created");
call mtr.add_suppression("InnoDB: The log sequence numbers [0-9]+ and [0-9]+ in ibdata files do not match the log sequence number [0-9]+ in the ib_logfiles");

# restarting server with bigger 'innodb-log-file-size' and 'max-allowed-packet'
# required for inserting long (~16MB) strings
--let $restart_parameters= restart:--innodb-log-file-size=256M --max-allowed-packet=48M
--source include/restart_mysqld.inc

if($row_format == 'COMPRESSED')
{
  --let $check_buffer_pool_evict = `SELECT version() LIKE '%debug%'`
}

# saving global variables
SET @saved_innodb_file_format = @@global.innodb_file_format;
SET @saved_innodb_compressed_columns_zip_level = @@global.innodb_compressed_columns_zip_level;

--disable_warnings
eval SET GLOBAL innodb_file_format = $file_format;
--enable_warnings

# setting zip level to 0 means skipping compressing for all values
# (compressed column header will still be added)
SET GLOBAL innodb_compressed_columns_zip_level = 0;

# test the following column types:
# - VARCHAR
# - NATIONAL VARCHAR
# - VARBINARY
# - TINYBLOB
# - TINYTEXT
# - BLOB
# - TEXT
# - MEDIUMBLOB
# - MEDIUMTEXT
# - LONGBLOB
# - LONGTEXT
# - JSON

--let $base_field_type= VARCHAR
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $base_field_type= NATIONAL VARCHAR
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $base_field_type= VARBINARY
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $type_length_limit= 255
--let $full_field_type= TINYTEXT
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $full_field_type= TINYBLOB
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $type_length_limit= 65535
--let $full_field_type= TEXT
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $full_field_type= BLOB
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $type_length_limit= 16777215
--let $full_field_type= MEDIUMTEXT
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $full_field_type= MEDIUMBLOB
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $type_length_limit= 4294967295
--let $full_field_type= LONGTEXT
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $full_field_type= LONGBLOB
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

--let $type_length_limit= 4294967295
--let $full_field_type= JSON
--source suite/innodb/include/xtradb_compressed_columns_check_type_set.inc

# restoring moddified global variables
--disable_warnings
SET GLOBAL innodb_file_format = @saved_innodb_file_format;
--enable_warnings
SET GLOBAL innodb_compressed_columns_zip_level = @saved_innodb_compressed_columns_zip_level;

# restarting the server with original options
--let $restart_parameters=
--source include/restart_mysqld.inc
