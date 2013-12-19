.. _upgrading_guide:

==========================================================
 Percona Server In-Place Upgrading Guide: From 5.1 to 5.5
==========================================================

In-place upgrades are those which are done using the existing data in the server. Generally speaking, this is stopping the server, installing the new server and starting it with the same data files. While they may not be suitable for high-complexity environments, they may be adequate for many scenarios.

Having this in mind, the changes in the in the 5.5 series can be grouped into 3 areas:

  * Server configuration

  * Server behavior and functioning

  * SQL changes

The following is a summary of the more relevant changes in the 5.5 series. For more details, see

  * :ref:`Percona Server documentation <dochome>`

  * http://dev.mysql.com/doc/refman/5.5/en/upgrading-from-previous-series.html


.. warning:: 
 Upgrade 5.1 to 5.5 on a crashed instance is not recommended. If the server instance has crashed, crash recovery should be run before proceeding with the upgrade.

Changes in Server Configuration
===============================

Features and Variables
----------------------

The configuration options and table columns for the following features have been modified in |Percona Server| 5.5:

.. list-table::
   :header-rows: 1

   * - Feature
     - 5.1 Series
     - 5.5 Series
   * - Improved InnoDB I/O Scalability
     - innodb_adaptive_checkpoint
     - innodb_adaptive_flushing_method
   * - Suppress Warning Messages
     - suppress_log_warning_1592
     - log_warnings_suppress
   * - Handle Corrupted Tables
     - innodb_pass_corrupt_table
     - innodb_corrupt_table_action
   * - Expand Table Import
     - innodb_expand_import
     - innodb_import_table_from_xtrabackup
   * - Dump/Restore of the Buffer Pool at Startup
     - innodb_auto_lru_dump
     - innodb_buffer_pool_restore_at_startup
   * - Slow Query Log
     - log_slow_timestamp_every
     - slow_query_log_timestamp_always
   * -  
     - slow_query_log_microseconds_timestamp
     - slow_query_log_timestamp_precision
   * -  
     - use_global_log_slow_control
     - slow_query_log_use_global_control
   * - Response Time Distribution
     - enable_query_response_time_stats
     - query_response_time_stats
   * - Multiple Rollback Segments
     - innodb_extra_rsegments
     - **(removed)**
   * - Dedicated Purge Thread
     - innodb_use_purge_thread
     - using upstream version

Shared Memory Buffer Pool
~~~~~~~~~~~~~~~~~~~~~~~~~

The :ref:`SHM buffer pool <innodb_buffer_pool_shm>` patch has been replaced with the safer :ref:`LRU Dump/Restore <innodb_lru_dump_restore>` patch, which provides similar improvements in restart performance and has the advantage of persisting across machine restarts.

The configuration variables for my.cnf have been kept for compatibility and warnings will be printed for the deprecated options (:variable:`innodb_buffer_pool_shm_key` and :variable:`innodb_buffer_pool_shm_checksum`) if used.

Instructions for disabling the SHM buffer pool can be found :ref:`here <innodb_buffer_pool_shm>` and for setting up LRU dump/restore :ref:`here <innodb_lru_dump_restore>`.

Multiple Rollback Segments
~~~~~~~~~~~~~~~~~~~~~~~~~~
Percona Server 5.1 offered a feature that enabled InnoDB to use
multiple rollback segments, relieving a major cause of resource
contention in write-intensive workloads. In MySQL 5.5, Oracle
implemented a similar feature, and so in Percona Server 5.5, the
``innodb_extra_rsegments`` option has been replaced by the MySQL 5.5
``innodb_rollback_segment`` option.


InnoDB Statistics
~~~~~~~~~~~~~~~~~

Three fields in table INNODB_INDEX_STATS were renamed:

.. list-table::
   :header-rows: 1

   * - 5.1 Series	
     - 5.5 Series
   * - row_per_keys
     - rows_per_key
   * - index_size
     - index_total_pages
   * - leaf_pages
     - index_leaf_pages

