.. _innodb_expand_undo_slots:

======================================
More Concurrent Transactions Available
======================================

|InnoDB| provides a fixed number of 1024 undo slots in its rollback segment, leaving room for 1024 transactions to run in parallel. If all the slots are used any new transaction will fail until a slot is freed, which can cause st:range: behaviors. This change provides a variable to expand the number of undo slots, up to 4072.

This option is provided for servers that run out of undo slots. Use it if you find the following warning in the error log: ``Warning: cannot find a free slot for an undo log``.

We discourage its use unless you get this warning, because it breaks compatibility with other programs. Specifically, it makes the datafiles unusable for ibbackup or for a |MySQL| server that is not run with this option.

When you enable the option, the maximum number of undo slots is extended to 4072, instead of the default fixed value of 1024.

You can then check whether the expanded slots (1025-4072) are used by starting :command:`mysqld` with ``innodb_extra_undoslots=OFF``:

  * If the expanded slots are used: :command:`mysqld` refuses to start and prints an error in the error log: ::

      InnoDB: Error: innodb_extra_undoslots option is disabled, but it was enabled before.
      InnoDB: The datafile is not normal for |MySQL|d and disabled innodb_extra_undoslots.
      InnoDB: Enable innodb_extra_undoslots if it was enabled before, and
      InnoDB: ### don't use this datafile with other |MySQL|d or ibbackup! ###
      InnoDB: Cannot continue operation for the safety. Calling exit(1).

  * If the expanded slots are not used: :command:`mysqld` starts and prints only a warning in the error log: ::

      InnoDB: Warning: innodb_extra_undoslots option is disabled, but it was enabled before.
      InnoDB: But extended undo slots seem not used, so continue operation.


System Variables
----------------

.. psdom:variable:: innodb_extra_undoslots

   :cli: Yes
   :Conf: Yes
   :scope: Global
   :dyn: No
   :vartype: BOOL
   :def: OFF
   :range: ON/OFF

If ``ON``, expands the number of undo slots to 4072.
