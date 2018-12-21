.. _data_at_rest_encryption:

================================================================================
Data at Rest Encryption
================================================================================

.. contents::
   :local:

.. _data-at-rest-encryption.prerequisite:

Prerequisites
================================================================================

Data at rest encryption requires that a keyring plugin, such as `keyring_file
<https://dev.mysql.com/doc/refman/8.0/en/keyring-file-plugin.html>`_ or
:ref:`keyring_vault_plugin` be installed and already loaded. To load the
``keyring`` plugin when starting the server, use the ``--early-plugin-load``
option:

.. code-block:: bash

   $ mysqld --early-plugin-load="keyring_file=keyring_file.so"

Altermatively, you can add this option to your configuration file:

.. code-block:: guess

   [mysqld]
   early-plugin-load=keyring_file.so

.. warning::

   Only one keyring plugin should be enabled at a time. Enabling multiple
   keyring plugins is not supported and may result in data loss.

.. seealso::

   |MySQL| Documentation:
      - `Installing a Keyring Plugin <https://dev.mysql.com/doc/refman/8.0/en/keyring-installation.html>`_
      - `The --early-plugin-load Option <https://dev.mysql.com/doc/refman/8.0/en/server-options.html#option_mysqld_early-plugin-load>`_


.. _data-at-rest-encryption.keyring.changing-default:

Changing the Default Keyring Encryption
================================================================================

When encryption is enabled and the server is configured to use the KEYRING
encryption, new tables use the default encryption key.

You many change this default encryption via the
:variable:`innodb_default_encryption_key_id` variable.

.. seealso::

   Configuring the way how tables are encrypted
      :variable:`innodb_encrypt_tables`

System Variables
--------------------------------------------------------------------------------

.. variable:: innodb_default_encryption_key_id

   :cli: ``--innodb-default-encryption-key-id``
   :dyn: Yes
   :scope: Session
   :vartype: Numeric
   :default: 0

The ID of the default encryption key. By default, this variable contains **0**
to encrypt new tables with the latest version of the key ``percona_innodb-0``.

To change the default value use the following syntax:

.. code-block:: guess

   mysql> SET innodb_default_encryption_key_id = NEW_ID

Here, **NEW_ID** is an unsigned 32-bit integer.

.. _data-at-rest-encryption.innodb-system-tablespace:

InnoDB System Tablespace Encryption
================================================================================

:Availabiliity: This feature is **Alpha** quality

The InnoDB system tablespace is encrypted by using master key encryption. The
server must be started with the ``--bootstrap`` option.

If the variable :variable:`innodb_sys_tablespace_encrypt` is set to ON and the
server has been started in the bootstrap mode, you may create an encrypted table
as follows:

.. code-block:: guess

   mysql> CREATE TABLE ... TABLESPACE=innodb_system ENCRYPTION='Y'

.. note::

   You cannot encrypt existing tables in the System tablespace.

It is not possible to convert the system tablespace from encrypted to
unencrypted or vice versa. A new instance should be created and user tables must
be transferred to the desired instance.

You can encrypt the already encrypted InnoDB system tablespace (key rotation)
with a new master key by running the following ``ALTER INSTANCE`` statement:

.. code-block:: guess

   mysql> ALTER INSTANCE ROTATE INNODB MASTER KEY

.. rubric:: Doublewrite Buffers

The two types of doublewrite buffers used in |Percona Server| are encrypted
differently.

When the InnoDB system tablespace is encrypted, the ``doublewrite buffer`` pages
are encrypted as well. The key which was used to encrypt the InnoDB system
tablespace is also used to encrypt the doublewrite buffer.

|Percona Server| encrypts the ``parallel doublewrite buffer`` with the respective
tablespace keys. Only encrypted tablespace pages are written as encrypted in the
parallel doublewrite buffer. Unencrypted tablespace pages will be written as
unencrypted.

.. important::

   A server instance bootstrapped with the encrypted InnoDB system tablespace
   cannot be downgraded. It is not possible to parse encrypted InnoDB system
   tablespace pages in a version of |Percona Server| lower than the version
   where the InnoDB system tablespace has been encrypted.


System variables
--------------------------------------------------------------------------------

.. variable:: innodb_sys_tablespace_encrypt

   :cli: ``--innodb-sys-tablespace-encrypt``
   :dyn: No
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``

Enables the encryption of the InnoDB System tablespace. It is essential that the
server is started with the ``--bootstrap`` option.

