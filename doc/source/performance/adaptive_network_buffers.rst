.. _adaptive_network_buffers:

===========================
Adaptive Network Buffers 
===========================

To find the buffer size of the current connection, use the ``network_buffer_length`` status variable. Add ``SHOW GLOBAL`` to review the cumulative buffer sizes for all connections. This variable can help to estimate the maximum size of the network buffer's overhead.

Network buffers grow towards the `max_allowed_packet <https://dev.mysql.com/doc/refman/8.0/en/server-system-variables.html#sysvar_max_allowed_packet>`_ size and do not shrink until the connection is terminated. For example, if the connections are selected at random from the pool, an occasional big query eventually increases the buffers of all connections. The combination of `max_allowed packet` set to a value between 64MB to 128MB and the connection number between 256 to 1024 can create a large memory overhead.

*Percona Server for MySQL* version 8.0.23-14 introduces the :ref:`net_buffer_shrink_interval` variable to solve this issue. The default value is 0 (zero). If you set the value higher than 0, Percona Server records the network buffer's maximum use size for the number of seconds set by `net_buffer_shrink_interval`. When the next interval starts, the network buffer is set to the recorded size. This action removes spikes in the buffer size.

You can achieve similar results by disconnecting and reconnecting the TCP connections, but this solution is a heavier process. This process disconnects and reconnects connections with small buffers. 

.. _net_buffer_shrink_interval:

.. rubric:: ``net_buffer_shrink_interval``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - --net-buffer-shrink-interval=#
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - integer 
   * - Default
     - 0

The interval is measured in seconds. The default value is 0, which disables the functionality. The minimum value is 0, and the maximum value is 31536000. 
