.. _log_warning_suppress:

===========================
 Suppress Warning Messages
===========================

This feature is intended to provide a general mechanism (using ``log_warnings_silence``) to disable certain warning messages to the log file. Currently, it is only implemented for disabling message #1592 warnings. This feature does not influence warnings delivered to a client. Please note that warning code needs to be a string:

.. code-block:: mysql

  mysql> SET GLOBAL log_warnings_suppress = '1592';
  Query OK, 0 rows affected (0.00 sec)


Version Specific Information
============================

  * :rn:`5.7.10-1`:
    Variable :variable:`log_warnings_suppress` ported from |Percona Server| 5.6.

  * :rn:`5.7.11-4`:
    Feature has been removed from |Percona Server| 5.7 because MySQL 5.7.11 has implemented a new system variable, `log_statements_unsafe_for_binlog <https://dev.mysql.com/doc/refman/5.7/en/replication-options-binary-log.html#sysvar_log_statements_unsafe_for_binlog>`_, which implements the same effect.

System Variables
================

.. variable:: log_warnings_suppress

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: SET
     :default: ``(empty string)``
     :range: ``(empty string)``, ``1592``

It is intended to provide a more general mechanism for disabling warnings than existed previously with variable :variable:`suppress_log_warning_1592`.
When set to the empty string, no warnings are disabled. When set to ``1592``, warning #1592 messages (unsafe statement for binary logging) are suppressed.
In the future, the ability to optionally disable additional warnings may also be added.


Related Reading
===============

  * `MySQL bug 42851 <http://bugs.mysql.com/bug.php?id=42851>`_

  * `MySQL InnoDB replication <http://dev.mysql.com/doc/refman/5.7/en/innodb-and-mysql-replication.html>`_

  * `InnoDB Startup Options and System Variables <http://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html>`_

  * `InnoDB Error Handling <http://dev.mysql.com/doc/refman/5.7/en/innodb-error-handling.html>`_
