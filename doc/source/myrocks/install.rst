.. _myrocks_install:

==================================
Percona MyRocks Installation Guide
==================================

Percona MyRocks is distributed as a separate package that can be enabled as a
plugin for |Percona Server| 8.0 and later versions.

.. note::

   File formats across different MyRocks variants may not be compatible.
   |Percona Server| supports only *Percona MyRocks*.  Migrating from one variant
   to another requires a logical data dump and reload.

.. contents::
   :local:

Installing Percona MyRocks
==========================

It is recommended to install Percona software from official repositories:

1. Configure Percona repositories as described in
   `Percona Software Repositories Documentation
   <https://www.percona.com/doc/percona-repo-config/index.html>`_.

#. Install Percona MyRocks using the corresponding package manager:

   * For Debian or Ubuntu::

      $ sudo apt-get install percona-server-rocksdb

   * For RHEL or CentOS::

      $ sudo yum install percona-server-rocksdb

After you install the Percona MyRocks package,
you should see the following output::

* This release of |Percona Server| is distributed with RocksDB storage engine.
* Run the following script to enable the RocksDB storage engine in Percona Server:

  .. code-block:: bash 

     $ ps-admin --enable-rocksdb -u <mysql_admin_user> -p[mysql_admin_pass] [-S <socket>] [-h <host> -P <port>]

Run the ``ps-admin`` script as system root user or with :program:`sudo`
and provide the MySQL root user credentials
to properly enable the RocksDB (MyRocks) storage engine:

.. code-block:: bash

   $ sudo ps-admin --enable-rocksdb -u root -pPassw0rd

   Checking if RocksDB plugin is available for installation ...
   INFO: ha_rocksdb.so library for RocksDB found at /usr/lib64/mysql/plugin/ha_rocksdb.so.

   Checking RocksDB engine plugin status...
   INFO: RocksDB engine plugin is not installed.

   Installing RocksDB engine...
   INFO: Successfully installed RocksDB engine plugin.

.. note::

   When you use the ``ps-admin`` script to enable Percona MyRocks, it
   performs the following:

   * Disables Transparent huge pages
   * Installs and enables the RocksDB plugin

If the script returns no errors,
Percona MyRocks should be successfully enabled on the server.
You can verify it as follows:

.. code-block:: mysql

   mysql> SHOW ENGINES;
   +---------+---------+----------------------------------------------------------------------------+--------------+------+------------+
   | Engine  | Support | Comment                                                                    | Transactions | XA   | Savepoints |
   +---------+---------+----------------------------------------------------------------------------+--------------+------+------------+
   | ROCKSDB | YES     | RocksDB storage engine                                                     | YES          | YES  | YES        |
   ...
   | InnoDB  | DEFAULT | Percona-XtraDB, Supports transactions, row-level locking, and foreign keys | YES          | YES  | YES        |
   +---------+---------+----------------------------------------------------------------------------+--------------+------+------------+
   10 rows in set (0.00 sec)

Note that the RocksDB engine is not set to be default,
new tables will still be created using the InnoDB (XtraDB) storage engine.
To make RocksDB storage engine default,
set ``default-storage-engine=rocksdb`` in the ``[mysqld]`` section
of :file:`my.cnf` and restart |Percona Server|.

Alternatively, you can add ``ENGINE=RocksDB``
after the ``CREATE TABLE`` statement
for every table that you create.

Removing Percona MyRocks
========================

It will not be possible to access tables created using the RocksDB engine
with another storage engine after you remove Percona MyRocks.
If you need this data, alter the tables to another storage engine.
For example, to alter the ``City`` table to InnoDB, run the following:

.. code-block:: mysql

   mysql> ALTER TABLE City ENGINE=InnoDB;

To disable and uninstall the RocksDB engine plugins,
use the ``ps-admin`` script as follows:

.. code-block:: bash

   $ sudo ps-admin --disable-rocksdb -u root -pPassw0rd

   Checking RocksDB engine plugin status...
   INFO: RocksDB engine plugin is installed.

   Uninstalling RocksDB engine plugin...
   INFO: Successfully uninstalled RocksDB engine plugin.

After the engine plugins have been uninstalled,
remove the Percona MyRocks package:

* For Debian or Ubuntu::

  $ sudo apt-get remove percona-server-rocksdb-8.0

* For RHEL or CentOS::

  $ sudo yum remove percona-server-rocksdb-80.x86_64

Finally, remove all the :ref:`myrocks_server_variables`
from the configuration file (:file:`my.cnf`)
and restart |Percona Server|.


