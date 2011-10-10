.. _innodb_deadlock_count:

==========================
 Count |InnoDB| Deadlocks
==========================

When running a transactional application you have to live with deadlocks. They are not problematic as long as they do not occur too frequently. The standard ``SHOW INNODB STATUS`` gives information on the latest deadlocks but it is not very useful when you want to know the total number of deadlocks or the number of deadlocks per unit of time. 

This change adds a status variable that keeps track of the number of deadlocks since the server startup, opening the way to a better knowledge of your deadlocks.

This feature was provided by Eric Bergen under BSD license (see `InnoDB Deadlock Count Patch <http://ebergen.net/wordpress/2009/08/27/innodb-deadlock-count-patch/>`_).

It adds a new global status variable (:variable:`innodb_deadlocks`) showing the number of deadlocks.*

You can use it with ``SHOW GLOBAL STATUS``, e.g.: ::

  mysql> SHOW GLOBAL_STATUS LIKE ``innodb_deadlocks``;
  +------------------+-------+
  | Variable_name    | Value |
  +------------------+-------+
  | innodb_deadlocks | 323   |
  +------------------+-------+

or with ``INFORMATION_SCHEMA``, e.g.: ::

  mysql> SELECT VARIABLE_VALUE FROM INFORMATION_SCHEMA.GLOBAL_STATUS WHERE VARIABLE_NAME = ``innodb_deadlocks``; 
  +----------------+
  | VARIABLE_VALUE |
  +----------------+
  | 323            |
  +----------------+

A deadlock will occur when at least two transactions are mutually waiting for the other to finish, thus creating a circular dependency that lasts until something breaks it. |InnoDB| is quite good at detecting deadlocks and generally returns an error instantly. Most transactional systems have no way to prevent deadlocks from occurring and must be designed to handle them, for instance by retrying the transaction that failed.


Version Specific Information
============================

  * 5.1.47-11.0:
    Full functionality available.

Status Variables
================

One new status variable was introduced by this feature.

.. variable:: innodb_deadlocks

     :vartype: LONG
     :scope: Global


Related Reading
===============

  * `Original post by Eric Bergen <http://ebergen.net/wordpress/2009/08/27/|InnoDB|-deadlock-count-patch/>`_

