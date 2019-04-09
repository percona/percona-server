# ==== Purpose ====
#
# Common test to check support for compressed columns on partitioned tables
# On multiple Storage Engines.
#
# ==== Usage ====
#
# --let $engine= ENGINE
# --source include/percona_partition_with_compressed_columns.inc
#
# Parameters:
#   $engine
#     Storage Engine to test

--echo #
--echo # PS-5174: Check COMPRESSED COLUMNS support on partitioned tables
--echo #

SET @dictionary_data = 'one' 'two' 'three' 'four';
CREATE COMPRESSION_DICTIONARY numbers (@dictionary_data);
--error ER_ILLEGAL_HA_CREATE_OPTION
eval CREATE TABLE comp1(
  id BIGINT UNSIGNED NOT NULL,
  a VARCHAR(250) COLUMN_FORMAT COMPRESSED WITH COMPRESSION_DICTIONARY numbers DEFAULT NULL
) ENGINE = $engine
PARTITION BY RANGE (id) (
  PARTITION p1 VALUES LESS THAN (100),
  PARTITION p2 VALUES LESS THAN MAXVALUE
);
DROP COMPRESSION_DICTIONARY numbers;
