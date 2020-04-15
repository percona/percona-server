.. _encrypting-temporary-files:

==========================================================
Encrypting Temporary Files
==========================================================

:Availability: This feature is of **Experimental** quality.

For InnoDB user-created temporary tables, created in a temporary tablespace
file, use the :variable:`innodb_temp_tablespace_encrypt` variable.

.. variable:: innodb_temp_tablespace_encrypt

   :cli: ``innodb-temp-tablespace-encrypt``
   :dyn: Yes
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``

When this variable is set to ``ON``, the server encrypts the global temporary
tablespace (:file: `ibtmp*` files) and the session temporary tablespaces
(:file: `#innodb_temp/temp_*.ibt` files). The variable does not enforce the
encryption of currently open temporary files and does not rebuild the system
temporary tablespace to encrypt data which has already been written.

The ``CREATE TEMPORARY TABLE`` does not support the ``ENCRYPTION`` clause. The
``TABLESPACE`` clause cannot be set to innodb_temporary.

The global temporary tablespace datafile ``ibtmp1`` contains temporary table
undo logs while intrinsic temporary tables and user-created temporary tables
are located in the encrypted session temporary tablespace.

To create new temporary tablespaces unencrypted, the following variables must
be set to ``OFF`` at runtime:

* :variable:`innodb_temp_tablespace_encrypt`

* :variable:`default_table_encryption`

Any existing encrypted user-created temporary files and intrinsic temporary
tables remain in an encrypted session.

Temporary tables are only destroyed when the session is disconnected.

The `default_table_encryption` setting in my.cnf determines if a temporary
table is encrypted.

If the `innodb_temp_tablespace_encrypt` = "OFF" and the
`default_table_encryption` ="ON", the user-created temporary tables are
encrypted. The temporary tablespace datafile ``ibtmp1``, which contains undo
logs, is not encrypted.

If the ``innodb_temp_tablespace_encrypt`` is "ON" for the system tablespace,
InnoDB generates an encryption key and encrypts the system temporary
tablespace.  If you reset the encryption to "OFF", all subsequent pages are
written to an unencrypted tablespace. Any generated keys are not erased to
allow encrypted tables and undo data to be decrypted.

.. important::

    To use this option, the keyring plugin must be loaded. If the keyring is
    not loaded the server generates an error and refuses to create new
    temporary tables.

Temporary files are currently used in |Percona Server| for the following
purposes:

  * Filesort - for example, when you run a `SELECT` statement with `SQL_BIG_RESULT` hints

  * Binary log transactional caches

  * Group Replication caches

For each temporary file, an encryption key is generated locally and only
maintained in memory for the lifetime of the temporary file and the key is
discarded afterwards.

System Variables
----------------------

.. variable:: encrypt_tmp_files

   :cli: ``--encrypt_tmp_files``
   :dyn: No
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``

This variable turns "ON" the encryption of temporary files created by |Percona
Server|.

  .. seealso::

    |MySQL| Documentation
    https://dev.mysql.com/doc/refman/8.0/en/create-temporary-table.html

.. seealso::

    :ref:`using-keyring-plugin`

    :ref:`encrypting-system-tablespace`
