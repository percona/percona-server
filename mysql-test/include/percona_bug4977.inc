# PS-4977 : [PS8QA] Flushing the table with read lock could lead to a server
#           crash if PS_metadata locks is queried
#
# Common test include file that requires $engine to be specified

eval CREATE TABLE t0(a INT) ENGINE=$engine;

FLUSH TABLE t0 WITH READ LOCK;

# crash here on memcpy
SELECT COUNT(*) > 0 FROM performance_schema.metadata_locks;

UNLOCK TABLES;

DROP TABLE t0;
