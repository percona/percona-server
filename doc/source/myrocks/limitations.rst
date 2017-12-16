.. _myrocks_limitations:

===================
MyRocks Limitations
===================

The MyRocks storage engine lacks the following features compared to InnoDB:

* `Online DDL <https://dev.mysql.com/doc/refman/5.7/en/innodb-online-ddl.html>`_

* `ALTER TABLE ... EXCHANGE PARTITION
  <https://dev.mysql.com/doc/refman/5.7/en/partitioning-management-exchange.html>`_

* `SAVEPOINT <https://dev.mysql.com/doc/refman/5.7/en/savepoint.html>`_

* `Transportable tablespace <https://dev.mysql.com/doc/refman/5.7/en/innodb-transportable-tablespace-examples.html>`_

* `Foreign keys <https://dev.mysql.com/doc/refman/5.7/en/create-table-foreign-keys.html>`_

* `Spatial indexes <https://dev.mysql.com/doc/refman/5.7/en/using-spatial-indexes.html>`_

* `Fulltext indexes <https://dev.mysql.com/doc/refman/5.7/en/innodb-fulltext-index.html>`_

* `Gap locks <https://dev.mysql.com/doc/refman/5.7/en/innodb-locking.html#innodb-gap-locks>`_

* `Group Replication <https://dev.mysql.com/doc/refman/5.7/en/group-replication.html>`_ 

You should also consider the following:

* :file:`*_bin` (e.g. ``latin1_bin``) or binary collation should be used
  on ``CHAR`` and ``VARCHAR`` indexed columns.
  By default, MyRocks prevents creating indexes with non-binary collations
  (including ``latin1``).
  You can optionally use it by setting
  :variable:`rocksdb_strict_collation_exceptions` to ``t1``
  (table names with regex format),
  but non-binary covering indexes other than ``latin1``
  (excluding ``german1``) still require a primary key lookup
  to return the ``CHAR`` or ``VARCHAR`` column.

* Either ``ORDER BY DESC`` or ``ORDER BY ASC`` is slow.
  This is because of "Prefix Key Encoding" feature in RocksDB.
  See http://www.slideshare.net/matsunobu/myrocks-deep-dive/58 for details.
  By default, ascending scan is faster and descending scan is slower.
  If the "reverse column family" is configured,
  then descending scan will be faster and ascending scan will be slower.
  Note that InnoDB also imposes a cost
  when the index is scanned in the opposite order.

* MyRocks does not support operating as either a master or a slave
  in any replication topology that is not exclusively row-based.
  Statement-based and mixed-format binary logging is not supported.
  For more information, see `Replication Formats
  <https://dev.mysql.com/doc/refman/5.7/en/replication-formats.html>`_.

