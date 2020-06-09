.. _vault:

================================================================================
Information about HashiCorp Vault
================================================================================

The ``keyring_vault`` plugin can store the encryption keys inside the `HashiCorp
Vault <https://www.hashicorp.com/products/vault/data-protection>`_.

.. important::

   The ``keyring_vault`` plugin works with the *kv secrets engine* (version 1).

   .. seealso::

      HashiCorp Documentation:
         - `Installing Vault <https://www.vaultproject.io/docs/install/index.html>`_
         - `KV Secrets Engine - Version 1 <https://www.vaultproject.io/docs/secrets/kv/kv-v1.html>`_

      Production Hardening
         https://learn.hashicorp.com/vault/operations/production-hardening

.. admonition:: Related information
   
   - :ref:`using-keyring-plugin`
   - :ref:`rotating-master-key`

.. _vault/custom-server.setting-up:

Setting up a custom Vault server
================================================================================

The ``keyring_vault`` plugin interacts with a |vault-server| to save and retrieve
encryption keys. On |vault-server|, all keys are stored securely and under
strict control. This section provides a general overview of how to set up custom
server that you can experiment with.

.. _vault/custom-server.setting-up/installing:

Installing the ``vault`` command
--------------------------------------------------------------------------------

In order to use |vault-server| features, you need to install the |command.vault|
provided by |hashicorp-vault|. The installation procedure is straightforward:

1. Download the installation package from the |hashicorp-vault| site and unzip
   it.
#. Place the ``vault`` binary to a location in your `PATH`.
#. Run the ``vault`` command (:ref:`Command not found instead of the usage
   information <vault/tip/command-not-found>`):

   .. code-block:: bash

      $ vault

      Usage: vault <command> [args]

      Common commands:
         read        Read data and retrieves secrets
         write       Write data, configuration, and secrets
         delete      Delete secrets and configuration
         list        List data or secrets
         login       Authenticate locally
         agent       Start a Vault agent
         server      Start a Vault server
         status      Print seal and HA status
         unwrap      Unwrap a wrapped secret
      ...

.. _vault/tip/command-not-found:

.. tip:: Command not found instead of the usage information

   If you receive a ``command not found` message instead, then ``vault`` is not
   in your `PATH`. To check if the directory with `vault` is included, run:

   $ echo $PATH

.. seealso:: `Vault documentation: Installing Vault
	     <https://learn.hashicorp.com/vault/getting-started/install>`_
   
.. _vault/custom-server.setting-up/starting:

Starting the Vault server
--------------------------------------------------------------------------------

There are two options to start the Vault server. In dev mode, you can use it for
experimentation: 

.. code-block:: bash

   $ vault server -dev

Run this way, |vault| features automatic authentication and in-memory
storage. |vault-server| is not suitable for production use in this mode.

Alternatively, you can *deploy* |vault-server| in your environment. For
deployment, you use *hcl* configuration files (custom format similar to JSON
developed by |hashicorp-vault|) where you can set values of multiple
|vault-server| options.

.. code-block:: text

   storage "consul" {
   address = "127.0.0.1:8500"
   path    = "vault/"
   disable_mlock = true
   }

   listener "tcp" {
   address     = "127.0.0.1:8200"
   tls_disable = 1
   disable_mlock = true
   }

.. seealso::

   |hashicorp-vault| Documentation:
      - `Deploying Vault server <https://learn.hashicorp.com/vault/getting-started/deploy>`_
      - `Vault server configuration options <https://www.vaultproject.io/docs/configuration>`_

To replace the *inmemory* storage used in *dev* mode, you may use |consul|, which
provides a distributed key-value storage. |consul| is installed separately. The
installation procedure for |consul| is similar to that for |vault|: download the
installation package for your system and copy the |file.consul| binary to a
directory in the ``PATH`` environment variable. Then, start |command.consul|:

