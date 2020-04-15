.. _encrypting-doublewrite-buffers:

=======================================================================
Encrypting Doublewrite Buffers
=======================================================================

The two types of doublewrite buffers used in |Percona Server| are encrypted differently.

When the InnoDB system tablespace is encrypted, the ``doublewrite buffer`` pages
are encrypted as well. The key which was used to encrypt the InnoDB system
tablespace is also used to encrypt the doublewrite buffer.

|Percona Server| encrypts the ``parallel doublewrite buffer`` with the respective
tablespace keys. Only encrypted tablespace pages are written as encrypted in the
parallel doublewrite buffer. Unencrypted tablespace pages will be written as
unencrypted.

 .. variable:: innodb_parallel_dblwr_encrypt

    :cli: ``--innodb-parallel-dblwr-encrypt``
    :dyn: Yes
    :scope: Global
    :vartype: Boolean
    :default: ``OFF``

Enables the encryption of the parallel doublewrite buffer. For encryption, uses
the key of the tablespace where the parallel doublewrite buffer is used.

.. seealso::

    :ref:`encrypting-system-tablespace`

    :ref:`encrypting-tablespaces`

    :ref:`encrypting-tables`
