.. _psaas_utility_user:

==============
 Utility user
==============

|Percona Server| has implemented ability to have a |MySQL| user who has system access to do administrative tasks but limited access to user schema. This feature is especially useful to those operating |MySQL| As A Service. 

This user has a mixed and special scope of abilities and protection:

  * Utility user will not appear in the mysql.user table and can not be modified by any other user, including root.

  * Utility user will not appear in :table:`USER_STATISTICS`, :table:`CLIENT_STATISTICS` or :table:`THREAD_STATISTICS` tables.

  * Utility user's queries may appear in the general and slow logs.

  * Utility user doesn't have the ability create, modify, delete or see any schemas or data not specified (except for information_schema).

  * Utility user may modify all visible, non read-only system variables (see :ref:`expanded_option_modifiers` functionality).

  * Utility user may see, create, modify and delete other system users only if given access to the mysql schema.

  * Regular users may be granted proxy rights to the utility user but any attempt to impersonate the utility user will fail. The utility user may not be granted proxy rights on any regular user. For example running: GRANT PROXY ON utility_user TO regular_user; will not fail, but any actual attempt to impersonate as the utility user will fail. Running: `GRANT PROXY ON regular_user TO utility_user;` will fail when utility_user is an exact match or is more specific than than the utility user specified.

When the server starts, it will note in the log output that the utility user exists and the schemas that it has access to.

In order to have the ability for a special type of MySQL user, which will have a very limited and special amount of control over the system and can not be see or modified by any other user including the root user, three new options have been added.

Option :variable:`utility_user` specifies the user which the system will create and recognize as the utility user. The host in the utility user specification follows conventions described in the `MySQL manual <http://dev.mysql.com/doc/refman/5.5/en/connection-access.html>`_, i.e. it allows wildcards and IP masks. Anonymous user names are not permitted to be used for the utility user name.

This user must not be an exact match to any other user that exists in the mysql.user table. If the server detects that the user specified with this option exactly matches any user within the mysql.user table on start up, the server will report an error and shut down gracefully. If host name wildcards are used and a more specific user specification is identified on start up, the server will report a warning and continue. 

 Example: :option:`--utility_user` =frank@% and frank@localhost exists within the mysql.user table.

If a client attempts to create a MySQL user that matches this user specification exactly or if host name wildcards are used for the utility user and the user being created has the same name and a more specific host, the creation attempt will fail with an error.

 Example: :option:`--utility_user` =frank@% and CREATE USER 'frank@localhost';

As a result of these requirements, it is strongly recommended that a very unique user name and reasonably specific host be used and that any script or tools test that they are running within the correct user by executing 'SELECT CURRENT_USER()' and comparing the result against the known utility user.

Option :variable:`utility_user_password` specifies the password for the utility user and MUST be specified or the server will shut down gracefully with an error.

 Example: :option:`--utility_user_password` =`Passw0rD`;

Option :variable:`utility_user_schema_access` specifies the name(s) of the schema(s) that the utility user will have access to read write and modify. If a particular schema named here does not exist on start up it will be ignored. If a schema by the name of any of those listed in this option is created after the server is started, the utility user will have full access to it.

 Example: :option:`--utility_user_schema_access` =schema1,schema2,schema3 ;

Option :variable:`utility_user_privileges` allows a comma-separated list of extra access privileges to grant to the utility user.

 Example: :option:`--utility-user-privileges` ="CREATE, DROP, LOCK TABLES"

System Variables
================

.. variable:: utility_user

     :version 5.5.27-28.0: Implemented
     :cli: Yes
     :conf: utility_user=<user@host>
     :scope: Global
     :dyn: No
     :vartype: String
     :default: NULL

Specifies a MySQL user that will be added to the internal list of users and recognized as the utility user.

.. variable:: utility_user_password

     :version 5.5.27-28.0: Implemented
     :cli: Yes
     :conf: utility_user_password=<password>
     :scope: Global
     :dyn: No
     :vartype: String
     :default: NULL

Specifies the password required for the utility user.

.. variable:: utility_user_schema_access

     :version 5.5.27-28.0: Implemented
     :cli: Yes
     :conf: utility_user_schema_access=<schema>,<schema>,<schema>
     :scope: Global
     :dyn: No
     :vartype: String
     :default: NULL

Specifies the schemas that the utility user has access to in a comma delimited list.

.. variable:: utility_user_privileges

   :version 5.5.34-32.0: Implemented
   :cli: Yes
   :conf: utility_user_privileges=<privilege1>,<privilege2>,<privilege3>
   :scope: Global
   :dyn: No
   :vartype: String
   :default: NULL

This variable can be used to specify a comma-separated list of extra access privileges to grant to the utility user. Supported values for the privileges list are: ``SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, RELOAD, SHUTDOWN, PROCESS,FILE, GRANT, REFERENCES, INDEX, ALTER, SHOW DATABASES, SUPER, CREATE TEMPORARY TABLES, LOCK TABLES,EXECUTE, REPLICATION SLAVE, REPLICATION CLIENT, CREATE VIEW, SHOW VIEW, CREATE ROUTINE, ALTER ROUTINE, CREATE USER, EVENT, TRIGGER, CREATE TABLESPACE``
