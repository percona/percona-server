.. _compatibility: 

==============================================================
Options that make XtraDB tablespaces not compatible with MySQL
==============================================================

Fast checksums
==============

Enabling :variable:`innodb_fast_checksum` will use more CPU-efficient algorithm, based on 4-byte words which can be beneficial for some workloads. Once enabled, turning it off will require table to be dump/imported again, since |Percona Server| will fail to start on data files created when :variable:`innodb_fast_checksums` was enabled.

In case you've migrated from |Percona Server| to |MySQL| you could get the "corrupted checksum" error message. In order to recover that table you'll need to:

  1) Reinstall Percona Server to read your tables that were created with fast checksums. 
  2) Dump the tables (or temporarily convert them to MyISAM). 
  3) Install stock MySQL (or at least disable fast checksums). 
  4) Restore the InnoDB tables (or convert back from MyISAM). 

.. note::

   This feature has been deprecated after |Percona Server| :rn:`5.5.28-29.2` and it is not available in |Percona Server| 5.6, because the `innodb_checksum_algorithm <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_checksum_algorithm>`_ feature in |MySQL| 5.6 makes it redundant.

Page sizes other than 16KiB
===========================

This is controlled by variable :variable:`innodb_page_size`. Changing the page size for an existing database is not supported. Table will need to be dumped/imported again if compatibility with |MySQL| is required.

.. note:: This feature has been deprecated in the |Percona Server| :rn:`5.5.30-30.2`. It has been replaced by the `upstream <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_page_size>`_ version released in |MySQL| 5.6.4.

Relocation of the doublewrite buffer
====================================

Variable :variable:`innodb_doublewrite_file` provides an option to put the buffer on a dedicated disk in order to parallelize I/O activity on the buffer and on the tablespace. Only in case of crash recovery this variable cannot be changed, in all other cases it can be turned on/off without breaking the compatibility. 

.. note:: This feature has not been ported to |Percona Server| 5.6.
