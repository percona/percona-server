.. _8.0.28-20:

================================================================================
*Percona Server for MySQL* 8.0.28-20 (2022-06-20)
================================================================================

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 8.0.28-20
includes all the features and bug fixes available in the
`MySQL 8.0.28 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-28.html>`__
in addition to enterprise-grade features developed by Percona.

.. include:: ../_res/rn/ps-mysql-blurb.txt

.. contents::
   :local:

Release Highlights
=================================================

New features and improvements introduced in *Percona Server for MySQL* 8.0.28-20:

* Percona Server for MySQL implements `encryption functions and variables <https://www.percona.com/doc/percona-server/8.0/security/encryption-functions.html>`_ to manage the encryption range. The functions may take an algorithm argument. Encryption converts plaintext into ciphertext using a key and an encryption algorithm. You can also use the user-defined functions with the PEM format keys generated externally by the OpenSSL utility.

* Percona Server for MySQL adds support for the `Amazon Key Management Service (AWS KMS) component <https://www.percona.com/doc/percona-server/8.0/security/using-amz-kms.html>`__.

* ZenFS file system plugin for RocksDB is updated to version 2.1.0.

* Memory leak and error detectors (Valgrind or AddressSanitizer) provide detailed stack traces from dynamic libraries (plugins and components). The detailed stack traces make it easier to identify and fix the issues.

Other improvements and bug fixes introduced by Oracle for *MySQL* 8.0.28 and included in Percona Server for MySQL are the following:

* The ``ASCII`` shortcut for ``CHARACTER SET latin1`` and ``UNICODE`` shortcut for ``CHARACTER SET ucs2`` are deprecated and raise a warning to use ``CHARACTER SET`` instead. The shortcuts will be removed in a future version.
* A stored function and a loadable function with the same name can share the same namespace. Add the schema name when invoking a stored function in the shared namespace. The server generates a warning when function names collide.
* InnoDB supports ``ALTER TABLE ... RENAME COLUMN`` operations when using ``ALGORITHM=INSTANT``. 
* The limit for ``innodb_open_files`` now includes temporary tablespace files. The temporary tablespace files were not counted in the ``innodb_open_files`` in previous versions.

Find the full list of bug fixes and changes in the `MySQL 8.0.28 Release Notes <https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-28.html>`__.

New Features
=================================================

* :jirabug:`PS-7044`: Implements support for `encryption user-defined functions (UDFs) for OpenSSL <https://www.percona.com/doc/percona-server/8.0/security/encryption-functions.html>`__.
* :jirabug:`PS-7672`: Implements support for the `Amazon Key Management Service component in Percona Server for MySQL <https://www.percona.com/doc/percona-server/8.0/security/using-amz-kms.html>`__.
* :jirabug:`PS-7748`: Implements the ability to log error messages to a memory buffer.

Improvements
=================================================

* :jirabug:`PS-8103`: Memory leak and error detectors (Valgrind or AddressSanitizer) provide detailed stack traces from dynamic libraries (plugins and components). The detailed stack traces make it easier to identify and fix the issues.
* ZenFS file system plugin for RocksDB is updated to version 2.1.0.

Bugs fixed
=================================================

* :jirabug:`PS-6029`: Data masking ``gen_rnd_us_phone()`` function had a different format compared to MySQL upstream version.
* :jirabug:`PS-8136`: ``LOCK TABLES FOR BACKUP`` did not prevent InnoDB key rotation. Due to this behavior, Percona Xtrabackup couldn't fetch the key in case the key was rotated after starting the backup.
* :jirabug:`PS-8143`: Fixed the memory leak in ``File_query_log::set_rotated_name()``. 
* :jirabug:`PS-7894`: When a query to the MyRocks table was interrupted due to the ``MAX_EXECUTION time`` option, an incorrect error message was received. (Thanks to user hagrid-the-developer for reporting this issue.)
* :jirabug:`PS-8158`: There was access to possibly not initialized memory. (Thanks to Rinat Ibragimov for reporting this issue.)
* :jirabug:`PS-5008`: Fixed the memory leak in ``sync_latch_meta_init()`` after mysqld shutdown.
* **zenfs** utility failed when a user tried to restore a single file into a specified ZenFS path.
* RocksDB in ZenFS mode ignored OPTIONS-<NNN> files after the restart.
* RocksDB in ZenFS mode always created PersistentCache on the POSIX file system instead of creating it on ZenFS.

.. include:: ../_res/rn/useful-links.txt
