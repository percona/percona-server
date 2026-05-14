.. _expanded_innodb_fast_index_creation:

============================
Expanded Fast Index Creation
============================

.. note::

 This feature implementation is considered BETA quality.

Percona has implemented several changes related to |MySQL|'s fast index creation feature. This feature expands the ``ALTER TABLE`` command by adding a new clause that provides online index renaming capability, that is renaming indexes without rebuilding the whole table.

Enabling Expanded Fast Index Creation
=====================================

Fast index creation was implemented in |MySQL| as a way to speed up the process of adding or dropping indexes on tables with many rows. However, cases have been found in which fast index creation creates an inconsistency between |MySQL| and |InnoDB| data dictionaries.

This feature implements a session variable that enables extended fast index creation. Besides optimizing DDL directly, :variable:`expand_fast_index_creation` may also optimize index access for subsequent DML statements because using it results in much less fragmented indexes.


:command:`mysqldump`
--------------------

A new option, ``--innodb-optimize-keys``, was implemented in :command:`mysqldump`. It changes the way |InnoDB| tables are dumped, so that secondary and foreign keys are created after loading the data, thus taking advantage of fast index creation. More specifically:

  * ``KEY``, ``UNIQUE KEY``, and ``CONSTRAINT`` clauses are omitted from ``CREATE TABLE`` statements corresponding to |InnoDB| tables.

  * An additional ``ALTER TABLE`` is issued after dumping the data, in order to create the previously omitted keys.

``ALTER TABLE``
---------------

When ``ALTER TABLE`` requires a table copy, secondary keys are now dropped and recreated later, after copying the data. The following restrictions apply:

  * Only non-unique keys can be involved in this optimization.

  * If the table contains foreign keys, or a foreign key is being added as a part of the current ``ALTER TABLE`` statement, the optimization is disabled for all keys.

``OPTIMIZE TABLE``
------------------

Internally, ``OPTIMIZE TABLE`` is mapped to ``ALTER TABLE ... ENGINE=innodb`` for |InnoDB| tables. As a consequence, it now also benefits from fast index creation, with the same restrictions as for ``ALTER TABLE``.


Caveats
-------

|InnoDB| fast index creation uses temporary files in tmpdir for all indexes being created. So make sure you have enough tmpdir space when using :variable:`expand_fast_index_creation`. It is a session variable, so you can temporarily switch it off if you are short on tmpdir space and/or don’t want this optimization to be used for a specific table. 

There’s also a number of cases when this optimization is not applicable:
  * ``UNIQUE`` indexes in ``ALTER TABLE`` are ignored to enforce uniqueness where necessary when copying the data to a temporary table;

  * ``ALTER TABLE`` and ``OPTIMIZE TABLE`` always process tables containing foreign keys as if :variable:`expand_fast_index_creation` is OFF to avoid dropping keys that are part of a FOREIGN KEY constraint;

  * :command:`mysqldump --innodb-optimize-keys` ignores foreign keys because |InnoDB| requires a full table rebuild on foreign key changes. So adding them back with a separate ``ALTER TABLE`` after restoring the data from a dump would actually make the restore slower;

  * :command:`mysqldump --innodb-optimize-keys` ignores indexes on ``AUTO_INCREMENT`` columns, because they must be indexed, so it is impossible to temporarily drop the corresponding index;

  * :command:`mysqldump --innodb-optimize-keys` ignores the first UNIQUE index on non-nullable columns when the table has no ``PRIMARY KEY`` defined, because in this case |InnoDB| picks such an index as the clustered one.

Version Specific Information
============================

  * :rn:`5.6.10-60.2`
    Variable :variable:`expand_fast_index_creation` implemented.
    This variable is controlling whether fast index creation optimizations made by Percona are used.

System Variables
================

.. variable:: expand_fast_index_creation

     :cli: Yes
     :conf: No
     :scope: Local/Global
     :dyn: Yes
     :vartype: Boolean
     :default: OFF
     :range: ON/OFF

Other Reading
=============

  * `Improved InnoDB fast index creation <http://www.mysqlperformanceblog.com/2011/11/06/improved-innodb-fast-index-creation/>`_
  * `Thinking about running OPTIMIZE on your InnoDB Table? Stop! <http://www.mysqlperformanceblog.com/2010/12/09/thinking-about-running-optimize-on-your-innodb-table-stop/>`_

