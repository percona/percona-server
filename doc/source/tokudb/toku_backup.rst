.. _toku_backup:

==================
Percona TokuBackup
==================

Percona |TokuBackup| is an open-source hot backup utility for |MySQL| servers running the |TokuDB| storage engine (including |Percona Server| and |MariaDB|). It does not lock your database during backup. The |TokuBackup| library intercepts system calls that write files and duplicates the writes to the backup directory.

.. contents::
   :local:

Installing From Binaries
------------------------

|TokuBackup| is included with |Percona Server| :rn:`5.6.26-74.0` and later versions. Installation can be performed with the ``ps_tokudb_admin`` script.

To install |Percona TokuBackup|:

1. Run ``ps_tokudb_admin --enable-backup`` to add the ``preload-hotbackup`` option into **[mysqld_safe]** section of :file:`my.cnf`.

  .. code-block:: bash

    $ sudo ps_tokudb_admin --enable-backup
    Checking SELinux status...
    INFO: SELinux is disabled.

    Checking if preload-hotbackup option is already set in config file...
    INFO: Option preload-hotbackup is not set in the config file.

    Checking TokuBackup plugin status...
    INFO: TokuBackup plugin is not installed.

    Adding preload-hotbackup option into /etc/my.cnf
    INFO: Successfully added preload-hotbackup option into /etc/my.cnf
    PLEASE RESTART MYSQL SERVICE AND RUN THIS SCRIPT AGAIN TO FINISH INSTALLATION!

2. Restart mysql service

  .. code-block:: bash

    $ sudo service mysql restart

3. Run ``ps_tokudb_admin --enable-backup`` again to finish installation of |TokuBackup| plugin

  .. code-block:: bash

    $ sudo ps_tokudb_admin --enable-backup
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

To run |Percona TokuBackup|, the backup destination directory must exist, be writable and owned by the same user under which |MySQL| server is running (usually ``mysql``) and empty. Once this directory is created, the backup can be run using the following command:

.. code-block:: mysql

  mysql> set tokudb_backup_dir='/path_to_empty_directory';

.. note:: Setting the :variable:`tokudb_backup_dir` variable automatically starts the backup process to the specified directory. Percona TokuBackup will take full backup each time, currently there is no incremental backup option

Restoring From Backup
---------------------

|Percona TokuBackup| does not have any functionality for restoring a backup. You can use :command:`rsync` or :command:`cp` to restore the files. You should check that the restored files have the correct ownership and permissions.

.. note:: Make sure that the datadir is empty and that |MySQL| server is shut down before restoring from backup. You can't restore to a datadir of a running mysqld instance (except when importing a partial backup).

The following example shows how you might use the :command:`rsync` command to restore the backup:

.. code-block:: bash

  $ rsync -avrP /data/backup/ /var/lib/mysql/

Since attributes of files are preserved, in most cases you will need to change their ownership to *mysql* before starting the database server. Otherwise, the files will be owned by the user who created the backup.

.. code-block:: bash

  $ chown -R mysql:mysql /var/lib/mysql

If you have changed default |TokuDB| data directory (:variable:`tokudb_data_dir`) or |TokuDB| log directory (:variable:`tokudb_log_dir`) or both of them, you will see separate folders for each setting in backup directory after taking backup. You'll need to restore each folder separately:

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

|TokuBackup| updates the *PROCESSLIST* state while the backup is in progress. You can see the output by running ``SHOW PROCESSLIST`` or ``SHOW FULL PROCESSLIST``.

Excluding Source Files
**********************

You can exclude certain files and directories based on a regular expression set in the :variable:`tokudb_backup_exclude` session variable. If the source file name matches the excluded regular expression, then the source file is excluded from backup.

For example, to exclude all :file:`lost+found` directories from backup, use the following command:

.. code-block:: mysql

  mysql> SET tokudb_backup_exclude='/lost\\+found($|/)';

Throttling Backup Rate
**********************

You can throttle the backup rate using the :variable:`tokudb_backup_throttle` session-level variable. This variable throttles the write rate in bytes per second of the backup to prevent TokuBackup from crowding out other jobs in the system. The default and max value is 18446744073709551615.

