.. _5.7.36-39:

================================================================================
*Percona Server for MySQL* 5.7.36-39
================================================================================

:Date: December 22, 2021
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.7/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.36-39
includes all the features and bug fixes available in the
`MySQL 5.7.36 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-36.html>`__.,
in addition to enterprise-grade features developed by Percona.

Percona Server for MySQL® is a free, fully compatible, enhanced, and open
source drop-in replacement for any MySQL database. It provides superior
performance, scalability, and instrumentation.

Percona Server for MySQL is trusted by thousands of enterprises to provide
better performance and concurrency for their most demanding workloads. 
It delivers more value to MySQL server users with optimized performance,
greater performance scalability and availability, enhanced backups, and
increased visibility. `Commercial support contracts are available
<https://www.percona.com/services/support/mysql-support>`__.

Release Highlights
=================================================

The following lists some of the bug fixes for MySQL 5.7.36, provided by Oracle, and included in Percona Server for MySQL:

* Fix for the possibility for a deadlock or failure when an undo log truncate operation is initiated after an upgrade from *MySQL* 5.6 to *MySQL* 5.7.
* Fix for when a parent table initiates a cascading ``SET NULL`` operation on the child table, the virtual column can be set to NULL instead of the value derived from the parent table.
* On a view, the query digest for each SELECT statement is now based on the SELECT statement and not the view definition, which was the case for earlier versions.
Find the complete list of bug fixes and changes in `MySQL 5.7.36 Release Notes <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-36.html>`__.

Improvements
==============================================

* :jirabug:`PS-6730` The `Last_errno` field in the Slow Query Log only reports the errors.

Bugs Fixed
===============================================

* :jirabug:`PS-7868`: Documentation - remove a reference to a 5.7 SELinux repository in the Yum installation document. (Thanks to user Simon Avery for reporting this issue)
* :jirabug:`PS-7958`: Fix for a MySQL exit when using a full-text search index with a special character.
* :jirabug:`PS-1484`: Fix for slow log rotation when the file name has an extension. 