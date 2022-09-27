.. _tokudb_quickstart:

================================================================================
Getting Started with TokuDB
================================================================================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

:Operating Systems: *TokuDB* is currently supported on 64-bit Linux only.
:Memory: *TokuDB* requires at least 1GB of main memory.

	 For the best results, run with at least 2GB of main memory.
:Disk space and configuration:
   Make sure to allocate enough disk space for data, indexes and logs.

   | Due to high compression, *TokuDB* may achieve up to 25x space savings on data
   | and indexes over *InnoDB*.

Creating Tables and Loading Data 
================================================================================

*TokuDB* tables are created the same way as other tables in *MySQL* by
specifying :mysql:`ENGINE=TokuDB` in the table definition. For example, the
following command creates a table with a single column and uses the *TokuDB*
storage engine to store its data:

.. code-block:: mysql

   CREATE TABLE table (
   id INT(11) NOT NULL) ENGINE=TokuDB;

.. rubric:: Loading data
	    
Once *TokuDB* tables have been created, data can be inserted or loaded using
standard *MySQL* insert or bulk load operations. For example, the following
command loads data from a file into the table:

.. code-block:: sql

   LOAD DATA INFILE file
   INTO TABLE table;

.. note:: 

   For more information about loading data, see the MySQL 8.0 reference
   manual.

Migrating Data from an Existing Database
================================================================================

Use the following command to convert an existing table for the *TokuDB* storage
engine:

.. code-block:: mysql

   ALTER TABLE table
   ENGINE=TokuDB;

.. rubric:: Bulk Loading Data

The *TokuDB* bulk loader imports data much faster than regular *MySQL* with
*InnoDB*. To make use of the loader you need flat files in either comma
separated or tab separated format. The *MySQL* :mysql:`LOAD DATA INFILE`
statement will invoke the bulk loader if the table is empty. Keep in mind that
while this is the most convenient and, in most cases, the fastest way to
initialize a *TokuDB* table, it may not be replication safe if applied to the
source.

.. seealso::

   MySQL Documentation: :mysql:`LOAD DATA INFILE`
      http://dev.mysql.com/doc/refman/8.0/en/load-data.html

To obtain the logical backup and then bulk load into *TokuDB*, follow these
steps:

1. *Create a logical backup of the original table.* The easiest way to achieve
   this is using :mysql:`SELECT ... INTO OUTFILE`. Keep in mind that the file will be
   created on the server:  :mysql:`SELECT * FROM table INTO OUTFILE 'file.csv';`

#. *Copy the output file* either to the destination server or the client machine
   from which you plan to load it.

#. *Load the data into the server* using :mysql:`LOAD DATA INFILE`. If loading from
   a machine other than the server use the keyword ``LOCAL`` to point to the
   file on local machine. Keep in mind that you will need enough disk space on
   the temporary directory on the server since the local file will be copied
   onto the server by the *MySQL* client utility:
   :mysql:`LOAD DATA [LOCAL] INFILE 'file.csv';`

It is possible to create the CSV file using either :program:`mysqldump` or the
*MySQL* client utility as well, in which case the resulting file will reside on
a local directory. In these 2 cases you have to make sure to use the correct
command line options to create a file compatible with :mysql:`LOAD DATA INFILE`.

The bulk loader will use more space than normal for logs and temporary files
while running, make sure that your file system has enough disk space to process
your load. As a rule of thumb, it should be approximately 1.5 times the size of
the raw data.

.. note::

   Please read the original MySQL Documentation to understand the needed privileges and
   replication issues around :mysql:`LOAD DATA INFILE`.

Considerations to Run TokuDB in Production
================================================================================

In most cases, the default options should be left in-place to run *TokuDB*,
however it is a good idea to review some of the configuration parameters.

.. rubric:: Memory allocation

*TokuDB* will allocate 50% of the installed RAM for its own cache (global
variable :ref:`tokudb_cache_size`). While this is optimal in most
situations, there are cases where it may lead to memory over allocation. If the
system tries to allocate more memory than is available, the machine will begin
swapping and run much slower than normal.

It is necessary to set the :ref:`tokudb_cache_size` to a value other than
the default in the following cases:

Running other memory heavy processes on the same server as TokuDB
   In many cases, the database process needs to share the system with other
   server processes like additional database instances, http server, application
   server, e-mail server, monitoring systems and others. In order to properly
   configure TokuDB's memory consumption, it's important to understand how much
   free memory will be left and assign a sensible value for *TokuDB*. There is
   no fixed rule, but a conservative choice would be 50% of available RAM while
   all the other processes are running. If the result is under 2 GB, you should
   consider moving some of the other processes to a different system or using a
   dedicated database server.

   :ref:`tokudb_cache_size` is a static variable, so it needs to be set
   before starting the server and cannot be changed while the server is
   running. For example, to set up TokuDB's cache to 4G, add the following line
   to your :file:`my.cnf` file:

   .. code-block:: bash

      tokudb_cache_size = 4G

