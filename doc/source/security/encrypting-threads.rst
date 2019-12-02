.. _encrypting-threads:

================================================================================
Working with Background Encryption Threads
================================================================================

:Availabiliity: This feature is **Experimental**.

Encryption threads in the background allow you to perform some encryption and
decryption tasks in real-time.

You would use encryption threads for the following purposes:

* Encryption threads can encrypt existing tablespaces. Encryption
  threads allow encryption to be applied to all or some of the existing
  tablespaces, you can exclude tablespaces from rotation, in a background process. You
  can encrypt existing tablespaces with the Master key, but you must do this
  operation by tablespace.

* Encryption threads encrypt tables with a key from a keyring. The Master key
  encrypts tables by a key and is stored in the encryption header of the
  tablespace.

* Encryption threads allow key rotation. In an encryption thread rotation, the
  operation re-encrypts each tablespace page by page. The Master key rotation
  does not re-encrypt each page, only the tablespace encryption header.

If you have tablespaces encrypted with the Master key and you enable
encryption threads, the tablespaces are re-encrypted with the keyring key in a
background process.

.. note::

    While encryption threads are enabled, you cannot convert the tablespaces to
    Master key encryption. To convert the tablespaces, you must disable the
    encryption threads.

:Availability: This feature is **Experimental** quality.

.. variable:: innodb_encryption_threads

    :cli: ``--innodb-encryption-threads``
    :dyn: Yes
    :scope: Global
    :vartype: Numeric
    :default: 0

This variable works in combination with the
:variable:`default_table_encryption` variable set to ``ONLINE_TO_KEYRING``.
This variable configures the number of threads for background encryption.
For the online encryption, the value must be greater than **zero**.

.. variable:: innodb_online_encryption_rotate_key_age

    :cli: ``--innodb-online-encryption-rotate-key-age``
    :dyn: Yes
    :scope: Global
    :vartype: Numeric
    :default: 1

Defines the rotation for the re-encryption of a table encrypted using KEYRING.
The value of this variable determines the how frequently the encrypted tables
are re-encrypted.

For example, the following values would trigger a re-encryption in the
following intervals:

*  The value is **1**, the table is re-encrypted on each key rotation.
*  The value is **2**, the table is re-encrypted on every other key rotation.
*  The value is **10**, the table is re-encrypted on every tenth key rotation.

You should select the value which best fits your operational requirements.

Using Keyring Encryption
-------------------------------------------

:Availability: This feature is **Experimental** quality.

Keyring management is enabled for each table, per file table, separately when
you set encryption in the ``ENCRYPTION`` clause to ``KEYRING`` in the supported
SQL statement.

* CREATE TABLE ... ENCRYPTION='KEYRING'
* ALTER TABLE ... ENCRYPTION='KEYRING'

.. note::

    Running an ``ALTER TABLE ... ENCRYPTION='N'`` on a table created with
    ``ENCRYPTION='KEYRING'`` converts the table to the existing MySQL schema,
    tablespace, or table encryption state.

.. seealso::

    :ref:`using-keyring-plugin`

.. rubric:: Preparation

Setting the default_table_encryption to ONLINE_TO_KEYRING, all tables 
Before enabling Advanced Encryption Key Rotation with default_table_encryption
set to ONLINE_TO_KEYRING, review
INFORMATION_SCHEMA.INNODB_TABLESAPCES_ENCRYPTION to verify if the excluded
tables or tablespaces are listed.
if a tablespace is excluded from encryption threads, check
``INFORMATION_SCHEMA.TABLESPACE_ENCRYPTION``. The tablespace should be
listed and have the ``EXCLUDED`` field set to 'Y'. If the tablespace is not
listed,
issue ``ALTER TABLESPACE ... ENCRYPTION='N'`` to exclude the tablespace.

.. rubric:: Encrypting Temporary Files

To create unencrypted temporary tablespaces, the following variables must be
set to ``OFF`` at runtime:

    * ``innodb_temp_tablespace_encrypt``
    * ``default_table_encryption``

Any existing encrypted user-created temporary files and intrinsic temporary
tables remain in an encrypted session. Temporary tables are only destroyed when
the session is disconnected.

The ``default_table_encryption`` setting located in the my.cnf file determines
if a temporary table is encrypted.

.. list-table::
    :widths: 15 30
    :header-rows: 1

    * - Conditions
      - Outcome
    * - ``innodb_temp_tablespace_encrypt`` = "OFF" and
        ``default_table_encryption`` = "ON"
      - User-created temporary tables are encrypted.

        The temporary tablespace datafile, ``ibtmp1``, is not encrypted
    * - ``innodb_temp_tablespace_encrypt`` = "ON" for the system tablespace
      - The system temporary tablespace is encrypted.

        If you reset the variable to "OFF", all subsequent pages are written to
        an unencrypted tablespace. Any generated keys are not erased to allow
        encrypted tables and undo data to be decrypted.

For each temporary file, an encryption
