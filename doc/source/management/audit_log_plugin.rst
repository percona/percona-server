.. _audit_log_plugin:

================================================================================
 Audit Log Plugin
================================================================================

Percona Audit Log Plugin provides monitoring and logging of connection and query
activity that were performed on specific server. Information about the activity
will be stored in the XML log file where each event will have its ``NAME``
field, its own unique ``RECORD_ID`` field and a ``TIMESTAMP`` field. This
implementation is alternative to the `MySQL Enterprise Audit Log Plugin
<dev.mysql.com/doc/refman/8.0/en/audit-log-plugin.html>`_

Audit Log plugin produces the log of following events:

* **Audit** - Audit event indicates that audit logging started or
  finished. ``NAME`` field will be ``Audit`` when logging started and
  ``NoAudit`` when logging finished. Audit record also includes server version
  and command-line arguments.

  Example of the Audit event: :: 

   <AUDIT_RECORD
   "NAME"="Audit"
   "RECORD"="1_2014-04-29T09:29:40"
   "TIMESTAMP"="2014-04-29T09:29:40 UTC"
   "MYSQL_VERSION"="5.6.17-65.0-655.trusty"
   "STARTUP_OPTIONS"="--basedir=/usr --datadir=/var/lib/mysql --plugin-dir=/usr/lib/mysql/plugin --user=mysql --log-error=/var/log/mysql/error.log --pid-file=/var/run/mysqld/mysqld.pid --socket=/var/run/mysqld/mysqld.sock --port=3306"
   "OS_VERSION"="x86_64-debian-linux-gnu",
   />

