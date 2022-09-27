.. _data_at_rest_encryption:

================================================================================
Data at Rest Encryption
================================================================================

.. contents::
   :local:

Data security is a concern for institutions and organizations. ``Transparent
Data Encryption (TDE)`` or ``Data at Rest Encryption`` encrypts
data files. Data at rest is
any data which is not accessed or changed frequently, stored on different
types of storage devices. Encryption ensures that if an unauthorized user
accesses the data files from the file system, the user cannot read contents.

If the user uses master key encryption, the MySQL keyring plugin stores the
InnoDB master key, used for the master key encryption implemented by *MySQL*.
The master key is also used to encrypt redo logs, and undo logs, along with the
tablespaces.

The InnoDB tablespace encryption has the following components:

    * The database instance has a master key for tablespaces and a master key
      for binary log encryption.

    * Each tablespace has a tablespace key. The key is used to encrypt the
      Tablespace data pages. Encrypted tablespace keys are written on
      tablespace header. In the master key implementation, the tablespace key
      cannot be changed unless you rebuild the table.

Two separate keys allow the master key to be rotated in a minimal operation.
When the master key is rotated, each tablespace key is decrypted and
re-encrypted with the new master key. Only the first page of every tablespace
(.ibd) file is read and written during the key rotation.

An InnoDB tablespace file is comprised of multiple logical and physical pages.
Page 0 is the tablespace header page and keeps the metadata for the tablespace.
The encryption information is stored on page 0 and the tablespace key is
encrypted.

A buffer pool page is not encrypted. An encrypted page is decrypted at the I/O
layer and added to the buffer pool and used to access the data. The page is
encrypted by the I/O layer before the page is flushed to disk.

.. note::

   *Percona XtraBackup* version 8 supports the backup of encrypted general
   tablespaces. Features which are not Generally Available (GA) in |Percona
   Server| are not supported in version 8.

.. seealso::

    :ref:`vault`

    :ref:`using-keyring-plugin`

    :ref:`encrypting-tables`

    :ref:`encrypting-tablespaces`

    :ref:`encrypting-system-tablespace`

    :ref:`encrypting-temporary-files`

    :ref:`verifying-encryption`

    :ref:`encrypting-doublewrite-buffers`

    :ref:`encrypting-binlogs`

    :ref:`encrypting-redo-log`

    :ref:`undo-tablespace-encryption`

    :ref:`rotating-master-key`
    
    :ref:`encrypting-threads`
