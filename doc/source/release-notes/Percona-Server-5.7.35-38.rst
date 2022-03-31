.. _PS-5.7.35-38:

================================================================================
*Percona Server for MySQL* 5.7.35-38
================================================================================

:Date: August 18, 2021
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.7/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.35-38
includes all the features and bug fixes available in
`MySQL 5.7.35 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-35.html>`_
in addition to `enterprise-grade features <https://www.percona.com/software/mysql-database/percona-server/feature-comparison>`__ developed by Percona.


Bugs Fixed
================================================================================

* :jirabug:`PS-1346`: LP #1163232: Anomaly with ``opt_log_slow_slave_statements``.
* :jirabug:`PS-1344`: LP #1160436: The ``log_slow_statement`` is called unconditionally
* :jirabug:`PS-7582`: Segmentation fault with the data masking plugin
* :jirabug:`PS-1108`: LP #1704163: Changing a column from uncompressed to compressed for JSON crashes the server
* :jirabug:`PS-2433`: LP #1234346: Include a timestamp in the slow query log file when initializing a new file
* :jirabug:`PS-1955`: LP #1088529: The ``log_slow_verbosity`` help text missing the "minimal", "standard", and "full" options
* :jirabug:`PS-1116`: LP #1719506: Audit plugin reports "command_class=error" for server-side prepared statements.
* :jirabug:`PS-7755`: InnoDB: tried to purge ``non-delete-marked`` record (Upstream :mysqlbug:`86485`).
* :jirabug:`PS-6802`: Configure fails with make-4.3 with CMake Error at storage/rocksdb/CMakeLists.txt:152 (STRING) (Thanks to user whissi for reporting this issue)
* :jirabug:`PS-1659`: LP #1508909: Connect without proxy information hangs if ``proxy_protocol_networks`` is enabled
* :jirabug:`PS-7746`: Possible double call to `free_share()` in ha_innobase::open()


