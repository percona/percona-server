.. _encrypting-system-tablespace:

==================================================================
Encrypting the System Tablespace
==================================================================

*Percona Server for MySQL* supports system tablespace encryption. The InnoDB system
tablespace may be encrypted with the master key encryption or the keyring
encryption with advanced encryption key rotation. 

Keyring encryption is a **tech preview** feature.

.. seealso::

    :ref:`encrypting-threads`.  

The limitation is the following:

* You cannot convert the system tablespace from the encrypted state to the
  unencrypted state, or the unencrypted state to the encrypted state. If a 
  conversion is needed, create a new instance with the
  system tablespace in the required state and transfer the user tables to that instance.

.. important::

    A server instance initialized with the encrypted InnoDB system tablespace
    cannot be downgraded. It is not possible to parse encrypted InnoDB system
    tablespace pages in a version of *Percona Server for MySQL* lower than the version
    where the InnoDB system tablespace has been encrypted.

To enable system tablespace encryption, edit the my.cnf file with the following:

* Add the :ref:`innodb_sys_tablespace_encrypt`
* Edit the `innodb_sys_tablespace_encrypt` value to "ON"

System tablespace encryption can only be enabled with the ``--initialize``
option

You can create an encrypted table as follows:

.. code-block:: MySQL

    mysql> CREATE TABLE table_name TABLESPACE=innodb_system ENCRYPTION='Y';


System Variables
------------------------------------------------------------------

.. _innodb_sys_tablespace_encrypt:

.. rubric:: ``innodb_sys_tablespace_encrypt``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb-sys-tablespace-encrypt``
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

Enables the encryption of the InnoDB system tablespace. 

.. seealso::

   *MySQL* Documentation: mysql system Tablespace Encryption
   https://dev.mysql.com/doc/refman/8.0/en/innodb-data-encryption.html#innodb-mysql-tablespace-encryption-enabling-disabling

   *MySQL* Documentation: ``--initialize`` option
      https://dev.mysql.com/doc/refman/8.0/en/server-options.html#option_mysqld_initialize

Re-Encrypt the System Tablespace
----------------------------------

You can re-encrypt the system tablespace key with master key rotation. When
the master key is rotated, the tablespace key is decrypted and re-encrypt
with the new master key. Only the first page of the tablespace (.ibd) file is
read and written during the key rotation. The tables in the tablespace are not
re-encrypted.

The command is as follows:

.. code-block:: MySQL

  mysql> ALTER INSTANCE ROTATE INNODB MASTER KEY;

.. seealso::

    :ref:`rotating-master-key`

    :ref:`using-keyring-plugin`
