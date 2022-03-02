.. _audit_log_plugin:

================================================================================
 Audit Log Plugin
================================================================================

Percona Audit Log Plugin provides monitoring and logging of connection and query
activity that were performed on specific server. Information about the activity
is stored in a log file. This
implementation is alternative to the `MySQL Enterprise Audit Log Plugin
<https://dev.mysql.com/doc/refman/8.0/en/audit-log.html>`_

Audit logging documents the database usage. You can use the log for troubleshooting. 

* **Audit** - Audit event indicates that audit logging started or finished. ``NAME`` field will be ``Audit`` when logging started and ``NoAudit`` when logging finished. Audit record also includes server version and command-line arguments.

Example of the Audit event: :: 

 <AUDIT_RECORD
  "NAME"="Audit"
  "RECORD"="1_2014-04-29T09:29:40"
  "TIMESTAMP"="2014-04-29T09:29:40 UTC"
  "MYSQL_VERSION"="5.6.17-65.0-655.trusty"
  "STARTUP_OPTIONS"="--basedir=/usr --datadir=/var/lib/mysql --plugin-dir=/usr/lib/mysql/plugin --user=mysql --log-error=/var/log/mysql/error.log --pid-file=/var/run/mysqld/mysqld.pid --socket=/var/run/mysqld/mysqld.sock --port=3306"
  "OS_VERSION"="x86_64-debian-linux-gnu",
  />

