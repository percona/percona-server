.. _tokudb_installation:

=====================
 TokuDB Installation
=====================

|Percona Server| is compatible with the separately available |TokuDB| storage engine package. The |TokuDB| engine must be separately downloaded and then enabled as a plug-in component. This package can be installed alongside with standard |Percona Server| 5.7 releases and does not require any specially adapted version of |Percona Server|.

The |TokuDB| storage engine is a scalable, ACID and MVCC compliant storage engine that provides indexing-based query improvements, offers online schema modifications, and reduces slave lag for both hard disk drives and flash memory. This storage engine is specifically designed for high performance on write-intensive workloads which is achieved with Fractal Tree indexing. To learn more about Fractal Tree indexing, you can visit the following `Wikipedia page <http://en.wikipedia.org/wiki/Fractal_tree_index>`_.

.. warning:: 

  Only the `Percona supplied <http://www.percona.com/downloads/Percona-Server-5.7/LATEST/>`_ |TokuDB| engine should be used with |Percona Server| 5.7. A |TokuDB| engine downloaded from other sources is not compatible. |TokuDB| file formats are not the same across |MySQL| variants. Migrating from one variant to any other variant requires a logical data dump and reload.

Prerequisites 
=============

``libjemalloc`` library
-----------------------

|TokuDB| storage engine requires ``libjemalloc`` library 3.3.0 or greater. If the version in the distribution repository is lower than that you can use one from :ref:`Percona Software Repositories <installation>` or download it from somewhere else.

If the ``libjemalloc`` wasn't installed and enabled before it will be automatically installed when installing the |TokuDB| storage engine package by using the :program:`apt`` or :program:`yum` package manager, but |Percona Server| instance should be restarted for ``libjemalloc`` to be loaded. This way ``libjemalloc`` will be loaded with ``LD_PRELOAD``. You can also enable ``libjemalloc`` by specifying :variable:`malloc-lib` variable in the ``[mysqld_safe]`` section of the :file:`my.cnf` file: :: 

  [mysqld_safe]
  malloc-lib= /path/to/jemalloc


Transparent huge pages
----------------------

|TokuDB| won't be able to start if the transparent huge pages are enabled. `Transparent huge pages <https://access.redhat.com/site/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Performance_Tuning_Guide/s-memory-transhuge.html>`_ is feature available in the newer kernel versions. You can check if the Transparent huge pages are enabled with: ::
  
  $ cat /sys/kernel/mm/transparent_hugepage/enabled

   [always] madvise never

If transparent huge pages are enabled and you try to start the TokuDB engine you'll get the following message in you :file:`error.log`: ::

 Transparent huge pages are enabled, according to /sys/kernel/mm/redhat_transparent_hugepage/enabled
 Transparent huge pages are enabled, according to /sys/kernel/mm/transparent_hugepage/enabled

You can `disable <http://www.oracle-base.com/articles/linux/configuring-huge-pages-for-oracle-on-linux-64.php#disabling-transparent-hugepages>`_ transparent huge pages permanently by passing ``transparent_hugepage=never`` to the kernel in your bootloader (**NOTE**: For this change to take an effect you'll need to reboot your server).

You can disable the transparent huge pages by running the following command as root (**NOTE**: Setting this will last only until the server is rebooted): 
  
.. code-block:: bash

  echo never > /sys/kernel/mm/transparent_hugepage/enabled
  echo never > /sys/kernel/mm/transparent_hugepage/defrag

Installation
============

|TokuDB| storage engine for |Percona Server| is currently available in our :ref:`apt <apt_repo>` and :ref:`yum <yum_repo>` repositories.

You can install the |Percona Server| with |TokuDB| engine by using the apt/yum commands:

.. code-block:: bash

 [root@centos ~]# yum install Percona-Server-tokudb-57.x86_64

or

.. code-block:: bash

 root@wheezy:~# apt-get install percona-server-tokudb-5.7

.. _tokudb_quick_install:

Enabling the TokuDB Storage Engine
==================================

Once the |TokuDB| server package has been installed following output will be shown:

.. code-block:: bash

  * This release of Percona Server is distributed with TokuDB storage engine.
     * Run the following script to enable the TokuDB storage engine in Percona Server:

      ps-admin --enable-tokudb -u <mysql_admin_user> -p[mysql_admin_pass] [-S <socket>] [-h <host> -P <port>]

     * See http://www.percona.com/doc/percona-server/5.7/tokudb/tokudb_installation.html for more installation details

     * See http://www.percona.com/doc/percona-server/5.7/tokudb/tokudb_intro.html for an introduction to TokuDB


|Percona Server| has implemented ``ps_tokudb_admin`` script to make the enabling the |TokuDB| storage engine easier. This script will automatically disable Transparent huge pages, if they're enabled, and install and enable the |TokuDB| storage engine with all the required plugins. You need to run this script as root or with :program:`sudo`. The script should only be used for local installations and should not be used to install TokuDB to a remote server. After you run the script with required parameters:


.. code-block:: bash

   ps-admin --enable-tokudb -uroot -pPassw0rd
   
Following output will be displayed:

.. code-block:: bash

   Checking if Percona server is running with jemalloc enabled...
   >> Percona server is running with jemalloc enabled.

   Checking transparent huge pages status on the system...
   >> Transparent huge pages are currently disabled on the system.

   Checking if thp-setting=never option is already set in config file...
   >> Option thp-setting=never is not set in the config file.
   >> (needed only if THP is not disabled permanently on the system)

   Checking TokuDB plugin status...
   >> TokuDB plugin is not installed.

   Adding thp-setting=never option into /etc/mysql/my.cnf
   >> Successfuly added thp-setting=never option into /etc/mysql/my.cnf

   Installing TokuDB engine...
   >> Successfuly installed TokuDB plugin.

If the script returns no errors, |TokuDB| storage engine should be successfully enabled on your server. You can check it out by running:

.. code-block:: mysql

  mysql> SHOW ENGINES;
  ...
   | TokuDB | YES | Tokutek TokuDB Storage Engine with Fractal Tree(tm) Technology | YES | YES | YES |
  ...

Enabling the TokuDB Storage Engine Manually
===========================================

If you don't want to use ``ps-admin`` script you'll need to manually install the storage engine ad required plugins. 

.. code-block:: mysql

 INSTALL PLUGIN tokudb SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_file_map SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_fractal_tree_info SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_fractal_tree_block_map SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_trx SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_locks SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_lock_waits SONAME 'ha_tokudb.so';
 INSTALL PLUGIN tokudb_background_job_status SONAME 'ha_tokudb.so';

After the engine has been installed it should be present in the engines list. To check if the engine has been correctly installed and active: 

.. code-block:: mysql

 mysql> SHOW ENGINES;
 ...
 | TokuDB | YES | Tokutek TokuDB Storage Engine with Fractal Tree(tm) Technology | YES | YES | YES |
 ...

To check if all the |TokuDB| plugins have been installed correctly you should run:

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
 | TokuDB_background_job_status  | ACTIVE   | INFORMATION SCHEMA | ha_tokudb.so | GPL     |
 ...

TokuDB Version
==============

|TokuDB| storage engine version can be checked with: 

.. code-block:: mysql
  
   mysql> SELECT @@tokudb_version;
   +------------------+
   | @@tokudb_version |
   +------------------+
   | 5.7.10-1rc1      |
   +------------------+
   1 row in set (0.00 sec)


Upgrade
=======

Installing the |TokuDB| package is compatible with existing server setup and databases.
