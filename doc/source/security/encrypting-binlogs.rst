.. _encrypting-binlogs:

=======================================================================
Encrypting Binary Logs
=======================================================================

Binary log encryption at rest ensures the server-generated binary logs are
encrypted in persistent storage.

After you have enabled the ``binlog_encryption`` option and the keyring is
available, you can encrypt new binary logs and relay logs on disk. Only the data
content is encrypted.

In replication, the master sends the stream of decrypted binary log events to a
slave, in transport the data is encrypted by SSL connections. Master and slaves
use separate keyring storages and are able to use different keyring plugins.

When an encrypted binary log is dumped, and the operation involves decryption,
and done using ``mysqlbinlog`` with ``--read-from-remote-server`` option.

.. note::

    The `--read-from-remote-server`  option only applies to the binary logs.
    Encrypted relay logs can not be dumped or decrypted with this option.

Attempting a  binary log encryption without the keyring generates a MySQL error.

The Binary log encryption uses two tiers:

    * File password

    * Binary log encryption key

The file password encrypts the content of a single binary file or relay log
file. The binary log encryption key is used to encrypt the file password and is
stored in the keyring.

Enabling Binary Log Encryption
-------------------------------

To enable the ``binlog_encryption`` option you must set the option in a startup
configuration file, such as the my.cnf file.

.. code-block:: MySQL

    binlog_encryption=ON

Verifying the Encryption Setting
----------------------------------

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

.. seealso::

    |MySQL| Documentation:
    `Encrypting Binary Log Files and Relay Log Files <https://dev.mysql.com/doc/refman/8.0/en/replication-binlog-encryption.html>`__

Upgrading from |Percona Server| 8.0.15-5 to any Higher Version
----------------------------------------------------------------

Starting from release :rn:`8.0.15-5`, |Percona Server| uses the upstream
implementation of binary log encryption. The variable `encrypt-binlog` is
removed and the related command line option `--encrypt-binlog` is not
supported. It is important to remove the `encrypt-binlog` variable from your
configuration file before you attempt to upgrade either from another release
in the |Percona Server| |version| series or from |Percona Server| 5.7.
Otherwise, a server boot error will be generated reporting an unknown
variable. The implemented binary log encryption is compatible with the older
format. The encrypted binary log used in a previous version of MySQL 8.0
series or Percona Server for MySQL series is supported.

.. variable:: encrypt_binlog

      :version-info:  removed in :rn:`8.0.15-5`
      :cli: ``--encrypt-binlog``
      :dyn: No
      :scope: Global
      :vartype: Boolean
      :default: ``OFF``

The variable turns on binary log and relay log encryption.

.. seealso::

    :ref:`encrypting-tables`

    :ref:`encrypting-tablespaces`

    :ref:`encrypting-system-tablespace`

    :ref:`encrypting-temporary-files`

    :ref:`encrypting-doublewrite-buffers`

    :ref:`encrypting-redo-log`

    :ref:`undo-tablespace-encryption`
