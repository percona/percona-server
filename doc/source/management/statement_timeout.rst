.. _statement_timeout:

===================
 Statement Timeout
===================

.. note::

 This feature implementation is considered ALPHA quality.

|Percona Server| has implemented a statement timeout feature. This feature can be used to limit the query execution time by specifying the timeout value in the :variable:`max_statement_time` variable. After the specified number of miliseconds is reached the server will attempt to abort the statement and return the following error to the client: :: 

  ERROR 1877 (70101): Query execution was interrupted, max_statement_time exceeded


Version Specific Information
============================

  * :rn:`5.6.13-61.0`:
    Statement timeout implemented. This feature with some changes was ported from `Twitter <https://github.com/twitter/mysql/wiki/Statement-Timeout>`_ |MySQL| patches.

System Variables
================

.. variable:: max_statement_time

     :version 5.6.13-61.0: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global, Session
     :dyn: Yes
     :vartype: Numeric
     :values: 0 - 18446744073709551615
     :default: 0 (no timeout is enforced)
     :unit: milisecond

This system variable is used to specify the maximum execution time for any statement. After specified number of miliseconds is reached server will attempt to abort the statement.

.. variable:: have_statement_timeout

     :version 5.6.13-61.0: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Boolean
     :values: YES/NO

This system variable shows if the feature is supported for the current operating system.

Status Variables
================

.. variable:: Max_statement_time_exceeded

     :version 5.6.13-61.0: Introduced.
     :scope: Global
     :type: Numeric

This status variable shows the number of statements that exceeded execution time limit.

.. variable:: Max_statement_time_set

     :version 5.6.13-61.0: Introduced.
     :scope: Global
     :type: Numeric

This status variable shows the number of statements for which execution time limit was set.

.. variable:: Max_statement_time_set_failed

     :version 5.6.13-61.0: Introduced.
     :scope: Global
     :type: Numeric

This status variable shows the number of statements for which execution time limit could not be set, that can happen if some OS-related limits were exceeded.
