.. _data_at_rest_encryption:

=======================
Data at Rest Encryption
=======================

.. contents::
   :local:

.. _innodb_general_tablespace_encryption:

InnoDB general tablespace encryption
====================================

In |Percona Server| :rn:`5.7.20-18` existing tablespace encryption support
has been extended to handle general tablespaces. A general tablespace is either
fully encrypted, covering all the tables inside, either not encrypted at all.
It is not possible to have only some of the tables in the general tablespace
encrypted.

This feature extends the  `CREATE TABLESPACE
<https://dev.mysql.com/doc/refman/5.7/en/create-tablespace.html>`_
statement to accept the ``ENCRYPTION='Y/N'`` option.

Prerequisites
-------------

This feature requires a keyring plugin, for example `keyring_file
<https://dev.mysql.com/doc/refman/5.7/en/keyring-file-plugin.html>`_ or
:ref:`keyring_vault_plugin` to be loaded before it can be used.

.. warning::

  Only one keyring plugin should be enabled at a time. Enabling multiple
  keyring plugins is unsupported and may result in data loss.

Usage
-----

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
-------

To create an encrypted tablespace run:

.. code-block:: mysql

  mysql> CREATE TABLESPACE foo ADD DATAFILE 'foo.ibd' ENCRYPTION='Y';

To add an encrypted table to that table space run:

.. code-block:: mysql

   mysql> CREATE TABLE t1 (a INT, b TEXT) TABLESPACE foo ENCRYPTION="Y";

Trying to add unencrypted table to this table space will result in an error:

.. code-block:: mysql

  mysql> CREATE TABLE t3 (a INT, b TEXT) TABLESPACE foo ENCRYPTION="N";
  ERROR 1478 (HY000): InnoDB: Tablespace `foo` can contain only an ENCRYPTED tables.

.. note::

  |Percona XtraBackup| currently doesn't support backup of encrypted general
  tablespaces.

Checking
--------

If there is a general tablespace which doesn't include tables yet, sometimes
user needs to find out whether it is encrypted or not (this task is easier for
single tablespaces since you can check table info).

A ``flag`` field in the ``INFORMATION_SCHEMA.INNODB_SYS_TABLESPACES`` has bit
number 13 set if tablespace is encrypted. This bit can be ckecked with 
``flag & 8192`` expression in the following way::

  >SELECT space, name, flag, (flag & 8192) != 0 AS encrypted FROM INFORMATION_SCHEMA.INNODB_SYS_TABLESPACES WHERE name in ('foo', 'test/t2', 'bar', 'noencrypt');
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

.. variable:: innodb_sys_tablespace_encrypt

  :version 5.7.23-23: Implemented
  :cli: ``--innodb-sys-tablespace-encrypt``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``Off``	      

When this variable is enabled, all data in the InnoDB system tablespace are
encrypted.

This feature is considered **ALPHA** quality.

.. variable:: innodb_parallel_dblwr_encrypt

  :version 5.7.23-23: Implemented
  :cli: ``--innodb_parallel_dblwr_encrypt``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``Off``	      

When this variable is enabled, all data in the parallel double write buffer are
encrypted.

This feature is considered **ALPHA** quality.

.. variable:: innodb_temp_tablespace_encrypt

  :version 5.7.21-21: Implemented
  :cli: ``--innodb-temp-tablespace-encrypt``
  :dyn: Yes
  :scope: Global
  :vartype: Boolean
  :default: ``Off``

This feature is considered **BETA** quality.

When this option is turned on, the server starts to encrypt temporary tablespace
and temporary |InnoDB| file-per-table tablespaces. The option does not force
encryption of temporary tables which are currently opened, and it doesn't
rebuild system temporary tablespace to encrypt data which are already written.

Since temporary tablespace is created fresh at each server startup, it will not
contain unencrypted data if this option specified as server argument.

Turning this option off at runtime makes server to create all subsequent
temporary file-per-table tablespaces unencrypted, but does not turn off
encryption of system temporary tablespace.

.. note:: To use this option, the keyring plugin must be loaded, otherwise
   server will give an error message and refuse to create new temporary tables.

