.. _tokudb_file_management:

======================
TokuDB file management
======================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

As mentioned in the :ref:`tokudb_files_and_file_types` *Percona FT* is
extremely pedantic about validating its data set. If a file goes missing or
can't be accessed, or seems to contain some nonsensical data, it will
assert, abort or fail to start. It does this not to annoy you, but to try to
protect you from doing any further damage to your data.

This document contains examples of common file maintenance operations and
instructions on how to safely execute these operations.

The :ref:`tokudb_dir_per_db` option addressed two shortcomings the :ref:`renaming of data files
<improved_table_renaming_functionality>` on table/index rename, and the ability
to :ref:`group data files together <improved_directory_layout_functionality>`
within a directory that represents a single database. This feature is enabled
by default.

The :ref:`tokudb_dir_cmd` variable can be used to edit the contents of the TokuDB/PerconaFT directory map.

Moving TokuDB data files to a location outside of the default MySQL datadir
---------------------------------------------------------------------------

*TokuDB* uses the location specified by the :ref:`tokudb_data_dir`
variable for all of its data files. If the :ref:`tokudb_data_dir` variable
is not explicitly set, *TokuDB* will use the location specified by the servers
`datadir` for these files.

The *TokuDB* data files are protected from concurrent process access by the
``__tokudb_lock_dont_delete_me_data`` file that is located in the same
directory as the *TokuDB* data files.

*TokuDB* data files may be moved to other locations with symlinks left behind
in their place. If those symlinks refer to files on other physical data
volumes, the :ref:`tokudb_fs_reserve_percent` monitor will not traverse
the symlink and monitor the real location for adequate space in the file
system.

To safely move your TokuDB data files:

1. Shut the server down cleanly.

#. Change the :ref:`tokudb_data_dir` in your :file:`my.cnf` configuration
   file to the location where you wish to store your *TokuDB* data files.

#. Create your new target directory.

#. Move your ``*.tokudb`` files and your ``__tokudb_lock_dont_delete_me_data``
   from the current location to the new location.

#. Restart your server.

Moving TokuDB temporary files to a location outside of the default MySQL datadir
--------------------------------------------------------------------------------

*TokuDB* will use the location specified by the :ref:`tokudb_tmp_dir`
variable for all of its temporary files. If :ref:`tokudb_tmp_dir` variable
is not explicitly set, *TokuDB* will use the location specified by the
:ref:`tokudb_data_dir` variable. If the :ref:`tokudb_data_dir`
variable is also not explicitly set, *TokuDB* will use the location specified
by the servers `datadir` for these files.

*TokuDB* temporary files are protected from concurrent process access by the
``__tokudb_lock_dont_delete_me_temp`` file that is located in the same
directory as the *TokuDB* temporary files.

If you locate your *TokuDB* temporary files on a physical volume that is
different from where your *TokuDB* data files or recovery log files are
located, the :ref:`tokudb_fs_reserve_percent` monitor will not monitor their location for adequate space in the file system.

To safely move your *TokuDB* temporary files:

1. Shut the server down cleanly. A clean shutdown will ensure that there are no
   temporary files that need to be relocated.

#. Change the :ref:`tokudb_tmp_dir` variable in your :file:`my.cnf`
   configuration file to the location where you wish to store your new *TokuDB* temporary files.

#. Create your new target directory.

#. Move your ``__tokudb_lock_dont_delete_me_temp`` file from the current
   location to the new location.

#. Restart your server.

Moving TokuDB recovery log files to a location outside of the default MySQL datadir
-----------------------------------------------------------------------------------

TokuDB will use the location specified by the :ref:`tokudb_log_dir`
variable for all of its recovery log files. If the :ref:`tokudb_log_dir`
variable is not explicitly set, TokuDB will use the location specified by the
servers source/glossary.rst`datadir` for these files.

The *TokuDB* recovery log files are protected from concurrent process access by
the ``__tokudb_lock_dont_delete_me_logs`` file that is located in the same
directory as the *TokuDB* recovery log files.

TokuDB recovery log files may be moved to another location with symlinks left
behind in place of the :ref:`tokudb_log_dir`. If that symlink refers to a directory on another physical data volume, the
:ref:`tokudb_fs_reserve_percent` monitor will not traverse the symlink and
monitor the real location for adequate space in the file system.

To safely move your *TokuDB* recovery log files:

1. Shut the server down cleanly.

#. Change the :ref:`tokudb_log_dir` in your :file:`my.cnf` configuration
   file to the location where you wish to store your TokuDB recovery log files.

#. Create your new target directory.

#. Move your ``log*.tokulog*`` files and your
   ``__tokudb_lock_dont_delete_me_logs`` file from the current location to the
   new location.

#. Restart your server.

.. _improved_table_renaming_functionality:

Improved table renaming functionality
-------------------------------------

When you rename a *TokuDB* table via SQL, the data files on disk keep their
original names and only the mapping in the *Percona FT* directory file is
changed to map the new dictionary name to the original internal file names.
This makes it difficult to quickly match database/table/index names to their
actual files on disk, requiring you to use the
:ref:`refTOKUDB_FILE_MAP` table to cross reference.

