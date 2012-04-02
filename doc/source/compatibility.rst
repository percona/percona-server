.. _compatibility: 

==============================================================
Options that make XtraDB tablespaces not compatible with MySQL
==============================================================

Expanded undo slots
===================

Enabling :variable:`innodb_extra_undoslots` breaks compatibility with other programs. Specifically, it makes the datafiles unusable for ibbackup or for a MySQL server that is not run with this option.

Fast checksums
==============

Enabling :variable:`innodb_fast_checksum` will use more CPU-efficient algorithm, based on 4-byte words which can be beneficial for some workloads. Once enabled, turning it off will require table to be dump/imported again, since it will fail to start on data files created when :variable:`innodb_fast_checksums` was enabled.

Page sizes other than 16KiB
===========================

This is controlled by variable :variable:`innodb_page_size`. Changing the page size for an existing database is not supported. Table will need to be dumped/imported again if compatibility with |MySQL| is required.

Relocation of the doublewrite buffer
====================================

Variable :variable:`innodb_doublewrite_file` provides an option to put the buffer on a dedicated disk in order to parallelize I/O activity on the buffer and on the tablespace. Only in case of crash recovery this variable cannot be changed, in all other cases it can be turned on/off without breaking compatibility. 
