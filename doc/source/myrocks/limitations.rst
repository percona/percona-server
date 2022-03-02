.. _myrocks_limitations:

===================
MyRocks Limitations
===================

The MyRocks storage engine lacks the following features compared to InnoDB:

   * `Online DDL <https://dev.mysql.com/doc/refman/8.0/en/innodb-online-ddl.html>`_ is not supported due to the lack of atomic DDL support. 
       - There is no ``ALTER TABLE ... ALGORITHM=INSTANT`` functionality
       - A partition management operation only supports the ``COPY`` algorithms, which rebuilds the partition table and moves the data based on the new ``PARTITION ... VALUE`` definition. In the case of ``DROP PARTITION``, the data not moved to another partition is deleted.
   * `ALTER TABLE .. EXCHANGE PARTITION <https://dev.mysql.com/doc/refman/8.0/en/partitioning-management-exchange.html>`_. 
   * `SAVEPOINT <https://dev.mysql.com/doc/refman/8.0/en/savepoint.html>`_
   * `Transportable tablespace <https://dev.mysql.com/doc/refman/8.0/en/innodb-table-import.html>`_
   * `Foreign keys <https://dev.mysql.com/doc/refman/8.0/en/create-table-foreign-keys.html>`_
   * `Spatial indexes <https://dev.mysql.com/doc/refman/8.0/en/using-spatial-indexes.html>`_
   * `Fulltext indexes <https://dev.mysql.com/doc/refman/8.0/en/innodb-fulltext-index.html>`_
   * `Gap locks <https://dev.mysql.com/doc/refman/8.0/en/innodb-locking.html#innodb-gap-locks>`_
   * `Group Replication <https://dev.mysql.com/doc/refman/8.0/en/group-replication.html>`_
   * `Partial Update of LOB in InnoDB <https://mysqlserverteam.com/mysql-8-0-optimizing-small-partial-update-of-lob-in-innodb/>`_
    

As of |Percona Server| version 8.0.23-14, `Generated Columns <https://dev.mysql.com/doc/refman/8.0/en/create-table-generated-columns.html>`_ and index are supported. Generated columns are not supported in versions earlier than 8.0.23-14.

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

* MyRocks does not support operating as either a source or a replica
  in any replication topology that is not exclusively row-based.
  Statement-based and mixed-format binary logging is not supported.
  For more information, see `Replication Formats
  <https://dev.mysql.com/doc/refman/8.0/en/replication-formats.html>`_.

* As of 8.0.17, InnoDB supports `multi-valued indexes <https://dev.mysql.com/doc/refman/8.0/en/create-index.html#create-index-multi-valued>`__. MyRocks does not support this feature.

* As of 8.0.17, InnoDB supports the use of the `Clone Plugin <https://dev.mysql.com/doc/refman/8.0/en/clone-plugin.html>`__ and the Clone Plugin API. MyRocks tables do not support either these features.

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

      If you are loading large data without enabling :variable:`rocksdb_bulk_load`
      or :variable:`rocksdb_commit_in_the_middle`, please make sure transaction
      size is small enough. All modifications of the ongoing transactions are
      kept in memory.

* With partitioned tables that use the |TokuDB| or |MyRocks| storage engine,
  the upgrade only works with native partitioning.

  .. seealso::

     |MySQL| Documentation: Preparing Your Installation for Upgrade
        https://dev.mysql.com/doc/refman/8.0/en/upgrade-prerequisites.html

* The |MyRocks| storage engine does not support the |sql.no-wait| and
  |sql.skip-locked| modifiers introduced in the |InnoDB| storage
  engine with |MySQL| 8.0.

.. include:: ../.res/replace.concept.txt

* |Percona Server| 8.0 and Unicode 9.0.0 standards have defined a change in the
  handling of binary collations. These collations are handled as NO PAD,
  trailing spaces are included in key comparisons. A binary collation comparison
  may result in two unique rows inserted and does not generate a`DUP_ENTRY`
  error. MyRocks key encoding and comparison does not account for this
  character set attribute.

*  As of |Percona Server| version 8.0.23-14, MyRocks supports `explicit DEFAULT value expressions <https://dev.mysql.com/doc/refman/8.0/en/data-type-defaults.html>`__. From version 8.0.13-3 to version 8.0.22-13, MyRocks did not support these expressions.

* |Percona Server| 8.0.16 does not support encryption for the MyRocks
  storage engine. At this time, during an ``ALTER TABLE`` operation, MyRocks mistakenly detects all InnoDB tables as encrypted. Therefore, any attempt to ``ALTER`` an InnoDB table to MyRocks fails.

  As a workaround, we recommend a manual move of the table. The following  steps are the same as the ``ALTER TABLE ... ENGINE=...`` process:

* Use ``SHOW CREATE TABLE ...`` to return the InnoDB table definition.

* With the table definition as the source, perform
  a ``CREATE TABLE ... ENGINE=RocksDB``.

* In the new table, use ``INSERT INTO <new table> SELECT * FROM <old table>``.

  .. note::

    With MyRocks and with large tables, it is recommended to set the session variable ``rocksdb_bulk_load=1`` during the load to prevent running out of memory. This recommendation is because of the MyRocks large transaction limitation.

  .. seealso::

    MyRocks Data Loading
    https://www.percona.com/doc/percona-server/8.0/myrocks/data_loading.html
    
* MySQL has `spatial data types <https://dev.mysql.com/doc/refman/8.0/en/spatial-type-overview.html>`__ . These data types are not supported by MyRocks.
