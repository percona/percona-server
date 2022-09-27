.. _encrypting-binlogs:

=======================================================================
Encrypting Binary Log Files and Relay Log Files
=======================================================================

Binary log file and relay log file encryption at rest ensures the
server-generated binary logs are encrypted in persistent storage.

Upgrading from *Percona Server for MySQL* 8.0.15-5 to any Higher Version
----------------------------------------------------------------

Starting from the release :ref:`8.0.15-5`, *Percona Server for MySQL* uses the upstream
implementation of binary log file and relay log file encryption.

The :ref:`encrypt-binlog` variable is
removed, and the related command-line option `--encrypt-binlog` is not
supported. It is important to remove the `encrypt-binlog` variable from your
configuration file before you attempt to upgrade either from another release
in the *Percona Server for MySQL* 8.0 series or from *Percona Server for MySQL* 5.7.
Otherwise, a server boot error is generated, and reports an unknown
variable.

The implemented binary log file encryption is compatible with the older
format. The encrypted binary log file used in a previous version of MySQL 8.0
series or Percona Server for MySQL series is supported.

Architecture
------------

The Binary log encryption uses the following tiers:

    * File password

    * Binary log file encryption key

The file password encrypts the content of a single binary file or relay log
file. The binary log encryption key encrypts the file password and the key
is stored in the keyring.

Implementation
---------------

After you have enabled the :ref:``binlog_encryption`` variable and the keyring is
available, you can encrypt the data content for new binary log files and relay
log files. Only the data content is encrypted. Attempting a binary log file or
relay log file encryption without the keyring generates a MySQL error.

In replication, the source maintains the binary log and the replica maintains a binary
log copy called the relay log. The source copies a stream of decrypted binary
log events to a replica using SSL connections to encrypt the stream. The events
are re-executed on the replica.  The source and replicas can use separate
keyring storages and different keyring plugins.

When the :ref:`binlog_encryption` is set to ``OFF``, the server rotates the
binary log files and the relay log files and all new log files are unencrypted.
The encrypted files are not unencrypted, but the server can read the files.

When an encrypted binary log is dumped, and this operation involves decryption,
use ``mysqlbinlog`` with the ``--read-from-remote-server`` option.

.. note::

    The `--read-from-remote-server` option only applies to the binary logs.
    Encrypted relay logs can not be dumped or decrypted with this option.
    
Enabling Binary Log Encryption
-------------------------------

In versions *Percona Server for MySQL* 8.0.15-5 and later, set the `binlog_encryption` variable
to ``ON`` in a startup configuration file, such as :file:`my.cnf`. The variable
is set to ``OFF`` by default.

.. code-block:: text

    binlog_encryption=ON

Verifying the Encryption
-------------------------

To verify if the binary log encryption option is enabled, run the following
statement:

.. code-block:: mysql

    mysql> SHOW BINARY LOGS;

    +-------------------+----------------+---------------+
    | Log_name          | File_size      | Encrypted     |
    +-------------------+----------------+---------------+
    | binlog.00011      | 72367          | No            |
    | binlog:00012      | 71503          | No            |
    | binlog:00013      | 73762          | Yes           |
    +-------------------+----------------+---------------+

The ``SHOW BINARY LOGS`` statement displays the name, size, and if a binary log
file is encrypted or unencrypted.

Binary log file variables
--------------------------

.. _encrypt_binlog:

.. rubric:: ``encrypt_binlog``
 
.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--encrypt-binlog``
   * - Dynamic
     - No
   * - Scope
     - Global
   * - Data type
     - Boolean
   * - Default
     - ``OFF``

The variable was removed in :ref:`8.0.15-5`.

This variable enables or disables the binary log file and relay log file encryption.

.. seealso::

    *MySQL* Documentation:
    `Encrypting Binary Log Files and Relay Log Files <https://dev.mysql.com/doc/refman/8.0/en/replication-binlog-encryption.html>`_

.. seealso::

    :ref:`encrypting-tables`

    :ref:`encrypting-tablespaces`

    :ref:`encrypting-system-tablespace`

    :ref:`encrypting-temporary-files`


