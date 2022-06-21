.. _tokudb_files_and_file_types:

===========================
TokuDB files and file types
===========================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

The *TokuDB* file set consists of many different files that all serve various
purposes.

If you have any *TokuDB* data your data directory should look similar to this:

.. code-block:: bash

  root@server:/var/lib/mysql# ls -lah
  ...
  -rw-rw----  1 mysql mysql  76M Oct 13 18:45 ibdata1
  ...
  -rw-rw----  1 mysql mysql  16K Oct 13 15:52 tokudb.directory
  -rw-rw----  1 mysql mysql  16K Oct 13 15:52 tokudb.environment
  -rw-------  1 mysql mysql    0 Oct 13 15:52 __tokudb_lock_dont_delete_me_data
  -rw-------  1 mysql mysql    0 Oct 13 15:52 __tokudb_lock_dont_delete_me_environment
  -rw-------  1 mysql mysql    0 Oct 13 15:52 __tokudb_lock_dont_delete_me_logs
  -rw-------  1 mysql mysql    0 Oct 13 15:52 __tokudb_lock_dont_delete_me_recovery
  -rw-------  1 mysql mysql    0 Oct 13 15:52 __tokudb_lock_dont_delete_me_temp
  -rw-rw----  1 mysql mysql  16K Oct 13 15:52 tokudb.rollback
  ...

This document lists the different types of *TokuDB* and *Percona Fractal Tree*
files, explains their purpose, shows their location and how to move them
around.

tokudb.environment
------------------

This file is the root of the *Percona FT* file set and contains various bits of
metadata about the system, such as creation times, current file format
versions, etc.

*Percona FT* will create/expect this file in the directory specified by the
*MySQL* `datadir`.

tokudb.rollback
---------------

Every transaction within *Percona FT* maintains its own transaction rollback
log. These logs are stored together within a single *Percona FT* dictionary
file and take up space within the *Percona FT* cachetable (just like any other
*Percona FT* dictionary).

The transaction rollback logs will ``undo`` any changes made by a transaction
if the transaction is explicitly rolled back, or rolled back via recovery as a
result of an uncommitted transaction when a crash occurs.

*Percona FT* will create/expect this file in the directory specified by the
*MySQL* `datadir`.

tokudb.directory
----------------

*Percona FT* maintains a mapping of a dictionary name (example:
``sbtest.sbtest1.main``) to an internal file name (example:
``_sbtest_sbtest1_main_xx_x_xx.tokudb``). This mapping is stored within this
single *Percona FT* dictionary file and takes up space within the *Percona FT*
cachetable just like any other *Percona FT* dictionary.

*Percona FT* will create/expect this file in the directory specified by the
*MySQL* `datadir`.

Dictionary files
----------------

*TokuDB* dictionary (data) files store actual user data. For each *MySQL*
table there will be:

* One ``status`` dictionary that contains metadata about the table.

* One ``main`` dictionary that stores the full primary key (an imaginary key is
  used if one was not explicitly specified) and full row data.

* One ``key`` dictionary for each additional key/index on the table.

These are typically named:
``_<database>_<table>_<key>_<internal_txn_id>.tokudb``

*Percona FT* creates/expects these files in the directory specified by
:ref:`tokudb_data_dir` if set, otherwise the *MySQL* ``datadir`` is used.

Recovery log files
------------------

The *Percona FT* recovery log records every operation that modifies a
*Percona FT* dictionary. Periodically, the system will take a snapshot of the
system called a checkpoint. This checkpoint ensures that the modifications
recorded within the *Percona FT* recovery logs have been applied to the
appropriate dictionary files up to a known point in time and synced to disk.

These files have a rolling naming convention, but use:
``log<log_file_number>.tokulog<log_file_format_version>``.

*Percona FT* creates/expects these files in the directory specified by
:ref:`tokudb_log_dir` if set, otherwise the *MySQL* `datadir` is used.

*Percona FT* does not track what log files should or shouldn't be present. Upon
startup, it discovers the logs in the log directory, and replays them in order.
If the wrong logs are present, the recovery aborts and possibly damages the
dictionaries.

Temporary files
---------------

*Percona FT* might need to create some temporary files in order to perform some
operations. When the bulk loader is active, these temporary files might grow to
be quite large.

As different operations start and finish, the files will come and go.

There are no temporary files left behind upon a clean shutdown,

*Percona FT* creates/expects these files in the directory specified by
:ref:`tokudb_tmp_dir` if set. If not, the :ref:`tokudb_data_dir` is
used if set, otherwise the *MySQL* `datadir` is used.

Lock files
----------

*Percona FT* uses lock files to prevent multiple processes from accessing and
writing to the files in the assorted *Percona FT* functionality areas. Each
lock file will be in the same directory as the file(s) that it is protecting.

These empty files are only used as semaphores across processes. They are safe
to delete/ignore as long as no server instances are currently running and using
the data set.

``__tokudb_lock_dont_delete_me_environment``

``__tokudb_lock_dont_delete_me_recovery``

``__tokudb_lock_dont_delete_me_logs``

``__tokudb_lock_dont_delete_me_data``

``__tokudb_lock_dont_delete_me_temp``

*Percona FT* is extremely pedantic about validating its data set. If a file
goes missing or unfound, or seems to contain some nonsensical data, it will
assert, abort or fail to start. It does this not to annoy you, but to try to
protect you from doing any further damage to your data.