For more information, see its documentation documentation.

Process List
~~~~~~~~~~~~

The columns ROWS_EXAMINED, ROWS_SENT, and ROWS_READ have been added to the SHOW PROCESSLIST command and the table PROCESSLIST.

For more information, see its documentation documentation.

Grepping Old Variables
----------------------

You can check if old variables are being used in your configuration file by issuing the following line in a shell: ::

  egrep -ni 'innodb_adaptive_checkpoint|suppress_log_warning_1592|innodb_pass_corrupt_table|innodb_expand_import|innodb_auto_lru_dump|log_slow_timestamp_every|slow_query_log_microseconds_timestamp|use_global_log_slow_control|enable_query_response_time_stats|innodb_buffer_pool_shm_key|innodb_buffer_pool_shm_checksum' /PATH/TO/my.cnf

New Features
------------

You may also want to check the new features available in |Percona Server| 5.5:

  * Multiple Adaptive Hash Search Partitions

  * Crash-Resistant Replication

  * Show Engine InnoDB Status

  * Plugins

All plugins not included with Percona Server will have to be recompiled for |Percona Server| 5.5. There is a new plugin interface that complements the plugin API, plugins must be recompiled and linked to libmysqlservices. The plugins bundled with the server are already linked, you can list the installed plugins with the ``SHOW PLUGINS`` statement: ::

  mysql> SHOW PLUGINS; 
  +-----------------------+--------+--------------------+---------+---------+
  | Name                  | Status | Type               | Library | License |
  +-----------------------+--------+--------------------+---------+---------+
  | binlog                | ACTIVE | STORAGE ENGINE     | NULL    | GPL     |
   ...
  +-----------------------+--------+--------------------+---------+---------+

For more information, see:

  * http://dev.mysql.com/doc/refman/5.5/en/plugin-services.html

  * http://dev.mysql.com/doc/refman/5.5/en/plugin-installing-uninstalling.html

Upgrading from MySQL 5.1
------------------------

If you are upgrading from |MySQL| 5.1 instead of |Percona Server| 5.1, you should take into account that the |InnoDB| Plugin has been included in the standard |MySQL| 5.5 distribution as default for the InnoDB storage engine.

This change does not affect |Percona Server| as it has the |XtraDB| storage engine - an enhanced version of |InnoDB| - built-in since the 5.1 series. If you are migrating from |MySQL| 5.1.X, and you were using the |InnoDB| plugin, make sure to remove it from the configuration file by deleting the following two lines from the ``[mysqld]`` section: ::

  [mysqld]
  ignore-builtin-innodb  # <- DELETE
  plugin-load=innodb=ha_innodb_plugin.so # <- DELETE
 
otherwise, the server won't start. Strictly speaking, the ignore-builtin-innodb option will disable |XtraDB| in |Percona Server| 5.5 if set, and the server will not start if no other default storage engine is specified (i.e. ``default-storage-engine=MyISAM``).

Also, the variable innodb_file_io_threads has been replaced by innodb_read_io_threads and innodb_write_io_threads (these variables were already introduced in Percona Server 5.1). All of them defaults to 4, you should replace the old variable with the two new ones with the proper value (or delete it if the default - 4 - is acceptable).


Changes in Server Behavior and Functioning
===========================================

Privileges
----------

The schema of the grants tables in |MySQL| 5.5 has changed and a new table has been introduced, :table:`proxy_priv`.

The conversion to the new schema will be handled by :command:`mysql_upgrade` (see below).

Logs
----

The server will not rename the current log file with the suffix ``-old`` when issuing a ``FLUSH LOGS`` statement.

The renaming must be done by the user before flushing. It is important to note this as if it is not renamed before, the past log will be lost.

Numeric calculations
--------------------

On the numeric side, the server includes a new a library for conversions between strings and numbers, ``dtoa``.

