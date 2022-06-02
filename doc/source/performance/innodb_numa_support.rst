.. _innodb_numa_support:

===========================
 Improved ``NUMA`` support
===========================

In cases where the buffer pool memory allocation was bigger than size of the node, system would start swapping already allocated memory even if there is available memory on other node. This is would happen if the default source/glossary.rst`NUMA` memory allocation policy was selected. In that case system would favor one node more than other which caused the node to run out of memory. Changing the allocation policy to interleaving, memory will be allocated in round-robin fashion over the available node. This can be done by using the upstream `innodb_numa_interleave <http://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_numa_interleave>`_. This feature extends the upstream implementation by implementing the :variable:`flush_caches` variable.

It is generally recommended to enable all of the options together to maximize the performance effects on the ``NUMA`` architecture.

Version Specific Information
============================

 * :rn:`5.7.10-1`: 
    Feature ported from |Percona Server| 5.6

 * :rn:`5.7.22-22`:
    Feature reverted from the upstream implementation back to the one ported from |Percona Server| 5.6, in which `innodb_numa_interleave <http://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_numa_interleave>`_ variable not only enables NUMA memory interleaving  at InnoDB buffer pool allocation, but allocates buffer pool with MAP_POPULATE, forcing interleaved allocation at the buffer pool initialization time.

Command-line Options for mysqld_safe
====================================

.. variable:: flush_caches

     :cli: Yes
     :conf: Yes
     :location: mysqld_safe
     :dyn: No
     :vartype: Boolean
     :default: ``0`` (OFF)
     :range: ``0``/``1``

When enabled (set to ``1``) this will flush and purge buffers/caches before starting the server to help ensure ``NUMA`` allocation fairness across nodes. This option is useful for establishing a consistent and predictable behavior for normal usage and/or benchmarking.

Other Reading
=============

 * `The MySQL "swap insanity" problem and the effects of the NUMA architecture <http://blog.jcole.us/2010/09/28/mysql-swap-insanity-and-the-numa-architecture/>`_
 * `A brief update on NUMA and MySQL <http://blog.jcole.us/2012/04/16/a-brief-update-on-numa-and-mysql/>`_
