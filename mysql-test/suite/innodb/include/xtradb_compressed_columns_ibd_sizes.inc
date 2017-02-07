# $number_of_steps - defines how meny times the number of records will be doubled
# $field_type - the type of the field which will be used to store JSON value (BLOB, TEXT, JSON, etc.)
# $json_value - string representation of the JSON value which will be inserted

--echo # ********************
--echo # *** $field_type
--echo # ********************

--echo # Creating three tables with identical structure but different
--echo # column compression attributes.

--disable_query_log

eval CREATE TABLE $uncompressed_table_name(
  id INT,
  a $field_type
) ENGINE=InnoDB;

eval CREATE TABLE $compressed_table_name(
  id INT,
  a $field_type COLUMN_FORMAT COMPRESSED
) ENGINE=InnoDB;

eval CREATE TABLE $dictionary_compressed_table_name(
  id INT,
  a $field_type COLUMN_FORMAT COMPRESSED WITH COMPRESSION_DICTIONARY names
) ENGINE=InnoDB;

if($field_type == 'JSON')
{
  eval SET @json_value = CAST('$json_value' AS JSON);
  --let $comparison_rhs = CAST(@json_value AS JSON)
}
if($field_type != 'JSON')
{
  eval SET @json_value = '$json_value';
  --let $comparison_rhs = @json_value
}

--echo # Filling tables with a big number of records.
eval INSERT INTO $uncompressed_table_name VALUES(0, @json_value);
eval INSERT INTO $compressed_table_name VALUES(0, @json_value);
eval INSERT INTO $dictionary_compressed_table_name VALUES(0, @json_value);

--let $i=$number_of_steps
while ($i)
{
  eval INSERT INTO $uncompressed_table_name SELECT * FROM $uncompressed_table_name;
  --dec $i
}

--let $i=$number_of_steps
while ($i)
{
  eval INSERT INTO $compressed_table_name SELECT * FROM $compressed_table_name;
  --dec $i
}

--let $i=$number_of_steps
while ($i)
{
  eval INSERT INTO $dictionary_compressed_table_name SELECT * FROM $dictionary_compressed_table_name;
  --dec $i
}

--echo # Checking tables integrity. All records must have the same value.
eval SELECT COUNT(*) = POW(2, $number_of_steps) AS number_of_rows_for_uncompressed_matches
  FROM $uncompressed_table_name WHERE a = $comparison_rhs;
eval SELECT COUNT(*) = POW(2, $number_of_steps) AS number_of_rows_for_compressed_matches
  FROM $compressed_table_name WHERE a = $comparison_rhs;
eval SELECT COUNT(*) = POW(2, $number_of_steps) AS number_of_rows_for_dictionary_compressed_matches
  FROM $dictionary_compressed_table_name WHERE a = $comparison_rhs;

--echo # Checking .ibd file sizes for filled tables.
--let MYSQLD_DATADIR=`SELECT @@datadir`
--let TABLES_TO_CHECK= $uncompressed_table_name $compressed_table_name $dictionary_compressed_table_name
--let IBD_SIZES_OUTPUT_FILE = $MYSQLTEST_VARDIR/tmp/ibd_sizes.tmp

perl;
  my $fname= $ENV{'IBD_SIZES_OUTPUT_FILE'};
  open(FILE, ">", $fname) or die;
  my @table_names = split(' ', $ENV{'TABLES_TO_CHECK'});
  foreach $table_name (@table_names) {
    my $tablespace = $ENV{'MYSQLD_DATADIR'}."test/".$table_name.".ibd";
    my $size = -s "$tablespace";
    print FILE "$table_name\t$size\n";
  }
  close FILE;
EOF

CREATE TABLE ibd_info(
  table_name VARCHAR(255) NOT NULL UNIQUE,
  ibd_size BIGINT UNSIGNED NOT NULL
);

--eval LOAD DATA INFILE '$IBD_SIZES_OUTPUT_FILE' INTO TABLE ibd_info
--remove_file $IBD_SIZES_OUTPUT_FILE

--echo # Checking if the size of the 'compressed' table is smaller than 'uncompressed' one.
eval SELECT
  (SELECT ibd_size FROM ibd_info WHERE table_name = '$compressed_table_name')
  <
  (SELECT ibd_size FROM ibd_info WHERE table_name = '$uncompressed_table_name')
  AS comressed_is_smaller_than_uncompressed
;

--echo # Checking if the size of the 'dictionary_compressed' table is smaller than 'compressed' one.
eval SELECT
  (SELECT ibd_size FROM ibd_info WHERE table_name = '$dictionary_compressed_table_name')
  <
  (SELECT ibd_size FROM ibd_info WHERE table_name = '$compressed_table_name')
  AS dictionary_comressed_is_smaller_than_compressed
;

--enable_query_log

--echo # Cleaning up.
eval DROP TABLE ibd_info, $uncompressed_table_name, $compressed_table_name, $dictionary_compressed_table_name;
