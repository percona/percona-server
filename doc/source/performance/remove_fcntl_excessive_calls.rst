.. _remove_fcntl_excessive_calls:

=============================================
 Remove Excessive Function Calls (``fcntl``)
=============================================

.. warning::

  This feature has been deprecated in |Percona Server| :rn:`5.1.66-14.1`.

This change removes a bottleneck at the client/server protocol level for high concurrency workloads.

When reading a packet from a socket, the read can be performed either in non-blocking mode or in blocking mode. The non-blocking mode was originally chosen because it avoids the cost of setting up an alarm in case of a timeout: thus the first attempt to read is done in non-blocking mode, and only if it fails, the next attempts are done in blocking mode.

However, this behavior can hurt performance as the switch from non-blocking mode to blocking mode is expensive, requiring calls to the fcntl function and calls into the kernel.

The solution is to use socket timeouts, with the ``SO_RCVTIMEO`` and ``SO_SNDTIMEO`` options. This way, the timeouts are automatically handled by the operating system, which means that all reads can be done in blocking mode.


Version Specific Information
============================

  * :rn:`5.1.49-12.0`: 
    Feature implemented 
  * :rn:`5.1.55-12.6`: 
    Feature updated
  * :rn:`5.1.66-14.1`:
    Feature deprecated
  * :rn:`5.1.66-14.2`:
    Feature removed


Other Reading
=============

  * `fcntl Bottleneck <http://www.facebook.com/note.php?note_id=404965725932>`_

  * `Use of non-blocking mode for sockets limits performance <http://bugs.mysql.com/bug.php?id=54790>`_

