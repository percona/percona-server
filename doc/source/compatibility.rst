.. _compatibility: 

==============================================================
Options that make XtraDB tablespaces not compatible with MySQL
==============================================================

Page sizes other than 16KiB
===========================

This is controlled by variable :variable:`innodb_page_size`. Changing the page size for an existing database is not supported. Table will need to be dumped/imported again if compatibility with |MySQL| is required.

Relocation of the doublewrite buffer
====================================

Variable :variable:`innodb_doublewrite_file` provides an option to put the buffer on a dedicated disk in order to parallelize I/O activity on the buffer and on the tablespace. Only in case of crash recovery this variable cannot be changed, in all other cases it can be turned on/off without breaking the compatibility. 
