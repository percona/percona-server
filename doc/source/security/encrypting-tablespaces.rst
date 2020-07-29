..  _encrypting-tablespaces:

======================================================
Encrypting a Schema or a General Tablespace
======================================================

|Percona Server| uses the same encryption architecture as |MySQL|, a two-tier
system consisting of a master key and tablespace keys. The master key can be
changed, or rotated in the keyring, as needed. Each of the tablespace keys, when
decrypted, remain the same.

The feature requires the keyring plugin.

Setting the Default for Schemas and General Tablespace Encryption
=======================================================================

The tables in a general tablespace are either all encrypted or all unencrypted.
A tablespace cannot contain a mixture of encrypted tables and unencrypted
tables.

In versions before |Percona Server| 8.0.16-7, use the variable
:variable:`innodb_encrypt_tables`.

.. variable:: innodb_encrypt_tables

    :cli: ``--innodb-encrypt-tables``
    :removed: version 8.0.16-7
    :dyn: Yes
    :scope: Global
    :vartype: Text
    :default: ``OFF``

The variable is considered **deprecated** and was removed in version 8.0.16-7.
The default setting is "OFF".

The encryption of a schema or a general tablespace is determined by the
:variable:`default_table_encryption` variable unless you specify the
ENCRYPTION clause in the CREATE SCHEMA or CREATE TABLESPACE statement. This
variable is implemented in |Percona Server| version 8.0.16-7.

You can set the :variable:`default_table_encryption` variable in an individual
connection.

.. code-block:: MySQL

    mysql> SET default_table_encryption=ON;

System Variable
---------------------

.. variable:: default_table_encryption

    :cli: ``default-table-encryption``
    :dyn: Yes
    :scope: Session
    :vartype: Text
    :default: OFF

Defines the default encryption setting for schemas and general tablespaces. The
variable allows you to create or alter schemas or tablespaces without specifying
the ENCRYPTION clause. The default encryption setting applies only to schemas
and general tablespaces and is not applied to the MySQL system tablespace.

The variable has the following possible values:

.. rubric:: ON

New tables are encrypted. To create unencrypted tables add ``ENCRYPTION="N"`` to
the ``CREATE TABLE`` or ``ALTER TABLE`` statement.

.. rubric:: OFF

By default, new tables are unencrypted. To create encrypted tables add
``ENCRYPTION="Y"`` to the ``CREATE TABLE`` or ``ALTER TABLE`` statement. 

.. rubric:: FORCE

New tables are created with the Master key. Using the `ENCRYPTION=NO` to `CREATE
TABLE` or `ALTER TABLE` generates an error and the table is not created or
altered.

To encrypt an unencrypted table with an `ALTER TABLE` statement the
`ENCRYPTION=YES` must be explicitly used.

.. rubric:: KEYRING_ON

:Availablilty: This value is **Experimental** quality.
        
New tables are created with the keyring as the default encryption. You may
specify a numeric key identifier and use a specific `percona-innodb-` key from
the keyring instead of the default key:

    .. code-block:: MySQL

         mysql> CREATE TABLE ... ENCRYPTION=`KEYRING` ENCRYPTION_KEY=ID=NEW_ID

`NEW_ID` is an unsigned 32-bit integer that refers to the numerical part of the
`percona-innodb-` key. When you assign a numerical identifier in the
`ENCRYPTION_KEY_ID` clause, the server uses the latest version of the
corresponding key. For example, `ENCRYPTION_KEY_ID=2` refers to the latest
version of the `percona_innodb-2` key from the keyring.

.. rubric:: FORCE_KEYRING

:Availablilty: This value is **Experimental** quality.

New tables are created encrypted and the keyring encryption is enforced.

.. rubric:: ONLINE_TO_KEYRING

:Availablilty: This value is **Experimental** quality.

It is only possible to apply the keyring encryption when creating or altering
tables.

.. note::

    The `ALTER TABLE` statement changes the current encryption mode only if you
    use the `ENCRYPTION` clause.

.. seealso::

      MySQL Documentation: default_table_encryption
      https://dev.mysql.com/doc/refman/8.0/en/server-system-variables.html

.. _merge-sort-encryption:

.. rubric:: Merge-sort-encryption


.. variable:: innodb_encrypt_online_alter_logs

    :cli: ``--innodb_encrypt-online-alter-logs``
    :dyn: Yes
    :scope: Global
    :vartype: Boolean
    :default: OFF

This variable simultaneously turns on the encryption of files used by InnoDB for
full text search using parallel sorting, building indexes using merge sort, and
online DDL logs created by InnoDB for online DDL. Encryption is available for
file merges used in queries and backend processes.

Setting Tablespace `ENCRYPTION` without the Default Setting
----------------------------------------------------------------

If you do not set the default encryption setting, you can create general
tablespaces with the ``ENCRYPTION`` setting.

.. code-block:: MySQL

    mysql> CREATE TABLESPACE tablespace_name ENCRYPTION='Y';

All tables contained in the tablespace are either encrypted or not encrypted.
You cannot encrypted only some of the tables in a general tablespace. This
feature extends the  `CREATE TABLESPACE
<https://dev.mysql.com/doc/refman/8.0/en/create-tablespace.html>`_ statement to
accept the ``ENCRYPTION='Y/N'`` option.

.. note::

   Prior to |Percona Server| 8.0.13, the ``ENCRYPTION`` option was specific to
   the ``CREATE TABLE`` or ``SHOW CREATE TABLE`` statement. As of |Percona Server|
   8.0.13, this option is a tablespace attribute and  no longer  allowed with the
   ``CREATE TABLE`` or ``SHOW CREATE TABLE`` statement except for file-per-table
   tablespaces.

In an encrypted general tablespace, an attempt to create an unencrypted table
generates the following error:

.. code-block:: MySQL

    mysql> CREATE TABLE t3 (a INT, b TEXT) TABLESPACE foo ENCRYPTION='N';
    ERROR 1478 (HY0000): InnoDB: Tablespace 'foo' can contain only ENCRYPTED tables.

An attempt to create or to move any tables, including partitioned ones, to a
general tablespace with an incompatible encryption setting are diagnosed and
the process is aborted.

If you must move tables between incompatible tablespaces, create tables with the
same structure in another tablespace and run ``INSERT INTO SELECT`` from each of
the source tables into the destination tables.

Exporting an Encrypted General Tablespace
------------------------------------------------------------------

You can only export encrypted file-per-table tablespaces

.. seealso::

    :ref:`encrypting-tables`

    :ref:`encrypting-system-tablespace`

    :ref:`encrypting-temporary-files`

    :ref:`verifying-encryption`
