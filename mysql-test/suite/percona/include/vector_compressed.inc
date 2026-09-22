--echo #
--echo # PS-11299: vector indexes on compressed tables.
--echo #
--echo # Two separate questions that are easy to run together and should not
--echo # be. A vector index refuses a KEY_BLOCK_SIZE of its own - it has no
--echo # B-tree, so there are no pages of its own for the number to describe.
--echo # The table underneath it is a different matter: ROW_FORMAT=COMPRESSED
--echo # there is supported, and the auxiliary table holding the graph takes
--echo # the base table's format, so the graph is compressed along with it.
--echo #
--echo # This body runs at three page sizes, one wrapper test each. Above
--echo # 16k InnoDB has no
--echo # compressed format at all, so the compressed half is refused by
--echo # InnoDB rather than by anything of ours, and the test says so.
--echo #

--echo # The page size this run got. If it is not the one the wrapper's
--echo # .opt file asked for then the rest of the file is running somewhere
--echo # else and proves nothing about that page size.
SELECT @@innodb_page_size;

--let $compressible = `SELECT @@innodb_page_size <= 16384`
--let $aux_fmt = SELECT name, row_format FROM information_schema.innodb_tables WHERE name LIKE CONCAT((SELECT DATABASE()), '/percona\\_vec\\_hnsw\\_%')

--echo #
--echo # === KEY_BLOCK_SIZE on the vector index itself: always refused ===
--echo #
--error ER_NOT_SUPPORTED_YET
CREATE TABLE t1 (id BIGINT UNSIGNED PRIMARY KEY, v VECTOR(4) NOT NULL,
                 VECTOR KEY vk (v) TYPE hnsw (M = 4) KEY_BLOCK_SIZE=8);

--echo # And through ALTER, which is a different code path to the same rule.
CREATE TABLE t1 (id BIGINT UNSIGNED PRIMARY KEY, v VECTOR(4) NOT NULL);
--error ER_NOT_SUPPORTED_YET
ALTER TABLE t1 ADD VECTOR KEY vk (v) TYPE hnsw (M = 4) KEY_BLOCK_SIZE=8;

--echo # Without it the same statement is fine, so the refusal is about
--echo # KEY_BLOCK_SIZE and not about the rest of the clause.
ALTER TABLE t1 ADD VECTOR KEY vk (v) TYPE hnsw (M = 4);
SHOW CREATE TABLE t1;
DROP TABLE t1;

if (!$compressible) {
  --echo #
  --echo # === Above 16k there is no compressed row format ===
  --echo #
  --echo # Nothing vector-specific here: InnoDB refuses the table itself, with
  --echo # or without a vector index on it. Recorded so the boundary is
  --echo # visible rather than assumed.
  --echo #
  --error ER_ILLEGAL_HA
  CREATE TABLE t2 (id BIGINT UNSIGNED PRIMARY KEY, v VECTOR(4) NOT NULL,
                   VECTOR KEY vk (v) TYPE hnsw (M = 4)) ROW_FORMAT=COMPRESSED;
  --error ER_ILLEGAL_HA
  CREATE TABLE t2 (id BIGINT UNSIGNED PRIMARY KEY, c INT) ROW_FORMAT=COMPRESSED;
}

