.. _process_list:

=============
Process List
=============

This page describes Percona changes to both the standard |MySQL| ``SHOW PROCESSLIST`` command and the standard |MySQL| ``INFORMATION_SCHEMA`` table ``PROCESSLIST``.

INFORMATION_SCHEMA Tables
=========================

.. table:: INFORMATION_SCHEMA.PROCESSLIST

   This table implements modifications to the standard |MySQL| ``INFORMATION_SCHEMA`` table ``PROCESSLIST``.

   :column ID: The connection identifier.
   :column USER: The |MySQL| user who issued the statement.
   :column HOST: The host name of the client issuing the statement.
   :column DB: The default database, if one is selected, otherwise NULL.
   :column COMMAND: The type of command the thread is executing.
   :column TIME: The time in seconds that the thread has been in its current state.
   :column STATE: An action, event, or state that indicates what the thread is doing.
   :column INFO: The statement that the thread is executing, or NULL if it is not executing any statement.
   :column TIME_MS: The time in milliseconds that the thread has been in its current state.
   :column ROWS_EXAMINED: The number of rows examined by the statement being executed.
   :column ROWS_SENT:	The number of rows sent by the statement being executed.
   :column ROWS_READ: The number of rows read by the statement being executed.


Example Output
==============

``SHOW PROCESSLIST`` Command: ::

  mysql> show processlist;
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+-----------+---------------+-----------+
  | Id   | User      | Host      | db     | Command | Time | State      | Info                                         | ROWS_SENT | ROWS_EXAMINED | ROWS_READ |
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+-----------+---------------+-----------+
  |    2 | root      | localhost | test   | Query   |    0 | NULL       | SHOW PROCESSLIST                             |         0 |             0 |         1 |
  |   14 | root      | localhost | test   | Query   |    0 | User lock  | SELECT GET_LOCK(``t``,1000)                    |         0 |             0 |         1 |
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+-----------+---------------+-----------+

Table :table:`PROCESSLIST`: ::

  mysql> select * from information_schema.PROCESSLIST;
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+----------+---------------+-----------+-----------+
  | ID   | USER      | HOST      | DB     | COMMAND | TIME | STATE      | INFO                                         | TIME_MS  | ROWS_EXAMINED | ROWS_SENT | ROWS_READ |
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+----------+---------------+-----------+-----------+
  |   14 | root      | localhost | test   | Query   |    0 | User lock  | SELECT GET_LOCK(``t``,1000)                    |        1 |             0 |         0   |         1 |
  |    2 | root      | localhost | test   | Query   |    0 | executing  | SELECT * from INFORMATION_SCHEMA.PROCESSLIST |        0 |             0 |         0 |         1 |
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+----------+---------------+-----------+-----------+

 
