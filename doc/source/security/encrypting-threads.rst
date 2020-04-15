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
:variable:`default_table_encryption` variable set to ``ONLINE_TO_KEYRING``. This variable
configures the number of threads for background encryption. For the online
encryption, the value must be greater than **zero**. 

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


