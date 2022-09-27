.. _jemalloc-profiling:

=================================================
Jemalloc Memory Allocation Profiling
=================================================

Implemented in :ref:`8.0.25-15`, *Percona Server for MySQL* can take advantage of the memory-profiling ability of the jemalloc allocator. This ability provides a method to investigate memory-related issues.


Requirements
----------------

This memory-profiling requires :ref:`jemalloc_detected`. This read-only variable returns ``true`` if jemalloc with the profiling-enabled option is being used by *Percona Server for MySQL*. 

As root, customize jemalloc with the following flags:

.. list-table::
    :widths: 20 40
    :header-rows: 1

    * - Option
      - Description
    * - `--enable-stats`
      - Enables statistics-gathering ability
    * - `--enable-prof`
      - Enables heap profiling and the ability to detect leaks. 

Using ``LD_PRELOAD``. Build the library, configure the malloc configuration with the ``prof:true`` string, and then use ``LD_PRELOAD`` to  preload the ``libjemalloc.so`` library. The libprocess ``MemoryProfiler`` class detects the library automatically and enables the profiling support.

The following is an example of the required commands:

.. sourcecode:: bash

    ./configure --enable-stats --enable-prof && make && make install
    MALLOC_CONF=prof:true 
    LD_PRELOAD=/usr/lib/libjemalloc.so

.. note:

    Ensure the ``libjemalloc.so`` exists in the LD path.

Use *Percona Server for MySQL* with jemalloc with profiling enabled
----------------------------------------------------------

To detect if jemalloc is set, run the following command:

.. sourcecode::  mysql

    SELECT @@jemalloc_detected;

To enable jemalloc profiling in a MySQL client, run the following command:

.. sourcecode:: bash

    set global jemalloc_profiling=on;


The :ref:`malloc_stats_totals` table returns the statistics, in bytes, of the memory usage. The command takes no parameters and returns the results as a table.

The following example commands display this result:

.. sourcecode:: mysql

    use performance_schema;

    SELECT * FROM malloc_stats_totals;
    +----+------------+------------+------------+-------------+------------+
    | id | ALLOCATION | MAPPED     | RESIDENT   | RETAINED    | METADATA   |
    +----+------------+------------+------------+-------------+------------+
    |  1 | 390977528  | 405291008  | 520167424  | 436813824   | 9933744    |
    +----+------------+------------+------------+-------------+------------+
    1 row in set (0.00 sec)

The :ref:`malloc_stats` table returns the cumulative totals, in bytes, of several statistics per type of arena. The command takes no parameters and returns the results as a table.

The following example commands display this result:

.. sourcecode:: mysql

    use performance_schema;

    mysql> SELECT * FROM malloc_stats ORDER BY TYPE DESC LIMIT 3;
    +--------+-------------+-------------+-------------+-------------+
    | TYPE   | ALLOCATED   | NMALLOC     | NDALLOC     | NRESQUESTS  |
    +--------+-------------+-------------+-------------+-------------+
    | small  | 23578872    | 586156      | 0           | 2649417     |
    | large  | 367382528   | 2218        | 0           | 6355        |
    | huge   | 0           | 0           | 0           | |
    +--------+-------------+-------------+-------------+-------------+
    3 rows in set (0.00 sec)

Dumping the profile
---------------------

The profiling samples the ``malloc()`` calls and stores the sampled stack traces in a separate location in memory. These samples can be dumped into the filesystem. A dump returns a detailed view of the state of the memory. 

The process is global; therefore, only a single concurrent run is available and only the most recent runs are stored on disk. 

Use the following command to create a profile dump file:

.. sourcecode:: mysql

    flush memory profile;

The generated memory profile dumps are written to the `/tmp` directory.

You can analyze the dump files with ``jeprof`` program, which must be installed on the host system in the appropriate path. This program is a perl script that post-processes the dump files in their raw format. The program has no connection to the ``jemalloc`` library and the version numbers are not required to match.

To verify the dump, run the following command:

.. sourcecode:: bash

    ls /tmp/jeprof_mysqld*
    /tmp/jeprof_mysqld.1.0.170013202213
    jeprof --show_bytes /tmp/jeprof_mysqld.1.0.170013202213 jeprof.*.heap

You can also access the memory profile to plot a graph of the memory use. This ability requires that ``jeprof`` and ``dot`` are in the `/tmp` path. For the graph to display useful information, the binary file must contain symbol information.


