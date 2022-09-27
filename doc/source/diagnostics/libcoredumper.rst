.. libcoredumper:

==================================
Using libcoredumper
==================================

:Availability: This tool is in tech preview.

This feature was implemented in *Percona Server for MySQL* 8.0.21-12 and has been tested
against the version's supported platforms. The tool may not be supported on future platforms. You should test before putting this tool into production.

A core dump file is the documented moment of a computer when either the computer or an application exits. Developers examine the dump as one of the tasks when searching for the cause of a failure.

The ``libcoredumper`` is a free and Open Source fork of ``google-coredumper``,
enhanced to work on newer Linux versions, and GCC and CLANG.

.. rubric:: Enabling the ``libcoredumper``

Enable core dumps for troubleshooting purposes. 

To enable the ``libcoredumper``, add the ``coredumper`` variable to the
``mysqld`` section of ``my.cnf``. This variable is independent of the
older ``core-file`` variable.

The variable can have the following possible values: 

.. tabularcolumns:: |p{5cm}|p{11cm}|

.. list-table::
   :header-rows: 1

   * - Value 
     - Description
   * - Blank
     - The core dump is saved under MySQL datadir and named ``core``.
   * - A path ending with /
     - The core dump is saved under the specified directory and named ``core``.
   * - Full path with a filename
     - The core dump is saved under the specified directory and filename

Restart the server.

.. rubric:: Verifying the ``libcoredumper`` is Active

MySQL writes to the log when generating a core file and delegates the core
dump operation to the Linux kernel. An example of the log message is the
following:

.. code-block:: text

    Writing a core file

MySQL using the ``libcoredumper`` to generate the file creates the following
message in the log:

.. code-block:: text

    Writing a core file using lib coredumper

Every core file adds a crash timestamp instead of a PID for the following
reasons:

* Correlates the core file with the crash. MySQL prints a UTC timestamp
  on the crash log.

    .. code-block:: bash

        10:02:09 UTC - mysqld got signal 11;

* Stores multiple core files.

.. note:: 

    For example, operators and containers run as the process id of PID 1. 
    If the process ID is used to identify the core file, each container crash 
    generates a core dump that overwrites the previous core file.

.. rubric:: Disabling the libcoredumper

You can disable the libcoredumper. A core file may contain sensitive data and
takes disk space. 

To disable the ``libcoredumper`` you must do the following:

1. In the ``mysqld`` section of my.cnf, remove the ``libcoredumper`` variable.

2. Restart the server.


