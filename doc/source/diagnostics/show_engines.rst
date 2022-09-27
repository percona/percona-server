.. _show_engines:

======================
 Show Storage Engines
======================

This feature changes the comment field displayed when the ``SHOW STORAGE ENGINES`` command is executed and |XtraDB| is the storage engine.

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

  * `8.0.12-1`: The feature was ported from *Percona Server for MySQL* 5.7.