.. seealso::

   |MySQL| Documentation: ``--bootstrap`` option
      https://dev.mysql.com/doc/refman/8.0/en/server-options.html#option_mysqld_bootstrap

.. variable:: innodb_parallel_dblwr_encrypt

   :cli: ``--innodb-sys-tablespace-encrypt``
   :dyn: Yes
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``

Enables the encryption of the parallel doublewrite buffer. For encryption, uses
the key of the tablespace where the parallel doublewrite buffer is used.


.. _innodb_general_tablespace_encryption:

InnoDB General Tablespace Encryption
================================================================================

In |Percona Server| :rn:`5.7.20-18` existing tablespace encryption support has
been extended to handle general tablespaces. A general tablespace is either
fully encrypted, covering all the tables inside, or not encrypted at all.
It is not possible to have encrypted only some of the tables in a general
tablespace.

This feature extends the  `CREATE TABLESPACE
<https://dev.mysql.com/doc/refman/8.0/en/create-tablespace.html>`_
statement to accept the ``ENCRYPTION='Y/N'`` option.


Usage
================================================================================

General tablespace encryption is enabled by the following syntax extension:

.. code-block:: mysql

   mysql> CREATE TABLESPACE tablespace_name ... ENCRYPTION='Y'

Attempts to create or to move tables, including partitioned ones, to a general
tablespace with an incompatible encryption setting are diagnosed and aborted.

As you cannot move tables between encrypted and unencrypted tablespaces,
you will need to create another table, add it to a specific tablespace and run
``INSERT INTO SELECT`` from the table you want to move from, and then you will
get encrypted or decrypted table with your desired content.

Example
================================================================================

To create an encrypted tablespace run: :mysql:`CREATE TABLESPACE foo ADD DATAFILE 'foo.ibd' ENCRYPTION='Y';`

To add an encrypted table to that table space run: :mysql:`CREATE TABLE t1 (a INT, b TEXT) TABLESPACE foo ENCRYPTION="Y";`

Trying to add unencrypted table to this table space will result in an error:

.. code-block:: mysql

   mysql> CREATE TABLE t3 (a INT, b TEXT) TABLESPACE foo ENCRYPTION="N";
   ERROR 1478 (HY000): InnoDB: Tablespace `foo` can contain only an ENCRYPTED tables.

.. note::

   |Percona XtraBackup| currently doesn't support backup of encrypted general
   tablespaces.

Checking
================================================================================

If there is a general tablespace which doesn't include tables yet, sometimes
user needs to find out whether it is encrypted or not (this task is easier for
single tablespaces since you can check table info).

A ``flag`` field in the ``INFORMATION_SCHEMA.INNODB_TABLESPACES`` has bit
number 13 set if tablespace is encrypted. This bit can be ckecked with 
``flag & 8192`` expression in the following way:

.. code-block:: mysql

  SELECT space, name, flag, (flag & 8192) != 0 AS encrypted FROM INFORMATION_SCHEMA.INNODB_TABLESPACES WHERE name in ('foo', 'test/t2', 'bar', 'noencrypt');


.. admonition:: Output

   .. code-block:: guess

      +-------+-----------+-------+-----------+
      | space | name      | flag  | encrypted |
      +-------+-----------+-------+-----------+
      |    29 | foo       | 10240 |      8192 |
      |    30 | test/t2   |  8225 |      8192 |
      |    31 | bar       | 10240 |      8192 |
      |    32 | noencrypt |  2048 |         0 |
      +-------+-----------+-------+-----------+
      4 rows in set (0.01 sec)

System Variables
----------------

.. variable:: innodb_temp_tablespace_encrypt

  :cli: ``--innodb-temp-tablespace-encrypt``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``Off``

When this option is turned on, server starts to encrypt temporary tablespace
and temporary |InnoDB| file-per-table tablespaces. The option does not force
encryption of temporary tables which are currently opened, and it doesn't
rebuild system temporary tablespace to encrypt data which are already written.

Since temporary tablespace is created fresh at each server startup, it will not
contain unencrypted data if this option specified as server argument.

Turning this option off at runtime makes server to create all subsequent
temporary file-per-table tablespaces unencrypted, but does not turn off
encryption of system temporary tablespace.

This feature is considered **BETA** quality.

.. note:: To use this option, keyring plugin must be loaded, otherwise server
   will give error message and refuse to create new temporary tables.