.. code-block:: mysql

  mysql> SET tokudb_backup_throttle=1000000;

Restricting Backup Target
*************************

You can restrict the location of the destination directory where the backups can be located using the :variable:`tokudb_backup_allowed_prefix` system-level variable. Attempts to backup to a location outside of the specified directory or its children will result in an error.

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

|Percona TokuBackup| uses two variables to capture errors. They are :variable:`tokudb_backup_last_error` and :variable:`tokudb_backup_last_error_string`. When |TokuBackup| encounters an error, these will report on the error number and the error string respectively. For example, the following output shows these parameters following an attempted backup to a directory that was not empty:

.. code-block:: mysql

  mysql> SET tokudb_backup_dir='/tmp/backupdir';
  ERROR 1231 (42000): Variable 'tokudb_backup_dir' cannot be set to the value of '/tmp/backupdir'

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

Performing a TokuDB Hotbackup and Replication
-----------------------------------------------

TokuDB Hotbackup is a solution which allows you to do a backup as a database snapshot. A snapshot is a point-in-time view of the database. The Hotbackup is a special library which intercepts specific system calls used to duplicate data. This solution generates copied files which contain the same content as the original files.

The transaction log should be flushed upon transaction commit, set the :variable:``tokudb_commit_sync`` variable to "ON". When the binlog is flushed, these changes are copied to the backup. Replication often uses backup replication to create slaves. You must know the last executed GTID or binary log position for both the slave configuration and the master configuration.

The process to bootstrap a slave is as follows:

      1. Copy the backup to another server you intend to slave to your master,
         which produced the backup, as if you were doing a server restore.

      2. Start this copied/second/slave server instance and allow and wait for crashrecovery to run.

      3. Run ``SHOW MASTER STATUS`` on the slave server instance to obtain the current binlog position for the slave server.

      4. Shut down the slave server instance and set it up as your actual slave by editing the serverid and making typical changes to my.cnf.

      5. Restart the slave server instance.

      6. On the slave server instance, run the ``CHANGE MASTER TO`` statement with the information from the ``SHOW MASTER STATUS`` result.

The slave server should now be up and running

Limitations and known issues
----------------------------

* You must disable |InnoDB| asynchronous IO if backing up |InnoDB| tables with |TokuBackup|. Otherwise you will have inconsistent, unrecoverable backups. The appropriate setting is ``innodb_use_native_aio=0``.

* To be able to run Point-In-Time-Recovery you must manually get the binary log position.

* Transactional storage engines (|TokuDB| and |InnoDB|) will perform recovery on the backup copy of the database when it is first started.

* Tables using non-transactional storage engines (|MyISAM|) are not locked during the copy and may report issues when starting up the backup. It is best to avoid operations that modify these tables at the end of a hot backup operation (adding/changing users, stored procedures, etc.).

* The database is copied locally to the path specified in :file:`/path/to/backup`. This folder must exist, be writable, be empty, and contain enough space for a full copy of the database.

* |TokuBackup| always makes a backup of the |MySQL| :variable:`datadir` and optionally the :variable:`tokudb_data_dir`, :variable:`tokudb_log_dir`, and the binary log folder. The latter three are only backed up separately if they are not the same as or contained in the |MySQL| :variable:`datadir`. None of these three folders can be a parent of the |MySQL| :variable:`datadir`.

* No other directory structures are supported. All |InnoDB|, |MyISAM|, and other storage engine files must be within the |MySQL| :variable:`datadir`.

* |TokuBackup| does not follow symbolic links.

* |TokuBackup| does not backup |MySQL| configuration file(s).

* |TokuBackup| does not backup tablespaces if they are out of :variable:`datadir`.

* Due to upstream bug :mysqlbug:`80183`, |TokuBackup| cannot recover backed-up table data if backup was taken while running ``OPTIMIZE TABLE`` or ``ALTER TABLE ... TABLESPACE``.

* |TokuBackup| does not support incremental backups.
