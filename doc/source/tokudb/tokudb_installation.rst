.. _tokudb_installation:

=====================
 TokuDB Installation
=====================

.. warning:: 

   This feature is considered **ALPHA** quality and it isn't recommended for production use.

|Percona Server| has added support for TokuDB storage engine in the :rn:`5.6.16-64.0-tokudb` release. 

`TokuDB <http://www.tokutek.com/products/tokudb-for-mysql/>`_ is a scalable, ACID and MVCC compliant storage engine that provides indexing-based query improvements, offers online schema modifications, and reduces slave lag for both hard disk drives and flash memory. This storage engine is specifically designed for high performance on write-intensive workloads which is achieved with Fractal Tree indexing.

TokuDB is currently supported only for 64-bit Linux distributions.

Installation
============

|Percona Server| with TokuDB is currently available in our :ref:`apt experimental <apt_repo>` and :ref:`yum testing <yum_repo>` repositories. 

You can install the |Percona Server| with TokuDB engine by using the apt/yum commands:

.. code-block:: bash

 [root@centos ~]# yum install Percona-Server-tokudb-56.x86_64

or 

.. code-block:: bash

 root@wheezy:~# apt-get install percona-server-tokudb-5.6

.. note::

 TokuDB won't be able to start if the transparent huge pages are enabled. `Transparent huge pages <https://access.redhat.com/site/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Performance_Tuning_Guide/s-memory-transhuge.html>`_ is feature available in the newer kernel versions. You can check if the Transparent huge pages are enabled with: ::
  
   $ cat /sys/kernel/mm/transparent_hugepage/enabled

   [always] madvise never

 You can `disable <http://www.oracle-base.com/articles/linux/configuring-huge-pages-for-oracle-on-linux-64.php#disabling-transparent-hugepages>`_ them by passing ``transparent_hugepage=never`` to the kernel in your bootloader or by running: :: 

  $ echo never > /sys/kernel/mm/transparent_hugepage/enabled
  $ echo never > /sys/kernel/mm/transparent_hugepage/defrag


Upgrade
=======

Upgrading from other |Percona Server| 5.5 and 5.6 releases should work without problems (you should read :ref:`changed_in_56` before upgrading from |Percona Server| 5.5 to |Percona Server| 5.6 with TokuDB engine). 


Enabling the TokuDB support
===========================

This plugin requires manual installation because it isn't installed by default.

.. code-block:: mysql

 mysql> INSTALL PLUGIN tokudb SONAME 'ha_tokudb.so';
 mysql> INSTALL PLUGIN tokudb_file_map SONAME 'ha_tokudb.so';
 mysql> INSTALL PLUGIN tokudb_fractal_tree_info SONAME 'ha_tokudb.so';
 mysql> INSTALL PLUGIN tokudb_fractal_tree_block_map SONAME 'ha_tokudb.so';
 mysql> INSTALL PLUGIN tokudb_trx SONAME 'ha_tokudb.so';
 mysql> INSTALL PLUGIN tokudb_locks SONAME 'ha_tokudb.so';
 mysql> INSTALL PLUGIN tokudb_lock_waits SONAME 'ha_tokudb.so';

After the engine has been installed it should be present in the engines list. To check if the engine has been correctly installed and active: 

.. code-block:: mysql

 mysql> SHOW ENGINES;
 ...
 | TokuDB | YES | Tokutek TokuDB Storage Engine with Fractal Tree(tm) Technology | YES | YES | YES |
 ...

To check if all the TokuDB plugins have been installed correctly you should run:

.. code-block:: mysql

 mysql> SHOW PLUGINS;
 ...
 | TokuDB                        | ACTIVE   | STORAGE ENGINE     | ha_tokudb.so | GPL     |
 | TokuDB_file_map               | ACTIVE   | INFORMATION SCHEMA | ha_tokudb.so | GPL     |
 | TokuDB_fractal_tree_info      | ACTIVE   | INFORMATION SCHEMA | ha_tokudb.so | GPL     |
 | TokuDB_fractal_tree_block_map | ACTIVE   | INFORMATION SCHEMA | ha_tokudb.so | GPL     |
 | TokuDB_trx                    | ACTIVE   | INFORMATION SCHEMA | ha_tokudb.so | GPL     |
 | TokuDB_locks                  | ACTIVE   | INFORMATION SCHEMA | ha_tokudb.so | GPL     |
 | TokuDB_lock_waits             | ACTIVE   | INFORMATION SCHEMA | ha_tokudb.so | GPL     |
 ...

Other Reading
=============

* `Official TokuDB Documentation <http://www.tokutek.com/resources/product-docs/>`_