* **Connect**/**Disconnect** - Connect record event will have ``NAME`` field ``Connect`` when user logged in or login failed, or ``Quit`` when connection is closed. Additional fields for this event are ``CONNECTION_ID``, ``STATUS``, ``USER``, ``PRIV_USER``, ``OS_LOGIN``, ``PROXY_USER``, ``HOST``, and ``IP``. ``STATUS`` will be  ``0`` for successful logins and non-zero for failed logins.

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

* **Query** - Additional fields for this event are: ``COMMAND_CLASS`` (values come from the ``com_status_vars`` array in the :file:`sql/mysqld.cc`` file in a MySQL source distribution. Examples are ``select``, ``alter_table``, ``create_table``, etc.), ``CONNECTION_ID``, ``STATUS`` (indicates error when non-zero), ``SQLTEXT`` (text of SQL-statement), ``USER``, ``HOST``, ``OS_USER``, ``IP``. Possible values for the ``NAME`` name field for this event are ``Query``, ``Prepare``, ``Execute``, ``Change user``, etc..

.. note::
    The ``statement/sql/%``  populates the audit log command_class field. For example, the ``SELECT name FROM performance_schema.setup_instruments WHERE name LIKE "statement/sql/%"`` query.
    
    The %statement/com%`` entry populates the audit log command_class field as lowercase text. For example, the ``SELECT name FROM performance_schema.setup_instruments WHERE name LIKE '%statement/com%'`` query. If you run a 'ping' command, then the command_class field is 'ping', and for 'Init DB', the command_class field is 'init db'.

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

The audit Log plugin is installed, but, by default, is not enabled when you install |Percona Server|. To check if the plugin is enabled run the following commands:

.. sourcecode:: mysql

    mysql> SELECT * FROM information_schema.PLUGINS WHERE PLUGIN_NAME LIKE '%audit%';
    Empty set (0.00 sec)

    mysql> SHOW variables LIKE 'audit%';
    Empty set (0.01 sec)

    mysql> SHOW variables LIKE 'plugin%';
    +---------------+------------------------+
    | Variable_name | Value                  |
    +---------------+------------------------+
    | plugin_dir    | /usr/lib/mysql/plugin/ |
    +---------------+------------------------+
    1 row in set (0.00 sec)

.. note::

    The location of the MySQL plugin directory depends on the operating system and may be different on your system. 

The following command enables the plugin: 

.. code-block:: mysql

   mysql> INSTALL PLUGIN audit_log SONAME 'audit_log.so';

Run the following command to verify if the plugin was installed correctly:

.. sourcecode:: mysql

    mysql> SELECT * FROM information_schema.PLUGINS WHERE PLUGIN_NAME LIKE '%audit%'\G
    *************************** 1. row ***************************
              PLUGIN_NAME: audit_log
            PLUGIN_VERSION: 0.2
            PLUGIN_STATUS: ACTIVE
              PLUGIN_TYPE: AUDIT
      PLUGIN_TYPE_VERSION: 4.1
            PLUGIN_LIBRARY: audit_log.so
    PLUGIN_LIBRARY_VERSION: 1.7
            PLUGIN_AUTHOR: Percona LLC and/or its affiliates.
        PLUGIN_DESCRIPTION: Audit log
            PLUGIN_LICENSE: GPL
              LOAD_OPTION: ON
    1 row in set (0.00 sec)

You can review the audit log variables with the following command:

.. sourcecode:: mysql

    mysql> SHOW variables LIKE 'audit%';
    +-----------------------------+---------------+
    | Variable_name               | Value         |
    +-----------------------------+---------------+
    | audit_log_buffer_size       | 1048576       |
    | audit_log_exclude_accounts  |               |
    | audit_log_exclude_commands  |               |
    | audit_log_exclude_databases |               |
    | audit_log_file              | audit.log     |
    | audit_log_flush             | OFF           |
    | audit_log_format            | OLD           |
    | audit_log_handler           | FILE          |
    | audit_log_include_accounts  |               |
    | audit_log_include_commands  |               |
    | audit_log_include_databases |               |
    | audit_log_policy            | ALL           |
    | audit_log_rotate_on_size    | 0             |
    | audit_log_rotations         | 0             |
    | audit_log_strategy          | ASYNCHRONOUS  |
    | audit_log_syslog_facility   | LOG_USER      |
    | audit_log_syslog_ident      | percona-audit |
    | audit_log_syslog_priority   | LOG_INFO      |
    +-----------------------------+---------------+
    18 rows in set (0.00 sec)


The audit Log plugin generates a log of following events:

* **Audit** - Audit event indicates that audit logging started or finished. ``NAME`` field will be ``Audit`` when logging started and ``NoAudit`` when logging finished. Audit record also includes server version and command-line arguments.

   An example of an Audit event: 

   .. sourcecode:: xml

      <AUDIT_RECORD
         NAME="Audit"
         RECORD="1_2021-06-30T11:56:53"
         TIMESTAMP="2021-06-30T11:56:53 UTC"
         MYSQL_VERSION="5.7.34-37"
         STARTUP_OPTIONS="--daemonize --pid-file=/var/run/mysqld/mysqld.pid"
         OS_VERSION="x86_64-debian-linux-gnu"
      />

* **Connect**/**Disconnect** - Connect record event will have ``NAME`` field ``Connect`` when user logged in or login failed, or ``Quit`` when connection is closed. 
   The additional fields for this event are the following:

   * ``CONNECTION_ID``

   * ``STATUS``

   * ``USER``

   * ``PRIV_USER``

   * ``OS_LOGIN``

   * ``PROXY_USER``

   * ``HOST``

   * ``IP``

   The value for ``STATUS`` is ``0`` for successful logins and non-zero for failed logins.

   An example of a Disconnect event: 

   .. sourcecode:: xml

      <AUDIT_RECORD
         NAME="Quit"
         RECORD="5_2021-06-29T19:33:03"
         TIMESTAMP="2021-06-29T19:34:38Z"
         CONNECTION_ID="14"
         STATUS="0"
         USER="root"
         PRIV_USER="root"
         OS_LOGIN=""
         PROXY_USER=""
         HOST="localhost"
         IP=""
         DB=""
      />

