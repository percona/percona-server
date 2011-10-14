.. _innodb_fast_index_creation:

=====================
 Fast Index Creation
=====================

Percona has implemented several changes related to |MySQL| 's fast index creation feature. This feature extends the ``ALTER TABLE`` command by adding a new clause that provides online index renaming capability, that is renaming indexes without rebuilding the whole table.

Disabling Fast Index Creation
=============================

Fast index creation was implemented in |MySQL| as a way to speed up the process of adding or dropping indexes on tables with many rows. However, cases have been found in which fast index creation creates an inconsistency between |MySQL| and |InnoDB| data dictionaries.

This feature implements a session variable that disables fast index creation. This causes indexes to be created in the way they were created before fast index creation was implemented. While this is slower, it avoids the problem of data dictionary inconsistency between |MySQL| and |InnoDB|.


:command:`mysqldump`
--------------------

A new option, ``--innodb-optimize-keys``, was implemented in :command:`mysqldump`. It changes the way |InnoDB| tables are dumped, so that secondary and foreign keys are created after loading the data, thus taking advantage of fast index creation. More specifically:

  * ``KEY``, ``UNIQUE KEY``, and ``CONSTRAINT`` clauses are omitted from ``CREATE TABLE`` statements corresponding to |InnoDB| tables.

  * An additional ``ALTER TABLE`` is issued after dumping the data, in order to create the previously omitted keys.

Delaying foreign key creation does not introduce any additional risks, as :command:`mysqldump` always prepends its output with ``SET FOREIGN_KEY_CHECKS=0`` anyway.


``ALTER TABLE``
---------------

When ``ALTER TABLE`` requires a table copy, secondary keys are now dropped and recreated later, after copying the data. The following restrictions apply:

  * Only non-unique keys can be involved in this optimization.

  * If the table contains foreign keys, or a foreign key is being added as a part of the current ``ALTER TABLE`` statement, the optimization is disabled for all keys.

``OPTIMIZE TABLE``
------------------

Internally, ``OPTIMIZE TABLE`` is mapped to ``ALTER TABLE ... ENGINE=innodb`` for |InnoDB| tables. As a consequence, it now also benefits from fast index creation, with the same restrictions as for ``ALTER TABLE``.


Version Specific Information
============================

  * 5.1.49-12.0: 
    Variable :variable:`fast_index_creation` implemented.

  * 5.1.56-12.7, 5.5.11-21.2:
    Expanded the applicability of fast index creation to :command:`mysqldump`, ``ALTER TABLE``, and ``OPTIMIZE TABLE``.


System Variables
================

.. variable:: fast_index_creation

     :cli: Yes
     :conf: No
     :scope: Local
     :dyn: Yes
     :vartype: Boolean
     :default: ON
     :range: ON/OFF


Other Reading
=============

  * `Thinking about running OPTIMIZE on your InnoDB Table? Stop! <http://www.mysqlperformanceblog.com/2010/12/09/thinking-about-running-optimize-on-your-innodb-table-stop/>`_
