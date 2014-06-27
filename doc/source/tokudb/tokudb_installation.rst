.. _tokudb_installation:

=====================
 TokuDB Installation
=====================

|Percona Server| has added support for TokuDB storage engine in the :rn:`5.6.16-64.0-tokudb` release, and TokuDB storage engine is available as a separate package from :rn:`5.6.17-66.0` release.

`TokuDB <http://www.tokutek.com/products/tokudb-for-mysql/>`_ is a scalable, ACID and MVCC compliant storage engine that provides indexing-based query improvements, offers online schema modifications, and reduces slave lag for both hard disk drives and flash memory. This storage engine is specifically designed for high performance on write-intensive workloads which is achieved with Fractal Tree indexing.

TokuDB is currently supported only for 64-bit Linux distributions.

Prerequisites 
=============

``libjemalloc`` library
-----------------------

TokuDB storage engine requires ``libjemalloc`` library 3.3.0 or greater. If the version in the distribution repository is lower than that you can use one from :ref:`Percona Software Repositories <installation>` or download it from somewhere else.

If the ``libjemalloc`` wasn't installed and enabled before it will be automatically installed when installing the TokuDB storage engine package by using the ``apt`` or ``yum`` package manager, but |Percona Server| instance should be restarted for ``libjemalloc`` to be loaded. This way ``libjemalloc`` will be loaded with ``LD_PRELOAD``. You can also enable ``libjemalloc`` by specifying :variable:`malloc-lib` variable in the ``[mysqld_safe]`` section of the :file:`my.cnf` file: :: 

  [mysqld_safe]
  malloc-lib= /path/to/jemalloc


Transparent huge pages
----------------------

TokuDB won't be able to start if the transparent huge pages are enabled. `Transparent huge pages <https://access.redhat.com/site/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Performance_Tuning_Guide/s-memory-transhuge.html>`_ is feature available in the newer kernel versions. You can check if the Transparent huge pages are enabled with: ::
  
  $ cat /sys/kernel/mm/transparent_hugepage/enabled

   [always] madvise never

If transparent huge pages are enabled and you try to start the TokuDB engine you'll get the following message in you :file:`error.log`: ::

 Transparent huge pages are enabled, according to /sys/kernel/mm/redhat_transparent_hugepage/enabled
 Transparent huge pages are enabled, according to /sys/kernel/mm/transparent_hugepage/enabled

You can `disable <http://www.oracle-base.com/articles/linux/configuring-huge-pages-for-oracle-on-linux-64.php#disabling-transparent-hugepages>`_ them by passing ``transparent_hugepage=never`` to the kernel in your bootloader or by running the following command as root: 
  
.. code-block:: bash

  echo never > /sys/kernel/mm/transparent_hugepage/enabled
  echo never > /sys/kernel/mm/transparent_hugepage/defrag

Installation
============

TokuDB storage engine for |Percona Server| is currently available in our :ref:`apt <apt_repo>` and :ref:`yum <yum_repo>` repositories.

You can install the |Percona Server| with TokuDB engine by using the apt/yum commands:

.. code-block:: bash

 [root@centos ~]# yum install Percona-Server-tokudb-56.x86_64

or

.. code-block:: bash

 root@wheezy:~# apt-get install percona-server-tokudb-5.6


Enabling the TokuDB Storage Engine
==================================

This storage engine requires manual installation if there is a root password already set up during the new installation or upgrade. 

.. code-block:: mysql

 INSTALL PLUGIN tokudb SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_file_map SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_fractal_tree_info SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_fractal_tree_block_map SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_trx SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_locks SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_lock_waits SONAME 'ha_tokudb.so';

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

TokuDB Version
==============

TokuDB storage engine version can be checked with: 

.. code-block:: mysql
  
   mysql> SELECT @@tokudb_version;
   +------------------+
   | @@tokudb_version |
   +------------------+
   | tokudb-7.1.7-rc7 |
   +------------------+
   1 row in set (0.00 sec)


Upgrade
=======

Installing the TokuDB package is compatible with existing server setup and databases.

Version Specific Information
============================

 * :rn:`5.6.17-66.0`
    TokuDB storage engine available as a separate |Percona Server| package.


Other Reading
=============

* `Official TokuDB Documentation <http://www.tokutek.com/resources/product-docs/>`_
