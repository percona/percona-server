.. _tokudb_file_management:

======================
TokuDB file management
======================

As mentioned in the :ref:`tokudb_files_and_file_types` |Percona FT| is
extremely pedantic about validating its data set. If a file goes missing or
can't be accessed, or seems to contain some nonsensical data, it will
assert, abort or fail to start. It does this not to annoy you, but to try to
protect you from doing any further damage to your data.

This document contains examples of common file maintenance operations and
instructions on how to safely execute these operations.

Beginning in Percona Server :rn:`5.6.33-79.0` a new server option was
introduced called :variable:`tokudb_dir_per_db`. This feature addressed two
shortcomings the :ref:`renaming of data files
<improved_table_renaming_functionality>` on table/index rename, and the ability
to :ref:`group data files together <improved_directory_layout_functionality>`
within a directory that represents a single database. This feature is disabled
by default. 

Moving TokuDB data files to a location outside of the default MySQL datadir
---------------------------------------------------------------------------

|TokuDB| uses the location specified by the :variable:`tokudb_data_dir`
variable for all of its data files. If the :variable:`tokudb_data_dir` variable
is not explicitly set, |TokuDB| will use the location specified by the servers
:term:`datadir` for these files.

The |TokuDB| data files are protected from concurrent process access by the
``__tokudb_lock_dont_delete_me_data`` file that is located in the same
directory as the |TokuDB| data files.

|TokuDB| data files may be moved to other locations with symlinks left behind
in their place. If those symlinks refer to files on other physical data
volumes, the :variable:`tokudb_fs_reserve_percent` monitor will not traverse
the symlink and monitor the real location for adequate space in the file
system.

To safely move your TokuDB data files:

1. Shut the server down cleanly.

#. Change the :variable:`tokudb_data_dir` in your :file:`my.cnf` configuration
   file to the location where you wish to store your |TokuDB| data files.

#. Create your new target directory.

#. Move your ``*.tokudb`` files and your ``__tokudb_lock_dont_delete_me_data``
   from the current location to the new location.

#. Restart your server.

Moving TokuDB temporary files to a location outside of the default MySQL datadir
--------------------------------------------------------------------------------

|TokuDB| will use the location specified by the :variable:`tokudb_tmp_dir`
variable for all of its temporary files. If :variable:`tokudb_tmp_dir` variable
is not explicitly set, |TokuDB| will use the location specified by the
:variable:`tokudb_data_dir` variable. If the :variable:`tokudb_data_dir`
variable is also not explicitly set, |TokuDB| will use the location specified
by the servers :term:`datadir` for these files.

|TokuDB| temporary files are protected from concurrent process access by the
``__tokudb_lock_dont_delete_me_temp`` file that is located in the same
directory as the |TokuDB| temporary files.

If you locate your |TokuDB| temporary files on a physical volume that is
different from where your |TokuDB| data files or recovery log files are
located, the :variable:`tokudb_fs_reserve_percent` monitor will not monitor
their location for adequate space in the file system.

To safely move your |TokuDB| temporary files:

1. Shut the server down cleanly. A clean shutdown will ensure that there are no
   temporary files that need to be relocated.

#. Change the :variable:`tokudb_tmp_dir` variable in your :file:`my.cnf`
   configuration file to the location where you wish to store your new |TokuDB|
   temporary files.

#. Create your new target directory.

#. Move your ``__tokudb_lock_dont_delete_me_temp`` file from the current
   location to the new location.

#. Restart your server.

Moving TokuDB recovery log files to a location outside of the default MySQL datadir
-----------------------------------------------------------------------------------

|TokuDB| will use the location specified by the :variable:`tokudb_log_dir`
variable for all of its recovery log files. If the :variable:`tokudb_log_dir`
variable is not explicitly set, |TokuDB| will use the location specified by the
servers :term:`datadir` for these files.

The |TokuDB| recovery log files are protected from concurrent process access by
the ``__tokudb_lock_dont_delete_me_logs`` file that is located in the same
directory as the |TokuDB| recovery log files.

|TokuDB| recovery log files may be moved to another location with symlinks left
behind in place of the :variable:`tokudb_log_dir`. If that symlink refers to a
directory on another physical data volume, the
:variable:`tokudb_fs_reserve_percent` monitor will not traverse the symlink and
monitor the real location for adequate space in the file system.

To safely move your |TokuDB| recovery log files:

1. Shut the server down cleanly.

#. Change the :variable:`tokudb_log_dir` in your :file:`my.cnf` configuration
   file to the location where you wish to store your |TokuDB| recovery log
   files.

#. Create your new target directory.

#. Move your ``log*.tokulog*`` files and your
   ``__tokudb_lock_dont_delete_me_logs`` file from the current location to the
   new location.

#. Restart your server.

.. _improved_table_renaming_functionality:

Improved table renaming functionality
-------------------------------------

When you rename a |TokuDB| table via SQL, the data files on disk keep their
original names and only the mapping in the |Percona FT| directory file is
changed to map the new dictionary name to the original internal file names.
This makes it difficult to quickly match database/table/index names to their
actual files on disk, requiring you to use the
:table:`INFORMATION_SCHEMA.TOKUDB_FILE_MAP` table to cross reference.

Beginning with |Percona Server| :rn:`5.6.33-79.0` a new server option was
introduced called :variable:`tokudb_dir_per_db` to address this issue. 

When :variable:`tokudb_dir_per_db` is enabled (``OFF`` by default), this is no
longer the case. When you rename a table, the mapping in the |Percona FT|
directory file will be updated and the files will be renamed on disk to reflect
the new table name.

.. _improved_directory_layout_functionality:

Improved directory layout functionality
---------------------------------------

Many users have had issues with managing the huge volume of individual files
that |TokuDB| and |Percona FT| use.

Beginning with |Percona Server| :rn:`5.6.33-79.0` a new server option was
introduced called :variable:`tokudb_dir_per_db` to address this issue. 

When :variable:`tokudb_dir_per_db` variable is enabled (``OFF`` by default),
all new tables and indices will be placed within their corresponding database
directory within the :file:`tokudb_data_dir` or server :term:`datadir`.

If you have :variable:`tokudb_data_dir` variable set to something other than
the server :term:`datadir`, |TokuDB| will create a directory matching the name
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

  Two renames are needed because |MySQL| doesn't allow you to rename a table to
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