.. variable:: innodb_encrypt_tables

  :version 5.7.21-21: Implemented
  :cli: ``--innodb-encrypt-tables``
  :dyn: Yes
  :scope: Global
  :vartype: Text
  :default: ``OFF``

This feature is considered **BETA** quality.

This variable has 3 possible values. ``ON`` makes |InnoDB| tables encrypted by
default. ``FORCE`` disables creation of unencrypted tables. ``OFF`` restores
the like-before behavior.

.. note:: ``innodb_encrypt_tables=ON`` still allows to create unencrypted
   table with ``ENCRYPTED=NO`` statement, and also allows to create unencrypted
   general tablespace.

.. note:: ``ALTER TABLE`` statement used without explicit ``ENCRYPTION=XXX``
   does not change current table encryption mode even if
   :variable:`innodb_encrypt_tables` is set to ``ON`` or ``FORCE``.

Binary log encryption
=====================

A new option, implemented since |Percona Server| :rn:`5.7.20-19`, is
encryption of binary and relay logs, triggered by the
:variable:`encrypt_binlog` variable.

Besides turning :variable:`encrypt_binlog` ``ON``, this feature requires both
`master_verify_checksum
<https://dev.mysql.com/doc/refman/5.7/en/replication-options-binary-log.html#sysvar_master_verify_checksum>`_
and `binlog_checksum
<https://dev.mysql.com/doc/refman/5.7/en/replication-options-binary-log.html#sysvar_binlog_checksum>`_
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

System Variables
----------------

.. variable:: encrypt_binlog

  :version 5.7.20-19: Implemented
  :cli: ``--encrypt-binlog``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

The variable turns on binary and relay logs encryption.

Temporary file encryption
=========================

A new feature, implemented since |Percona Server| :rn:`5.7.22-22`, is
encryption of temporary files, triggered by the :variable:`encrypt-tmp-files`
option.

This feature is considered **BETA** quality.

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

  :version 5.7.22-22: Implemented
  :cli: ``--encrypt-tmp-files``
  :dyn: No
  :scope: Global
  :vartype: Boolean
  :default: ``OFF``

The option turns on encryption of temporary files created by |Percona Server|.

.. _keyring_vault_plugin:

Keyring Vault plugin
====================

In |Percona Server| :rn:`5.7.20-18` a ``keyring_vault`` plugin has been
implemented that can be used to store the encryption keys inside the
`Hashicorp Vault server <https://www.vaultproject.io>`_.

Installation
------------

The safest way to load the plugin is to do it on the server startup by
using `--early-plugin-load variable
<https://dev.mysql.com/doc/refman/5.7/en/server-options.html#option_mysqld_early-plugin-load>`_
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
<https://dev.mysql.com/doc/refman/5.7/en/keyring-udfs-general-purpose.html>`_
manual.

To enable the functions you'll need to install the ``keyring_udf`` plugin:

.. code-block:: mysql

  mysql> INSTALL PLUGIN keyring_udf SONAME 'keyring_udf.so';

Usage
-----

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

  :version 5.7.20-18: Implemented
  :cli: ``--keyring-vault-config``
  :dyn: Yes
  :scope: Global
  :vartype: Text
  :default:

This variable is used to define the location of the
:ref:`keyring_vault_plugin` configuration file.

.. variable:: keyring_vault_timeout

  :version 5.7.21-20: Implemented
  :cli: ``--keyring-vault-timeout``
  :dyn: Yes
  :scope: Global
  :vartype: Numeric
  :default: ``15``

This variable allows to set the duration in seconds for the Vault server
connection timeout. Default value is ``15``. Allowed range is from ``1``
second to ``86400`` seconds (24 hours). The timeout can be also completely
disabled to wait infinite amount of time by setting this variable to ``0``.

Other reading
-------------

* `Vault Documentation <https://www.vaultproject.io/docs/index.html>`_
* `General-Purpose Keyring Key-Management Functions
  <https://dev.mysql.com/doc/refman/5.7/en/keyring-udfs-general-purpose.html>`_
