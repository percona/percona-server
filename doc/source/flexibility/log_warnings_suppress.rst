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

  * `8.0.12-1`: The feature was ported from *Percona Server for MySQL* 5.7

System Variables
================

.. _log_warnings_suppress:

.. rubric:: ``log_warnings_suppress``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - SET
   * - Default
     - ``(empty string)``
   * - Range
     - ``(empty string)``, ``1592``

It is intended to provide a more general mechanism for disabling warnings than existed previously with variable :ref:`suppress_log_warning_1592`.
When set to the empty string, no warnings are disabled. When set to ``1592``, warning #1592 messages (unsafe statement for binary logging) are suppressed.
In the future, the ability to optionally disable additional warnings may also be added.


Related Reading
===============

  * `MySQL bug 42851 <http://bugs.mysql.com/bug.php?id=42851>`_

  * `MySQL InnoDB replication <http://dev.mysql.com/doc/refman/8.0/en/innodb-and-mysql-replication.html>`_

  * `InnoDB Startup Options and System Variables <http://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html>`_

  * `InnoDB Error Handling <http://dev.mysql.com/doc/refman/8.0/en/innodb-error-handling.html>`_
