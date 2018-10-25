#
# Helper include file to test ft_query_extra_word_chars functionality
#
# required arg:
# - $ft_query_extra_word_chars = setup: to create words table and run_queries SP
# - $ft_query_extra_word_chars = cleanup: to drop them
#

if ($ft_query_extra_word_chars == 'setup')
{
  --echo #
  --echo # Setup ft_query_extra_word_chars testing
  --echo #

  SET NAMES utf8mb4;

  CREATE TABLE words(id INT PRIMARY KEY AUTO_INCREMENT,
                     a TEXT COLLATE utf8mb4_bin);
  INSERT INTO words (a) VALUES ('abcdef');
  INSERT INTO words (a) VALUES ('ąbčdėf');
  INSERT INTO words (a) VALUES ('ąbč_dėf');
  INSERT INTO words (a) VALUES ('ąbč!dėf');
  INSERT INTO words (a) VALUES ('ąbč,dėf');
  INSERT INTO words (a) VALUES ('ąbč dėf');
  # Words containing default FTS boolean search syntax characters
  INSERT INTO words (a) VALUES ('+bčę');
  INSERT INTO words (a) VALUES ('>ąbč <dėf');
  INSERT INTO words (a) VALUES ('-(ąbč xyz)');
  INSERT INTO words (a) VALUES ('ąbč* ~dėf');
  INSERT INTO words (a) VALUES ('single"');
  INSERT INTO words (a) VALUES ('-ųtū');

  delimiter |;
  CREATE PROCEDURE run_queries(table_name VARCHAR(20), query_type ENUM('nl', 'bool'))
  BEGIN
          DECLARE q, fts_query VARCHAR(100);
          SET @q = CONCAT("SELECT a AS matches FROM ",
                          table_name,
                          " WHERE MATCH(a) AGAINST (? IN ");
          IF query_type = 'nl'
             THEN SET @q = CONCAT(@q, "NATURAL LANGUAGE");
             ELSE SET @q = CONCAT(@q, "BOOLEAN");
          END IF;
          SET @q = CONCAT(@q, " MODE)");

          PREPARE query_stmt FROM @q;
          SET @fts_query = 'č,d'; SELECT @fts_query; EXECUTE query_stmt USING @fts_query;
          SET @fts_query = '+bc'; SELECT @fts_query; EXECUTE query_stmt USING @fts_query;
          SET @fts_query = 'single'; SELECT @fts_query; EXECUTE query_stmt USING @fts_query;
          # Surprising result with default parser, NL query, extra chars ON: '-' becomes a part
          # of the query token, result is empty set
          SET @fts_query = '-ųtū'; SELECT @fts_query; EXECUTE query_stmt USING @fts_query;
          SET @fts_query = '"-ųtū"'; SELECT @fts_query; EXECUTE query_stmt USING @fts_query;
          DEALLOCATE PREPARE query_stmt;
  END|
  delimiter ;|
}

if ($ft_query_extra_word_chars == 'cleanup')
{
  DROP PROCEDURE run_queries;
  DROP TABLE words;
}
