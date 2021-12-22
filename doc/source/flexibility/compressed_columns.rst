.. _compressed_columns:

====================================
Compressed columns with dictionaries
====================================

In :rn:`5.7.17-11` |Percona Server| has been extended with a new per-column
compression feature. It is a data type modifier, independent from user-level SQL
and InnoDB data compression, that causes the data stored in the column to be
compressed on writing to storage and decompressed on reading. For all other
purposes, the data type is identical to the one without the modifier, i.e. no
new data types are created. Compression is done by using the ``zlib`` library.

Additionally, it is possible to pre-define a set of strings for each compressed
column to achieve a better compression ratio on relatively small individual
data items.

This feature provides:

* a better compression ratio for text data which consist of a large number of
  predefined words (e.g. JSON or XML) using compression methods with static
  dictionaries
* a way to select columns in the table to compress (in contrast to the InnoDB
  row compression method)
  
This feature is based on a patch provided by Weixiang Zhai.

Specifications
==============

The feature is limited to InnoDB/XtraDB storage engine and to columns of the
following data types:

* ``BLOB`` (including ``TINYBLOB``, ``MEDIUMBLOB``, ``LONGBLOG``)

* ``TEXT`` (including ``TINYTEXT``, ``MEDUUMTEXT``, ``LONGTEXT``)

* ``VARCHAR`` (including ``NATIONAL VARCHAR``)

* ``VARBINARY``

* ``JSON``

The syntax to declare a compressed column is using an extension of an existing
``COLUMN_FORMAT`` modifier: ``COLUMN_FORMAT COMPRESSED``. If this modifier is
applied to an unsupported column type or storage engine, an error is returned.

The compression can be specified:

* when creating a table:
  ``CREATE TABLE ... (..., foo BLOB COLUMN_FORMAT COMPRESSED, ...);``
  
* when altering a table and modifying a column to the compressed format:
  ``ALTER TABLE ... MODIFY [COLUMN] ... COLUMN_FORMAT COMPRESSED``, or
  ``ALTER TABLE ... CHANGE [COLUMN] ... COLUMN_FORMAT COMPRESSED``.

Unlike Oracle MySQL, compression is applicable to generated stored columns. Use
this syntax extension as follows:

.. code-block:: mysql

  mysql> CREATE TABLE t1(
	  id INT,
	  a BLOB,
	  b JSON COLUMN_FORMAT COMPRESSED,
	  g BLOB GENERATED ALWAYS AS (a) STORED COLUMN_FORMAT COMPRESSED WITH COMPRESSION_DICTIONARY numbers
        ) ENGINE=InnoDB;

To decompress a column, specify a value other than ``COMPRESSED`` to
``COLUMN_FORMAT``: ``FIXED``, ``DYNAMIC``, or ``DEFAULT``. If there is a column
compression/decompression request in an ``ALTER TABLE``, it is forced to the
``COPY`` algorithm.

Two new variables: :variable:`innodb_compressed_columns_zip_level` and
:variable:`innodb_compressed_columns_threshold` have been implemented.

Compression dictionary support
==============================

To achieve a better compression ratio on relatively small individual data items,
it is possible to pre-define a compression dictionary, which is a set of strings
for each compressed column.

Compression dictionaries can be represented as a list of words in the form of a
string (comma or any other character can be used as a delimiter although not
required). In other words, ``a,bb,ccc``, ``a bb ccc`` and ``abbccc`` will have
the same effect. However, the latter is more space-efficient. Quote symbol
quoting is handled by regular SQL quoting. Maximum supported dictionary length
is 32506 bytes (``zlib`` limitation).

The compression dictionary is stored in a new system InnoDB table.
As this table is of the data dictionary kind, concurrent reads are
allowed, but writes are serialized, and reads are blocked by writes. Table read
through old read views are unsupported, similarly to InnoDB internal DDL
transactions.

Example
-------

In order to use the compression dictionary you need to create it. This
can be done by running:

