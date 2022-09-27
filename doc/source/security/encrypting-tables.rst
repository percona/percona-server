.. _encrypting-tables:

=========================================================
Encrypting File-Per-Table Tablespace
=========================================================

An file-per-table tablespace stores the
table data and the indexes for a single InnoDB table. In this tablespace
configuration, each table is stored in an .ibd file.

The architecture for data at rest encryption for file-per-table tablespace
has two tiers:

* Master key
* Tablespace keys.

The keyring plugin must be installed and enabled. The
file_per_table tablespace inherits the schema default encryption
setting,unless you explicitly define encryption in the ``CREATE TABLE``
statement.

An example of the ``CREATE TABLE`` statement:

.. code-block:: mysql

   mysql> CREATE TABLE sample (id INT, mytext varchar(255)) ENCRYPTION='Y';

An example of an ``ALTER TABLE`` statement.

.. code-block:: mysql

    mysql> ALTER TABLE ... ENCRYPTION='Y';

Without the ``ENCRYPTION`` option in the `ALTER TABLE` statement, the table's
encryption state does not change. An encrypted table remains encrypted. An
unencrypted table remains unencrypted.

.. seealso::

  *MySQL* Documentation:
  -  `File-Per-Table Encryption <https://dev.mysql.com/doc/refman/8.0/en/innodb-data-encryption.html#innodb-data-encryption-enabling-disabling>`__

.. seealso::

    :ref:`encrypting-tablespaces`

    :ref:`encrypting-temporary-files`
