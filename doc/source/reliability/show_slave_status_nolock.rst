.. _show_slave_status_nolock:

=================================
 Lock-Free ``SHOW SLAVE STATUS``
=================================

The ``STOP SLAVE`` and ``SHOW SLAVE STATUS`` commands can conflict due to a global lock in the situation where one thread on a slave attempts to execute a ``STOP SLAVE`` command, while a second thread on the slave is already running a command that takes a long time to execute.

If a ``STOP SLAVE`` command is given in this situation, it will wait and not complete execution until the long-executing thread has completed its task. If another thread now executes a ``SHOW SLAVE STATUS`` command while the STOP SLAVE command is waiting to complete, the ``SHOW SLAVE STATUS`` command will not be able to execute while the ``STOP SLAVE`` command is waiting.

This features modifies the ``SHOW SLAVE STATUS`` syntax to allow: ::

  SHOW SLAVE STATUS NOLOCK

This will display the slave's status as if there were no lock, allowing the user to detect and understand the situation that is occurring.

**NOTE:** The information given when ``NOLOCK`` is used may be slightly inconsistent with the actual situation while the lock is being held.

Status Variables
================

.. variable:: Com_show_slave_status_nolock

   :vartype: Numeric
   :varscope: Global/Session

The :variable:`Com_show_slave_status_nolock` statement counter variable indicates the number of times the statement ``SHOW SLAVE STATUS NOLOCK`` has been executed.

Version Specific Information
============================

  * :rn:`5.5.8-20.0`: Introduced