.. variable:: innodb_encrypt_tables

  :cli: ``--innodb-encrypt-tables``
  :dyn: Yes
  :scope: Global
  :vartype: Text
  :default: ``OFF``

The implementation of the behavior controlled by this variable is considered
**BETA** quality.

This variable was ported from MariaDB and then extended to support key rotation. This
variable has the following possible values:

.. rubric:: ON

New tables are created encrypted. You can create an unencrypted table by using
the ``ENCRYPTION=NO`` clause to the ``CREATE TABLE`` or ``ALTER TABLE``
statement.

.. rubric:: OFF

By default, newly created tables are not encrypted. Add the ``ENCRYPTION=NO``
clause in the ``CREATE TABLE`` or ``ALTER TABLE`` statement to create an
encrypted table.

.. rubric:: FORCE

New tables are created encrypted with the master key. Passing ``ENCRYPTION=NO``
to ``CREATE TABLE`` or ``ALTER TABLE`` will result in an error and the table
will not be created or altered.

If you alter a table which was created without encryption, note that it will not
be encrypted unless you use the ``ENCRYPTION`` clause explicitly.

.. rubric:: KEYRING_ON

:Availability: This value is **Alpha** quality

New tables are created encrypted with the keyring as the default encryption. You
may specify a numeric key identifier and use a specific ``percona-innodb-`` key
from the keyring instead of the default key:

.. code-block:: guess

   mysql> CREATE TABLE ... ENCRYPTION=’KEYRING’ ENCRYPTION_KEY_ID=NEW_ID

**NEW_ID** is an unsigned 32-bit integer that refers to the numerical part of
the ``percona_innodb-`` key.  When you assign a numerical identifer in the
``ENCRYPTION_KEY_ID`` clause, the server uses the latest version of the
corresponding key. For example, the clause ``ENCRYPTION_KEY_ID=2`` refers to the
latest version of the ``percona_innodb-2`` key from the keyring.

In case the ``percona-innodb-`` key with the requested ID does not exist in the
keyring, |Percona Server| will create it with version 1. If a new
``percona-innodb-`` key cannot be created with the requested ID, the whole
``CREATE TABLE`` statement fails

.. rubric:: FORCE_KEYRING
	    
:Availability: This value is **Alpha** quality

New tables are created encrypted and keyring encryption is enforced.

.. rubric:: ONLINE_TO_KEYRING

:Availability: This value is **Alpha** quality

All tables created or altered without the ``ENCRYPTION=NO`` clause 
are encrypted with the latest version of the default encryption key. If a table
being altered is already encrypted with the master key, the table is recreated
encrypted with the latest version of the default encryption key.

.. rubric:: ONLINE_TO_KEYRING_FORCE

:Availability: This value is **Alpha** quality

It is only possible to apply the keyring encryption when creating or altering
tables.

.. note::

   The ``ALTER TABLE`` statement changes the current encryption mode only if you
   use the ``ENCRYPTION`` clause.

.. seealso::

   |MariaDB| Documentation: ``innodb_encrypt_tables`` Option
      https://mariadb.com/kb/en/library/xtradbinnodb-server-system-variables/#innodb_encrypt_tables

.. variable:: innodb_online_encryption_threads

   :cli: ``--innodb-online-encryption-threads``
   :dyn: Yes
   :scope: Global
   :vartype: Numeric
   :default: 1

This variable works in combination with the :variable:`innodb_encrypt_tables`
variable set to ``ONLINE_TO_KEYRING``. This variable configures the number of
threads for background encryption. For the online encryption to work, this
variable must contain a value greater than **zero**.

.. variable:: innodb_online_encryption_rotate_key_age

   :cli: ``--innodb-online-encryption-rotate-key-age``
   :dyn: Yes
   :scope: Global
   :vartype: Numeric
   :default: 1

By using this variable, you can re-encrypt the table encrypted using
KEYRING. The value of this variable determines how frequently the encrypted
tables should be encrypted again. If it is set to **1**, the encrypted table is
re-encrypted on each key rotation. If it is set to **2**, the table is encrypted
on every other key rotation.

Binary log encryption
=====================

The encryption of binary and relay logs is triggered by the
:variable:`encrypt_binlog` variable.

Besides turning :variable:`encrypt_binlog` ``ON``, this feature requires both
`master_verify_checksum
<https://dev.mysql.com/doc/refman/8.0/en/replication-options-binary-log.html#sysvar_master_verify_checksum>`_
and `binlog_checksum
<https://dev.mysql.com/doc/refman/8.0/en/replication-options-binary-log.html#sysvar_binlog_checksum>`_
variables to be turned ``ON``.