Run the following command:

.. sourcecode:: bash

    jeprof --dot /usr/sbin/mysqld /tmp/jeprof_mysqld.1.0.170013202213 > /tmp/jeprof1.dot
    dot --Tpng /tmp/jeprof1.dot > /tmp/jeprof1.png 

.. note:: 

    An example of `allocation graph <https://github.com/jemalloc/jemalloc/wiki/Use-Case%3A-Leak-Checking>`__. 


PERFORMANCE_SCHEMA Tables
---------------------------------

In :ref:`8.0.25.14`, the following tables are implemented to retrieve memory allocation statistics for a running instance or return the cumulative number of allocations requested or allocations returned for a running instance.

More information about the stats that are returned can be found in `jemalloc <http://jemalloc.net/jemalloc.3.html>`__.

.. _malloc_stats_totals:

malloc_stats_totals
---------------------

The current stats for allocations. All measurements are in bytes.

.. list-table::
    :widths: 20 40
    :header-rows: 1

    * - Column Name
      - Description
    * - ALLOCATED
      - The total amount the application allocated
    * - ACTIVE 
      - The total amount allocated by the application of active pages. A multiple of the page size and this value is greater than or equal to the `stats.allocated` value. The sum does not include allocator metadata pages and `stats.arenas.<i>.pdirty` or `stats.arenas.<i>.pmuzzy`.
    * - MAPPED
      - The total amount in chunks that are mapped by the allocator in active extents. This value does not include inactive chunks. The value is at least as large as the `stats.active` and is a multiple of the chunk size.
    * - RESIDENT
      - A maximum number the allocator has mapped in physically resident data pages. All allocator metadata pages and unused dirty pages are included in this value. Pages may not be physically resident if they correspond to demand-zeroed virtual memory that has not yet been touched. This value is a maximum rather than a precise value and is a multiple of the page size. The value is greater than the `stats.active`.
    * - RETAINED
      - The amount retained by the virtual memory mappings of the operating system. This value does not include any returned mappings. This type of memory, usually de-committed, untouched, or purged. The value is associated with physical memory and is excluded from mapped memory statistics.
    * - METADATA
      - The total amount dedicated to metadata. This value contains the base allocations which are used for bootstrap-sensitive allocator metadata structures. Transparent huge pages usage is not included.

.. _malloc_stats:

malloc_stats
-----------------

The cumulative number of allocations requested or allocations returned for a running instance.

.. list-table::
    :widths: 20 40
    :header-rows: 1

    * - Column Name
      - Description
    * - Type
      - The type of object: small, large, and huge
    * - ALLOCATED
      - The number of bytes that are currently allocated to the application.
    * - NMALLOC
      - A cumulative number of times an allocation was requested from the arena's bins. The number includes times when the allocation satisfied an allocation request or filled a relevant `tcache` if `opt.tcache` is enabled.
    * - NDALLOC
      - A cumulative number of times an allocation was returned to the arena's bins. The number includes times when the allocation was deallocated or flushed the relevant `tcache` if `opt.tcache` is enabled.
    * - NREQUESTS
      - The cumulative number of allocation requests satisfied.

System Variables
------------------

The following variables have been added:

.. _jemalloc_detected:

jemalloc_detected
+++++++++++++++++++

Description: This read-only variable returns ``true`` if jemalloc with profiling enabled is detected. The following options are required:

    * Jemalloc is installed and compiled with profiling enabled

    * *Percona Server for MySQL* is configured to use jemalloc by using the environment variable ``LD_PRELOAD``. 

    * The environment variable ``MALLOC_CONF`` is set to ``prof:true``.

The following options are:

    * Scope: Global
    * Variable Type: Boolean
    * Default Value: false
 
.. _jemalloc_profiling:

jemalloc_profiling
+++++++++++++++++++

Description: Enables jemalloc profiling. The variable requires :ref:`jemalloc_detected`.

    * Command Line: --jemalloc_profiling[=(OFF|ON)]
    * Config File: Yes
    * Scope: Global
    * Dynamic: Yes
    * Variable Type: Boolean
    * Default Value: OFF

Disable Profiling
--------------------

To disable jemalloc profiling, in a MySQL client, run the following command:

.. sourcecode:: mysql

    set global jemalloc_profiling=off;