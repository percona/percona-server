.. _gap_locks_detection:

===================
Gap locks detection
===================

The `Gap locks
<https://dev.mysql.com/doc/refman/8.0/en/innodb-locking.html#innodb-gap-locks>`_
detection is based on a Facebook *MySQL* patch.

If a transactional storage engine does not support gap locks (for example
MyRocks) and a gap lock is being attempted while the transaction isolation
level is either ``REPEATABLE READ`` or ``SERIALIZABLE``, the following SQL
error will be returned to the client and no actual gap lock will be taken
on the effected rows.

.. code-block:: mysql

  ERROR HY000: Using Gap Lock without full unique key in multi-table or multi-statement transactions is not allowed. You need to either rewrite queries to use all unique key columns in WHERE equal conditions, or rewrite to single-table, single-statement transaction.

