.. _log_warning_suppress:

===========================
 Suppress Warning Messages
===========================

This feature is intended to provide a general mechanism (using ``log_warnings_silence``) to disable certain warning messages to the log file. Currently, it is only implemented for disabling message #1592 warnings.


Version Specific Information
============================

  * 5.1.47-11.0: 
    System variable :variable:`suppress_log_warning_1592` introduced.

  * 5.5.8-20.0:
    System variable :variable:`suppress_log_warning_1592` replaced by :variable:`log_warnings_silence`.

  * 5.5.10-20.1:
    Renamed variable :variable:`log_warnings_silence` to :variable:`log_warnings_suppress`.

System Variables
================

.. variable:: suppress_log_warning_1592

     :version 5.1.47-11.0: Introduced.
     :version 5.5.8-20.0: Deleted.
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

This variable was obsoleted, beginning in release 5.5.8-20.0. It still exists in versions 5.1.47-11.0 to 5.1.54-12.5.

.. variable:: log_warnings_suppress

     :version 5.5.8-20.0: Introduced.
     :version 5.5.10-20.1: Renamed.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: SET
     :default: ``(empty string)``
     :range: ``(empty string)``, ``1592``

This variable was added in beta release ``5.5.8-20.0`` as :variable:`log_warnings_silence` and renamed in release 5.5.10-20.1.

It is intended to provide a more general mechanism for disabling warnings than existed previously with variable suppress_log_warning_1592.

When set to the empty string, no warnings are disabled. When set to ``1592``, warning #1592 messages (unsafe statement for binary logging) are suppressed.

In the future, the ability to optionally disable additional warnings may also be added.


Related Reading
===============

  * `MySQL bug 42851 <http://bugs.mysql.com/bug.php?id=42851>`_

  * `MySQL InnoDB replication <http://dev.mysql.com/doc/refman/5.1/en/innodb-and-mysql-replication.html>`_

  * `InnoDB Startup Options and System Variables <http://dev.mysql.com/doc/refman/5.1/en/innodb-parameters.html>`_

  * `InnoDB Error Handling <http://dev.mysql.com/doc/refman/5.1/en/innodb-error-handling.html>`_
