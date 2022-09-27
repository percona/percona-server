.. _toku_backup:

==================
Percona TokuBackup
==================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

Percona *TokuBackup* is an open-source hot backup utility for *MySQL* servers running the *TokuDB* storage engine (including *Percona Server for MySQL* and *MariaDB*). It does not lock your database during backup. The *TokuBackup* library intercepts system calls that write files and duplicates the writes to the backup directory.

.. note:: This feature is currently considered *tech preview* and should not be used in a production environment. 

.. contents::
   :local:

Installing From Binaries
------------------------

The installation of *TokuBackup* can be performed with the **ps-admin** script.

To install *Percona TokuBackup* complete the following steps. Run the following commands as root or by using the **sudo** command.

1. Run **ps-admin.enable-tokubackup** to add the ``preload-hotbackup`` option into **[mysqld_safe]** section of :file:`my.cnf`.

   .. admonition:: Output

      .. code-block:: bash

	 Checking SELinux status...
	 INFO: SELinux is disabled.

	 Checking if preload-hotbackup option is already set in config file...
	 INFO: Option preload-hotbackup is not set in the config file.

	 Checking TokuBackup plugin status...
	 INFO: TokuBackup plugin is not installed.

	 Adding preload-hotbackup option into /etc/my.cnf
	 INFO: Successfully added preload-hotbackup option into /etc/my.cnf
	 PLEASE RESTART MYSQL SERVICE AND RUN THIS SCRIPT AGAIN TO FINISH INSTALLATION!