This library provides the basis for an improved conversion between string or ``DECIMAL`` values and approximate-value (``FLOAT`` or ``DOUBLE``) numbers. Also, all numeric operators and functions on integer, floating-point and ``DECIMAL values`` throw an ``out of range`` error (``ER_DATA_OUT_OF_RANGE``) rather than returning an incorrect value or ``NULL``.

If an application rely on previous numeric results, it may have to be adjusted to the new precision or behavior.

Replication
-----------

When upgrading in a replication environment, a change in handling of ``IF NOT EXISTS`` results in an incompatibility for statement-based replication from a |MySQL| 5.1 master prior to 5.1.51 to a |MySQL| 5.5 slave.

If you use ``CREATE TABLE IF NOT EXISTS ... SELECT`` statements, upgrade the master first to 5.1.51 or higher.

Note that this differs from the usual replication upgrade advice of upgrading the slave first.

Indexes
-------

The stopword file is loaded and searched using ``latin1`` if ``character_set_server`` is ``ucs2``, ``utf16``, or ``utf32``. If any table was created with ``FULLTEXT`` indexes while the server character set was ``ucs2``, ``utf16``, or ``utf32``, it should be repaired using this statement ``REPAIR TABLE tbl_name QUICK;``.

Error Messages
--------------

The ``--language`` option has been deprecated and is an alias for ``--lc-messages-dir`` and ``--lc-messages``.

Also, error messages are now constructed in ``UTF-8`` and returned with ``character_set_results`` encoding.

Unicode Support
---------------

The Unicode implementation has been extended to provide support for supplementary characters that lie outside the Basic Multilingual Plane (BMP), introducing the ``utf16``, ``utf32`` and ``utf8mb4`` charsets.

If you are considering upgrading from utf8 to utf8mb4 to take advantage of the supplementary characters, you may have to adjust the size of the fields and indexes in the future. See http://dev.mysql.com/doc/refman/5.5/en/charset-unicode-upgrading.html.

Upgrading to ``utf8mb4`` will not take place unless you explicitly change the charset, i.e. with a ALTER TABLE… statement.

Changes in SQL
--------------

The following changes require modifications in the SQL statements in the client side:

  * ``INTO`` clauses are no longer accepted in nested SELECT statements. Modify the SQL statements to not contain the clause.

  * Alias declarations outside ``table_reference`` are not allowed for multiple-table ``DELETE`` statements. Modify those statements to use aliases only inside ``table_reference`` part.

  * Alias resolution does not require qualification and alias reference should not be qualified with the database name.

  * New reserved words:

    * ``GENERAL``

    * ``IGNORE_SERVER_IDS``

    * ``MASTER_HEARTBEAT_PERIOD``

    * ``MAXVALUE``

    * ``RESIGNAL``

    * ``SIGNAL``

    * ``SLOW``

  * ``TRUNCATE TABLE`` fails for a |XtraDB| table if there are any ``FOREIGN KEY`` constraints from other tables that reference the table. As |XtraDB| always use the fast truncation technique in 5.5 - equivalent to ``DROP TABLE`` and ``CREATE TABLE`` - you should modify the SQL statements to issue ``DELETE FROM table_name`` for such tables instead of ``TRUNCATE TABLE`` or an error will be returned in that cases.

BEFORE STARTING: FULL BACKUP
============================

Before starting the upgrade, a full backup of the data must be done. Doing a full backup will guarantee us the safety of going back without consequences if something goes wrong. After all, it's only one line: ::

  $ innobackupex --user=DBUSER --password=SECRET /path/where/to/store/backup/

This will backup all the data in your server to a time stamped subdirectory of the path provided.

|innobackupex| is a *Perl* script distributed with |XtraBackup|, a hot-backup utility for |MySQL| -based servers that doesn't block your database during the backup. If you don't have |XtraBackup| installed already, instructions can be found `here <http://www.percona.com/doc/percona-xtrabackup/>`_.

You should backup your entire configuration file - :file:`my.cnf` - also. The file is usually located in :file:`/etc/mysql/` or :file:`/etc/` or as :file:`.my.cnf` in user's home directory, ::

  $ cp /etc/mysql/my.cnf /path/where/to/store/backup/

