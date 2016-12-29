# test for inserting and reading back strings of various lengths
# (2^N +/- 2 bytes)

--let $field_length= 0
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 1
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 2
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc

--let $field_length= 126
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 127
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 128
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 129
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 130

--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 254
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 255
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 256
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 257
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 258
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc

--let $field_length= 8190
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 8191
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 8192
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 8193
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 8194
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc

--let $field_length= 16382
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 16383
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 16384
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 16385
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 16386
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc

--let $field_length= 32766
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 32767
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 32768
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 32769
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc
--let $field_length= 32770
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc

# this is the max size for VARCHAR columns
--let $field_length= 65532
--source suite/innodb/include/xtradb_compressed_columns_check_type.inc

# include checks for larger values only in case of BLOBs / TEXTs
if($full_field_type)
{
  --let $field_length= 65534
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 65535
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 65536
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 65537
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 65538
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc

  --let $field_length= 16777214
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 16777215
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 16777216
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 16777217
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 16777218
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc

  --let $field_length= 33554430
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 33554431
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 33554432
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 33554433
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
  --let $field_length= 33554434
  --source suite/innodb/include/xtradb_compressed_columns_check_type.inc
}
