.. _innodb_buffer_pool_shm:

===========================
 Shared Memory Buffer Pool
===========================

The :ref:`SHM buffer pool <innodb_buffer_pool_shm>` patch, which provided the ability to use a shared memory segment for the buffer pool to enable faster server restarts, has been removed. Instead, we recommend using the :ref:`LRU Dump/Restore <innodb_lru_dump_restore>` patch which provides similar improvements in restart performance.

Replacement is due to ``SHM`` buffer pool both being very invasive and not widely used. Improved restart times are better provided by the much safer ``LRU D/R`` patch which has the advantage of also persisting across machine restarts.

The configuration variables for :file:`my.cnf` have been kept for compatibility and warnings will be printed for the deprecated options (:variable:`innodb_buffer_pool_shm_key` and :variable:`innodb_buffer_pool_shm_checksum`) if used.

Instructions for disabling the ``SHM`` buffer pool can be found :ref:`here <innodb_buffer_pool_shm>`.

Instructions on setting up ``LRU`` dump/restore can be found :ref:`here <innodb_lru_dump_restore>`.

Version Specific Information
============================

  * 5.1.49-12.0:
    Feature introduced.

  * 5.1.50-12.1:
    System variable :variable:`innodb_buffer_pool_shm_checksum` added.

  * 5.5.8-20.0:
    First Percona Server 5.5 release, also included Shared Memory
    Buffer Pool.

  * 5.5.13-20.4:
    Feature removed, as LRU Dump/Restore is less invasive, more
    reliable and a better solution.
