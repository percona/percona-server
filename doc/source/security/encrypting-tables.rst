.. _encrypting-tables:

=========================================================
Encrypting File-Per-Tablespace Tables
=========================================================

InnoDB can use a tablespace file for each InnoDB table and creates and stores the
table data and the indexes in a single data file. In this tablespace
configuration, each table is stored in an .ibd file.

If you require a specific table to be encrypted, configure the InnoDB table
stored in ``innodb_file_per_table`` tablespace. The default value is enabled for
the `innodb_file_per_table` option, unless you have explicitly specified the
``innodb_file_per_table`` to be ``OFF`` in your my.cnf file.

The architecture for data at rest encryption has two tiers:

* Master key
* Tablespace keys.

For encryption, you must have the keyring plugin installed and enabled. The
file_per_table tablespace inherits the schema default encryption
setting,unless you explicitly define encryption in the ``CREATE TABLE``
statement.

An example of the ``CREATE TABLE`` statement:

.. code-block:: mysql

   mysql> CREATE TABLE myexample (id INT, mytext varchar(255)) ENCRYPTION='Y';

An example of an ``ALTER TABLE`` statement.

.. code-block:: MySQL

    mysql> ALTER TABLE myexample ENCRYPTION='Y';

Without the ``ENCRYPTION`` option in the `ALTER TABLE` statement, the table's
encryption state does not change. An encrypted table remains encrypted. An
unencrypted table remains unencrypted.

.. seealso::

  |MySQL| Documentation:
  -  `File-Per-Table Encryption <https://dev.mysql.com/doc/refman/8.0/en/innodb-data-encryption.html#innodb-data-encryption-enabling-disabling>`__

.. seealso::

    :ref:`encrypting-tablespaces`

    :ref:`encrypting-temporary-files`
