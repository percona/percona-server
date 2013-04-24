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

  * :rn:`5.1.49-rel12.0`
    Feature introduced.

  * :rn:`5.1.50-rel12.1`
    System variable :variable:`innodb_buffer_pool_shm_checksum` added.

  * :rn:`5.1.58-12.9`
    Feature removed, as LRU Dump/Restore is less invasive, more
    reliable and a better solution.

System Variables
================

.. variable:: innodb_buffer_pool_shm_key

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Boolean
     :default: OFF
     :range: ON/OFF

.. variable:: innodb_buffer_pool_shm_checksum

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Boolean
     :default: ON
     :range: ON/OFF
