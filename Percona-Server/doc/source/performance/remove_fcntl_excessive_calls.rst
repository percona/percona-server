.. _remove_fcntl_excessive_calls:

=============================================
 Remove Excessive Function Calls (``fcntl``)
=============================================

This change removes a bottleneck at the client/server protocol level for high concurrency workloads.

When reading a packet from a socket, the read can be performed either in non-blocking mode or in blocking mode. The non-blocking mode was originally chosen because it avoids the cost of setting up an alarm in case of a timeout: thus the first attempt to read is done in non-blocking mode, and only if it fails, the next attempts are done in blocking mode.

However, this behavior can hurt performance as the switch from non-blocking mode to blocking mode is expensive, requiring calls to the fcntl function and calls into the kernel.

The solution is to use socket timeouts, with the ``SO_RCVTIMEO`` and ``SO_SNDTIMEO`` options. This way, the timeouts are automatically handled by the operating system, which means that all reads can be done in blocking mode.


.. Version Specific Information

..  Percona Server Version	 Comments
.. 5.1.49-12.0	 Ported from Facebook; full functionality available.
.. 5.1.55-12.6, 5.5.10-20.1	 Ported an updated version in which several incorrect lines in the original implementation changed.

.. Other Information

.. Author/Origin	Facebook
.. Bugs fixed	#606810, #724674
.. This is a port of the official MySQL version of the original Facebook change. You can see the commits for both the original implementation and the updated version in Bazaar.


Other Reading
=============

  * `fcntl Bottleneck <http://www.facebook.com/note.php?note_id=404965725932>`_

  * `Use of non-blocking mode for sockets limits performance <http://bugs.mysql.com/bug.php?id=54790>`_