* **Query** - Additional fields for this event are: ``COMMAND_CLASS`` (values come from the ``com_status_vars`` array in the :file:`sql/mysqld.cc`` file in a MySQL source distribution. 

   Examples are ``select``, ``alter_table``, ``create_table``, etc.), ``CONNECTION_ID``, ``STATUS`` (indicates an error when the vaule is non-zero), ``SQLTEXT`` (text of SQL-statement), ``USER``, ``HOST``, ``OS_USER``, ``IP``. 
   
   The possible values for the ``NAME`` name field for this event are ``Query``, ``Prepare``, ``Execute``, ``Change user``, etc.

   An example of the Query event: 

   .. sourcecode:: xml

      <AUDIT_RECORD
         NAME="Query"
         RECORD="4_2021-06-29T19:33:03"
         TIMESTAMP="2021-06-29T19:33:34Z"
         COMMAND_CLASS="show_variables"
         CONNECTION_ID="14"
         STATUS="0"
         SQLTEXT="show variables like 'audit%'"
         USER="root[root] @ localhost []"
         HOST="localhost"
         OS_USER=""
         IP=""
         DB=""
      />



Log Format
==========

The plugin supports the following log formats: ``OLD``, ``NEW``, ``JSON``, and ``CSV``. The ``OLD``format and the``NEW`` format are based on XML. The ``OLD`` format defines each log record with XML attributes. The ``NEW`` format defines each log record with XML tags. The information logged is the same for all four formats. The :variable:`audit_log_format` variable controls the log format choice.

An example of the ``OLD`` format: 

.. sourcecode:: xml

  <AUDIT_RECORD
    NAME="Query"
    RECORD="3_2021-06-30T11:56:53"
    TIMESTAMP="2021-06-30T11:57:14 UTC"
    COMMAND_CLASS="select"
    CONNECTION_ID="3"
    STATUS="0"
    SQLTEXT="select * from information_schema.PLUGINS where PLUGIN_NAME like '%audit%'"
    USER="root[root] @ localhost []"
    HOST="localhost"
    OS_USER=""
    IP=""
    DB=""
  />

An example of the ``NEW`` format: 

.. sourcecode:: xml

  <AUDIT_RECORD>
    <NAME>Query</NAME>
    <RECORD>16684_2021-06-30T16:07:41</RECORD>
    <TIMESTAMP>2021-06-30T16:08:06 UTC</TIMESTAMP>
    <COMMAND_CLASS>select</COMMAND_CLASS>
    <CONNECTION_ID>2</CONNECTION_ID>
    <STATUS>0</STATUS>
    <SQLTEXT>select id, holder from one</SQLTEXT>
    <USER>root[root] @ localhost []</USER>
    <HOST>localhost</HOST>
    <OS_USER></OS_USER>
    <IP></IP>
    <DB></DB>

An example of the ``JSON`` format: 

.. sourcecode:: json

  {"audit_record":{"name":"Query","record":"13149_2021-06-30T15:03:11","timestamp":"2021-06-30T15:07:58 UTC","command_class":"show_databases","connection_id":"2","status":0,"sqltext":"show databases","user":"root[root] @ localhost []","host":"localhost","os_user":"","ip":"","db":""}}

An example of the ``CSV`` format: 

.. sourcecode:: text

  "Query","22567_2021-06-30T16:10:09","2021-06-30T16:19:00 UTC","select","2",0,"select count(*) from one","root[root] @ localhost []","localhost","","",""

.. _streaming_to_syslog:

Streaming the audit log to syslog
=================================

To stream the audit log to syslog you'll need to set :variable:`audit_log_handler` variable to ``SYSLOG``. To control the syslog file handler, the following variables can be used: :variable:`audit_log_syslog_ident`, :variable:`audit_log_syslog_facility`, and :variable:`audit_log_syslog_priority` These variables have the same meaning as appropriate parameters described in the `syslog(3) manual <http://linux.die.net/man/3/syslog>`_.

.. note::

   The actions for the variables: :variable:`audit_log_strategy`, :variable:`audit_log_buffer_size`, :variable:`audit_log_rotate_on_size`, :variable:`audit_log_rotations` are captured only with ``FILE`` handler. 

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
   to set another one will fail. An empty string means an empty list.

.. note::

   Changes of :variable:`audit_log_include_accounts` and
   :variable:`audit_log_exclude_accounts` do not apply to existing server
   connections.

Example
-------

The following example adds users who will be monitored:

.. code-block:: mysql

   mysql> SET GLOBAL audit_log_include_accounts = 'user1@localhost,root@localhost';
   Query OK, 0 rows affected (0.00 sec)

If you try to add users to both the include list and the exclude list, the server returns the following error:

.. code-block:: mysql

   mysql> SET GLOBAL audit_log_exclude_accounts = 'user1@localhost,root@localhost';
   ERROR 1231 (42000): Variable 'audit_log_exclude_accounts' can't be set to the value of 'user1@localhost,root@localhost'

To switch from filtering by included user list to the excluded user list or back,
first set the currently active filtering variable to ``NULL``:

.. code-block:: mysql

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

To see which user accounts have been added to the exclude list, run the following command:

.. code-block:: mysql

   mysql> SELECT @@audit_log_exclude_accounts;
   +------------------------------+
   | @@audit_log_exclude_accounts |
   +------------------------------+
   | 'user'@'host'                |
   +------------------------------+
   1 row in set (0.00 sec)

Account names from :table:`mysql.user` table are logged in the
audit log. For example when you create a user:

.. code-block:: mysql

   mysql> CREATE USER 'user1'@'%' IDENTIFIED BY '111';
   Query OK, 0 rows affected (0.00 sec)

When ``user1`` connects from ``localhost``, the user is listed:

.. code-block:: none

   <AUDIT_RECORD
    NAME="Connect"
    RECORD="2_2021-06-30T11:56:53"
    TIMESTAMP="2021-06-30T11:56:53 UTC"
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

To exclude ``user1`` from logging in |Percona Server| 8.0, set:

.. code-block:: mysql

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
   the attempt to set another one will fail. An empty string is defined as an empty list.

.. note::

   If both the :variable:`audit_log_exclude_commands` variable and the 
   :variable:`audit_log_include_commands` variable are ``NULL``, all commands are logged.

Example
-------

The available command types can be listed by running:

.. code-block:: mysql

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

You can add commands to the ``include`` filter by running:

.. code-block:: mysql

   mysql> SET GLOBAL audit_log_include_commands= 'set_option,create_db';

Create a database with the following command:

.. code-block:: mysql

  mysql> CREATE DATABASE sample;

The action is captured in the audit log:

.. code-block:: xml

  <AUDIT_RECORD>
    <NAME>Query</NAME>
    <RECORD>24320_2021-06-30T17:44:46</RECORD>
    <TIMESTAMP>2021-06-30T17:45:16 UTC</TIMESTAMP>
    <COMMAND_CLASS>create_db</COMMAND_CLASS>
    <CONNECTION_ID>2</CONNECTION_ID>
    <STATUS>0</STATUS>
    <SQLTEXT>CREATE DATABASE sample</SQLTEXT>
    <USER>root[root] @ localhost []</USER>
    <HOST>localhost</HOST>
    <OS_USER></OS_USER>
    <IP></IP>
    <DB></DB>
  </AUDIT_RECORD>

To switch the command type filtering type from included type list to the excluded list
or back, first reset the currently-active list to ``NULL``:

.. code-block:: mysql

   mysql> SET GLOBAL audit_log_include_commands = NULL;
   Query OK, 0 rows affected (0.00 sec)

   mysql> SET GLOBAL audit_log_exclude_commands= 'set_option,create_db';
   Query OK, 0 rows affected (0.00 sec)

.. note::

  A stored procedure has the ``call_procedure`` command type. All
  the statements executed within the procedure have the same type
  ``call_procedure`` as well.

.. _filtering_by_database:

Filtering by database
=====================

The filtering by an SQL database is implemented by two global variables:
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

.. code-block:: mysql

   mysql> SET GLOBAL audit_log_include_databases = 'test,mysql,db1';
   Query OK, 0 rows affected (0.00 sec)

   mysql> SET GLOBAL audit_log_include_databases= 'db1','db3';
   Query OK, 0 rows affected (0.00 sec)

If you you try to add databases to both include and exclude lists server will
show you the following error:

.. code-block:: mysql

   mysql> SET GLOBAL audit_log_exclude_databases = 'test,mysql,db1';
   ERROR 1231 (42000): Variable 'audit_log_exclude_databases can't be set to the value of 'test,mysql,db1'

To switch from filtering by included database list to the excluded one or back,
first set the currently active filtering variable to ``NULL``:

.. code-block:: mysql

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
   :dyn: No
   :scope: Global
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