2. Restart mysql service: :bash:`service mysql restart
3. Run :program:`ps-admin --enable-tokubackup` again to finish the installation of the *TokuBackup* plugin.

   .. admonition:: Output

      .. code-block:: bash

	 Checking SELinux status...
	 INFO: SELinux is disabled.

	 Checking if preload-hotbackup option is already set in config file...
	 INFO: Option preload-hotbackup is set in the config file.

	 Checking TokuBackup plugin status...
	 INFO: TokuBackup plugin is not installed.

	 Checking if Percona Server is running with libHotBackup.so preloaded...
	 INFO: Percona Server is running with libHotBackup.so preloaded.

	 Installing TokuBackup plugin...
	 INFO: Successfully installed TokuBackup plugin.

Making a Backup
---------------

To run *Percona TokuBackup*, the backup destination directory must
exist, be writable and owned by the same user under which *MySQL*
server is running (usually ``mysql``) and empty.

Once this directory is created, the backup can be run using the
following command:

.. code-block:: mysql

   mysql> set tokudb_backup_dir='/path_to_empty_directory';

.. note::

   Setting the :ref:`tokudb_backup_dir` variable automatically
   starts the backup process to the specified directory. Percona
   TokuBackup will take full backup each time, currently there is no
   incremental backup option

If you get any error on this step (e.g. caused by some
misconfiguration), the `Reporting Errors`_ section explains how to
find out the reason.

Restoring From Backup
---------------------

*Percona TokuBackup* does not have any functionality for restoring a
backup. You can use :command:`rsync` or :command:`cp` to restore the
files. You should check that the restored files have the correct
ownership and permissions.

.. note::

   Make sure that the datadir is empty and that *MySQL* server is shut
   down before restoring from backup. You can't restore to a datadir
   of a running mysqld instance (except when importing a partial
   backup).

The following example shows how you might use the :command:`rsync` command to restore the backup:

.. code-block:: bash

   $ rsync -avrP /data/backup/ /var/lib/mysql/

Since attributes of files are preserved, in most cases you will need to change their ownership to *mysql* before starting the database server. Otherwise, the files will be owned by the user who created the backup.

.. code-block:: bash

  $ chown -R mysql:mysql /var/lib/mysql

If you have changed default *TokuDB* data directory (:ref:`tokudb_data_dir`) or *TokuDB* log directory (:ref:`tokudb_log_dir`) or both of them, you will see separate folders for each setting in backup directory after taking backup. You'll need to restore each folder separately:

.. code-block:: bash

  $ rsync -avrP /data/backup/mysql_data_dir/ /var/lib/mysql/
  $ rsync -avrP /data/backup/tokudb_data_dir/ /path/to/original/tokudb_data_dir/
  $ rsync -avrP /data/backup/tokudb_log_dir/ /path/to/original/tokudb_log_dir/
  $ chown -R mysql:mysql /var/lib/mysql
  $ chown -R mysql:mysql /path/to/original/tokudb_data_dir
  $ chown -R mysql:mysql /path/to/original/tokudb_log_dir

Advanced Configuration
----------------------

.. contents::
   :local:

Monitoring Progress
*******************

*TokuBackup* updates the *PROCESSLIST* state while the backup is in progress. You can see the output by running ``SHOW PROCESSLIST`` or ``SHOW FULL PROCESSLIST``.

Excluding Source Files
**********************

You can exclude certain files and directories based on a regular expression set in the :ref:`tokudb_backup_exclude` session variable. If the source file name matches the excluded regular expression, then the source file is excluded from backup.

For example, to exclude all :file:`lost+found` directories from backup, use the following command:

.. code-block:: mysql

  mysql> SET tokudb_backup_exclude='/lost\\+found($|/)';

.. note::

   The server ``pid`` file is excluded by default. If you're providing your own
   additions to the exclusions and have the ``pid`` file in the default
   location, you will need to add the mysqld_safe.pid entry.

Throttling Backup Rate
**********************

You can throttle the backup rate using the :ref:`tokudb_backup_throttle` session-level variable. This variable throttles the write rate in bytes per second of the backup to prevent TokuBackup from crowding out other jobs in the system. The default and max value is 18446744073709551615.

.. code-block:: mysql

  mysql> SET tokudb_backup_throttle=1000000;

Restricting Backup Target
*************************

You can restrict the location of the destination directory where the backups can be located using the :ref:`tokudb_backup_allowed_prefix` system-level variable. Attempts to backup to a location outside of the specified directory or its children will result in an error.

The default is ``null``, backups have no restricted locations. This read-only variable can be set in the :file:`my.cnf` configuration file and displayed with the ``SHOW VARIABLES`` command:

.. code-block:: mysql

  mysql> SHOW VARIABLES LIKE 'tokudb_backup_allowed_prefix';
  +------------------------------+-----------+
  | Variable_name                | Value     |
  +------------------------------+-----------+
  | tokudb_backup_allowed_prefix | /dumpdir  |
  +------------------------------+-----------+


Reporting Errors
****************

*Percona TokuBackup* uses two variables to capture errors. They are :ref:`tokudb_backup_last_error` and :ref:`tokudb_backup_last_error_string`. When *TokuBackup* encounters an error, these will report on the error number and the error string respectively. For example, the following output shows these parameters following an attempted backup to a directory that was not empty:

.. code-block:: mysql

  mysql> SET tokudb_backup_dir='/tmp/backupdir';
  ERROR 1231 (42000): Variable 'tokudb_backup_dir' can't be set to the value of '/tmp/backupdir'

  mysql> SELECT @@tokudb_backup_last_error;
  +----------------------------+
  | @@tokudb_backup_last_error |
  +----------------------------+
  |                         17 |
  +----------------------------+

  mysql> SELECT @@tokudb_backup_last_error_string;
  +---------------------------------------------------+
  | @@tokudb_backup_last_error_string                 |
  +---------------------------------------------------+
  | tokudb backup couldn't create needed directories. |
  +---------------------------------------------------+

Using TokuDB Hot Backup for Replication
***************************************

TokuDB Hot Backup makes a transactionally consistent copy of the TokuDB
files while applications read and write to these files. The TokuDB hot
backup library intercepts certain system calls that writes files and duplicates
the writes on backup files while copying files to the backup directory. The
copied files contain the same content as the original files.

TokuDB Hot Backup also has an API. This API includes the ``start capturing`` and
``stop capturing`` commands. The "capturing" command starts the process, when a
portion of a file is copied to the backup location, and this portion is changed,
these changes are also applied to the backup location.

Replication often uses backup replication to create replicas. You must know the
last executed global transaction identifier (GTID) or binary log position both
for the replica and source configuration.

To lock tables, use ``FLUSH TABLE WITH READ LOCK`` or use the smart locks like
``LOCK TABLES FOR BACKUP`` or ``LOCK BINLOG FOR BACKUP``.

During the copy process, the binlog is flushed, and the changes are copied to
backup by the "capturing" mechanism. After everything has been copied, and the
"capturing" mechanism is still running, use the ``LOCK BINLOG FOR BACKUP``.
After this statement is executed, the binlog is flushed, the changes are
captured, and any queries that could change the binlog position or executed GTID
are blocked.

After this command, we can stop capturing and retrieve the last executed GTID or
binlog log position and unlock the binlog.

After a backup is taken, there are the following files in the backup directory:

* tokubackup_slave_info
* tokubackup_binlog_info

These files contain information for replica and source. You can use this
information to start a new replica from the source or replica.

The ``SHOW MASTER STATUS`` and ``SHOW SLAVE STATUS`` commands provide the
information.

.. important::

    As of *MySQL* 8.0.22, the ``SHOW SLAVE STATUS`` statement is
    `deprecated <https://dev.mysql.com/doc/refman/8.0/en/show-replicas.html>`_.
    Use ``SHOW REPLICA STATUS`` instead.
    