The :ref:`tokudb_dir_per_db` variable is implemented to address this issue.

When :ref:`tokudb_dir_per_db` is enabled (``ON`` by default), this is no
longer the case. When you rename a table, the mapping in the *Percona FT*
directory file will be updated and the files will be renamed on disk to reflect
the new table name.

.. _improved_directory_layout_functionality:

Improved directory layout functionality
---------------------------------------

Many users have had issues with managing the huge volume of individual files
that *TokuDB* and *Percona FT* use. The :ref:`tokudb_dir_per_db` variable
addresses this issue.

When :ref:`tokudb_dir_per_db` variable is enabled (``ON`` by default),
all new tables and indices will be placed within their corresponding database
directory within the :file:`tokudb_data_dir` or server `datadir`.

If you have :ref:`tokudb_data_dir` variable set to something other than
the server `datadir`, *TokuDB* will create a directory matching the name
of the database, but upon dropping of the database, this directory will remain
behind.

Existing table files will not be automatically relocated to their corresponding
database directory.

You can easily move a tables data files into the new scheme and proper database
directory with a few steps:

.. code-block:: mysql

  mysql> SET GLOBAL tokudb_dir_per_db=true;
  mysql> RENAME TABLE <table> TO <tmp_table>;
  mysql> RENAME TABLE <tmp_table> TO <table>;

.. note::

  Two renames are needed because *MySQL* doesn't allow you to rename a table to
  itself. The first rename, renames the table to the temporary name and moves
  the table files into the owning database directory. The second rename sets the
  table name back to the original name. Tables can also be renamed/moved across
  databases and will be placed correctly into the corresponding database
  directory.

.. warning::

  You must be careful with renaming tables in case you have used any tricks to
  create symlinks of the database directories on different storage volumes, the
  move is not a simple directory move on the same volume but a physical copy
  across volumes. This can take quite some time and prevent access to the table
  being moved during the copy.

System Variables
================

.. _tokudb_dir_cmd:

.. rubric:: ``tokudb_dir_cmd``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - String

This variable is used to send commands to edit *TokuDB* directory files.

.. warning::

  Use this variable only if you know what you are doing otherwise it
  **WILL** lead to data loss.

Status Variables
================

.. _tokudb_dir_cmd_last_error:

.. rubric:: ``tokudb_dir_cmd_last_error``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Scope
     - Global
   * - Data type
     - Numeric

This variable contains the error number of the last executed command by using
the :ref:`tokudb_dir_cmd` variable.

.. _tokudb_dir_cmd_last_error_string:

.. rubric:: ``tokudb_dir_cmd_last_error_string``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Scope
     - Global
   * - Data type
     - Numeric

This variable contains the error string of the last executed command by using
the :ref:`tokudb_dir_cmd` variable.


..
  .. _editing_tokudb_files_with_tokudb_dir_cmd:

  Editing *TokuDB* directory map with :ref:`tokudb_dir_cmd`
  --------------------------------------------------------------

  .. note::

    This feature is currently considered *Experimental*.

  The :ref:`tokudb_dir_cmd` variable can be used to edit the *TokuDB*
  directory map.  **WARNING:** Use this variable only if you know what you're
  doing otherwise it **WILL** lead to data loss.

  This method can be used if any kind of system issue causes the loss of specific
  :file:`.tokudb` files for a given table, because the *TokuDB* tablespace file
  mapping will then contain invalid (nonexistent) entries, visible in
  :table:`INFORMATION_SCHEMA.TokuDB_file_map` table.

  This variable is used to send commands to edit directory file. The format of
  the command line is the following:

  .. code-block:: text

    command arg1 arg2 .. argn

  I.e, if we want to execute some command the following statement can be used:

  .. code-block:: mysql

    SET tokudb_dir_cmd = "command arg1 ... argn"

  Currently the following commands are available:

  * ``attach dictionary_name internal_file_name`` - attach internal_file_name to
    a dictionary_name, if the dictionary_name exists override the previous value,
    add new record otherwise
  * ``detach dictionary_name`` - remove record with corresponding
    dictionary_name, the corresponding internal_file_name file stays untouched
  * ``move old_dictionary_name new_dictionary_name`` - rename (only)
    dictionary_name from old_dictionary_name to new_dictionary_name

  Information about the dictionary_name and internal_file_name can be found in
  the :table:`TokuDB_file_map` table:

  .. code-block:: mysql

    mysql> SELECT dictionary_name, internal_file_name FROM INFORMATION_SCHEMA.TokuDB_file_map;
    +------------------------------+---------------------------------------------------------+
    | dictionary_name              | internal_file_name                                      |
    +------------------------------+---------------------------------------------------------+
    | ./world/City-key-CountryCode | ./_world_sql_340a_39_key_CountryCode_12_1_1d_B_1.tokudb |
    | ./world/City-main            | ./_world_sql_340a_39_main_12_1_1d_B_0.tokudb            |
    | ./world/City-status          | ./_world_sql_340a_39_status_f_1_1d.tokudb               |
    +------------------------------+---------------------------------------------------------+
