.. _replication_skip_single_statement:

===========================
 Replication Stop Recovery
===========================

After a replication stop, this feature allows skipping a user-specified number of events from the binary log, rather than all events remaining in the current event group.

The discussion of the system variable ``sql_slave_skip_counter`` in the `MySQL 5.1 Reference Manual notes <http://dev.mysql.com/doc/refman/5.1/en/set-global-sql-slave-skip-counter.html>`_:

When using this statement, it is important to understand that the binary log is actually organized 
as a sequence of groups known as event groups. Each event group consists of a sequence of events.

 * For transactional tables, an event group corresponds to a transaction.

 * For nontransactional tables, an event group corresponds to a single SQL statement.

A single transaction can contain changes to both transactional and nontransactional tables.

When you use SET GLOBAL sql_slave_skip_counter to skip events and the result is in the middle of a group, the slave continues to skip events until it reaches the end of the group. Execution then starts with the next event group.

When used, this feature modifies the standard |MySQL| behavior described above. It provides the user additional flexibility in recovering from a replication stop, but it should be used with caution. In particular, :variable:`sql_slave_statement_skip_counter` and ``sql_slave_skip_counter`` should not be used at the same time (i.e., they should not both be non-zero), as this may cause unintended behavior.


Version Specific Information
============================

  * 5.1.49-12.0:
    Full functionality released.

System Variables
================

The following status variable was introduced by this feature.

.. variable:: sql_slave_statement_skip_counter

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 0
     :range: 1-MAXLONGINT

This variable specifies the number of events in the current event group to skip.

**IMPORTANT:** As mentioned above, never set both this variable and sql_slave_skip_counter to non-zero values at the same time. The behavior that will result from this is unknown.

Consider this case: ::

  SET GLOBAL sql_slave_statement_skip_counter=n;
  SET GLOBAL sql_slave__skip_counter=m;

The first statement will skip the next ``n`` events in the current event group. At that point, one would expect the second statement to operate as described in the |MySQL| documentation for ``sql_slave_skip_counter``. However, depending on the values of ``n`` and ``m`` and the contents of the binary log, the effects of the interactions of the two statements can involve very complicated scenarios. **THESE SCENARIOS HAVE NOT BEEN TESTED**.

Example
=======

In the example here, a master server with two slaves is assumed. A table is created on the master and then a replication error occurs.

Then the difference between using the new system variable :variable:`sql_slave_statement_skip_counter` and the |MySQL| system variable ``sql_slave_skip_counter`` to repair the replication stop is shown.

Setup
-----

  - Prepare the master server: ::

      DROP TABLE IF EXISTS t;
      CREATE TABLE t(id INT UNIQUE PRIMARY KEY) ENGINE=|InnoDB|;
      INSERT INTO t VALUES (1),(2);

  - Prepare Slave #1 and Slave #2: ::

      DROP TABLE IF EXISTS t;
      CREATE TABLE t(id INT UNIQUE PRIMARY KEY) ENGINE=|InnoDB|;
      INSERT INTO t VALUES (1),(2),(3),(5);

  - Start replication of the master to the two slaves.

Create the Replication Error
----------------------------

Run the following on the master: ::

  START TRANSACTION;
  INSERT INTO t VALUES (3);
  INSERT INTO t VALUES (4);
  COMMIT;

Both slaves will fail with the following error: ::

  Error ``Duplicate entry ``3`` for key ``PRIMARY```` on query. Default database: ``test``. Query: ``INSERT INTO t VALUES (3)``

Repair the Replication Error
----------------------------

Now, let's compare the effects of using ``sql_slave_skip_counter`` to do the repair versus using :variable:`sql_slave_statement_skip_counter` to do it.

Slave Repair Using sql_slave_skip_counter
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To repair either slave, do the following: ::

  SET GLOBAL sql_slave_skip_counter=1;
  START SLAVE;

This will cause the slave to skip the following statement: ::

  INSERT INTO t VALUES (3);

In addition, since we are in the middle of an event group in the binary log, all other events in the event group will also be skipped, since that is the standard behavior of ``sql_slave_skip_counter``. In this case, the following statements will also be skipped: ::

  INSERT INTO t VALUES (4);
  COMMIT;

Now run the following on the slave: ::

  SELECT * FROM t;

This will give the result: ::

  id
  1
  2
  3
  5

Since the table's original contents are unchanged, this shows that ``sql_slave_skip_counter`` caused the entire event group to be skipped.

Slave Repair Using sql_slave_statement_skip_counter
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To repair either slave, do the following: ::

  SET GLOBAL sql_slave_statement_skip_counter=1;
  START SLAVE;

This will cause the slave to skip the following statement: ::

  INSERT INTO t VALUES (3);

However, unlike with ``sql_slave_skip_counter``, this is the only event that will be skipped. Every other event in the current event group in the binary log will be executed. In this case, that means these statements will not be skipped; they will also be executed: ::

  INSERT INTO t VALUES (4);
  COMMIT;

Now, we can see the difference in results when we run: ::

  SELECT * FROM t;

Now, our results are: ::

  id
  1
  2
  3
  4
  5

In this case, ``sql_slave_statement_skip_counter`` caused the server to skip only single statement, not the entire remainder of the event group. The result is that the original table has been updated.


Other Reading
-------------

  * `MySQL 5.1 Reference Manual - "SET GLOBAL sql_slave_skip_counter Syntax" <http://dev.mysql.com/doc/refman/5.1/en/set-global-sql-slave-skip-counter.html>`_