.. code-block:: mysql

   mysql> SET @dictionary_data = 'one' 'two' 'three' 'four';
   Query OK, 0 rows affected (0.00 sec)

   mysql> CREATE COMPRESSION_DICTIONARY numbers (@dictionary_data);
   Query OK, 0 rows affected (0.00 sec)

To create a table that has both compression and compressed dictionary support
you should run:

.. code-block:: mysql

   mysql> CREATE TABLE t1(
           id INT,
           a BLOB COLUMN_FORMAT COMPRESSED,
           b BLOB COLUMN_FORMAT COMPRESSED WITH COMPRESSION_DICTIONARY numbers
         ) ENGINE=InnoDB;

The following example shows how to insert a sample of JSON data into the table:

.. code-block:: mysql

  SET @json_value =
   '[\n'
   ' {\n'
   ' "one" = 0,\n'
   ' "two" = 0,\n'
   ' "three" = 0,\n'
   ' "four" = 0\n'
   ' },\n'
   ' {\n'
   ' "one" = 0,\n'
   ' "two" = 0,\n'
   ' "three" = 0,\n'
   ' "four" = 0\n'
   ' },\n'
   ' {\n'
   ' "one" = 0,\n'
   ' "two" = 0,\n'
   ' "three" = 0,\n'
   ' "four" = 0\n'
   ' },\n'
   ' {\n'
   ' "one" = 0,\n'
   ' "two" = 0,\n'
   ' "three" = 0,\n'
   ' "four" = 0\n'
   ' }\n'
   ']\n'
  ;

.. code-block:: mysql

  mysql> INSERT INTO t1 VALUES(0, @json_value, @json_value);
  Query OK, 1 row affected (0.01 sec)


INFORMATION_SCHEMA Tables
=========================

This feature implemented two new ``INFORMATION_SCHEMA`` tables.

.. table:: INFORMATION_SCHEMA.XTRADB_ZIP_DICT

  :column BIGINT(21)_UNSIGNED id: dictionary ID
  :column VARCHAR(64) name: dictionary name
  :column BLOB zip_dict: compression dictionary string

This table provides a view over the internal compression dictionary table.
``SUPER`` privilege is required to query it.

.. table:: INFORMATION_SCHEMA.XTRADB_ZIP_DICT_COLS

  :column BIGINT(21)_UNSIGNED table_id: table ID from ``INFORMATION_SCHEMA.INNODB_SYS_TABLES``
  :column BIGINT(21)_UNSIGNED column_pos: column position (starts from ``0`` as in ``INFORMATION_SCHEMA.INNODB_SYS_COLUMNS``)
  :column BIGINT(21)_UNSIGNED dict_id: dictionary ID

This table provides a view over the internal table that stores the mapping
between the compression dictionaries and the columns using them. ``SUPER``
privilege is require to query it.

Limitations
===========

Compressed columns cannot be used in indices (neither on their own nor as parts
of composite keys).

.. note::

  ``CREATE TABLE t2 AS SELECT * FROM t1`` will create a new table with a
  compressed column, whereas ``CREATE TABLE t2 AS SELECT CONCAT(a,'') AS a FROM
  t1`` will not create compressed columns.

  At the same time, after executing ``CREATE TABLE t2 LIKE t1`` statement,
  ``t2.a`` will have ``COMPRESSED`` attribute.

``ALTER TABLE ... DISCARD/IMPORT TABLESPACE`` is not supported for tables with
compressed columns. To export and import tablespaces with compressed columns,
you need to uncompress them first with: ``ALTER TABLE ... MODIFY ...
COLUMN_FORMAT DEFAULT``.

mysqldump command line parameters
=================================

By default, with no additional options, ``mysqldump`` will generate a MySQL
compatible SQL output.

All ``/*!50633 COLUMN_FORMAT COMPRESSED */`` and ``/*!50633 COLUMN_FORMAT
COMPRESSED WITH COMPRESSION_DICTIONARY <dictionary> */`` won't be in the dump.

