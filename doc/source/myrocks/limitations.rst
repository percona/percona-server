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

* `Generated Columns <https://dev.mysql.com/doc/refman/5.7/en/create-table-generated-columns.html>`__

* `Partial Update of LOB in InnoDB <https://mysqlserverteam.com/mysql-8-0-optimizing-small-partial-update-of-lob-in-innodb/>`_

You should also consider the following:

* :file:`*_bin` (e.g. ``latin1_bin``) or binary collation should be used
  on ``CHAR`` and ``VARCHAR`` indexed columns. The following binary collations
  are supported: ``binary``, ``latin1_bin``, and ``utf8_bin``.
  By default, MyRocks prevents creating indexes with non-binary collations
  (including ``latin1``).
  You can optionally use it by setting
  :ref:`rocksdb_strict_collation_exceptions` to ``t1``
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

* MyRocks does not support operating as either a source or a replica
  in any replication topology that is not exclusively row-based.
  Statement-based and mixed-format binary logging is not supported.
  For more information, see `Replication Formats
  <https://dev.mysql.com/doc/refman/5.7/en/replication-formats.html>`_.

* When converting from large MyISAM/InnoDB tables, either by using the
  ``ALTER`` or ``INSERT INTO SELECT`` statements it's recommended that you
  check the :ref:`Data loading <myrocks_data_loading>` documentation and
  create MyRocks tables as below (in case the table is sufficiently big it will
  cause the server to consume all the memory and then be terminated by the OOM
  killer):

  .. code-block:: mysql

    SET session sql_log_bin=0;
    SET session rocksdb_bulk_load=1;
    ALTER TABLE large_myisam_table ENGINE=RocksDB;
    SET session rocksdb_bulk_load=0;

   .. warning::

    If you are loading large data without enabling :ref:`rocksdb_bulk_load`
    or :ref:`rocksdb_commit_in_the_middle`, please make sure transaction
    size is small enough. All modifications of the ongoing transactions are
    kept in memory.

* The`XA protocol <https://dev.mysql.com/doc/refman/5.7/en/xa.html>`_ support,
  which allows distributed transactions combining multiple separate
  transactional resources, is an experimental feature in MyRocks: the 
  implementation is less tested, it may lack some functionality and be not as
  stable as in case of InnoDB.

* MySQL has `spatial data types <https://dev.mysql.com/doc/refman/5.7/en/spatial-type-overview.html>`__ . These data types are not supported by MyRocks. 
