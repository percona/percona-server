.. _tokudb_compression:

====================
 TokuDB Compression
====================

TokuDB storage engine provides new compression options for TokuDB tables.

These options are defined on per-table basis by setting ``ROW_FORMAT`` during a ``CREATE TABLE`` or ``ALTER TABLE``. If no value is specified for ``ROW_FORMAT`` ``zlib`` compression is used by default. Currently available formats are:

 * ``TOKUDB_ZLIB`` - This compression is using ``zlib`` library and provides mid-range compression with medium CPU utilization.
 * ``TOKUDB_QUICKLZ`` - This compression is using ``quicklz`` library and provides light compression with low CPU utilization.
 * ``TOKUDB_LZMA`` - This compression is using ``lzma`` library and provides the highest compression with high CPU utilization.
 * ``TOKUDB_UNCOMPRESSED`` - This option disables the compression.

Example
-------

In order to create a new table with the highest compression you need to specify the ``TOKUDB_LZMA`` as the ``ROW_FORMAT``: 

.. code-block:: mysql

 mysql> CREATE TABLE `City` (
   > `ID` int(11) NOT NULL AUTO_INCREMENT,
     `Name` char(35) NOT NULL DEFAULT '',
     `CountryCode` char(3) NOT NULL DEFAULT '',
     `District` char(20) NOT NULL DEFAULT '',
     `Population` int(11) NOT NULL DEFAULT '0',
     PRIMARY KEY (`ID`),
     KEY `CountryCode` (`CountryCode`)
   ) ENGINE=TokuDB, ROW_FORMAT=TOKUDB_LZMA;

In order to change the compression for this table you need to change the ``ROW_FORMAT`` with ``ALTER TABLE`` command:

.. code-block:: mysql

 mysql> ALTER TABLE City ROW_FORMAT=TOKUDB_ZLIB;

.. note:: 

  When the compression format is changed it will affect only newly written data. In order to change the old data you'll need to run ``OPTIMIZE TABLE`` on that table.

  .. code-block:: mysql

    mysql> OPTIMIZE TABLE City;
