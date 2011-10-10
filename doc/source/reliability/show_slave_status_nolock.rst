.. _show_slave_status_nolock:

=================================
 Lock-Free ``SHOW SLAVE STATUS``
=================================

The ``STOP SLAVE`` and ``SHOW SLAVE STATUS`` commands can conflict due to a global lock in the situation where one thread on a slave attempts to execute a ``STOP SLAVE`` command, while a second thread on the slave is already running a command that takes a long time to execute.

If a ``STOP SLAVE`` command is given in this situation, it will wait and not complete execution until the long-executing thread has completed its task. If another thread now executes a ``SHOW SLAVE STATUS`` command while the STOP SLAVE command is waiting to complete, the ``SHOW SLAVE STATUS`` command will not be able to execute while the ``STOP SLAVE`` command is waiting.

This features modifies the ``SLOW SLAVE STATUS`` syntax to allow: ::

  SLOW SLAVE STATUS NOLOCK

This will display the slave's status as if there were no lock, allowing the user to detect and understand the situation that is occurring.

**NOTE:** The information given when ``NOLOCK`` is used may be slightly inconsistent with the actual situation while the lock is being held.


Version Specific Information
============================

  * 5.1.52-12.3:
    Currently only the source code is distributed; the feature is not included in binaries.
