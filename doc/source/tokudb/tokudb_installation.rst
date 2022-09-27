.. _tokudb_installation:

================================================================================
 TokuDB Installation
================================================================================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

*Percona Server for MySQL* is compatible with the separately available *TokuDB* storage
engine package. The *TokuDB* engine must be separately downloaded and then
enabled as a plug-in component. This package can be installed alongside with
standard *Percona Server for MySQL* 8.0 releases and does not require any specially
adapted version of *Percona Server for MySQL*.

The *TokuDB* storage engine is a scalable, ACID and MVCC compliant storage
engine that provides indexing-based query improvements, offers online schema
modifications, and reduces replica lag for both hard disk drives and flash
memory. This storage engine is specifically designed for high performance on
write-intensive workloads which is achieved with Fractal Tree indexing. To learn
more about Fractal Tree indexing, you can visit the following `Wikipedia page
<http://en.wikipedia.org/wiki/Fractal_tree_index>`_.

.. warning::

   Only the `Percona supplied
   <http://www.percona.com/downloads/Percona-Server-8.0/LATEST/>`_
   *TokuDB* engine should be used with *Percona Server for MySQL* 8.0. A *TokuDB*
   engine downloaded from other sources is not compatible. *TokuDB*
   file formats are not the same across *MySQL* variants. Migrating
   from one variant to any other variant requires a logical data dump
   and reload.

Prerequisites
================================================================================

``libjemalloc`` library
--------------------------------------------------------------------------------

*TokuDB* storage engine requires ``libjemalloc`` library 3.3.0 or
greater. If the version in the distribution repository is lower than
that you can use one from :ref:`Percona Software Repositories
<installation>` or download it from somewhere else.

If the ``libjemalloc`` wasn't installed and enabled before it will be
automatically installed when installing the *TokuDB* storage engine
package by using the :program:`apt`` or :program:`yum` package
manager, but *Percona Server for MySQL* instance should be restarted for
``libjemalloc`` to be loaded. This way ``libjemalloc`` will be loaded
with ``LD_PRELOAD``. You can also enable ``libjemalloc`` by specifying
:ref:`malloc-lib` variable in the ``[mysqld_safe]`` section of
the :file:`my.cnf` file: ::

  [mysqld_safe]
  malloc-lib= /path/to/jemalloc

Transparent huge pages
--------------------------------------------------------------------------------

*TokuDB* won't be able to start if the transparent huge pages are
enabled. `Transparent huge pages
<https://access.redhat.com/site/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Performance_Tuning_Guide/s-memory-transhuge.html>`_
is feature available in the newer kernel versions. You can check if
the Transparent huge pages are enabled with: :bash:`cat /sys/kernel/mm/transparent_hugepage/enabled`

.. admonition:: Output

   .. code-block:: bash

      [always] madvise never

If transparent huge pages are enabled and you try to start the TokuDB
engine you'll get the following message in you :file:`error.log`: ::

 Transparent huge pages are enabled, according to /sys/kernel/mm/redhat_transparent_hugepage/enabled
 Transparent huge pages are enabled, according to /sys/kernel/mm/transparent_hugepage/enabled

You can `disable
<https://access.redhat.com/solutions/46111>`_
transparent huge pages permanently by passing
``transparent_hugepage=never`` to the kernel in your bootloader
(**NOTE**: For this change to take an effect you'll need to reboot
your server).

You can disable the transparent huge pages by running the following
command as root (**NOTE**: Setting this will last only until the
server is rebooted):

.. code-block:: bash

   echo never > /sys/kernel/mm/transparent_hugepage/enabled
   echo never > /sys/kernel/mm/transparent_hugepage/defrag

Installation
================================================================================

The *TokuDB* storage engine for *Percona Server for MySQL* is currently
available in our :ref:`apt <apt_repo>` and :ref:`yum <yum_repo>`
repositories.

You can install the *Percona Server for MySQL* with the *TokuDB* engine by using
the respective package manager:

:program:`yum`
   :bash:`yum install percona-server-tokudb.x86_64`
:program:`apt`
   :bash:`apt install percona-server-tokudb`

.. _tokudb_quick_install:

Enabling the TokuDB Storage Engine
================================================================================

Once the *TokuDB* server package is installed, the following output is shown:

