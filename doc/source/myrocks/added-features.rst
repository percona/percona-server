.. _added-features:

================================================================================
Updated Supported Features
================================================================================

The following is a list of the latest supported features:

* **Percona Server for MySQL** 8.0.27-18 adds support for ``SELECT FOR UPDATE SKIP LOCKED/NOWAIT``. The transaction isolation level must be ``READ COMMITTED``.
  
* **Percona Server for MySQL** 8.0.27-18 adds the ability to cancel ongoing manual compactions. The cancel methods are the following:

  * Using either Control+C (from a session) or KILL (from another session) for client sessions running manual compactions by ``SET GLOBAL rocksdb_compact_cf (variable)``.
  * Using a global variable ``rocksdb_cancel_manual_compactions`` to cancel all ongoing manual compactions.
    
* **Percona Server for MySQL** 8.0.23-14 adds supported for `Generated Columns <https://dev.mysql.com/doc/refman/8.0/en/create-table-generated-columns.html>`_ and index are supported. Generated columns are not supported in versions earlier than 8.0.23-14.

* **Percona Server for MySQL** 8.0.23-14 adds support for `explicit DEFAULT value expressions <https://dev.mysql.com/doc/refman/8.0/en/data-type-defaults.html>`__. From version 8.0.13-3 to version 8.0.22-13, MyRocks did not support these expressions.



