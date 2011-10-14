.. _innodb_expand_import_page:

===================
Expand Table Import
===================

Unlike MyISAM, |InnoDB| does not allow users to copy datafiles for a single table between servers. If exported with XtraBackup, a table can now be imported on another server running |XtraDB|.

This feature implements the abililty to import arbitrary .ibd files exported using the XtraBackup ``--export`` option. The :variable:`innodb_expand_import` variable makes to convert ``.ibd`` file during import process.

The normal version can import only the backed-up .ibd file at the same place.


Example
=======

Assuming that:

  * :variable:`innodb_expand_import` is set to ``1``.

  * the files (``.ibd`` and ``.exp``) are prepared by the ``xtrabackup --prepare --export`` command.

First create “exactly same” structured tables to the target database.

Then discard the tables as preparation of import, for example, ::

  mysql> set FOREIGN_KEY_CHECKS=0;
  Query OK, 0 rows affected (0.00 sec)

  mysql> alter table customer discard tablespace;
  Query OK, 0 rows affected (0.01 sec)

  mysql> alter table district discard tablespace;
  Query OK, 0 rows affected (0.01 sec)

  mysql> alter table history discard tablespace;
  Query OK, 0 rows affected (0.00 sec)

  ...
  put the .ibd and .exp files at the same place to .frm file.
  import the tables
  (command example)
  mysql> set FOREIGN_KEY_CHECKS=0;
  Query OK, 0 rows affected (0.00 sec)

  mysql> set global innodb_expand_import=1;
  Query OK, 0 rows affected (0.00 sec)

  mysql> alter table customer import tablespace;
  Query OK, 0 rows affected (0.17 sec)

  mysql> alter table district import tablespace;
  Query OK, 0 rows affected (0.00 sec)

  mysql> alter table history import tablespace;
  Query OK, 0 rows affected (0.04 sec)

  ...
  (.err file example)
  InnoDB: import: extended import of tpcc2/customer is started.
  InnoDB: import: 2 indexes are detected.
  InnoDB: Progress in %: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 done.
  InnoDB: import: extended import of tpcc2/district is started.
  InnoDB: import: 1 indexes are detected.
  InnoDB: Progress in %: 16 33 50 66 83 100 done.
  InnoDB: import: extended import of tpcc2/history is started.
  InnoDB: import: 3 indexes are detected.
  InnoDB: Progress in %: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 done.
  ...

Version Specific Information
============================

  * 5.5.10-20.1:
    Renamed variable :variable:`innodb_expand_import` to :variable:`innodb_import_table_from_xtrabackup`.

System Variables
================

.. variable:: innodb_expand_import

     :version 5.5.10-20.1: Renamed.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :default: 0
     :range: 0-1

If set to 1, ``.ibd`` file is converted (``space id``, ``index id``, etc.) with index information in ``.exp`` file during the import process (``ALTER TABLE ... IMPORT TABLESPACE`` command).

 This variable was renamed to :variable:`innodb_import_table_from_xtrabackup`, beginning in release 5.5.10-20.1. It still exists as :variable:`innodb_expand_import` in versions prior to that.


.. variable:: innodb_import_table_from_xtrabackup

     :version 5.5.10-20.1: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :default: 0
     :range: 0-1

If set to 1, ``.ibd`` file is converted (``space id``, ``index id``, etc.) with index information in .exp file during the import process (``ALTER TABLE ... IMPORT TABLESPACE`` command).

 This variable was added in release 5.5.10-20.1. Prior to that, it was named :variable:`innodb_expand_import`, which still exists in earlier versions.


.. Other Information


.. TODO

.. Make |XtraDB| to be enable to export .exp file by itself.

.. Suggestion 2 (expand "alter table ... discard tablespace")
.. New variable “innodb_export_at_discard = [0|1]”. When 1, |XtraDB| close the tablespace cleanly (no data in insertbuffer or to purge) and output .exp file at the same place to the .ibd file instead of deleting .ibd file only (default behavior), when “ALTER TABLE … DISCARD TABLESPACE”.

.. I think The default value should be 1 for safety, because 0 deletes the table data… LOCK TABLE also may be needed before the operation (error when doesn``t have LOCK?).

.. (example: move database named ``example``)

.. Source: (innodb_export_at_discard should be 1)

.. lock all tables in the database ``example``
.. "ALTER TABLE ... DISCARD TABLESPACE" for all tables in ``exmple``
.. unlock all tables in the database ``example``
..  (and we need to get all create table clause (e.g. "|MySQL|dump --no-data"))
.. obtain *.ibd *.exp as exported files
.. Target: (innodb_expand_import should be 1)

.. create all tables in ``example``
.. "ALTER TABLE ... DISCARD TABLESPACE" for all tables in ``exmple``
.. overwrite *.ibd and put *.exp from the Target
.. "ALTER TABLE ... IMPORT TABLESPACE" for all tables in ``exmple``
.. I think making the shell to do the above operations automatically is much easier than implement the new SQLs to do them…

.. Suggestion 1 (at shutdown [too simple... **rejected**...])
.. New variable “innodb_export_exp_at_shutdown = [0|1]”. When 1, |XtraDB| outputs .exp files for all |InnoDB| tables at clean shutdown. (works file_per_table mode inly)

.. XtraDB must treat also .exp files along with .ibd files. (e.g. delete files when delete table)


Other reading
=============

  * `Moving InnoDB tables between servers <http://www.mysqlperformanceblog.com/2009/06/08/impossible-possible-moving-innodb-tables-between-servers/>`_

  * `Copying InnoDB tables between servers <http://www.mysqlperformanceblog.com/2009/07/31/copying-innodb-tables-between-servers/>`_