.. warning::

   This section gives an overview of the steps required to set up an
   experimental environment. It is by no means a guide to set up a production
   ready environment.

.. code-block:: bash

   $ consul agent -dev

.. seealso::

   |hashicorp-vault| Documentation:
      - `Set up Consul to use encryption with the Gossip protocol
        <https://www.consul.io/docs/agent/encryption.html>`_
      - `More information about the Gossip protocol with Consul <https://www.consul.io/docs/internals/gossip.html>`_
      - `TLS configuration for Consul <https://www.vaultproject.io/docs/configuration/listener/tcp#tcp-listener>`_



Now, you can start |vault-server| passing the configuration file that you have
prepared as the value of the `-config` parameter:

.. code-block:: bash

   $ vault server -config=my-conf.hcl

For each new storage backend, you need to initialize |vault-server|. The
initialized storage is sealed, however.

.. code-block:: bash

   $ # Hint: Initialing a backend storage;
   $ #       it will have to be unsealed at the next stage
   $ vault operator init
   Unseal Key 1: S6eWP5jRxKBHElx2oswxfkeZwwH/XWg5Ei1LaHt9GcK4
   Unseal Key 2: Tnd+kJEeaO1yOIYTjFPgqSd8/dyUkprZw1zAYa62ISAs
   Unseal Key 3: I/kdckz2ELaWSeQS/iL+EffsBnbmMVyoI+T1DrFX5ATM
   Unseal Key 4: iQBHfQPtRqzPQYlBYpCtKeVWrE5C/OxB5O5jZDGYXEgz
   Unseal Key 5: E5j8VdttAxC+b/6tARAJRybYfyS1AiDqq5D/wJ4QcEbo

   Initial Root Token: s.oM1h6Inq6WOdppd5sML1gmpc
   ...

The storage is initialized but it is sealed. To be able to store encryption
keys, the storage must be unsealed. The output of `vault operator init` command
is very important for this purpose as it lists unseal keys. By default, there
five such keys and you need to provide, one by one, three of them. Here, you *do
not* pass them as parameters on the command line (which is highly insecure as
full command can be saved by your terminal emulator, such as `bash`):

.. code-block:: bash

   $ vault operator unseal

When prompted, you enter one of the unseal keys. |vault-server| saves its state
after each key is successfully entered. This means that every next key can be
provided at a later time or by a different user with sufficient permissions or,
even, from another computer as long as its configuration file points to the same
server.

.. code-block:: text
   :emphasize-lines: 10

   $ # Hint: Successfully entered key
   Unseal Key (will be hidden): 
   Key                Value
   ---                -----
   Seal Type          shamir
   Initialized        true
   Sealed             true
   Total Shares       5
   Threshold          3
   Unseal Progress    2/3
   Unseal Nonce       417d1e72-3698-a5a1-97a4-c9aafdde25ff
   Version            1.4.2
   HA Enabled         true

Note the line `Usealed Progress` which shows how many unseal keys have been
successfully provided so far. |vault-server| unseals the backend storage as soon
as you provide all unseal keys.

At this point, |vault-server| is ready for the `keyring_vault` plugin to store encryption keys.

.. seealso::

   |hashicorp-vault| Documentation:
      `Deploying Vault <https://learn.hashicorp.com/vault/getting-started/deploy>`_

   |percona| Database Performance Blog
      `Using the keyring_vault Plugin with Percona Server for MySQL 5.7
      <https://www.percona.com/blog/2018/09/17/using-the-keyring_vault-plugin-with-percona-server-for-mysql-5-7/>`_

.. admonition:: Acknowledgements

   This material is based on https://learn.hashicorp.com

.. |hashicorp-vault| replace:: Hashicorp Vault
.. |vault| replace:: vault
.. |command.vault| replace:: ``vault``
.. |vault-server| replace:: Vault server
.. |consul| replace:: Consul
.. |file.consul| replace:: :file:`consul`
.. |command.consul| replace:: ``consul``
