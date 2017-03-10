--let $partition_boundary = `SELECT POW(2, $number_of_steps - 1)`

--let $partition_spec = ENGINE=InnoDB
if($partitioning == 'simple')
{
  let $partition_spec = $partition_spec
  PARTITION BY RANGE (id) (
    PARTITION p0 VALUES LESS THAN ($partition_boundary) ENGINE = InnoDB,
    PARTITION p1 VALUES LESS THAN MAXVALUE ENGINE = InnoDB
  );
}
if($partitioning == 'sub')
{
  let $partition_spec = $partition_spec
  PARTITION BY RANGE (id)
  SUBPARTITION BY HASH(id)
  SUBPARTITIONS 2 (
    PARTITION p0 VALUES LESS THAN ($partition_boundary) ENGINE = InnoDB,
    PARTITION p1 VALUES LESS THAN MAXVALUE ENGINE = InnoDB
  );
}

--echo # Requiring innodb_file_per_table.
SET @old_innodb_file_per_table = @@global.innodb_file_per_table;
SET GLOBAL innodb_file_per_table = 1;

--echo # Defining compression dictionary content.
--disable_query_log
SET @dictionary_data =
  'Bartholomew'
  'Christopher'
  'Constantine'
  'Encarnacion'
  'Maximiliano'
  'Maximillian'
  'Miguelangel'
  'Christopher'
  'Margueritta'
  'Alessandro'
  'Alexzander'
  'Christoper'
  'Cornelious'
  'Cristopher'
  'Fitzgerald'
  'Florentino'
  'Kristoffer'
  'Kristopher'
  'Maximilian'
  'Montgomery'
  'Rutherford'
  'Stanislaus'
  'Victoriano'
  'Washington'
  'Wellington'
  'Alessandra'
  'Alexandrea'
  'Alexandria'
  'Alphonsine'
  'Antionette'
  'Antoinette'
  'Bernadette'
  'Bernardine'
  'Charlottie'
  'Charolette'
  'Christiana'
  'Clarabelle'
  'Clementina'
  'Clementine'
  'Concepcion'
  'Earnestine'
  'Evangelina'
  'Evangeline'
  'Francisqui'
  'Fredericka'
  'Georgeanna'
  'Georgianna'
  'Hildegarde'
  'Jacqueline'
  'Jacquelynn'
  'Josiephine'
  'Margaretha'
  'Margaretta'
  'Margarette'
  'Marguerite'
  'Marylouise'
  'Montserrat'
  'Sanjuanita'
  'Shirleyann'
  'Temperance'
  'Wilhelmina'
  'Wilhelmine'
;
--enable_query_log

--echo # Creating a compression dictionary.
CREATE COMPRESSION_DICTIONARY names (@dictionary_data);

--echo # Creating three tables with identical structure but different
--echo # compression attributes for the BLOB column.

--let $uncompressed_table_name = t1
--let $compressed_table_name = t2
--let $dictionary_compressed_table_name = t3

eval CREATE TABLE $uncompressed_table_name(
  id INT,
  a BLOB
) $partition_spec;

eval CREATE TABLE $compressed_table_name(
  id INT,
  a BLOB COLUMN_FORMAT COMPRESSED
) $partition_spec;

eval CREATE TABLE $dictionary_compressed_table_name(
  id INT,
  a BLOB COLUMN_FORMAT COMPRESSED WITH COMPRESSION_DICTIONARY names
) $partition_spec;

--disable_query_log

--echo # Defining a JSON-formatted value which can be well compressed with the created dictionary.
SET @json_value =
  '{\n'
  '  "Bartholomew" = 0,\n'
  '  "Christopher" = 0,\n'
  '  "Constantine" = 0,\n'
  '  "Encarnacion" = 0,\n'
  '  "Maximiliano" = 0,\n'
  '  "Maximillian" = 0,\n'
  '  "Miguelangel" = 0,\n'
  '  "Christopher" = 0,\n'
  '  "Margueritta" = 0,\n'
  '  "Alessandro"  = 0,\n'
  '  "Alexzander"  = 0,\n'
  '  "Christoper"  = 0,\n'
  '  "Cornelious"  = 0,\n'
  '  "Cristopher"  = 0,\n'
  '  "Fitzgerald"  = 0,\n'
  '  "Florentino"  = 0,\n'
  '  "Kristoffer"  = 0,\n'
  '  "Kristopher"  = 0,\n'
  '  "Maximilian"  = 0,\n'
  '  "Montgomery"  = 0,\n'
  '  "Rutherford"  = 0,\n'
  '  "Stanislaus"  = 0,\n'
  '  "Victoriano"  = 0,\n'
  '  "Washington"  = 0,\n'
  '  "Wellington"  = 0,\n'
  '  "Alessandra"  = 0,\n'
  '  "Alexandrea"  = 0,\n'
  '  "Alexandria"  = 0,\n'
  '  "Alphonsine"  = 0,\n'
  '  "Antionette"  = 0,\n'
  '  "Antoinette"  = 0,\n'
  '  "Bernadette"  = 0,\n'
  '  "Bernardine"  = 0,\n'
  '  "Charlottie"  = 0,\n'
  '  "Charolette"  = 0,\n'
  '  "Christiana"  = 0,\n'
  '  "Clarabelle"  = 0,\n'
  '  "Clementina"  = 0,\n'
  '  "Clementine"  = 0,\n'
  '  "Concepcion"  = 0,\n'
  '  "Earnestine"  = 0,\n'
  '  "Evangelina"  = 0,\n'
  '  "Evangeline"  = 0,\n'
  '  "Francisqui"  = 0,\n'
  '  "Fredericka"  = 0,\n'
  '  "Georgeanna"  = 0,\n'
  '  "Georgianna"  = 0,\n'
  '  "Hildegarde"  = 0,\n'
  '  "Jacqueline"  = 0,\n'
  '  "Jacquelynn"  = 0,\n'
  '  "Josiephine"  = 0,\n'
  '  "Margaretha"  = 0,\n'
  '  "Margaretta"  = 0,\n'
  '  "Margarette"  = 0,\n'
  '  "Marguerite"  = 0,\n'
  '  "Marylouise"  = 0,\n'
  '  "Montserrat"  = 0,\n'
  '  "Sanjuanita"  = 0,\n'
  '  "Shirleyann"  = 0,\n'
  '  "Temperance"  = 0,\n'
  '  "Wilhelmina"  = 0,\n'
  '  "Wilhelmine"  = 0,\n'
  '}\n'
