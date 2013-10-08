.. _mysqldump_ignore_create_error:

======================================
 Ignoring missing tables in mysqldump
======================================

In case table name was changed during the :command:`mysqldump` process taking place, :command:`mysqldump` would stop with error: :: 

   Couldn't execute 'show create table testtable'
   Table 'testdb.tabletest' doesn't exist (1146)\n")

This could happen if :command:`mysqldump` was taking a backup of a working slave and during that process table name would get changed. This error happens because :command:`mysqldump` takes the list of the tables at the beginning of the dump process but the ``SHOW CREATE TABLE`` happens just before the table is being dumped.

With this option :command:`mysqldump` will still show error to ``stderr``, but it will continue to work and dump the rest of the tables.

Version Specific Information
============================

  * :rn:`5.6.5-60.0`
    :command:`mysqldump` option :option:`--ignore-create-error` introduced


