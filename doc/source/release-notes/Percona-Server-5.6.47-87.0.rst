.. rn:: 5.6.47-87.0:
================================================================================
*Percona Server for MySQL* 5.6.47-87.0
================================================================================
:Date: January 21, 2020

:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.6/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.6.47-87.0
includes all the features and bug fixes available in
`MySQL 5.6.47 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-47.html>`_
in addition to enterprise-grade features developed by Percona.

Bugs Fixed
================================================================================

* :psbug:`1469`: The Memory storage engine detects an incorrect "is full"
  condition stuck when the space contained reusable memory chunks and the
  space could be reused.
* :psbug:`6023`: Percona Server exits when a Kerberos password is changed after
  the password has expired.
* :psbug:`5813`: Setting 'none' value for :variable:`slow_query_log_use_global_control` throws error.
* :psbug:`4541`: Document that maximum- and minimum- option modifiers should
  not be used with non-numeric system variables
* :psbug:`6750`: Red Hat Enterprise Linux 8 had a file conflict when installing client packages
* :psbug:`5675`: wrong row is updated (Upstream :mysqlbug:`96578`)
* :psbug:`5591`: mysql server crash after empty set sql query (Upstream :mysqlbug:`95236`)
* :psbug:`1673`: :variable:`ps_tokudb_admin` cannot install tokudb remotely
* :psbug:`5956`: Root session must not be allowed to kill :ref:`psaas_utility_user`
  session
* :psbug:`5952`: :ref:`psaas_utility_user` is visible in performance_schema
  .threads
* :psbug:`5642`: page tracker thread does not exit if startup fails
* :psbug:`4842`: Documentation -  :variable:`innodb_corrupt_table_action` does
  not list a default value
* :psbug:`5521`: Documentation - :variable:`tokudb_cache_size` definition
  missing