While this is not an "in-place" upgrade technically, where possible, doing a full dump of the server's data for restoring it later is recommended. By this way, the indexes from all tables will be rebuilt explicitly, and any binary compatibility issue will be avoided: ::

  $ mysqldump --user=root -p --all-databases --routines > mydata.sql

This is not possible in some cases because of available space or downtime requirements, but if it is feasible, it is highly recommended.

Upgrading using the Percona repositories
========================================

The easiest and recommended way of installing - where possible - is by using the |Percona| repositories.

Instructions for enabling the repositories in a system can be found in:

  * :doc:`Percona APT Repository <installation/apt_repo>`

  * :doc:`Percona YUM Repository <installation/yum_repo>`

``DEB``-based distributions
---------------------------

Having done the full backup (or dump if possible), stop the server: ::

  $ sudo /etc/init.d/mysqld stop

and proceed to do the modifications needed in your configuration file, as explained at the beginning of this guide.

.. note:: 
 For extra safety doing the slow InnoDB shutdown before the upgrade is recommended.

Then install the new server with: ::

  $ sudo apt-get install percona-server-server-5.5

The installation script will run automatically :command:`mysql_upgrade` to migrate to the new grant tables, rebuild the indexes where needed and then start the server.

Note that this procedure is the same for upgrading from |MySQL| 5.1 or 5.5 to |Percona Server| 5.5.

``RPM``-based distributions
---------------------------

Having done the full backup (and dump if possible), stop the server: ::

  $ /sbin/service mysql stop

and check your installed packages with: ::

  $ rpm -qa | grep Percona-Server
  Percona-Server-client-51-5.1.57-rel12.8.232.rhel5.i686.rpm
  Percona-Server-server-51-5.1.57-rel12.8.232.rhel5.i686.rpm
  Percona-Server-shared-51-5.1.57-rel12.8.232.rhel5.i686.rpm

You may have a forth, ``shared-compat``, which is for compatibility purposes.

After checking, proceed to remove them without dependencies: ::

  $ rpm -qa | grep Percona-Server | xargs rpm -e --nodeps

It is important that you remove it without dependencies as many packages may depend on these (as they replace ``mysql``) and will be removed if omitted.

Note that this procedure is the same for upgrading from |MySQL| 5.1 or 5.5 to |Percona Server| 5.5: just grep ``'^mysql-'`` instead of ``Percona-Server`` and remove them.

You will have to install the following packages:

  * ``Percona-Server-server-55``

  * ``Percona-Server-client-55``

::

  $ yum install Percona-Server-server-55 Percona-Server-client-55

Once installed, proceed to modify your configuration file - :file:`my.cnf` - and recompile the plugins if necessary, as explained at the beginning of this guide.

As the schema of the grant table has changed, the server must be started without reading them: ::

  $  /usr/sbin/mysqld --skip-grant-tables --user=mysql &

and use ``mysql_upgrade`` to migrate to the new grant tables, it will rebuild the indexes needed and do the modifications needed: ::

  $ mysql_upgrade
  ...
  OK

Once this is done, just restart the server as usual: ::

  $ /sbin/service mysql restart

If it can't find the PID file, kill the server and start it normally: ::

  $ killall /usr/sbin/mysqld
  $ /sbin/service mysql start

Upgrading using Standalone Packages
===================================

DEB-based distributions
-----------------------

Having done the full backup (and dump if possible), stop the server: ::

  $ sudo /etc/init.d/mysqld stop

and remove the installed packages with their dependencies: ::

  $ sudo apt-get autoremove percona-server-server-51 percona-server-client-51

Once removed, proceed to do the modifications needed in your configuration file, as explained at the beginning of this guide.

Then, download the following packages for your architecture:

  * ``percona-server-server-5.5``

  * ``percona-server-client-5.5``

  * ``percona-server-common-5.5``

  * ``libmysqlclient16``

