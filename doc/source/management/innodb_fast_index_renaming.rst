.. _innodb_fast_index_renaming:

=====================
 Fast Index Renaming
=====================

This feature provides a way to have a fast (online) way to rename an index without rebuilding the whole table. It does this by adding a ``RENAME INDEX`` clause to the ``ALTER TABLE`` statement: ::

  ALTER TABLE ... RENAME INDEX xxx TO yyy;

Several limitations exist:

  * The ``RENAME INDEX`` clause cannot be used with any other type of clause. If it is, an error message is displayed and an error code returned.

  * The ``RENAME INDEX`` clause can only be used when MyISAM or XtraDB is the storage engine. Otherwise, an error message will be displayed: ``wrong for the engine 'engine name'``.

  * Use of a partitioned table is not allowed. If ``ALTER TABLE`` is applied to a partitioned table, an error will occur.

  * If use of the ``RENAME INDEX`` clause results in an XtraDB error, an inconsistency between the index name in MySQL and Percona Server with XtraDB may occur.

Errors like ``duplicate key name`` or ``key name not found`` are not caused by ``RENAME INDEX``. These errors occur before processing of the ``RENAME INDEX`` clause occurs. So they don't cause an inconsistency problem.

An internally reserved InnoDB name (e.g., ``GEN_CLUST_INDEX``) cannot be used as an index name. If one is, an error is reported.

If any error is returned by the ``ALTER TABLE`` command due to the ``RENAME INDEX`` clause, the best way to resolve the problem is to re-create the table.


Version Specific Information
============================

  * 5.1.49-12.0:
    Not yet released externally

Error Codes
===========

  * ``error_er_wrong_usage``:
    Incorrect usage of %s and %s
