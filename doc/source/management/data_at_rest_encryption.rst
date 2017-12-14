.. _data_at_rest_encryption:

=======================
Data at Rest Encryption
=======================

This feature is considered **BETA** quality.

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

Configuration file should contain following information:

* ``vault_url`` - the address of the server where Vault is running. It can be a
  named address – like in example or ip address. The important part is that
  it should start with ``https://``

* ``secret_mount_point`` - the name of the mount point where ``keyring_vault``
  will store keys

* ``token`` - the token that ``keyring_vault`` should use when connecting to
  the Vault. When ``keyring_vault`` is to used only for transparent data
  encryption and not for ``keyring_udf`` plugin, the token allows for storing
  new keys in secret mount point. However if ``keyring_udf`` plugin is used
  with ``keyring_vault`` plugin this token also allows removing keys from
  vault. This is due to the fact that ``keyring_udf`` also supports
  ``keyring_key_remove`` operation.

* ``vault_ca [optional]`` - this variable needs to be specified only when
  Vault's CA certificate is not trusted by the machine that is going to connect
  to the Vault server. In this case this variable should point to CA
  certificate that was used to signed Vault's certificates.

An example of the configuration file looks like this: ::

  vault_url = https://vault.public.com:8202
  secret_mount_point = secret
  token = 58a20c08-8001-fd5f-5192-7498a48eaf20
  vault_ca = /data/keyring_vault_confs/vault_ca.crt

When a key is fetched from a ``keyring`` for the first time ``keyring_vault``
goes to the Vault server and retrieves key's type and data. Next it queries
the Vault server for the key type and data and caches it locally.

Key deletion will permanently delete key from in-memory hash map and the Vault
server.

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

Other reading
-------------

* `Vault Documentation <https://www.vaultproject.io/docs/index.html>`_
* `General-Purpose Keyring Key-Management Functions
  <https://dev.mysql.com/doc/refman/5.7/en/keyring-udfs-general-purpose.html>`_