At the moment of writing this guide, for *Ubuntu* Maverick on ``i686``, a way of doing this is: ::

  $ wget -r -l 1 -nd -A deb -R "*dev*" http://www.percona.com/redir/downloads/Percona-Server-5.5/Percona-Server-5.5.12-20.3/deb/maverick/x86_64/  

Install them in one command: ::

  $ sudo dpkg -i *.deb

The installation won't succeed as there will be missing dependencies. To handle this, use:

  $ apt-get -f install

and all dependencies will be handled by :command:`apt`.

The installation script will run automatically :command:`mysql_upgrade` to migrate to the new grant tables and rebuild the indexes where needed.

RPM-based distributions
-----------------------

Having done the full backup (and dump if possible), stop the server: ::

  $ /sbin/service mysql stop

and check your installed packages: ::

  $ rpm -qa | grep Percona-Server
  Percona-Server-client-51-5.1.57-rel12.8.232.rhel5.i686.rpm
  Percona-Server-server-51-5.1.57-rel12.8.232.rhel5.i686.rpm
  Percona-Server-shared-51-5.1.57-rel12.8.232.rhel5.i686.rpm

You may have a forth, ``shared-compat``, which is for compatibility purposes.

After checked that, proceed to remove them without dependencies: ::

  $ rpm -qa | grep Percona-Server | xargs rpm -e --nodeps

It is important that you remove it without dependencies as many packages may depend on these (as they replace ``mysql``) and will be removed if ommited.

Note that this procedure is the same for upgrading from |MySQL| 5.1 to |Percona Server| 5.5, just grep ``'^mysql-'`` instead of ``Percona-Server`` and remove them.

Download the following packages for your architecture:

  * ``Percona-Server-server-55``

  * ``Percona-Server-client-55``

  * ``Percona-Server-shared-55``

At the moment of writing this guide, a way of doing this is: ::

  $ wget -r -l 1 -nd -A rpm -R "*devel*,*debuginfo*" http://www.percona.com/redir/downloads/Percona-Server-5.5/Percona-Server-5.5.12-20.3/RPM/rhel5/i686/

Install them in one command: ::

  $ rpm -ivh Percona-Server-server-55-5.5.12-rel20.3.118.rhel5.i686.rpm \ 
  Percona-Server-client-55-5.5.12-rel20.3.118.rhel5.i686.rpm \
  Percona-Server-shared-55-5.5.12-rel20.3.118.rhel5.i686.rpm

If you don't install all “at the same time”, you will need to do it in a specific order - ``shared``, ``client``, ``server``: ::

  $ rpm -ivh Percona-Server-shared-55-5.5.12-rel20.3.118.rhel5.i686.rpm
  $ rpm -ivh Percona-Server-client-55-5.5.12-rel20.3.118.rhel5.i686.rpm
  $ rpm -ivh Percona-Server-server-55-5.5.12-rel20.3.118.rhel5.i686.rpm 

Otherwise, the dependencies won't be met and the installation will fail.

Once installed, proceed to modify your configuration file - :file:`my.cnf` - and recompile the plugins if necessary, as explained at the beginning of this guide.

As the schema of the grant table has changed, the server must be started without reading them: ::

  $ /usr/sbin/mysqld --skip-grant-tables --user=mysql &

and use :file:`mysql_upgrade` to migrate to the new grant tables, it will rebuild the indexes needed and do the modifications needed: ::

  $ mysql_upgrade

After this is done, just restart the server as usual: ::

  $ /sbin/service mysql restart

If it can't find the pid file, kill the server and start it normally: ::

  $ killall /usr/sbin/mysqld
  $ /sbin/service mysql start

Other Reading
=============

 * `Upgrading MySQL: Best Practices <http://www.percona.tv/percona-webinars/upgrading-mysql-best-practices>`_ webinar,

 * `Upgrading MySQL webinar questiones <http://www.mysqlperformanceblog.com/2012/06/28/upgrading-mysql-webinar-question/>`_
