.. _show_engines:

======================
 Show Storage Engines
======================

This feature changes the comment field displayed when the ``SHOW STORAGE ENGINES`` command is executed and XtraDB is the storage engine.

Before the Change: ::

  mysql> show storage engines;
  +------------+---------+----------------------------------------------------------------+--------------+------+------------+
  | Engine     | Support | Comment                                                        | Transactions | XA   | Savepoints |
  +------------+---------+----------------------------------------------------------------+--------------+------+------------+
  | InnoDB     | YES     | Supports transactions, row-level locking, and foreign keys     | YES          | YES  | YES        |
  ...
  +------------+---------+----------------------------------------------------------------+--------------+------+------------+

After the Change: ::

  mysql> show storage engines;
  +------------+---------+----------------------------------------------------------------------------+--------------+------+------------+ 
  | Engine     | Support | Comment                                                                    | Transactions |   XA | Savepoints |
  +------------+---------+----------------------------------------------------------------------------+--------------+------+------------+
  | InnoDB     | YES     | Percona-XtraDB, Supports transactions, row-level locking, and foreign keys |          YES | YES  | YES        |
  ...
  +------------+---------+----------------------------------------------------------------------------+--------------+------+------------+

Version-Specific Information
============================

  * :rn:`5.7.10-1`:
    Feature ported from |Percona Server| 5.6

