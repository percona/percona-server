.. _extended_set_var:
.. _set-statement-for.upstream.replacing:

================================================================================
Extended SET VAR Optimizer Hint
================================================================================

*Percona Server for MySQL* 8.0 extends the ``SET_VAR`` introduced in *MySQL* 8.0
effectively replacing the ``SET STATEMENT ... FOR`` statement. ``SET_VAR`` is an
optimizer hint that can be applied to session variables.

*Percona Server for MySQL* 8.0 extends the ``SET_VAR`` hint to support the
following:

- The ``OPTIMIZE TABLE`` statement
- MyISAM session variables
- Plugin or Storage Engine variables
- InnoDB Session variables
- The ``ALTER TABLE`` statement
- ``CALL stored_proc()`` statement
- The ``ANALYZE TABLE`` statement
- The ``CHECK TABLE`` statement
- The ``LOAD INDEX`` statement (used for MyISAM)
- The ``CREATE TABLE`` statement

*Percona Server for MySQL* 8.0 also supports setting the following variables by
using ``SET_VAR``:

.. hlist::
   :columns: 2


   - innodb_lock_wait_timeout
   - innodb_tmpdir
   - innodb_ft_user_stopword_table
   - block_encryption_mode
   - histogram_generation_max_mem_size
   - myisam_sort_buffer_size
   - myisam_repair_threads
   - myisam_stats_method
   - preload_buffer_size (used by MyISAM only)
  
.. seealso::

   *MySQL* Documentation: Variable-setting hint syntax
      https://dev.mysql.com/doc/refman/8.0/en/optimizer-hints.html#optimizer-hints-set-var
   