* **Connect**/**Disconnect** - Connect record event will have ``NAME`` field
  ``Connect`` when user logged in or login failed, or ``Quit`` when connection
  is closed. Additional fields for this event are ``CONNECTION_ID``, ``STATUS``,
  ``USER``, ``PRIV_USER``, ``OS_LOGIN``, ``PROXY_USER``, ``HOST``, and
  ``IP``. ``STATUS`` will be ``0`` for successful logins and non-zero for failed
  logins.

  Example of the Disconnect event: :: 

   <AUDIT_RECORD
   "NAME"="Quit"
   "RECORD"="24_2014-04-29T09:29:40"
   "TIMESTAMP"="2014-04-29T10:20:13 UTC"
   "CONNECTION_ID"="49"
   "STATUS"="0"
   "USER"=""
   "PRIV_USER"=""
   "OS_LOGIN"=""
   "PROXY_USER"=""
   "HOST"=""
   "IP"=""
   "DB"=""
   />

* **Query** - Additional fields for this event are: ``COMMAND_CLASS`` (values
  come from the ``com_status_vars`` array in the :file:`sql/mysqld.cc`` file in
  a MySQL source distribution. Examples are ``select``, ``alter_table``,
  ``create_table``, etc.), ``CONNECTION_ID``, ``STATUS`` (indicates error when
  non-zero), ``SQLTEXT`` (text of SQL-statement), ``USER``, ``HOST``,
  ``OS_USER``, ``IP``. Possible values for the ``NAME`` name field for this
  event are ``Query``, ``Prepare``, ``Execute``, ``Change user``, etc.

  Example of the Query event: :: 

   <AUDIT_RECORD
   "NAME"="Query"
   "RECORD"="23_2014-04-29T09:29:40"
   "TIMESTAMP"="2014-04-29T10:20:10 UTC"
   "COMMAND_CLASS"="select"
   "CONNECTION_ID"="49"
   "STATUS"="0"
   "SQLTEXT"="SELECT * from mysql.user"
   "USER"="root[root] @ localhost []"
   "HOST"="localhost"
   "OS_USER"=""
   "IP"=""
   />

Installation
============

Audit Log plugin is shipped with |Percona Server|, but it is not installed by
default. To enable the plugin you must run the following command:

.. code-block:: guess

   INSTALL PLUGIN audit_log SONAME 'audit_log.so';

You can check if the plugin is loaded correctly by running:

.. code-block:: guess

   SHOW PLUGINS;

Audit log should be listed in the output:
    
.. code-block:: guess

   +--------------------------------+----------+--------------------+--------------+---------+
   | Name                           | Status   | Type               | Library      | License |
   +--------------------------------+----------+--------------------+--------------+---------+
   ...
   | audit_log                      | ACTIVE   | AUDIT              | audit_log.so | GPL     |
   +--------------------------------+----------+--------------------+--------------+---------+

Log Format
==========

The audit log plugin supports four log formats: ``OLD``, ``NEW``, ``JSON``, and
``CSV``. ``OLD`` and ``NEW`` formats are based on XML, where the former outputs
log record properties as XML attributes and the latter as XML tags. Information
logged is the same in all four formats. The log format choice is controlled by
:variable:`audit_log_format` variable.

.. .. note::
.. 
..    The ``JSON`` format is fully compatible with that provided by
..    MySQL 8.0. Enterprise Edition.
.. 

Example of the ``OLD`` format: ::

 <AUDIT_RECORD
  "NAME"="Query"
  "RECORD"="2_2014-04-28T09:29:40"
  "TIMESTAMP"="2014-04-28T09:29:40 UTC"
  "COMMAND_CLASS"="install_plugin"
  "CONNECTION_ID"="47"
  "STATUS"="0"
  "SQLTEXT"="INSTALL PLUGIN audit_log SONAME 'audit_log.so'"
  "USER"="root[root] @ localhost []"
  "HOST"="localhost"
  "OS_USER"=""
  "IP"=""
 />

Example of the ``NEW`` format: :: 

 <AUDIT_RECORD>
  <NAME>Quit</NAME>
  <RECORD>10902_2014-04-28T11:02:54</RECORD>
  <TIMESTAMP>2014-04-28T11:02:59 UTC</TIMESTAMP>
  <CONNECTION_ID>36</CONNECTION_ID>
  <STATUS>0</STATUS>
  <USER></USER>
  <PRIV_USER></PRIV_USER>
  <OS_LOGIN></OS_LOGIN>
  <PROXY_USER></PROXY_USER>
  <HOST></HOST>
  <IP></IP>
  <DB></DB>
 </AUDIT_RECORD>

Example of the ``JSON`` format: ::

 {"audit_record":{"name":"Query","record":"4707_2014-08-27T10:43:52","timestamp":"2014-08-27T10:44:19 UTC","command_class":"show_databases","connection_id":"37","status":0,"sqltext":"show databases","user":"root[root] @ localhost []","host":"localhost","os_user":"","ip":""}}

Example of the ``CSV`` format: :: 

 "Query","49284_2014-08-27T10:47:11","2014-08-27T10:47:23 UTC","show_databases","37",0,"show databases","root[root] @ localhost []","localhost","",""

.. _streaming_to_syslog:

Streaming the audit log to syslog
=================================

To stream the audit log to syslog you'll need to set :variable:`audit_log_handler` variable to ``SYSLOG``. To control the syslog file handler, the following variables can be used: :variable:`audit_log_syslog_ident`, :variable:`audit_log_syslog_facility`, and :variable:`audit_log_syslog_priority` These variables have the same meaning as appropriate parameters described in the `syslog(3) manual <http://linux.die.net/man/3/syslog>`_.

.. note::

   Variables: :variable:`audit_log_strategy`, :variable:`audit_log_buffer_size`, :variable:`audit_log_rotate_on_size`, :variable:`audit_log_rotations` have effect only with ``FILE`` handler. 

.. _filtering_by_user:

Filtering by user
=================

The filtering by user feature adds two new global variables:
:variable:`audit_log_include_accounts` and
:variable:`audit_log_exclude_accounts` to specify which user accounts should be
included or excluded from audit logging.

.. warning:: 

   Only one of these variables can contain a list of users to be either
   included or excluded, while the other needs to be ``NULL``. If one of the
   variables is set to be not ``NULL`` (contains a list of users), the attempt
   to set another one will fail. Empty string means an empty list.

.. note::

   Changes of :variable:`audit_log_include_accounts` and
   :variable:`audit_log_exclude_accounts` do not apply to existing server
   connections.

Example
-------

Following example shows adding users who will be monitored: 

.. code-block:: guess

   mysql> SET GLOBAL audit_log_include_accounts = 'user1@localhost,root@localhost';
   Query OK, 0 rows affected (0.00 sec)

If you you try to add users to both include and exclude lists server will show
you the following error:

.. code-block:: guess

   mysql> SET GLOBAL audit_log_exclude_accounts = 'user1@localhost,root@localhost';
   ERROR 1231 (42000): Variable 'audit_log_exclude_accounts' can't be set to the value of 'user1@localhost,root@localhost'

To switch from filtering by included user list to the excluded one or back,
first set the currently active filtering variable to ``NULL``:

.. code-block:: guess

   mysql> SET GLOBAL audit_log_include_accounts = NULL;
   Query OK, 0 rows affected (0.00 sec)

   mysql> SET GLOBAL audit_log_exclude_accounts = 'user1@localhost,root@localhost';
   Query OK, 0 rows affected (0.00 sec)

   mysql> SET GLOBAL audit_log_exclude_accounts = "'user'@'host'";
   Query OK, 0 rows affected (0.00 sec)

   mysql> SET GLOBAL audit_log_exclude_accounts = '''user''@''host''';
   Query OK, 0 rows affected (0.00 sec)
  
   mysql> SET GLOBAL audit_log_exclude_accounts = '\'user\'@\'host\'';
   Query OK, 0 rows affected (0.00 sec)

To see what users are currently in the on the list you can run:

.. code-block:: guess

   mysql> SELECT @@audit_log_exclude_accounts;
   +------------------------------+
   | @@audit_log_exclude_accounts |
   +------------------------------+
   | 'user'@'host'                |
   +------------------------------+
   1 row in set (0.00 sec)

Account names from :table:`mysql.user` table are the one that are logged in the
audit log. For example when you create a user:

.. code-block:: guess

   mysql> CREATE USER 'user1'@'%' IDENTIFIED BY '111';
   Query OK, 0 rows affected (0.00 sec)

This is what you'll see when ``user1`` connected from ``localhost``:

.. code-block:: none

   <AUDIT_RECORD
    NAME="Connect"
    RECORD="4971917_2016-08-22T09:09:10"
    TIMESTAMP="2016-08-22T09:12:21 UTC"
    CONNECTION_ID="6"
    STATUS="0"
    USER="user1" ;; this is a 'user' part of account in 8.0
    PRIV_USER="user1"
    OS_LOGIN=""
    PROXY_USER=""
    HOST="localhost" ;; this is a 'host' part of account in 8.0
    IP=""
    DB=""
  />

To exclude ``user1`` from logging in |Percona Server| 8.0 you must set:

.. code-block:: guess

   SET GLOBAL audit_log_exclude_accounts = 'user1@%';

The value can be ``NULL`` or comma separated list of accounts in form
``user@host`` or ``'user'@'host'`` (if user or host contains comma).

.. _filtering_by_sql_command_type:

Filtering by SQL command type
=============================

The filtering by SQL command type adds two new global variables:
:variable:`audit_log_include_commands` and
:variable:`audit_log_exclude_commands` to specify which command types should be
included or excluded from audit logging.

.. warning:: 

   Only one of these variables can contain a list of command types to be
   either included or excluded, while the other needs to be ``NULL``. If one of
   the variables is set to be not ``NULL`` (contains a list of command types),
   the attempt to set another one will fail. Empty string means an empty list.

.. note:: 

   If both :variable:`audit_log_exclude_commands` and
   :variable:`audit_log_include_commands` are ``NULL`` all commands will be
   logged.

Example
-------

The available command types can be listed by running:

.. code-block:: guess

   mysql> SELECT name FROM performance_schema.setup_instruments WHERE name LIKE "statement/sql/%" ORDER BY name;
   +------------------------------------------+
   | name                                     |
   +------------------------------------------+
   | statement/sql/alter_db                   |
   | statement/sql/alter_db_upgrade           |
   | statement/sql/alter_event                |
   | statement/sql/alter_function             |
   | statement/sql/alter_procedure            |
   | statement/sql/alter_server               |
   | statement/sql/alter_table                |
   | statement/sql/alter_tablespace           |
   | statement/sql/alter_user                 |
   | statement/sql/analyze                    |
   | statement/sql/assign_to_keycache         |
   | statement/sql/begin                      |
   | statement/sql/binlog                     |
   | statement/sql/call_procedure             |
   | statement/sql/change_db                  |
   | statement/sql/change_master              |
   ...
   | statement/sql/xa_rollback                |
   | statement/sql/xa_start                   |
   +------------------------------------------+
   145 rows in set (0.00 sec)

You can add commands to the include filter by running:

.. code-block:: guess

   mysql> SET GLOBAL audit_log_include_commands= 'set_option,create_db';

If you now create a database:

.. code-block:: guess

   mysql> CREATE DATABASE world;

You'll see it the audit log:

.. code-block:: none

   <AUDIT_RECORD
     NAME="Query"
     RECORD="10724_2016-08-18T12:34:22"
     TIMESTAMP="2016-08-18T15:10:47 UTC"
     COMMAND_CLASS="create_db"
     CONNECTION_ID="61"
     STATUS="0"
     SQLTEXT="create database world"
     USER="root[root] @ localhost []"
     HOST="localhost"
     OS_USER=""
     IP=""
     DB=""
   />

To switch command type filtering type from included type list to excluded one
or back, first reset the currently-active list to ``NULL``:

.. code-block:: guess

   mysql> SET GLOBAL audit_log_include_commands = NULL;
   Query OK, 0 rows affected (0.00 sec)

   mysql> SET GLOBAL audit_log_exclude_commands= 'set_option,create_db';
   Query OK, 0 rows affected (0.00 sec)

.. note::

  Invocation of stored procedures have command type ``call_procedure``, and all
  the statements executed within the procedure have the same type
  ``call_procedure`` as well.

.. _filtering_by_database:

Filtering by database
=====================

The filtering by an SQL database is implemented via two global variables:
:variable:`audit_log_include_databases` and
:variable:`audit_log_exclude_databases` to specify which databases should be
included or excluded from audit logging.

.. warning:: 

   Only one of these variables can contain a list of databases to be either
   included or excluded, while the other needs to be ``NULL``. If one of the
   variables is set to be not ``NULL`` (contains a list of databases), the
   attempt to set another one will fail. Empty string means an empty list.


If query is accessing any of databases listed in
:variable:`audit_log_include_databases`, the query will be logged.
If query is accessing only databases listed in
:variable:`audit_log_exclude_databases`, the query will not be logged.
``CREATE TABLE`` statements are logged unconditionally.

.. note:: 

   Changes of :variable:`audit_log_include_databases` and
   :variable:`audit_log_exclude_databases` do not apply to existing server
   connections.

Example
-------

To add databases to be monitored you should run:

.. code-block:: guess

   mysql> SET GLOBAL audit_log_include_databases = 'test,mysql,db1';
   Query OK, 0 rows affected (0.00 sec)

   mysql> SET GLOBAL audit_log_include_databases= 'db1,```db3"`';
   Query OK, 0 rows affected (0.00 sec)

If you you try to add databases to both include and exclude lists server will
show you the following error:

.. code-block:: guess

   mysql> SET GLOBAL audit_log_exclude_databases = 'test,mysql,db1';
   ERROR 1231 (42000): Variable 'audit_log_exclude_databases can't be set to the value of 'test,mysql,db1'

To switch from filtering by included database list to the excluded one or back,
first set the currently active filtering variable to ``NULL``:

.. code-block:: guess

   mysql> SET GLOBAL audit_log_include_databases = NULL;
   Query OK, 0 rows affected (0.00 sec)

   mysql> SET GLOBAL audit_log_exclude_databases = 'test,mysql,db1';
   Query OK, 0 rows affected (0.00 sec)

System Variables
================

.. variable:: audit_log_strategy

   :cli: Yes
   :scope: Global
   :dyn: No
   :vartype: String
   :default: ASYNCHRONOUS
   :allowed values: ``ASYNCHRONOUS``, ``PERFORMANCE``, ``SEMISYNCHRONOUS``, ``SYNCHRONOUS``

This variable is used to specify the audit log strategy, possible values are:

* ``ASYNCHRONOUS`` - (default) log using memory buffer, do not drop messages if buffer is full
* ``PERFORMANCE`` - log using memory buffer, drop messages if buffer is full
* ``SEMISYNCHRONOUS`` - log directly to file, do not flush and sync every event
* ``SYNCHRONOUS`` - log directly to file, flush and sync every event

This variable has effect only when :variable:`audit_log_handler` is set to ``FILE``.

.. variable:: audit_log_file

   :cli: Yes
   :scope: Global
   :dyn: No
   :vartype: String
   :default: audit.log

This variable is used to specify the filename that's going to store the audit log. It can contain the path relative to the datadir or absolute path.

.. variable:: audit_log_flush

   :cli: Yes
   :scope: Global
   :dyn: Yes
   :vartype: String
   :default: OFF

When this variable is set to ``ON`` log file will be closed and reopened. This can be used for manual log rotation.

.. variable:: audit_log_buffer_size

     :cli: Yes
     :scope: Global
     :dyn: No
     :vartype: Numeric
     :default: 1 Mb

This variable can be used to specify the size of memory buffer used for logging, used when :variable:`audit_log_strategy` variable is set to ``ASYNCHRONOUS`` or ``PERFORMANCE`` values. This variable has effect only when :variable:`audit_log_handler` is set to ``FILE``.

.. variable:: audit_log_exclude_accounts

   :cli: Yes
   :scope: Global
   :dyn: Yes
   :vartype: String

This variable is used to specify the list of users for which
:ref:`filtering_by_user` is applied. The value can be ``NULL`` or comma
separated list of accounts in form ``user@host`` or ``'user'@'host'`` (if user
or host contains comma). If this variable is set, then
:variable:`audit_log_include_accounts` must be unset, and vice versa.

.. variable:: audit_log_exclude_commands

   :cli: Yes
   :scope: Global
   :dyn: Yes
   :vartype: String

This variable is used to specify the list of commands for which
:ref:`filtering_by_sql_command_type` is applied. The value can be ``NULL`` or
comma separated list of commands. If this variable is set, then
:variable:`audit_log_include_commands` must be unset, and vice versa.

.. variable:: audit_log_exclude_databases

   :cli: Yes
   :scope: Global
   :dyn: Yes
   :vartype: String

This variable is used to specify the list of commands for which
:ref:`filtering_by_database` is applied. The value can be ``NULL`` or
comma separated list of commands. If this variable is set, then
:variable:`audit_log_include_databases` must be unset, and vice versa.


.. variable:: audit_log_format

   :cli: Yes
   :scope: Global
   :dyn: No 
   :vartype: String
   :default: OLD
   :allowed values: ``OLD``, ``NEW``, ``CSV``, ``JSON``

This variable is used to specify the audit log format. The audit log plugin
supports four log formats: ``OLD``, ``NEW``, ``JSON``, and ``CSV``. ``OLD`` and
``NEW`` formats are based on XML, where the former outputs log record properties
as XML attributes and the latter as XML tags. Information logged is the same in
all four formats.

.. variable:: audit_log_include_accounts

    :cli: Yes
    :scope: Global
    :dyn: Yes
    :vartype: String

This variable is used to specify the list of users for which
:ref:`filtering_by_user` is applied. The value can be ``NULL`` or comma
separated list of accounts in form ``user@host`` or ``'user'@'host'`` (if user
or host contains comma). If this variable is set, then
:variable:`audit_log_exclude_accounts` must be unset, and vice versa.

.. variable:: audit_log_include_commands

    :cli: Yes
    :scope: Global
    :dyn: Yes
    :vartype: String

This variable is used to specify the list of commands for which
:ref:`filtering_by_sql_command_type` is applied. The value can be ``NULL`` or
comma separated list of commands. If this variable is set, then
:variable:`audit_log_exclude_commands` must be unset, and vice versa.

.. variable:: audit_log_include_databases

    :cli: Yes
    :scope: Global
    :dyn: Yes
    :vartype: String

This variable is used to specify the list of commands for which
:ref:`filtering_by_database` is applied. The value can be ``NULL`` or
comma separated list of commands. If this variable is set, then
:variable:`audit_log_exclude_databases` must be unset, and vice versa.

.. variable:: audit_log_policy

    :cli: Yes
    :scope: Global
    :dyn: Yes 
    :vartype: String
    :default: ALL
    :allowed values: ``ALL``, ``LOGINS``, ``QUERIES``, ``NONE``

This variable is used to specify which events should be logged. Possible values
are:

* ``ALL`` - all events will be logged
* ``LOGINS`` - only logins will be logged
* ``QUERIES`` - only queries will be logged
* ``NONE`` - no events will be logged

.. variable:: audit_log_rotate_on_size

    :cli: Yes
    :scope: Global
    :dyn: No 
    :vartype: Numeric
    :default: 0 (don't rotate the log file)

This variable is used to specify the maximum audit log file size. Upon reaching
this size the log will be rotated. The rotated log files will be present in the
same same directory as the current log file. A sequence number will be appended
to the log file name upon rotation. This variable has effect only when
:variable:`audit_log_handler` is set to ``FILE``.
 
.. variable:: audit_log_rotations

    :cli: Yes
    :scope: Global
    :dyn: No 
    :vartype: Numeric
    :default: 0 

This variable is used to specify how many log files should be kept when
:variable:`audit_log_rotate_on_size` variable is set to non-zero value. This
variable has effect only when :variable:`audit_log_handler` is set to ``FILE``.

.. variable:: audit_log_handler

    :cli: Yes
    :scope: Global
    :dyn: No 
    :vartype: String
    :default: FILE
    :allowed values: ``FILE``, ``SYSLOG``

This variable is used to configure where the audit log will be written. If it is
set to ``FILE``, the log will be written into a file specified by
:variable:`audit_log_file` variable. If it is set to ``SYSLOG``, the audit log
will be written to syslog.

.. variable:: audit_log_syslog_ident

    :cli: Yes
    :scope: Global
    :dyn: No 
    :vartype: String
    :default: percona-audit

This variable is used to specify the ``ident`` value for syslog. This variable
has the same meaning as the appropriate parameter described in the `syslog(3)
manual <http://linux.die.net/man/3/syslog>`_.

.. variable:: audit_log_syslog_facility
   
    :cli: Yes
    :scope: Global
    :dyn: No 
    :vartype: String
    :default: LOG_USER

This variable is used to specify the ``facility`` value for syslog. This
variable has the same meaning as the appropriate parameter described in the
`syslog(3) manual <http://linux.die.net/man/3/syslog>`_.

.. variable:: audit_log_syslog_priority

    :cli: Yes
    :scope: Global
    :dyn: No 
    :vartype: String
    :default: LOG_INFO

This variable is used to specify the ``priority`` value for syslog. This
variable has the same meaning as the appropriate parameter described in the
`syslog(3) manual <http://linux.die.net/man/3/syslog>`_.

Status Variables
================

.. variable:: Audit_log_buffer_size_overflow

    :vartype: Numeric
    :scope: Global

The number of times an audit log entry was either
dropped or written directly to the file due to its size being bigger
than :variable:`audit_log_buffer_size` variable.

Version Specific Information
============================

  * :rn:`8.0.12-1`
    Feature ported from |Percona Server| 5.7

  * :rn:`8.0.15-6`
    :variable:`Audit_log_buffer_size_overflow` variable implemented

