.. _audit_log_plugin:

==================
 Audit Log Plugin
==================

Percona Audit Log Plugin provides monitoring and logging of connection and query activity that were performed on specific server. Information about the activity will be stored in the XML log file where each event will have its ``NAME`` field, it's own unique ``RECORD_ID`` field and a ``TIMESTAMP`` field. This implementation is alternative to the `MySQL Enterprise Audit Log Plugin <dev.mysql.com/doc/refman/5.5/en/audit-log-plugin.html>`_

Audit Log plugin produces the log of following events:

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

* **Query** - Additional fields for this event are: ``COMMAND_CLASS`` (values come from the ``com_status_vars`` array in the :file:`sql/mysqld.cc`` file in a MySQL source distribution. Examples are ``select``, ``alter_table", "create_table", etc.), ``CONNECTION_ID``, ``STATUS`` (indicates error when non-zero), ``SQLTEXT`` (text of SQL-statement, statements are rewritten  to exclude passwords by default, this can be changed by :option:`--log-raw` option ), ``USER``, ``HOST``, ``OS_USER``, ``IP``. Possible values for the ``NAME`` name field for this event are ``Query``, ``Prepare``, ``Execute``, ``Change user``, etc.

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

Audit Log plugin is shipped with |Percona Server|, but it is not installed by default. To enable the plugin you must run the following command: 

.. code-block:: mysql

   INSTALL PLUGIN audit_log SONAME 'audit_log.so';

You can check if the plugin is loaded correctly by running:

.. code-block:: mysql

   SHOW PLUGINS;

Audit log should be listed in the output:
    
.. code-block:: mysql

   +--------------------------------+----------+--------------------+--------------+---------+
   | Name                           | Status   | Type               | Library      | License |
   +--------------------------------+----------+--------------------+--------------+---------+
   ...
   | audit_log                      | ACTIVE   | AUDIT              | audit_log.so | GPL     |
   +--------------------------------+----------+--------------------+--------------+---------+

Log Format
==========

Audit log plugin supports two formats. In one format (``OLD``) log record properties are saved as XML attributes and in the other (``NEW``) log recored properties are saved as XML tags. Audit log format can be set up with the :variable:`audit_log_format` variable.

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

System Variables
================

.. variable:: audit_log_strategy

     :version 5.6.17-65.0: Implemented
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

.. variable:: audit_log_file

     :version 5.6.17-65.0: Implemented
     :cli: Yes
     :scope: Global
     :dyn: No
     :vartype: String
     :default: audit.log

This variable is used to specify the filename that's going to store the audit log. It can contain the path relative to the datadir or absolute path.

.. variable:: audit_log_flush

     :version 5.6.17-65.0: Implemented
     :cli: Yes
     :scope: Global
     :dyn: No
     :vartype: String
     :default: OFF

When this variable is set to ``ON`` log file will be closed and reopened. This can be used for manual log rotation.

.. variable:: audit_log_buffer_size

     :version 5.6.17-65.0: Implemented
     :cli: Yes
     :scope: Global
     :dyn: No
     :vartype: Numeric
     :default: 4096

This variable can be used to specify the size of memory buffer used for logging, used when :variable:`audit_log_strategy` variable is set to ``ASYNCHRONOUS`` or ``PERFORMANCE`` values.

.. variable:: audit_log_format

     :version 5.6.17-65.0: Implemented
     :cli: Yes
     :scope: Global
     :dyn: No 
     :vartype: String
     :default: OLD
     :allowed values: ``OLD``, ``NEW``

This variable is used to specify the audit log format. When this variable is set to ``OLD`` information will be logged as XML attributes, and when is set to ``NEW`` it will be logged as XML tags.

.. variable:: audit_log_policy

     :version 5.6.17-65.0: Implemented
     :cli: Yes
     :scope: Global
     :dyn: Yes 
     :vartype: String
     :default: ALL
     :allowed values: ``ALL``, ``LOGINS``, ``QUERIES``, ``NONE``

This variable is used to specify which events should be logged. Possible values are: 

* ``ALL`` - all events will be logged
* ``LOGINS`` - only logins will be logged
* ``QUERIES`` - only queries will be logged
* ``NONE`` - no events will be logged

.. variable:: audit_log_rotate_on_size

     :version 5.6.17-65.0: Implemented
     :cli: Yes
     :scope: Global
     :dyn: No 
     :vartype: Numeric
     :default: 0 (don't rotate the log file)

This variable is used to specify the size of the audit log file. When this size is reached log will get rotated. Old log can be found in the same directory, audit log sequential number will be appended to the name specified in the :variable:`audit_log_file` variable.
 
.. variable:: audit_log_rotations

     :version 5.6.17-65.0: Implemented
     :cli: Yes
     :scope: Global
     :dyn: No 
     :vartype: Numeric
     :default: 0 

This variable is used to specify how many log files should be kept when :variable:`audit_log_rotate_on_size` variable is set to non-zero value.

Version Specific Information
============================

  * :rn:`5.6.17-65.0`
    Audit Log plugin has been implemented in |Percona Server|.