While replicating, master sends the stream of decrypted binary log events to a
slave (SSL connections can be set up to encrypt them in transport). That said,
masters and slaves use separate keyring storages and are free to use differing
keyring plugins.

Dumping of encrypted binary logs involves decryption, and can be done using
``mysqlbinlog`` with ``--read-from-remote-server`` option.

.. note:: Taking into account that ``--read-from-remote-server`` option  is only
   relevant to binary logs, encrypted relay logs can not be dumped/decrypted
   in this way.

.. rubric:: Upgrading from |Percona Server| |changed-version| to any higher version

The key format in the :ref:`keyring vault plugin
<keyring_vault_plugin>` was changed for binlog encryption in
|Percona Server| |changed-version| release. When you are upgrading from
|Percona Server| 5.7.20-19 to any version in |Percona Server| 5.7
or 8.0 series, the binary log encryption will work after you complete the following steps:

- Upgrade to a version higher than |Percona Server| |changed-version|
- Start the server without enabling the binary log encryption: :bash:`--encrypt_binlog=OFF`
- Enforce the the key rotation: :mysql:`SELECT rotate_system_key("percona_binlog")`
- Restart the server enabling the binary log encryption: :bash:`--encrypt_binlog=OFF`

.. |changed-version| replace:: 5.7.20-19


System Variables
----------------

.. variable:: encrypt_binlog

  :cli: ``--encrypt-binlog``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

The variable turns on binary and relay logs encryption.

Temporary file encryption
================================================================================

:Availability: This feature is of **Beta** quality.

The encryption of temporary files is triggered by the
:variable:`encrypt-tmp-files` option.

Temporary files are currently used in |Percona Server| for the following
purposes:

* filesort (for example, ``SELECT`` statements with ``SQL_BIG_RESULT`` hints),
* binary log transactional caches,
* Group Replication caches.

For each temporary file, an encryption key is generated locally, only kept
in memory for the lifetime of the temporary file, and discarded afterwards.

System Variables
----------------

.. variable:: encrypt-tmp-files

  :cli: ``--encrypt-tmp-files``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

The option turns on encryption of temporary files created by |Percona Server|.

.. _data-at-rest-encryption.key-rotation:

Key Rotation
================================================================================

The keyring management is enabled for each tablespace separately when you set
the encryption in the ``ENCRYPTION`` clause, to `KEYRING` in the supported SQL
statement:

- CREATE TABLE .. ENCRYPTION='KEYRING`
- ALTER TABLE ... ENCRYPTION='KEYRING'
- CREATE TABLESPACE tablespace_name … ENCRYPTION=’KEYRING’

.. note::

   Running ``ALTER TABLE .. ENCRYPTION=’Y’`` on the tablespace created with
   ``ENCRYPTION=’KEYRING’`` converts the table back to the existing MySQL
   scheme.

.. _keyring_vault_plugin:

Keyring Vault plugin
====================

The ``keyring_vault`` plugin can be used to store the encryption keys inside the
`Hashicorp Vault server <https://www.vaultproject.io>`_.

Installation
------------

The safest way to load the plugin is to do it on the server startup by
using `--early-plugin-load variable
<https://dev.mysql.com/doc/refman/8.0/en/server-options.html#option_mysqld_early-plugin-load>`_
option:

.. code-block:: bash

  --early-plugin-load="keyring_vault=keyring_vault.so" \
  --loose-keyring_vault_config="/home/mysql/keyring_vault.conf"

It should be loaded this way to be able to facilitate recovery for encrypted
tables.

.. warning::

  If server should be started with several plugins loaded early,
  ``--early-plugin-load`` should contain their list separated by semicolons. Also
  it's a good practice to put this list in double quotes so that semicolons
  do not create problems when executed in a script.

Apart from installing plugin you also need to set the
:variable:`keyring_vault_config` variable. This variable should point to the
keyring_vault configuration file, whose contents are discussed below.

This plugin supports the SQL interface for keyring key management described in
`General-Purpose Keyring Key-Management Functions
<https://dev.mysql.com/doc/refman/8.0/en/keyring-udfs-general-purpose.html>`_
manual.

To enable the functions you'll need to install the ``keyring_udf`` plugin:

.. code-block:: mysql

  mysql> INSTALL PLUGIN keyring_udf SONAME 'keyring_udf.so';