.. admonition:: Output

   * This release of Percona Server is distributed with TokuDB storage engine.
     * Run the following script to enable the TokuDB storage engine in Percona Server:

       :bash:`ps-admin --enable-tokudb -u <mysql_admin_user> -p[mysql_admin_pass] [-S <socket>] [-h <host> -P <port>]`

     * See http://www.percona.com/doc/percona-server/8.0/tokudb/tokudb_installation.html for more installation details

     * See http://www.percona.com/doc/percona-server/8.0/tokudb/tokudb_intro.html for an introduction to TokuDB

*Percona Server for MySQL* has implemented :program:`ps-admin` to make the enabling the
*TokuDB* storage engine easier. This script will automatically disable
Transparent huge pages, if they're enabled, and install and enable the
*TokuDB* storage engine with all the required plugins. You need to run
this script as root or with :program:`sudo`. The script should only
be used for local installations and should not be used to install
TokuDB to a remote server. After you run the script
with required parameters:

*Percona Server for MySQL* has implemented ``ps_tokudb_admin`` script to make the enabling the *TokuDB* storage engine easier. This script will automatically disable Transparent huge pages, if they're enabled, and install and enable the *TokuDB* storage engine with all the required plugins. You need to run this script as root or with :program:`sudo`. The script should only be used for local installations and should not be used to install TokuDB to a remote server. After you run the script with required parameters:

.. code-block:: bash

   $ ps-admin --enable-tokudb -uroot -pPassw0rd

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

If the script returns no errors, *TokuDB* storage engine should be successfully enabled on your server. You can check it out by running :mysql:`SHOW ENGINES;`

.. admonition:: Output

   .. code-block:: mysql

      ...
      | TokuDB | YES | Tokutek TokuDB Storage Engine with Fractal Tree(tm) Technology | YES | YES | YES |
      ...

Enabling the TokuDB Storage Engine Manually
===========================================

If you don't want to use :program:`ps-admin` you'll need to manually install
the storage engine ad required plugins.

.. code-block:: mysql

   INSTALL PLUGIN tokudb SONAME 'ha_tokudb.so';
   INSTALL PLUGIN tokudb_file_map SONAME 'ha_tokudb.so';
   INSTALL PLUGIN tokudb_fractal_tree_info SONAME 'ha_tokudb.so';
   INSTALL PLUGIN tokudb_fractal_tree_block_map SONAME 'ha_tokudb.so';
   INSTALL PLUGIN tokudb_trx SONAME 'ha_tokudb.so';
   INSTALL PLUGIN tokudb_locks SONAME 'ha_tokudb.so';
   INSTALL PLUGIN tokudb_lock_waits SONAME 'ha_tokudb.so';
   INSTALL PLUGIN tokudb_background_job_status SONAME 'ha_tokudb.so';

After the engine has been installed it should be present in the
engines list. To check if the engine has been correctly installed and
active: :mysql:`SHOW ENGINES;`

.. admonition:: Output

   .. code-block:: mysql

      ...
      | TokuDB | YES | Tokutek TokuDB Storage Engine with Fractal Tree(tm) Technology | YES | YES | YES |
      ...

To check if all the *TokuDB* plugins have been installed correctly you should run: :mysql:`SHOW PLUGINS;`

.. admonition:: Output

   .. code-block:: mysql

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

*TokuDB* storage engine version can be checked with: :mysql:`SELECT @@tokudb_version;`

.. admonition:: Output

   .. code-block:: mysql

      +------------------+
      | @@tokudb_version |
      +------------------+
      | 8.0.13-3         |
      +------------------+
      1 row in set (0.00 sec)


Upgrade
=======

Before upgrading to *Percona Server for MySQL* 8.0, make sure that your system is ready by
running :program:`mysqlcheck`: :bash:`mysqlcheck -u root -p --all-databases
--check-upgrade`

.. warning::

   With partitioned tables that use the *TokuDB* or *MyRocks* storage engine, the
   upgrade only works with native partitioning.

.. seealso::

   *MySQL* Documentation: Preparing Your Installation for Upgrade
      https://dev.mysql.com/doc/refman/8.0/en/upgrade-prerequisites.html

.. include:: ../.res/replace.txt
.. include:: ../.res/replace.program.txt
