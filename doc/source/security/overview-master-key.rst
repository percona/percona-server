.. _overview-master-key:

==============================================================================
Master Key Encryption Overview
==============================================================================

.. _innodb_general_tablespace_encryption:
.. _general-tablespace-encryption:

General Tablespace Encryption
----------------------------------------

.. rubric:: System Variables

The InnoDB system tablespace is encrypted by using master key encryption. The
server must be started with the ``--bootstrap`` option.

In |Percona Server| :rn:`5.7.20-18`, the existing tablespace encryption supports
 general tablespaces. A general tablespace is either
encrypted; all tables are encrypted, or not encrypted, no encrypted tables.
It is not possible to encrypt only some of the tables in a general
tablespace.

This feature extends the  `CREATE TABLESPACE
<https://dev.mysql.com/doc/refman/5.7/en/create-tablespace.html>`_
statement to accept the ``ENCRYPTION='Y/N'`` option.

If you must move a table between tablespaces with different encryption
settings, ceate a table with the same structure in the target tablespace and run
``INSERT INTO SELECT`` from the source table to the target table.

.. rubric:: System Variables

.. variable:: innodb_encrypt_tables

   :version 5.7.21-21: Implemented
   :cli: ``--innodb-encrypt-tables``
   :dyn: Yes
   :scope: Global
   :vartype: Text
   :default: ``OFF``

:Availability: This feature is **Experimental** quality.

The table lists the available values and descriptions:

.. list-table::
    :widths: 15 30
    :header-rows: 1

    * - Value
      - Description
    * - OFF
      - Disables encryption for new tables. The default value assign
        to the variable.
    * - ON
      - Enables encryption for new tables.
    * - FORCE
      - New tables are created with the master key. Adding the
        ``ENCRYPTION=NO`` to either the ``CREATE TABLE`` or ``ALTER TABLE``
        statement results in a warning.

        Existing plaintext InnoDB tables remain available.

.. note::

    The ``ALTER TABLE`` statment modifies the current encryption mode only if
    the ``ENCRYPTION`` clause is explictily added.

.. _doublewrite-buffer-encryption:

Doublewrite Buffer Encryption
------------------------------

:Availability: This feature is **Experimental** quality.

The two types of doublewrite buffers used in |Percona Server| are encrypted differently.

When the InnoDB system tablespace is encrypted, the ``doublewrite buffer`` pages
are also encrypted. The InnoDB system tablespace key encrypts the doublewrite buffer.

|Percona Server| encrypts the ``parallel doublewrite buffer`` with the respective
tablespace keys. Encrypted tablespace pages are written as encrypted in the
parallel doublewrite buffer. Unencrypted tablespace pages are written as
unencrypted.

 .. variable:: innodb_parallel_dblwr_encrypt

    :version 5.7.23-24: Implemented
    :cli: ``--innodb-parallel-dblwr-encrypt``
    :dyn: Yes
    :scope: Global
    :vartype: Boolean
    :default: ``OFF``

Enables the encryption of the parallel doublewrite buffer. Encryption uses
the tablespace key where the parallel doublewrite buffer is located.

.. _system-tablespace-encryption:

System Tablespace Encryption
-----------------------------

:Availability: This feature is **Experimental** quality.

.. rubric:: Limitations

You cannot convert an encrypted system tablespace to an unencrypted tablespace
or an unencrypted system tablespace to encrypted. If a conversion is needed, you
must create a new instance with the system tablespace in the desired state and
then transfer the user tables to that instance.

A server instance initialized with the encrypted InnoDB system tablespace cannot
be downgraded. It is not possible to parse encrypted InnoDB system tablespace
pages in a version of |Percona Server| lower than the version where the InnoDB 
system tablespace has been encrypted.

The server must be started with the 
`--initialize <https://dev.mysql.com/doc/refman/5.7/en/server-options.html#option_mysqld_initialize>`_ 
option. 

.. variable:: innodb_sys_tablespace_encrypt

    :version 5.7.23-24: Implemented
    :cli: ``--innodb-sys-tablespace-encrypt``
    :dyn: No
    :scope: Global
    :vartype: Boolean
    :default: ``OFF``

