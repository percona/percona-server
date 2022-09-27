.. _upgrading_tokudb_myrocks:

=================================================================================================
Upgrading from Systems that Use the *MyRocks* or *TokuDB* Storage Engine and Partitioned Tables
=================================================================================================

Due to the limitation imposed by *MySQL*, the storage engine provides support for partitioning. *MySQL* 8.0 only provides support for
partitioned table for the *InnoDB* storage engine. 

If you use partitioned tables with the *MyRocks* or *TokuDB* storage engine, the
upgrade may fail if you do not enable the native partitioning provided by the storage engine.

*TokuDB* is deprecated. For more information, see :ref:`tokudb_intro`.

Before you attempt the upgrade, check whether you have any tables that are not using the native partitioning.

.. code-block:: bash

   $ mysqlcheck -u root --all-databases --check-upgrade

If tables are found, **mysqlcheck** issues a warning:

.. admonition:: Output of **mysqlcheck** detecting a table that is not using the native partitioning

   .. code-block:: bash

      | comp_test.t1_RocksDB_lz4     OK
      | warning  : The partition engine, used by table '<table-name>',
      | is deprecated and will be removed in a future release. Please use native partitioning instead.

Enable either the `rocksdb_enable_native_partition` variable or
the `tokudb_enable_native_partition` variable depending on the storage
engine and restart the server. 

.. important::

   The `rocksdb_enable_native_partition` variable is **experimental** and should not be used in a production environment in **Percona Server for MySQL** 5.7 unless that environment is being upgraded.

Your next step is to alter the tables that are not using the native partitioning with the
:mysql:`UPGRADE PARTITIONING` clause:

.. code-block:: mysql

   ALTER TABLE <table-name> UPGRADE PARTITIONING

Complete these steps for each table that **mysqlcheck** list. Otherwise, the upgrade to 8.0 fails and your error log contains messages like the following:

.. code-block:: mysql

   2018-12-17T18:34:14.152660Z 2 [ERROR] [MY-013140] [Server] The 'partitioning' feature is not available; you need to remove '--skip-partition' or use MySQL built with '-DWITH_PARTITION_STORAGE_ENGINE=1'
   2018-12-17T18:34:14.152679Z 2 [ERROR] [MY-013140] [Server] Can't find file: './comp_test/t1_RocksDB_lz4.frm' (errno: 0 - Success)
   2018-12-17T18:34:14.152691Z 2 [ERROR] [MY-013137] [Server] Can't find file: './comp_test/t1_RocksDB_lz4.frm' (OS errno: 0 - Success)

.. seealso::

   *MySQL* Documentation: Partitioning Limitations Relating to Storage Engines
      https://dev.mysql.com/doc/refman/8.0/en/partitioning-limitations-storage-engines.html

Performing a Distribution upgrade in-place on a System with installed Percona packages
--------------------------------------------------------------------------------------------
The recommended process for performing a distribution upgrade on a system with
the Percona packages installed is the following:

    1. Record the installed Percona packages.
    2. Backup the data and configurations.
    3. Uninstall the Percona packages without removing the configuration file or data.
    4. Perform the upgrade by following the distribution upgrade instructions
    5. Reboot the system.
    6. Install the Percona packages intended for the upgraded version of the distribution.