When a new option :option:`enable-compressed-columns` is specified, all
``/*!50633 COLUMN_FORMAT COMPRESSED */`` will be left intact and all ``/*!50633
COLUMN_FORMAT COMPRESSED WITH COMPRESSION_DICTIONARY <dictionary> */`` will be
transformed into ``/*!50633 COLUMN_FORMAT COMPRESSED */``. In this mode the
dump will contain the necessary SQL statements to create compressed columns,
but without dictionaries.

When a new :option:`enable-compressed-columns-with-dictionaries` option is
specified, dump will contain all compressed column attributes and compression
dictionary.

Moreover, the following dictionary creation fragments will be added before
``CREATE TABLE`` statements which are going to use these dictionaries for the
first time.

.. code-block:: mysql

  /*!50633 DROP COMPRESSION_DICTIONARY IF EXISTS <dictionary>; */
  /*!50633 CREATE COMPRESSION_DICTIONARY <dictionary>(...); */

Two new options :option:`add-drop-compression-dictionary` and
:option:`skip-add-drop-compression-dictionary` will control if ``/*!50633 DROP
COMPRESSION_DICTIONARY IF EXISTS <dictionary> */`` part from previous paragraph
will be skipped or not. By default, :option:`add-drop-compression-dictionary`
mode will be used.

When both :option:`enable-compressed-columns-with-dictionaries` and
``--tab=<dir>`` (separate file for each table) options are specified, necessary
compression dictionaries will be created in each output file using the
following fragment (regardless of the values of
:option:`add-drop-compression-dictionary` and
:option:`skip-add-drop-compression-dictionary` options).

.. code-block:: mysql

  /*!50633 CREATE COMPRESSION_DICTIONARY IF NOT EXISTS <dictionary>(...); */

Downgrade scenario
==================

If it is necessary to perform |Percona Server| downgrade from a version
:rn:`5.7.17-11` (or newer) to a version older than :rn:`5.7.17-11` and if
user databases have one or more table with compressed columns, there are two
options to do this safely:

1. Use ``mysqldump`` in compatible mode (no compressed columns extensions must
   be specified).

2. Manually remove the ``COMPRESSED`` attribute from all columns which have it
   via ``ALTER TABLE ... MODIFY ... COLUMN_FORMAT DEFAULT`` before updating
   server binaries.
   In this case, the downgraded server can start safely with old data files.

Version Specific Information
============================

  * :rn:`5.7.17-11`
    Feature implemented in |Percona Server| 5.7

System Variables
================

.. variable:: innodb_compressed_columns_zip_level

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Numeric
   :default: 6
   :range: ``0``-``9``

This variable is used to specify the compression level used for compressed
columns. Specifying ``0`` will use no compression, ``1`` the fastest and ``9``
the best compression. Default value is ``6``.

.. variable:: innodb_compressed_columns_threshold

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Numeric
   :default: 96
   :range: ``1`` - ``2^64-1`` (or ``2^32-1`` for 32-bit release)

By default a value being inserted will be compressed if its length exceeds
:variable:`innodb_compressed_columns_threshold` bytes. Otherwise, it will be
stored in raw (uncompressed) form.

Please also notice that because of the nature of some data, its compressed
representation can be longer than the original value. In this case it does not
make sense to store such values in compressed form as |Percona Server| would
have to waste both memory space and CPU resources for unnecessary
decompression. Therefore, even if the length of such non-compressible values
exceeds :variable:`innodb_compressed_columns_threshold`, they will be stored in
an uncompressed form (however, an attempt to compress them will still be made).

This parameter can be tuned in order to skip unnecessary attempts of data
compression for values that are known in advance by the user to have bad
compression ratio of their first N bytes.

Other reading
=============

* `How to find a good/optimal dictionary for zlib 'setDictionary' when
  processing a given set of data?
  <http://stackoverflow.com/questions/2011653/how-to-find-a-good-optimal-dictionary-for-zlib-setdictionary-when-processing-a>`_
