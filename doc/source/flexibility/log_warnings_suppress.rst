.. _log_warning_suppress:

===========================
 Suppress Warning Messages
===========================

This feature is intended to provide a general mechanism (using ``log_warnings_silence``) to disable certain warning messages to the log file. Currently, it is only implemented for disabling message #1592 warnings. This feature does not influence warnings delivered to a client.


Version Specific Information
============================

  * :rn:`5.5.8-20.0`:
    System variable :variable:`log_warnings_silence` introduced.

  * :rn:`5.5.10-20.1`:
    Renamed variable :variable:`log_warnings_silence` to :variable:`log_warnings_suppress`.


System Variables
================

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

It is intended to provide a more general mechanism for disabling warnings than existed previously with variable :variable:`suppress_log_warning_1592`.

When set to the empty string, no warnings are disabled. When set to ``1592``, warning #1592 messages (unsafe statement for binary logging) are suppressed.

In the future, the ability to optionally disable additional warnings may also be added.


Related Reading
===============

  * `MySQL bug 42851 <http://bugs.mysql.com/bug.php?id=42851>`_

  * `MySQL InnoDB replication <http://dev.mysql.com/doc/refman/5.5/en/innodb-and-mysql-replication.html>`_

  * `InnoDB Startup Options and System Variables <http://dev.mysql.com/doc/refman/5.5/en/innodb-parameters.html>`_

  * `InnoDB Error Handling <http://dev.mysql.com/doc/refman/5.5/en/innodb-error-handling.html>`_
