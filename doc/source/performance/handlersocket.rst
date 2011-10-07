.. _handlersocket_page:

=================
 *HandlerSocket*
=================

Description
===========

*HandlerSocket* is a |MySQL| plugin that implements a ``NoSQL`` protocol for |MySQL|. This allows applications to communicate more directly with |MySQL| storage engines, without the overhead associated with using ``SQL``. This includes operations such as parsing and optimizing queries, as well as table handling operations (opening, locking, unlocking, closing). As a result, using *HandlerSocket* can provide much better performance for certain applications that using normal ``SQL`` application protocols.

Complete documentation on the *HandlerSocket* plugin, including installation and configuration options, is located here.

The plugin is disabled by default. To enable it in |Percona Server| with |XtraDB|, see below.

**Please be aware that this is currently an experimental feature, and use it as such.**


Version Specific Information
============================

  * 5.1.52-12.3
    Full functionality available.

Other Information

Author/Origin	 Akira Higuchi, DeNA Co., Ltd.

Enabling the Plugin
===================

Once *HandlerSocket* has been downloaded and installed on your system, there are two steps required to enable it.

First, add the following lines to the [mysqld] section of your :term:`my.cnf` file: ::

  loose_handlersocket_port = 9998
    # the port number to bind to for read requests
  loose_handlersocket_port_wr = 9999
    # the port number to bind to for write requests
  loose_handlersocket_threads = 16
    # the number of worker threads for read requests
  loose_handlersocket_threads_wr = 1
    # the number of worker threads for write requests
  open_files_limit = 65535
    # to allow handlersocket to accept many concurrent
    # connections, make open_files_limit as large as
    # possible.

Second, log in to mysql as root, and execute the following query: ::

  mysql> install plugin handlersocket soname 'handlersocket.so';

Testing the Plugin installation
===============================

If :file:`handlersocket.so` was successfully installed, it will begin accepting connections on ports 9998 and 9999. Executing a ``SHOW PROCESSLIST`` command should show *HandlerSocket* worker threads: ::

  mysql> SHOW PROCESSLIST;
  +----+-------------+-----------------+---------------+---------+------+-------------------------------------------+------------------+
  | Id | User        | Host            | db            | Command | Time | State                                     | Info             |
  +----+-------------+-----------------+---------------+---------+------+-------------------------------------------+------------------+
  |  1 | system user | connecting host | NULL          | Connect | NULL | handlersocket: mode=rd, 0 conns, 0 active | NULL             |
  |  2 | system user | connecting host | NULL          | Connect | NULL | handlersocket: mode=rd, 0 conns, 0 active | NULL             |
  ...
  | 16 | system user | connecting host | NULL          | Connect | NULL | handlersocket: mode=rd, 0 conns, 0 active | NULL             |
  | 17 | system user | connecting host | handlersocket | Connect | NULL | handlersocket: mode=wr, 0 conns, 0 active | NULL             |

To ensure *HandlerSocket* is working as expected, you can follow these steps:

Create a new table: ::

  mysql> CREATE TABLE t (
    id int(11) NOT NULL,
    col varchar(20) NOT NULL,
    PRIMARY KEY (id)
  ) ENGINE=InnoDB;

Insert a row with *HandlerSocket* (fields are separated by tabs): ::

  $ telnet 127.0.0.1 9999
  Trying 127.0.0.1...
  Connected to 127.0.0.1.
  Escape character is '^]'.
  P     1	test	t	PRIMARY	id	col
  0	1
  1	+	2	1       test value
  0	1

And check in SQL that the row has been written: ::

  mysql> SELECT * FROM t;
  +----+------------+
  | id | col        |
  +----+------------+
  |  1 | test value |
  +----+------------+

Configuration options
---------------------

*HandlerSocket* has many configuration options that are detailed `here <https://github.com/ahiguti/HandlerSocket-Plugin-for-MySQL/blob/master/docs-en/configuration-options.en.txt>`_.


Other Reading
=============

  * Yoshinori Matsunobu's blog post describing `HandlerSocket <http://yoshinorimatsunobu.blogspot.com/2010/10/using-mysql-as-nosql-story-for.html>`_

  * `Percona Server now both SQL and NOSQL <http://www.mysqlperformanceblog.com/2010/12/14/percona-server-now-both-sql-and-nosql/>`_
