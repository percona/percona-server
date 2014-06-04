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


Example Output
==============

``SHOW PROCESSLIST`` Command: ::

  mysql> show processlist;
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+
  | Id   | User      | Host      | db     | Command | Time | State      | Info                                         |
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+
  |    2 | root      | localhost | test   | Query   |    0 | NULL       | SHOW PROCESSLIST                             |
  |   14 | root      | localhost | test   | Query   |    0 | User lock  | SELECT GET_LOCK(``t``,1000)                  | 
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+

Table :table:`PROCESSLIST`: ::

  mysql> select * from information_schema.PROCESSLIST;
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+
  | ID   | USER      | HOST      | DB     | COMMAND | TIME | STATE      | INFO                                         |
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+
  |   14 | root      | localhost | test   | Query   |    0 | User lock  | SELECT GET_LOCK(``t``,1000)                  |
  |    2 | root      | localhost | test   | Query   |    0 | executing  | SELECT * from INFORMATION_SCHEMA.PROCESSLIST |
  +------+-----------+-----------+--------+---------+------+------------+----------------------------------------------+

 
