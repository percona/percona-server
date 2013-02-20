.. _process_list:

=============
Process List
=============

This page describes Percona changes to both the standard |MySQL| ``SHOW PROCESSLIST`` command and the standard |MySQL| ``INFORMATION_SCHEMA`` table ``PROCESSLIST``.

The changes that have been made as of version 5.6 of the server are:

  * :table:`PROCESSLIST` table:

    * added column ``TIME_MS``

Version Specific Information
============================

  * :rn:`5.6.5-60.0`:

    * Added column ``TIME_MS`` to table ``PROCESSLIST``.


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

Table :table:`PROCESSLIST`: ::

  mysql> SELECT * FROM INFORMATION_SCHEMA.PROCESSLIST;

  +----+------+-----------+--------------------+---------+------+-----------+----------------------------------------------+---------+
  | ID | USER | HOST      | DB                 | COMMAND | TIME | STATE     | INFO                                         | TIME_MS |
  +----+------+-----------+--------------------+---------+------+-----------+----------------------------------------------+---------+
  |  5 | root | localhost | information_schema | Query   |    0 | executing | select * from information_schema.PROCESSLIST |       0 |
  +----+------+-----------+--------------------+---------+------+-----------+----------------------------------------------+---------+
 