if ($compressible) {
  --echo #
  --echo # === A compressed table carrying a vector index ===
  --echo #
  CREATE TABLE t2 (id BIGINT UNSIGNED PRIMARY KEY, v VECTOR(4) NOT NULL,
                   VECTOR KEY vk (v) TYPE hnsw (M = 4)) ROW_FORMAT=COMPRESSED;
  INSERT INTO t2 VALUES (1, STRING_TO_VECTOR('[1,0,0,0]')),
                        (2, STRING_TO_VECTOR('[0,1,0,0]')),
                        (3, STRING_TO_VECTOR('[0,0,1,0]'));

  --echo # The aux table took the base table's row format.
  --replace_regex /percona_vec_hnsw_[0-9a-f]+_[0-9a-f]+/percona_vec_hnsw_<AUX>/
  --eval $aux_fmt

  --echo # The graph is whole and answers.
  SET SESSION innodb_interpreter = 'vec_aux_verify test/t2';
  SELECT @@session.innodb_interpreter_output;
  SELECT id FROM t2
   ORDER BY DISTANCE(v, STRING_TO_VECTOR('[0,0,1,0]'), 'euclidean') LIMIT 1;

  --echo # And still does once it has been read back off compressed pages.
  --source include/restart_mysqld.inc
  SELECT id FROM t2
   ORDER BY DISTANCE(v, STRING_TO_VECTOR('[0,1,0,0]'), 'euclidean') LIMIT 1;
  DROP TABLE t2;

  --echo #
  --echo # === Compressing a table that already has a vector index ===
  --echo #
  CREATE TABLE t3 (id BIGINT UNSIGNED PRIMARY KEY, v VECTOR(4) NOT NULL,
                   VECTOR KEY vk (v) TYPE hnsw (M = 4));
  INSERT INTO t3 VALUES (1, STRING_TO_VECTOR('[1,0,0,0]')),
                        (2, STRING_TO_VECTOR('[0,1,0,0]'));
  --replace_regex /percona_vec_hnsw_[0-9a-f]+_[0-9a-f]+/percona_vec_hnsw_<AUX>/
  --eval $aux_fmt

  --echo # The rebuild carries the graph across, and the aux follows the base
  --echo # into the compressed format.
  ALTER TABLE t3 ROW_FORMAT=COMPRESSED;
  --replace_regex /percona_vec_hnsw_[0-9a-f]+_[0-9a-f]+/percona_vec_hnsw_<AUX>/
  --eval $aux_fmt
  SET SESSION innodb_interpreter = 'vec_aux_verify test/t3';
  SELECT @@session.innodb_interpreter_output;
  SELECT id FROM t3
   ORDER BY DISTANCE(v, STRING_TO_VECTOR('[1,0,0,0]'), 'euclidean') LIMIT 1;

  --echo # And back out again.
  ALTER TABLE t3 ROW_FORMAT=DYNAMIC;
  --replace_regex /percona_vec_hnsw_[0-9a-f]+_[0-9a-f]+/percona_vec_hnsw_<AUX>/
  --eval $aux_fmt
  SELECT id FROM t3
   ORDER BY DISTANCE(v, STRING_TO_VECTOR('[0,1,0,0]'), 'euclidean') LIMIT 1;
  DROP TABLE t3;

  --echo #
  --echo # === Adding a vector index to a table that is already compressed ===
  --echo #
  CREATE TABLE t4 (id BIGINT UNSIGNED PRIMARY KEY, v VECTOR(4) NOT NULL)
    ROW_FORMAT=COMPRESSED;
  INSERT INTO t4 VALUES (1, STRING_TO_VECTOR('[1,0,0,0]')),
                        (2, STRING_TO_VECTOR('[0,1,0,0]'));
  ALTER TABLE t4 ADD VECTOR KEY vk (v) TYPE hnsw (M = 4);
  --replace_regex /percona_vec_hnsw_[0-9a-f]+_[0-9a-f]+/percona_vec_hnsw_<AUX>/
  --eval $aux_fmt
  SELECT id FROM t4
   ORDER BY DISTANCE(v, STRING_TO_VECTOR('[1,0,0,0]'), 'euclidean') LIMIT 1;
  DROP TABLE t4;

  --echo #
  --echo # === The two ends of the KEY_BLOCK_SIZE range ===
  --echo #
  --echo # 1 is the smallest compressed page InnoDB has, and the one where an
  --echo # aux row that does not fit would show up: each carries a vector and
  --echo # a neighbour list. page_size/1024 is the largest, where the pages
  --echo # are nominally compressed and nothing is squeezed. Both are valid at
  --echo # every page size that has a compressed format at all, so neither of
  --echo # these needs to tolerate an error.
  --echo #
  --let $max_kbs = `SELECT @@innodb_page_size DIV 1024`

  --echo # --- KEY_BLOCK_SIZE=1 ---
  CREATE TABLE k1 (id BIGINT UNSIGNED PRIMARY KEY, v VECTOR(256) NOT NULL,
                   VECTOR KEY vk (v) TYPE hnsw (M = 4))
    ROW_FORMAT=COMPRESSED KEY_BLOCK_SIZE=1;
  INSERT INTO k1 VALUES
    (1, STRING_TO_VECTOR(CONCAT('[', REPEAT('0.25,', 255), '0.25]'))),
    (2, STRING_TO_VECTOR(CONCAT('[', REPEAT('0.75,', 255), '0.75]')));
  SET SESSION innodb_interpreter = 'vec_aux_verify test/k1';
  SELECT @@session.innodb_interpreter_output;
  SELECT id FROM k1
   ORDER BY DISTANCE(v, (SELECT v FROM k1 WHERE id = 2), 'euclidean') LIMIT 1;
  DROP TABLE k1;

  --echo # --- KEY_BLOCK_SIZE = page_size/1024, the largest this page size takes ---
  --eval CREATE TABLE k2 (id BIGINT UNSIGNED PRIMARY KEY, v VECTOR(256) NOT NULL, VECTOR KEY vk (v) TYPE hnsw (M = 4)) ROW_FORMAT=COMPRESSED KEY_BLOCK_SIZE=$max_kbs
  INSERT INTO k2 VALUES
    (1, STRING_TO_VECTOR(CONCAT('[', REPEAT('0.25,', 255), '0.25]'))),
    (2, STRING_TO_VECTOR(CONCAT('[', REPEAT('0.75,', 255), '0.75]')));
  SET SESSION innodb_interpreter = 'vec_aux_verify test/k2';
  SELECT @@session.innodb_interpreter_output;
  SELECT id FROM k2
   ORDER BY DISTANCE(v, (SELECT v FROM k2 WHERE id = 1), 'euclidean') LIMIT 1;
  DROP TABLE k2;
}

--echo # Nothing of ours left behind:
--replace_regex /percona_vec_hnsw_[0-9a-f]+_[0-9a-f]+/percona_vec_hnsw_<AUX>/
--eval $aux_fmt
