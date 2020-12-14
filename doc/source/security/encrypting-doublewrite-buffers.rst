.. _encrypting-doublewrite-buffers:

=======================================================================
Encrypting Doublewrite Buffers
=======================================================================

As of |Percona Server| 8.0.20-11, the Percona version of the
doublewrite buffer is replaced with the MySQL implementation.

Until |MySQL| 8.0.20, the system tablespace contained the doublewrite
buffer. The key which encrypts the InnoDB system tablespace also
encrypts the ``doublewrite buffer`` pages.

Currently, |Percona| encrypts the ``doublewrite buffer`` using :variable:`innodb_parallel_dblwr_encrypt`. 
Encrypted tablespace pages are written as encrypted in the
doublewrite buffer using their respective tablespace keys. Unencrypted tablespace pages are not encrypted in the
doublewrite buffer.

 .. variable:: innodb_parallel_dblwr_encrypt

    :cli: ``--innodb-parallel-dblwr-encrypt``
    :dyn: Yes
    :scope: Global
    :vartype: Boolean
    :default: ``OFF``

Enables the encryption of the doublewrite buffer. The encryption uses
the key of the tablespace where the doublewrite buffer is used.

.. seealso::

    `MySQL Doublewrite Buffer <https://dev.mysql.com/doc/refman/8.0/en/innodb-doublewrite-buffer.html>`_


