.. _log_warning_suppress:

===========================
 Suppress Warning Messages
===========================

This feature is intended to provide a general mechanism (using ``log_warnings_silence``) to disable certain warning messages to the log file. Currently, it is only implemented for disabling message #1592 warnings. This feature does not influence warnings delivered to a client.


Version Specific Information
============================

  * :rn:`5.6.11-60.3`:
    Variable :variable:`log_warnings_suppress` ported from |Percona Server| 5.5.


System Variables
================

.. variable:: log_warnings_suppress

     :version 5.6.11-60.3: Introduced.
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

  * `MySQL InnoDB replication <http://dev.mysql.com/doc/refman/5.6/en/innodb-and-mysql-replication.html>`_

  * `InnoDB Startup Options and System Variables <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html>`_

  * `InnoDB Error Handling <http://dev.mysql.com/doc/refman/5.6/en/innodb-error-handling.html>`_
