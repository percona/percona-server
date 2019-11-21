.. _data_at_rest_encryption:

===============================================================================
Data at Rest Encryption
===============================================================================

.. contents::
   :local:

The purpose of encrypting the data at rest ensures that if an unauthorized user
is able to view the data files from the file system, the user cannot read
contents.
``Transparent Data Encryption (TDE)`` or ``Data at Rest Encryption`` encrypts
data files and log files.

Master key encryption has the following components:

* The database instance has a master key for tablespaces and a master key for
  binary log encryption.
* Each tablespace has a tablespace key which encrypts the data pages and is
  written in the tablespace header. The assigned tablespace key cannot be
  changed unless you rebuild the table.

The architecture of two separate keys allow the master key to be rotated in a
minimal operation. During the master key rotation, each tablespace key is
re-encrypted with the new master key. Only the first page of the tablespace file
(.ibd) is read and written during the rotation. An encrypted page is decrypted
at the I/O layer, added to the buffer pool and used to read and write the data.
A buffer pool page is not encrypted. The I/O layer encrypts the page before the
page is flushed to disk.

.. note::

   |Percona XtraBackup| version 2.4 supports the backup of encrypted general
   tablespaces.

.. seealso::

    :ref:`overview-master-key`

    :ref:`enabling-master-key`

