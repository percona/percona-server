.. _extended_show_grants:

================================================================================
Extended ``SHOW GRANTS``
================================================================================

In Oracle *MySQL* ``SHOW GRANTS`` displays only the privileges granted
explicitly to the named account. Other privileges might be available
to the account, but they are not displayed. For example, if an
anonymous account exists, the named account might be able to use its
privileges, but ``SHOW GRANTS`` will not display them. |Percona
Server| offers the ``SHOW EFFECTIVE GRANTS`` command to display
all the effectively available privileges to the account, including
those granted to a different account.

Example
================================================================================
If we create the following users:

.. code-block:: mysql

   mysql> CREATE USER grantee@localhost IDENTIFIED BY 'grantee1';
   Query OK, 0 rows affected (0.50 sec)

   mysql> CREATE USER grantee IDENTIFIED BY 'grantee2';
   Query OK, 0 rows affected (0.09 sec)

   mysql> CREATE DATABASE db2;
   Query OK, 1 row affected (0.20 sec)

   mysql> GRANT ALL PRIVILEGES ON db2.* TO grantee WITH GRANT OPTION;
   Query OK, 0 rows affected (0.12 sec)


* ``SHOW EFFECTIVE GRANTS`` output before the change: 

.. code-block:: mysql

   mysql> SHOW EFFECTIVE GRANTS;
   +----------------------------------------------------------------------------------------------------------------+
   | Grants for grantee@localhost                                                                                   |
   +----------------------------------------------------------------------------------------------------------------+
   | GRANT USAGE ON *.* TO 'grantee'@'localhost' IDENTIFIED BY PASSWORD '*9823FF338D44DAF02422CF24DD1F879FB4F6B232' |
   +----------------------------------------------------------------------------------------------------------------+
   1 row in set (0.04 sec)
   
Although the grant for the ``db2`` database isn't shown, ``grantee`` user has enough privileges to create the table in that database:

.. code-block:: bash

   user@trusty:~$ mysql -ugrantee -pgrantee1 -h localhost

.. code-block:: mysql

   mysql> CREATE TABLE db2.t1(a int);
   Query OK, 0 rows affected (1.21 sec)

* The output of ``SHOW EFFECTIVE GRANTS`` after the change shows all
  the privileges for the ``grantee`` user:

.. code-block:: mysql

   mysql> SHOW EFFECTIVE GRANTS;
   +----------------------------------------------------------------------------------------------------------------+
   | Grants for grantee@localhost                                                                                   |
   +----------------------------------------------------------------------------------------------------------------+
   | GRANT USAGE ON *.* TO 'grantee'@'localhost' IDENTIFIED BY PASSWORD '*9823FF338D44DAF02422CF24DD1F879FB4F6B232' |
   | GRANT ALL PRIVILEGES ON `db2`.* TO 'grantee'@'%' WITH GRANT OPTION                                             |
   +----------------------------------------------------------------------------------------------------------------+
   2 rows in set (0.00 sec)
  
Version-Specific Information
================================================================================

  * :ref:`8.0.12-1`: The feature was ported from *Percona Server for MySQL* 5.7.

Other reading
================================================================================

* :mysqlbug:`53645` - ``SHOW GRANTS`` not displaying all the applicable grants