In specific binlog formats, a binary log event can contain statements that
produce temporary tables on the replica side, and the result of further statements
may depend on the temporary table content. Typically, temporary tables are not
selected for backup because they are created in a separate directory. A backup
created with temporary tables created by binlog events can cause issues when
restored because the temporary tables are not restored. The data may be
inconsistent.

The following system variables :ref:`--tokudb-backup-safe-slave`, which
enables or disables the safe-slave mode, and
:ref:`--tokudb-backup-safe-slave-timeout`, which defines the maximum amount
of time in seconds to wait until temporary tables disappear.  The
``safe-slave`` mode, when used with ``LOCK BINLOG FOR BACKUP``, the replica SQL
thread is stopped and checked to see if temporary tables produced by the replica
exist or do not exist. If temporary tables exist, the replica SQL thread is
restarted until there are no temporary tables or a defined timeout is reached.

You should not use this option for group-replication.
Create a Backup with a Timestamp
*********************************

If you plan to store more than one backup in a location, you should add a
timestamp to the backup directory name.

A sample Bash script has this information:

.. code-block:: bash

   #!/bin/bash

   tm=$(date "+%Y-%m-%d-%H-%M-%S");
   backup_dir=$PWD/backup/$tm;
   mkdir -p $backup_dir;
   bin/mysql -uroot -e "set tokudb_backup_dir='$backup_dir'"

Limitations and known issues
----------------------------

* You must disable *InnoDB* asynchronous IO if backing up *InnoDB* tables with *TokuBackup*. Otherwise you will have inconsistent, unrecoverable backups. The appropriate setting is ``innodb_use_native_aio=0``.

* To be able to run Point-In-Time-Recovery you'll need to manually get the binary log position.

* Transactional storage engines (*TokuDB* and *InnoDB*) will perform recovery on the backup copy of the database when it is first started.

* Tables using non-transactional storage engines (*MyISAM*) are not locked during the copy and may report issues when starting up the backup. It is best to avoid operations that modify these tables at the end of a hot backup operation (adding/changing users, stored procedures, etc.).

* The database is copied locally to the path specified in :file:`/path/to/backup`. This folder must exist, be writable, be empty, and contain enough space for a full copy of the database.

* *TokuBackup* always makes a backup of the *MySQL* :ref:`datadir` and optionally the :ref:`tokudb_data_dir`, :ref:`tokudb_log_dir`, and the binary log folder. The latter three are only backed up separately if they are not the same as or contained in the *MySQL* :ref:`datadir`. None of these three folders can be a parent of the *MySQL* :ref:`datadir`.

* No other directory structures are supported. All *InnoDB*, *MyISAM*, and other storage engine files must be within the *MySQL* :ref:`datadir`.

* *TokuBackup* does not follow symbolic links.

* *TokuBackup* does not backup *MySQL* configuration file(s).

* *TokuBackup* does not backup tablespaces if they are out of :ref:`datadir`.

* Due to upstream bug :mysqlbug:`80183`, *TokuBackup* can't recover backed-up table data if backup was taken while running ``OPTIMIZE TABLE`` or ``ALTER TABLE ... TABLESPACE``.

* *TokuBackup* doesn't support incremental backups.
