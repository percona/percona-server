..  _encrypting-tablespaces:

======================================================
Encrypting a Schema or a General Tablespace
======================================================

*Percona Server for MySQL* uses the same encryption architecture as *MySQL*, a two-tier
system consisting of a master key and tablespace keys. The master key can be
changed, or rotated in the keyring, as needed. Each tablespace key, when
decrypted, remains the same.

The feature requires the keyring plugin.

Setting the Default for Schemas and General Tablespace Encryption
==================================================================

The tables in a general tablespace are either all encrypted or all unencrypted.
A tablespace cannot contain a mixture of encrypted tables and unencrypted
tables.

In versions before *Percona Server for MySQL* 8.0.16-7, use the variable
:ref:`innodb_encrypt_tables`.

.. _innodb_encrypt_tables:

.. rubric:: ``innodb_encrypt_tables``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb-encrypt-tables``
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Text
   * - Default
     - ``OFF``

The variable was removed in :ref:`8.0.16-7`.

The variable is considered **deprecated** and was removed in version 8.0.16-7.
The default setting is "OFF".

The encryption of a schema or a general tablespace is determined by the
:ref:`default_table_encryption` variable unless you specify the
ENCRYPTION clause in the CREATE SCHEMA or CREATE TABLESPACE statement. This
variable is implemented in *Percona Server for MySQL* version 8.0.16-7.

You can set the :ref:`default_table_encryption` variable in an individual
connection.

.. code-block:: mysql

    mysql> SET default_table_encryption=ON;

System Variable
----------------

.. _default_table_encryption:

.. rubric:: ``default_table_encryption``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``default-table-encryption``
   * - Scope
     - Session
   * - Dynamic
     - Yes
   * - Data type
     - Text
   * - Default
     - OFF

Defines the default encryption setting for schemas and general tablespaces. The
variable allows you to create or alter schemas or tablespaces without specifying
the ENCRYPTION clause. The default encryption setting applies only to schemas
and general tablespaces and is not applied to the MySQL system tablespace.

The variable has the following possible values:

.. tabularcolumns:: |p{5cm}|p{11cm}|

.. list-table::
   :header-rows: 1

   * - Value 
     - Description
   * - ON
     - New tables are encrypted. Add ``ENCRYPTION="N"`` to the ``CREATE TABLE`` or ``ALTER TABLE`` statement to create unencrypted tables.
   * - OFF
     - By default, new tables are unencrypted. Add ``ENCRYPTION="Y"`` to the ``CREATE TABLE`` or ``ALTER TABLE`` statement to create encrypted tables. 
   * - ONLINE_TO_KEYRING
     - :Availability: This value is **Experimental** quality.
       
       Converts a tablespace encrypted by a Master Key to use Advanced Encryption Key Rotation. You can only apply the keyring encryption when creating tables or altering tables.
   * - ONLINE_FROM_KEYRING_TO_UNENCRYPTED
     - :Availability: This value is **Experimental** quality
       
       Converts a tablespace encrypted by Advanced Encryption Key Rotation to unencrypted.

.. note::

    The `ALTER TABLE` statement changes the current encryption mode only if you
    use the `ENCRYPTION` clause.

.. seealso::

      MySQL Documentation: default_table_encryption
      https://dev.mysql.com/doc/refman/8.0/en/server-system-variables.html

.. _merge-sort-encryption:

.. rubric:: Merge-sort-encryption

.. _innodb_encrypt_online_alter_logs:

.. rubric:: ``innodb_encrypt_online_alter_logs``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb_encrypt-online-alter-logs``
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Boolean
   * - Default
     - OFF

This variable simultaneously turns on the encryption of files used by InnoDB for
full text search using parallel sorting, building indexes using merge sort, and
online DDL logs created by InnoDB for online DDL. Encryption is available for
file merges used in queries and backend processes.

Setting Tablespace `ENCRYPTION` without the Default Setting
----------------------------------------------------------------

If you do not set the default encryption setting, you can create general
tablespaces with the ``ENCRYPTION`` setting.

.. code-block:: mysql

    mysql> CREATE TABLESPACE tablespace_name ENCRYPTION='Y';

All tables contained in the tablespace are either encrypted or not encrypted.
You cannot encrypted only some of the tables in a general tablespace. This
feature extends the  `CREATE TABLESPACE
<https://dev.mysql.com/doc/refman/8.0/en/create-tablespace.html>`_ statement to
accept the ``ENCRYPTION='Y/N'`` option.

.. note::

   Prior to *Percona Server for MySQL* 8.0.13, the ``ENCRYPTION`` option was specific to
   the ``CREATE TABLE`` or ``SHOW CREATE TABLE`` statement. As of *Percona Server for MySQL*
   8.0.13, this option is a tablespace attribute and  no longer  allowed with the
   ``CREATE TABLE`` or ``SHOW CREATE TABLE`` statement except for file-per-table
   tablespaces.

In an encrypted general tablespace, an attempt to create an unencrypted table
generates the following error:

.. code-block:: mysql

    mysql> CREATE TABLE t3 (a INT, b TEXT) TABLESPACE foo ENCRYPTION='N';
    ERROR 1478 (HY0000): InnoDB: Tablespace 'foo' can contain only ENCRYPTED tables.

An attempt to create or to move any tables, including partitioned ones, to a
general tablespace with an incompatible encryption setting are diagnosed and
the process is aborted.

If you must move tables between incompatible tablespaces, create tables with the
same structure in another tablespace and run ``INSERT INTO SELECT`` from each of
the source tables into the destination tables.

Exporting an Encrypted General Tablespace
--------------------------------------------

You can only export encrypted file-per-table tablespaces

.. seealso::

    :ref:`encrypting-tables`

    :ref:`encrypting-system-tablespace`

    :ref:`encrypting-temporary-files`

    :ref:`verifying-encryption`
