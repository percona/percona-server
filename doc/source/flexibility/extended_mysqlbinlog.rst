.. _extended_mysqlbinlog:

========================
Extended ``mysqlbinlog``
========================

|Percona Server| has implemented protocol compression support for the
:command:`mysqlbinlog` command. 

You can request protocol compression when connecting to a remote server to
transfer binary log files. The protocol compression helps reduce the
bandwidth use and improves the transfer speed.

In the `mysqlbinlog utility
<https://dev.mysql.com/doc/refman/5.7/en/mysqlbinlog.html>`__ add either the
``--compress`` or ``-C`` flag to the command-line options.

.. code-block:: text

    mysqlbinlog [--compress|-C] --remote-server


Version Specific Information
============================

  * :rn:`5.7.10-1`
    Feature ported from |Percona Server| 5.6
