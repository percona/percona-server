DELIMITER //;

CREATE PROCEDURE populate_vector_table(table_name VARCHAR(64), dims INT, num_rows INT)
BEGIN
  SET @sql = CONCAT('INSERT INTO ', table_name, ' (id, v)
  WITH RECURSIVE
    seq(n)  AS (SELECT 0 UNION ALL SELECT n + 1 FROM seq  WHERE n < ', num_rows, '),
    dims(d) AS (SELECT 1 UNION ALL SELECT d + 1 FROM dims WHERE d < ', dims, ')
  SELECT s.n,
         string_to_vector(CONCAT(''['',
           (SELECT GROUP_CONCAT(RAND(s.n * ', dims, ' + d)) FROM dims), '']''))
  FROM seq s');
  PREPARE stmt FROM @sql;
  EXECUTE stmt;
  DEALLOCATE PREPARE stmt;

  SET @sql = CONCAT('ANALYZE TABLE ', table_name);
  PREPARE stmt FROM @sql;
  EXECUTE stmt;
  DEALLOCATE PREPARE stmt;
END;
//

DELIMITER ;//