The variable enables the encryption of the InnoDB system tablespace.

.. _temporary-tablespace-encryption:

Temporary Tablespace Encryption
--------------------------------

:Availability: This feature is **Experimental** quality.

To encrypt InnoDB user-created temporary tables, created in a temporary
tablespace file, use the ``innodb_temp_tablespace_encrypt`` variable.

.. variable:: innodb_temp_tablespace_encrypt

    :version 5.7.21-21: Implemented
    :cli: ``--innodb-temp-tablespace-encrypt``
    :dyn: Yes
    :scope: Global
    :vartype: Boolean
    :default: ``OFF``

This variable enables the encryption of temporary tablespace
and temporary |InnoDB| file-per-table tablespaces. The variable does not
encrypt currently open temporary tables or rebuild the system temporary
tablespace to encrypt data already written.

When this variable is enabled as a server argument at startup, the temporary
tablespace is created fresh and contains encrypted data .

Turning this variable off at runtime, the server creates all subsequent
temporary file-per-table tablespaces unencrypted, but the system temporary
tablespace remains encrypted.

The ``CREATE TEMPORARY TABLE`` does not support the ``ENCRYPTION`` clause. The
``TABLESPACE`` clause cannot be set to innodb_temporary.

.. note:: To use this option, keyring plugin must be loaded, otherwise server
   will give error message and refuse to create new temporary tables.

.. _ps.data-at-rest-encryption.redo-log:
.. _redo-log-encryption:

Redo Log Encryption
--------------------

:Availability: This feature is **Experimental** quality.

.. variable:: innodb_encrypt_online_alter_logs

   :version 5.7.21-21: Implemented
   :cli: ``--innodb-encrypt-online-alter-logs``
   :dyn: no
   :scope: Global
   :vartype: Boolean
   :default: OFF

This variable simultaneously turns on the encryption of files used by InnoDB for
full text search using parallel sorting, building indexes using merge sort, and
online DDL logs created by InnoDB for online DDL.

.. _data-at-rest-encryption.variable.innodb-scrub-log:

.. variable:: innodb_scrub_log

   :version 5.7.23-24: Implemented
   :cli: ``--innodb-scrub-log``
   :dyn: Yes
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``

Specifies if data scrubbing should be automatically applied to the redo log.


.. variable:: innodb_scrub_log_speed

   :version 5.7.23-24: Implemented
   :cli: ``--innodb-scrub-log-speed``
   :dyn: Yes
   :scope: Global
   :vartype: Text
   :default:

Specifies the velocity of data scrubbing (writing dummy redo log records) in bytes per second.


For `innodb_redo_log_encrypt`, the "ON" value is an alias for ``MASTER_KEY``
value.

The redo log data is encrypted when it is written to disk and decrypted when the
data is read from disk. Any redo log data in memory is unencrypted.

When `innodb_redo_log_encrypt` is enabled, any existing redo log pages stay
unencrypted, and new pages are encrypted.

If the redo log encryption is enabled, you must load the appropriate keyring
plugin and encryption key to perform a normal restart.

When `innodb_redo_log_encrypt` is
disabled, any existing pages remain encrypted, and new pages are not encrypted.

An attempt to encrypt the redo log fails if one of the following conditions is
true:

* Server started with no keyring specified

* A different redo log encryption method is defined then what was previously
  used on the same server.

.. _data-at-rest-encryption.undo-tablespace:
.. _undo-log-encryption:

Undo Log Encryption
--------------------

:Availability: This feature is **Experimental** quality.

Undo log data is encrypted using the
:variable:`innodb_undo_log_encrypt` option. You can edit this variable
in the configuration file, as a startup parameter, or during runtime as a global
variable.

.. variable:: innodb_undo_log_encrypt

    :version 5.7.23-24: Implemented
    :cli: ``--innodb_undo-log_encrypt``
    :dyn: Yes
    :scope: Global
    :vartype: Boolean
    :default: OFF

Defines if an undo log data is encrypted. The undo log data encryption is disabled by default.

