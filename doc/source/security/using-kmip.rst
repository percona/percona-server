.. _using-kmip:

==========================================================
Using the Key Management Interoperability Protocol (KMIP)
==========================================================

This feature is **technical preview** quality.

**Percona Server for MySQL** 8.0.27-18 adds support for the `OASIS Key Management Interoperability Protocol (KMIP) <https://docs.oasis-open.org/kmip/kmip-spec/v2.0/os/kmip-spec-v2.0-os.html>`__. This implementation was tested with the `PyKMIP server <https://pykmip.readthedocs.io/en/latest/server.html>`__ and the `HashiCorp Vault Enterprise KMIP Secrets Engine <https://www.vaultproject.io/docs/secrets/kmip>`__.

KMIP enables communication between key management systems and the database server. The protocol can do the following:

* Streamline encryption key management

* Eliminate redundant key management processes

Component installation
--------------------------------------

The KMIP component must be installed with a manifest. A keyring component is not loaded with the ``--early-plugin-load`` option on the server. The server uses a manifest and the component consults its configuration file during initialization. You should only load a keyring component with a manifest file. Do not use the ``INSTALL_COMPONENT`` statement, which loads the keyring components too late in the startup sequence of the server. For example, ``InnoDB`` requires the component, but because the components are registered in the ``mysql.component`` table, this table is loaded after ``InnoDB`` initialization. 

You should create a global manifest file named ``mysqld.my`` in the installation directory and, optionally, create a local manifest file, also named ``mysqld.my`` in a data directory.

To install a keyring component, you must do the following:

1. Write a manifest in a valid JSON format
2. Write a configuration file

A manifest file indicates which component to load. If the manifest file does not exist, the server does not load the component associated with that file. During startup, the server reads the global manifest file from the installation directory. The global manifest file can contain the required information or point to a local manifest file located in the data directory. If you have multiple server instances that use different keyring components use a local manifest file in each data directory to load the correct keyring component for that instance.

.. note:: 

    Enable only one keyring plugin or one keyring component at a time for each server instance. Enabling multiple keyring plugins or keyring components or mixing keyring plugins or keyring components is not supported and may result in data loss.

The following is an example of a global manifest file that does not use local manifests:

.. code-block:: json

    {
     "read_local_manifest": false,
     "components": "file:///component_keyring_kmip"
    }

The following is an example of a global manifest file that points to a local manifest file:

.. code-block:: json

    {
     "read_local_manifest": true
    }

The following is an example of a local manifest file:

.. code-block:: json

    {
     "components": "file:///component_keyring_kmip"
    }

The configuration settings are either in a global configuration file or a local configuration file. The settings are the same. The following **JSON** example of a configuration file.

.. code-block:: json

   {
    "server_addr": "127.0.0.1",
    "server_port": "5696",
    "client_ca": "client_certificate.pem",
    "client_key": "client_key.pem",
    "server_ca": "root_certificate.pem"
   }


For more information, see `Keyring Component installation <https://dev.mysql.com/doc/refman/8.0/en/keyring-component-installation.html>`__ 