Usage
--------------------------------------------------------------------------------

On plugin initialization ``keyring_vault`` connects to the Vault server using
credentials stored in the credentials file. Location of this file is specified
in by :variable:`keyring_vault_config`. On successful initialization it
retrieves keys signatures and stores them inside an in-memory hash map.

Configuration file should contain the following information:

* ``vault_url`` - the address of the server where Vault is running. It can be a
  named address, like one in the following example, or just an IP address. The
  important part is that it should begin with ``https://``.

* ``secret_mount_point`` - the name of the mount point where ``keyring_vault``
  will store keys.

* ``token`` - a token generated by the Vault server, which ``keyring_vault``
  will further use when connecting to the Vault. At minimum, this token should
  be allowed to store new keys in a secret mount point (when ``keyring_vault``
  is used only for transparent data encryption, and not for ``keyring_udf``
  plugin). If ``keyring_udf`` plugin is combined with ``keyring_vault``, this
  token should be also allowed to remove keys from the Vault (for the
  ``keyring_key_remove`` operation supported by the ``keyring_udf`` plugin).

* ``vault_ca [optional]`` - this variable needs to be specified only when the
  Vault's CA certificate is not trusted by the machine that is going to connect
  to the Vault server. In this case this variable should point to CA
  certificate that was used to sign Vault's certificates.

.. warning::
   
   Each ``secret_mount_point`` should be used by only one server - otherwise
   mixing encryption keys from different servers may lead to undefined
   behavior.
  
An example of the configuration file looks like this: ::

  vault_url = https://vault.public.com:8202
  secret_mount_point = secret
  token = 58a20c08-8001-fd5f-5192-7498a48eaf20
  vault_ca = /data/keyring_vault_confs/vault_ca.crt

When a key is fetched from a ``keyring`` for the first time the
``keyring_vault`` communicates with the Vault server, and retrieves the key
type and data. Next it queries the Vault server for the key type and data and
caches it locally.

Key deletion will permanently delete key from the in-memory hash map and the
Vault server.

.. note::

  |Percona XtraBackup| currently doesn't support backup of tables encrypted
  with :ref:`keyring_vault_plugin`.

System Variables
----------------

.. variable:: keyring_vault_config

  :cli: ``--keyring-vault-config``
  :dyn: Yes
  :scope: Global
  :vartype: Text
  :default:

This variable is used to define the location of the
:ref:`keyring_vault_plugin` configuration file.

.. variable:: keyring_vault_timeout

  :cli: ``--keyring-vault-timeout``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``15``

This variable allows to set the duration in seconds for the Vault server
connection timeout. Default value is ``15``. Allowed range is from ``1``
second to ``86400`` seconds (24 hours). The timeout can be also completely
disabled to wait infinite amount of time by setting this variable to ``0``.

.. _data-at-rest-encryption.data-scrubbing:

Data Scrubbing
================================================================================

:Availability: This feature is **Alpha** quality

While data encryption ensures that the existing data are not stored in plain
form, the data scrubbing literally removes the data once the user decides they
should be deleted. Compare this behavior with how the ``DELETE`` statement works
which only marks the affected data as *deleted* - the space claimed by this data
is overwritten with new data later.

Once enabled, data scrubbing works automatically on each tablespace
separately. To enable data scrubbing, you need to set the following variables:

- :variable:`innodb-background-scrub-data-uncompressed`
- :variable:`innodb-background-scrub-data-compressed`

Uncompressed tables can also be scrubbed immediately, independently of key
rotation or background threads. This can be enabled by setting the variable
:variable:`innodb-immediate-scrub-data-uncompressed`. This option is not supported for
compressed tables.

Note that data scrubbing is made effective by setting the
:variable:`innodb_online_encryption_threads` variable to a value greater than
**zero**.

System Variables
--------------------------------------------------------------------------------

.. variable:: innodb_background_scrub_data_compressed

   :cli: ``--innodb-background-scrub-data-compressed``
   :dyn: Yes
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``

.. variable:: innodb_background_scrub_data_uncompressed

   :cli: ``--innodb-background-scrub-data-uncompressed``
   :dyn: Yes
   :scope: Global
   :vartype: Boolean
   :default: ``OFF``

.. seealso::

   Vault Documentation
      https://www.vaultproject.io/docs/index.html
   General-Purpose Keyring Key-Management Functions
      https://dev.mysql.com/doc/refman/8.0/en/keyring-udfs-general-purpose.html
