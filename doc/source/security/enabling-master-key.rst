.. _enabling-master-key:

==============================================================================
Enabling Master Key Encryption
==============================================================================

.. _enabling-vault:

Enabling the Keyring Vault
---------------------------

To enable Master key vault encryption, the user must have
`SUPER
<https://dev.mysql.com/doc/refman/5.7/en/privileges-provided.html#priv_super>`_
privileges.

Add the following statements to my.cnf:

.. code-block:: MySQL

    [mysqld]
    early-plugin-load="keyring_valut=keyring_valut.so"
    loose-keyring_value_config="/home/mysql/keyring_vault.conf"

Restart MySQL.

Encrypting a General Tablespace
-------------------------------------------

.. _enabling-file-per-table:

The InnoDB system tablespace is encrypted by using master key encryption. The
server must be started with the `initialize
<https://dev.mysql.com/doc/refman/5.7/en/server-options.html#option_mysqld_initialize>`_ option.

If the variable :variable:`innodb_sys_tablespace_encrypt` is set to OFF and the
server has been started with ``initialize=ON``, you may create an encrypted
table
as follows:

.. code-block:: guess

    mysql> CREATE TABLE t1 (id INT) ENCRYPTION='Y';

To encrypt a file-per-table tablespace, add the `ENCRYPTION` option.

.. code-block:: MySQL

    mysql> ALTER TABLE t1 ENCRYPTION='Y';

You can also disable encryption for a file-per-table tablespace, set the
encryption to `N`.

.. code-block:: MySQL

    mysql> ALTER TABLE t1 ENCRYPTION='N';

.. _encrypting-system-tablespace:

Encrypting System Tablespace
------------------------------

:Availability: This feature is **Experimental**.

To enable system tablespace encryption, edit the my.cnf file with the following:

* Add ``innodb_sys_tablespace_encrypt``
* Edit the ``innodb_sys_tablespace_encrypt`` value to "ON"

System tablespace encryption can only be enabled with the ``--initialize``
option. You cannot encrypt existing tables in the System tablespace.
.. _encrypting-undo-log:

Encrypting the Undo Log Data
------------------------------

:Availability: This feature is **Experimental**.

You enable encryption for an undo log data by adding the following line to the
my.cnf file:

.. code-block:: MySQL

  [mysqld]
  innodb_undo_log_encrypt=ON

.. _encrypting-binary-logs:

Encrypting Binary Logs
-----------------------

:Availability: This feature is **Experimental** quality.

Enable encryption of binary logs by running the following statement:

.. code-block:: MySQL

    binlog_encryption=ON

You can rotate the encryption key used by |Percona Server| by running the
following statement:

.. code-block:: MySQL

    mysql> SELECT rotate_system_key("percona_binlog");

This command creates a new binlog encryption key in the keyring. The new key
encrypts the next binlog file.

.. _verifying-encryption:

Verifying the Encryption Setting
----------------------------------

For single tablespaces, verify the ENCRYPTION option using
`INFORMATION_SCHEMA.TABLES` and the `CREATE OPTIONS` settings.

.. code-block:: MySQL

    mysql> SELECT TABLE_SCHEMA, TABLE_NAME, CREATE_OPTIONS FROM
           INFORMATION_SCHEMA.TABLES WHERE CREATE_OPTIONS LIKE '%ENCRYPTION%';

    +----------------------+-------------------+------------------------------+
    | TABLE_SCHEMA         | TABLE_NAME        | CREATE_OPTIONS               |
    +----------------------+-------------------+------------------------------+
    |sample                | t1                | ENCRYPTION="Y"               |
    +----------------------+-------------------+------------------------------+

A ``flag`` field in the ``INFORMATION_SCHEMA.INNODB_TABLESPACES`` has the bit
number 13 set if the tablespace is encrypted. This bit can be checked with the
``flag & 8192`` expression with the following method:

.. code-block:: mysql

    SELECT space, name, flag, (flag & 8192) != 0 AS encrypted FROM
    INFORMATION_SCHEMA.INNODB_TABLESPACES WHERE name in ('foo', 'test/t2', 'bar',
    'noencrypt');

      +-------+-----------+-------+-----------+
      | space | name      | flag  | encrypted |
      +-------+-----------+-------+-----------+
      |    29 | foo       | 10240 |      8192 |
      |    30 | test/t2   |  8225 |      8192 |
      |    31 | bar       | 10240 |      8192 |
      |    32 | noencrypt |  2048 |         0 |
      +-------+-----------+-------+-----------+
      4 rows in set (0.01 sec)

:Availability: This feature is **Experimental**.

Encrypted table metadata is contained in the
``INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION`` table.

You must have the `Process
<https://dev.mysql.com/doc/refman/8.0/en/privileges-provided.html#priv_process>`__
privilege to view the table information.

.. note::

    This table is **Experimental** and may change in future releases.

.. code-block:: MySQL

  >desc INNODB_TABLESPACES_ENCRYPTION:

  +-----------------------------+--------------------+-----+----+--------+------+
  | Field                       | Type               | Null| Key| Default| Extra|
  +-----------------------------+--------------------+-----+----+--------+------+
  | SPACE                       | int(11) unsigned   | NO  |    |        |      |
  | NAME                        | varchar(655)       | YES |    |        |      |
  | ENCRYPTION_SCHEME           | int(11) unsigned   | NO  |    |        |      |
  | KEYSERVER_REQUESTS          | int(11) unsigned   | NO  |    |        |      |
  | MIN_KEY_VERSION             | int(11) unsigned   | NO  |    |        |      |
  | CURRENT_KEY_VERSION         | int(11) unsigned   | NO  |    |        |      |
  | KEY_ROTATION_PAGE_NUMBER    | bigint(21) unsigned| YES |    |        |      |
  | KEY_ROTATION_MAX_PAGE_NUMBER| bigint(21) unsigned| YES |    |        |      |
  | CURRENT_KEY_ID              | int(11) unsigned   | NO  |    |        |      |
  | ROTATING_OR_FLUSHING        | int(1) unsigned    | NO  |    |        |      |
  +-----------------------------+--------------------+-----+----+--------+------+

To verify if the binary log encryption option is enabled, run the following
statement:

.. code-block:: MySQL

    mysql>SHOW BINARY LOGS;

    +-------------------+----------------+---------------+
    | Log_name          | File_size      | Encrypted     |
    +-------------------+----------------+---------------+
    | binlog.00011      | 72367          | No            |
    | binlog:00012      | 71503          | No            |
    | binlog:00013      | 73762          | Yes           |
    +-------------------+----------------+---------------+

To allow for master Key rotation, you can encrypt an already encrypted InnoDB
system tablespace with a new master key by running the following ``ALTER
INSTANCE`` statement:

.. code-block:: guess

   mysql> ALTER INSTANCE ROTATE INNODB MASTER KEY;

.. seealso::

    `ALTER INSTANCE <https://dev.mysql.com/doc/refman/5.7/en/alter-instance.html>`_
