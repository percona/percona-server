.. _log_warnings_suppress_page:

===========================
 Suppress Warning Messages
===========================

This feature is intended to provide a general mechanism (using ``log_warnings_silence``) to disable certain warning messages to the log file. Currently, it is only implemented for disabling message #1592 warnings.


Version Specific Information
============================

  * :rn:`5.1.47-rel11.1`
    System variable :variable:`suppress_log_warning_1592` introduced.

System Variables
================

.. variable:: suppress_log_warning_1592

     :version 5.1.47-rel11.1: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Boolean
     :default: OFF
     :range: ON(='1592') / OFF

This has no effect on replication, but it can fill up your error log with unnecessary messages. This variable allows you to completely disable logging of this warning.

**NOTE:** Only |MySQL| 5.1 is subject to this bug. A partial solution has been published beginning with |MySQL| 5.1.37, but this bug still appears in some situations.

When ``ON``, disables reporting of warning #1592 (unsafe statement for binary logging).

All warnings #1592 will be disabled, so you will not be able to know if your statements are really safe to replicate anymore. Use it at your own risk and only if you understand what you are doing.

In some circumstances, |MySQL| will warn you that a statement is unsafe to replicate even though it is perfectly safe. For example: ::

  090213 16:58:54 [Warning] Statement is not safe to log in statement format.

Related Reading
===============

  * `MySQL bug #42851 <http://bugs.mysql.com/bug.php?id=42851>`_

  * `MySQL InnoDB replication <http://dev.mysql.com/doc/refman/5.1/en/innodb-and-mysql-replication.html>`_

  * `InnoDB Startup Options and System Variables <http://dev.mysql.com/doc/refman/5.1/en/innodb-parameters.html>`_

  * `InnoDB Error Handling <http://dev.mysql.com/doc/refman/5.1/en/innodb-error-handling.html>`_