;

--echo # Filling tables with a big number of records.
eval INSERT INTO $uncompressed_table_name VALUES(0, @json_value);
eval INSERT INTO $uncompressed_table_name VALUES(1, @json_value);
eval INSERT INTO $uncompressed_table_name VALUES($partition_boundary, @json_value);
eval INSERT INTO $uncompressed_table_name VALUES($partition_boundary + 1, @json_value);
eval INSERT INTO $compressed_table_name VALUES(0, @json_value);
eval INSERT INTO $compressed_table_name VALUES(1, @json_value);
eval INSERT INTO $compressed_table_name VALUES($partition_boundary, @json_value);
eval INSERT INTO $compressed_table_name VALUES($partition_boundary + 1, @json_value);
eval INSERT INTO $dictionary_compressed_table_name VALUES(0, @json_value);
eval INSERT INTO $dictionary_compressed_table_name VALUES(1, @json_value);
eval INSERT INTO $dictionary_compressed_table_name VALUES($partition_boundary, @json_value);
eval INSERT INTO $dictionary_compressed_table_name VALUES($partition_boundary + 1, @json_value);

--let $number_of_insert_selects = $number_of_steps
--dec $number_of_insert_selects
--dec $number_of_insert_selects

--let $i = $number_of_insert_selects
while ($i)
{
  eval INSERT INTO $uncompressed_table_name SELECT * FROM $uncompressed_table_name;
  --dec $i
}

--let $i = $number_of_insert_selects
while ($i)
{
  eval INSERT INTO $compressed_table_name SELECT * FROM $compressed_table_name;
  --dec $i
}

--let $i = $number_of_insert_selects
while ($i)
{
  eval INSERT INTO $dictionary_compressed_table_name SELECT * FROM $dictionary_compressed_table_name;
  --dec $i
}

--echo # Checking tables integrity. All records must have the same JSON value.
eval SELECT COUNT(*) = POW(2, $number_of_steps) AS number_of_rows_for_uncompressed_matches
  FROM $uncompressed_table_name WHERE a = @json_value;
eval SELECT COUNT(*) = POW(2, $number_of_steps) AS number_of_rows_for_compressed_matches
  FROM $compressed_table_name WHERE a = @json_value;
eval SELECT COUNT(*) = POW(2, $number_of_steps) AS number_of_rows_for_dictionary_compressed_matches
  FROM $dictionary_compressed_table_name WHERE a = @json_value;

--echo # Checking .ibd file sizes for filled tables.
--let MYSQLD_DATADIR=`SELECT @@datadir`
let FILES_TO_CHECK = `SELECT GROUP_CONCAT(REPLACE(path, './', ''))
  FROM information_schema.innodb_sys_tablespaces
  JOIN information_schema.innodb_sys_datafiles USING(space)
  WHERE name LIKE CONCAT(DATABASE(), '/$uncompressed_table_name%')
     OR name LIKE CONCAT(DATABASE(), '/$compressed_table_name%')
     OR name LIKE CONCAT(DATABASE(), '/$dictionary_compressed_table_name%')
`;
--let IBD_SIZES_OUTPUT_FILE = $MYSQLTEST_VARDIR/tmp/ibd_sizes.tmp

perl;
  my $fname= $ENV{'IBD_SIZES_OUTPUT_FILE'};
  open(FILE, ">", $fname) or die;
  my @file_names = split(',', $ENV{'FILES_TO_CHECK'});
  foreach $file_name (@file_names) {
    my $abs_file_name = $ENV{'MYSQLD_DATADIR'}.$file_name;
    my $size = -s "$abs_file_name";
    print FILE "$file_name\t$size\n";
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
  (SELECT SUM(ibd_size) FROM ibd_info WHERE table_name LIKE CONCAT(DATABASE(), '/$compressed_table_name%'))
  <
  (SELECT SUM(ibd_size) FROM ibd_info WHERE table_name LIKE CONCAT(DATABASE(), '/$uncompressed_table_name%'))
  AS comressed_is_smaller_than_uncompressed
;

--echo # Checking if the size of the 'dictionary_compressed' table is smaller than 'compressed' one.
eval SELECT
  (SELECT SUM(ibd_size) FROM ibd_info WHERE table_name LIKE CONCAT(DATABASE(), '/$dictionary_compressed_table_name%'))
  <
  (SELECT SUM(ibd_size) FROM ibd_info WHERE table_name LIKE CONCAT(DATABASE(), '/$compressed_table_name%'))
  AS dictionary_comressed_is_smaller_than_compressed
;

--enable_query_log

--echo # Cleaning up.
eval DROP TABLE ibd_info, $uncompressed_table_name, $compressed_table_name, $dictionary_compressed_table_name;
DROP COMPRESSION_DICTIONARY names;

--echo # Restoring innodb_file_per_table
SET GLOBAL innodb_file_per_table = @old_innodb_file_per_table;