Undo log data is encrypted when the data is read to disk. The data is decrypted
when the data is read from disk. When the undo log data is in memory, the data
is unencrypted.

The undo log data is encrypted and decrypted with the tablespace encryption key,
which is stored in the undo log file header.

.. note::

    If you disable encryption, any encrypted undo data remains encrypted. To
    remove this data, truncate the undo tablespace.

.. _binlog-encryption:

Binlog Encryption
------------------

Enable the ``binlog_encryption`` option to encrypt new binary log files and
relay
log files on disk. Only the data is encrypted.

A new option, implemented since |Percona Server| :rn:`5.7.20-19`, is
encryption of binary and relay logs, triggered by the
:variable:`encrypt_binlog` variable.

Besides turning :variable:`encrypt_binlog` ``ON``, this feature requires both
`master_verify_checksum
<https://dev.mysql.com/doc/refman/5.7/en/replication-options-binary-log.html#sysvar_master_verify_checksum>`_
and `binlog_checksum
<https://dev.mysql.com/doc/refman/5.7/en/replication-options-binary-log.html#sysvar_binlog_checksum>`_
variables to be turned ``ON``.

While replicating, master sends the stream of decrypted binary log events to a
slave (SSL connections can be set up to encrypt them in transport). That said,
masters and slaves use separate keyring storages and are free to use differing
keyring plugins.

.. note::

    You cannot start `mysqld` with binlog encryption and group replication.
    Binlog encryption is incompatible with binlog checksums, and group replication requires binlog checksums.

Dumping of encrypted binary logs involves decryption, and can be done using
``mysqlbinlog`` with ``--read-from-remote-server`` option.

.. note::

   Taking into account that ``--read-from-remote-server`` option  is only
   relevant to binary logs, encrypted relay logs can not be dumped or decrypted
   in this way.

.. rubric:: Upgrading from |Percona Server| 5.7.20-19 to any higher version

The key format in the :ref:`keyring vault plugin
<keyring_vault_plugin>` was changed for binlog encryption in |Percona
Server| 5.7.20-19 release. When you are upgrading from
|Percona Server| 5.7.20-19 to a higher version in the |Percona Server|
5.7 series or to a version prior to 8.0.15-5 in the |Percona Server|
8.0 series, the binary log encryption will work after you complete the
following steps:

1. Upgrade to a version higher than |Percona Server| 5.7.20-19
#. Start the server without enabling the binary log encryption: :bash:`--encrypt_binlog=OFF`
#. Enforce the key rotation: :mysql:`SELECT rotate_system_key("percona_binlog")`
#. Restart the server enabling the binary log encryption: :bash:`--encrypt_binlog=ON`

System Variables
----------------

.. variable:: encrypt_binlog

   :version 5.7.20-19: Implemented
   :cli: ``--encrypt-binlog``
   :dyn: No
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``

The variable turns on binary and relay logs encryption.


.. _data-at-rest-encryption.key-rotation:
.. _master-key-rotation:

Master Key Rotation
--------------------

Rotate the master key on a schedule or if you
suspect the key is compromised. The master key rotation operation changes the
master
key, and each tablespace key is re-encrypted and updated in the tablespace
headers. The operation does not affect tablespace data. The rotation operation
must complete before starting any tablespace encryption operation.

.. note::

    The rotation re-encrypts each tablespace key, but the key is not
    refreshed. To change a tablespace key, you should disable and then
    re-enable encryption.

To rotate the master key, you must have the `ENCRYPTION_KEY_ADMIN` or
`SUPER` privilege.

If the master key rotation operation is interrupted, the operation is rolled
forward when the server restarts. InnoDB reads the encryption data from the
tablespace header, and if the prior master key has encrypted specific tablespace
keys, InnoDB retrieves the prior master key from the keyring to decrypt the
tablespace key. InnoDB then re-encrypts the tablespace key with the new master
key.

Verifying Encryption
---------------------

If a general tablespace contains tables, check the table information
to see if the table is encrypted. If the general tablespace does not
contain tables, you must verify the tablespace.