System using *InnoDB* and *TokuDB*
   When using both the *TokuDB* and *InnoDB* storage engines, you need to manage
   the cache size for each. For example, on a server with 16 GB of RAM you could
   use the following values in your configuration file:
 
   .. code-block:: bash

      innodb_buffer_pool_size = 2G
      tokudb_cache_size = 8G

Using *TokuDB* with Federated or FederatedX tables
   The Federated engine in *MySQL* and FederatedX in *MariaDB* allow you to
   connect to a table on a remote server and query it as if it were a local
   table (please see the MySQL Documentation: 14.11. The FEDERATED Storage
   Engine for details). When accessing the remote table, these engines could
   import the complete table contents to the local server to execute a query. In
   this case, you will have to make sure that there is enough free memory on the
   server to handle these remote tables. For example, if your remote table is 8
   GB in size, the server has to have more than 8 GB of free RAM to process
   queries against that table without going into swapping or causing a kernel
   panic and crash the *MySQL* process. There are no parameters to limit the
   amount of memory that the Federated or FederatedX engine will allocate while
   importing the remote dataset.

Specifying the Location for Files
================================================================================

As with *InnoDB*, it is possible to specify different locations than the default
for TokuDB's data, log and temporary files. This way you may distribute the load
and control the disk space. The following variables control file location:

* :ref:`tokudb_data_dir`: This variable defines the directory where the TokuDB tables are stored. The default location for TokuDB's data files is the MySQL data directory.

* :ref:`tokudb_log_dir`: This variable defines the directory where the TokuDB log files are stored. The default location for TokuDB's log files is the MySQL data directory. Configuring a separate log directory is somewhat involved and should be done only if absolutely necessary. We recommend to keep the data and log files under the same directory.

* :ref:`tokudb_tmp_dir`: This variable defines the directory where the TokuDB bulk loader stores temporary files. The bulk loader can create large temporary files while it is loading a table, so putting these temporary files on a disk separate from the data directory can be useful. For example, it can make sense to use a high-performance disk for the data directory and a very inexpensive disk for the temporary directory. The default location for TokuDB's temporary files is the MySQL data directory.

Table Maintenance
================================================================================

The fractal tree provides fast performance by inserting small messages in the
buffers in the fractal trees instead of requiring a potential IO for an update
on every row in the table as required by a B-tree. Additional background
information on how fractal trees operate can be found here. For tables whose
workload pattern is a high number of sequential deletes, it may be beneficial to
flush these delete messages down to the basement nodes in order to allow for
faster access. The way to perform this operation is via the ``OPTIMIZE``
command.

The following extensions to the ``OPTIMIZE`` command have been added in *TokuDB*
version 7.5.5:

.. contents::
   :local:

Hot Optimize Throttling
--------------------------------------------------------------------------------

By default, table optimization will run with all available resources. To limit
the amount of resources, it is possible to limit the speed of table
optimization.  The :ref:`tokudb_optimize_throttle` session variable
determines an upper bound on how many fractal tree leaf nodes per second are
optimized.  The default is 0 (no upper bound) with a valid range of
[0,1000000]. For example, to limit the table optimization to 1 leaf node per
second, use the following setting: :mysql:`SET tokudb_optimize_throttle=1;`

Optimize a Single Index of a Table
--------------------------------------------------------------------------------

To optimize a single index in a table, the
:ref:`tokudb_optimize_index_name` session variable can be set to select the
index by name. For example, to optimize the primary key of a table:

.. code-block:: mysql

   SET tokudb_optimize_index_name='primary'; 
   OPTIMIZE TABLE t;

Optimize a Subset of a Fractal Tree Index
--------------------------------------------------------------------------------

For patterns where the left side of the tree has many deletions (a common
pattern with increasing id or date values), it may be useful to delete a
percentage of the tree. In this case, it is possible to optimize a subset of a
fractal tree starting at the left side. The
:ref:`tokudb_optimize_index_fraction` session variable controls the size of
the sub tree. Valid values are in the range [0.0,1.0] with default 1.0 (optimize
the whole tree). For example, to optimize the leftmost 10% of the primary key:

.. code-block:: mysql

   SET tokudb_optimize_index_name='primary'; 
   SET tokudb_optimize_index_fraction=0.1;
   OPTIMIZE TABLE t;

.. include:: ../.res/replace.txt
.. include:: ../.res/replace.program.txt
.. include:: ../.res/replace.concept.txt
