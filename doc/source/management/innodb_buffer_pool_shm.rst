===========================
 Shared Memory Buffer Pool
===========================

This features provides the option of storing the buffer pool in a shared memory segment between restarts of the server. When the buffer pool is extremely large, writing and reading it to and from disk between restarts can take a great deal of time. By storing the buffer pool for reuse in a shared memory segment, which is persistent between restarts of the server, these time-consuming I/O operations are avoided.

In order to reuse the buffer pool stored in shared memory, |InnoDB| must shut down cleanly before the server is restarted. If |InnoDB| doesn``t shut down cleanly, the shared memory segment should be removed.

If :variable:`innodb_buffer_pool_shm_key` is non-zero when |InnoDB| restarts, |InnoDB| tries to reconnect and reuse the existing buffer pool in shared memory. |InnoDB| will not start with an error. If an error occurs on startup, the shared memory segment will also need to be removed.

Restrictions required in order to use the buffer pool in shared memory are as follows:

  * The |InnoDB| executable cannot be changed between restarts.

  * The value of :variable:`innodb_page_size` can't be changed between restarts.

  * The value of :variable:`innodb_buffer_pool_size` can't be changed between restarts. If it is changed, an error will be reported, e.g.: ::

     InnoDB: Error: srv_buf_pool_size is different (shm=85899345920 current=75161927680).
     InnoDB: Fatal error: cannot allocate the memory for the buffer pool

For most errors resulting from the use of the feature, the solution is to manually remove/reinitialize the shared memory segment. There is also no provision for removing the shared memory segment automatically when it is no longer needed to store the buffer pool. This must also be done by manually removing the shared memory segment.

Removing the Shared Memory Segment
==================================

Removal / reinitialization of the shared memory segment can be done as follows.

First, use the :command:`ipcs` command to see the currently allocated memory segments. Examine the Shared Memory Segments part of the output to determine the key that identifies the shared memory segment. In this case, let``s assume a single shared memory segment exists, the one being used to contain the buffer pool. The output would appear as follows: ::

  > ipcs
  
  ------ Shared Memory Segments --------
  key        shmid      owner      perms      bytes      nattch     status      
  0x00001561 3735554    root      600        88165400576 1                       

The value in the field “key” is the hexadecimal value of :variable:`innodb_buffer_pool_shm_key`. The segment can now be removed by using the value of this field in the :variable:`ipcrm` command: ::

  > ipcrm -M 0x00001561

|InnoDB| Messages
=================

Segment Size Too Large
----------------------

One |InnoDB| message you may see is: ::

  Warning: Failed to allocate 88165400576 bytes. (new) errno 22

This may be because the size of the segment you are trying to allocate is exceeding ``SHMAX``. You can see the current value of ``SHMAX`` by doing: ::

  > cat /proc/sys/kernel/shmmax

If that is the problem, increase the value of ``SHMAX`` so that is large enough, e.g., by doing: ::

  echo 137438953472 > /proc/sys/kernel/shmmax

Shared Memory Alignment
-----------------------

When |InnoDB| uses shared memory segments, it may need to align them properly after a shutdown and restart. As a result, on restart, you may see output similar to the following when the buffer pool is being stored in shared memory: ::

  InnoDB: Buffer pool in the shared memory segment should be converted.
  InnoDB: Previous frames in address      : 0x2aab2ddf1000
  InnoDB: Previous frames were located    : 0x2aaf724bc000
  InnoDB: Current frames should be located: 0x2aab2ddf0000
  InnoDB: Pysical offset                  : -4096 (0xfffffffffffff000)
  InnoDB: Logical offset (frames)         : -18327846912 (0xfffffffbbb934000)
  InnoDB: Logical offset (blocks)         : -18327842816 (0xfffffffbbb935000)
  InnoDB: Aligning physical offset... Done.
  InnoDB: Aligning logical offset... Done.

This indicates |InnoDB| has successfully done the shared memory realignment.


Version Specific Information
============================

  * 5.1.49-12.0:
    Feature introduced.

  * 5.1.50-12.1:
    System variable :variable:`innodb_buffer_pool_shm_checksum` added.

Variables Provided
==================

The folllowing system variables are provided by this feature.

.. variable:: innodb_buffer_pool_shm_key

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No	
     :vartype: UINT
     :default: 0
     :range: 0-INT_MAX32

The default value of ``0`` indicates shared memory should not be used to store the buffer pool; the buffer pool is written to disk between restarts as usual.

If the value is non-zero, it specifies the key of the shared memory segment in which to store the buffer pool.

The range of :variable:`innodb_buffer_pool_shm_key` is system dependent, from zero to usually the maximum value of an UNSIGNED INTEGER. It is an input parameter to the shmget system function. For details, see your system IPC manual.

.. variable:: innodb_buffer_pool_shm_checksum

     :cli: No
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Boolean
     :default: On
     :range: On/Off

Checksum validation of the shared memory buffer pool is performed at startup and shutdown when :variable:`innodb_buffer_pool_shm_checksum` is enabled. It is enabled by default. Startup and shutdown are slower when checksum validation is enabled, but enabling it adds additional protection against the shared memory region becoming corrupted.
