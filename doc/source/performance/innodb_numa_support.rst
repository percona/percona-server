.. _innodb_numa_support:

===========================
 Improved ``NUMA`` support
===========================

In cases where the buffer pool memory allocation was bigger than size of the node, system would start swapping already allocated memory even if there is available memory on other node. This is would happen if the default :term:`NUMA` memory allocation policy was selected. In that case system would favor one node more than other which caused the node to run out of memory. Changing the allocation policy to interleaving, memory will be allocated in round-robin fashion over the available node. This can be done by using the mysqld_safe :variable:`numa_interleave` option. **NOTE:** In order for this feature to work correctly ``mysqld_safe`` needs to be started as ``root``.

Another improvement implemented is preallocating the pages in the buffer pool on startup with :variable:`innodb_buffer_pool_populate` variable. This forces ``NUMA`` allocation decisions to be made immediately while the buffer cache is clean.

It is generally recommended to enable all of the options together to maximize the performance effects on the ``NUMA`` architecture.

Version Specific Information
============================

 * :rn:`5.6.11-60.3`
    Improved ``NUMA`` support implemented. This feature was ported from Twitter's |MySQL| patches.

System Variables
================

.. variable:: innodb_buffer_pool_populate

     :cli: Yes
     :conf: Yes
     :location: mysqld
     :scope: Global
     :dyn: No
     :vartype: Boolean
     :default: OFF
     :range: ON/OFF

When this variable is enabled, |InnoDB| preallocates pages in the buffer pool on startup to force ``NUMA`` allocation decisions to be made immediately while the buffer cache is clean.

Command-line Options for mysqld_safe
====================================

.. variable:: flush_caches

     :cli: Yes
     :conf: Yes
     :location: mysqld_safe
     :dyn: No
     :vartype: Boolean
     :default: OFF
     :range: ON/OFF

When enabled this will flush and purge buffers/caches before starting the server to help ensure ``NUMA`` allocation fairness across nodes. This option is useful for establishing a consistent and predictable behavior for normal usage and/or benchmarking.

.. variable:: numa_interleave

     :cli: Yes
     :conf: Yes
     :location: mysqld_safe
     :dyn: No
     :vartype: Boolean
     :default: OFF
     :range: ON/OFF

When this option is enabled, mysqld will run with its memory interleaved on all ``NUMA`` nodes by starting it with ``numactl --interleave=all``. In case there is just 1 CPU/node, allocations will be "interleaved" between that node.

Other Reading
=============

 * `The MySQL "swap insanity" problem and the effects of the NUMA architecture <http://blog.jcole.us/2010/09/28/mysql-swap-insanity-and-the-numa-architecture/>`_
 * `A brief update on NUMA and MySQL <http://blog.jcole.us/2012/04/16/a-brief-update-on-numa-and-mysql/>`_
