.. _using-amz-kms:

================================================================================
Using the Amazon Key Management Service (AWS KMS)
================================================================================

This feature is **technical preview** quality.

**Percona Server for MySQL** 8.0.28-20 adds support for the `Amazon Key
Management Server (AWS KMS) <https://aws.amazon.com/kms/>`__. Percona Server
generates the keyring keys. Amazon Web Services (AWS) encrypts the keyring data.

The AWS KMS lets you create and manage cryptographic keys across AWS services. For more information, see the
`AWS Key Management Service Documentation <https://docs.aws.amazon.com/kms/>`__.

To use the AWS KMS component, do the following:

* Have an AWS user account. This account has an access key and a secret key.
* Create a KMS key ID. The KMS key can then be referenced in the configuration
  either by its ID, alias (the key can have any number of aliases), or ARN.


Component installation
--------------------------------------

You should only load the AWS KMS component with a manifest file. The server uses
this manifest file and the
component consults its configuration file during initialization.

For more information, see `Installing and Uninstalling Components
<https://dev.mysql.com/doc/refman/8.0/en/component-loading.html>`__

You should create a global manifest file named ``mysqld.my`` in the installation directory and, optionally, create a local manifest file, also named ``mysqld.my`` in a data directory.

To install a KMS component, do the following:

1. Write a manifest in a valid JSON format
2. Write a configuration file

A manifest file indicates which component to load. The server does not load the
component if the manifest file associated with the component does not exist.
During startup, the server reads the global manifest file from the installation
directory. The global manifest file can contain the required information or
point to a local manifest file located in the data directory. If you have
multiple server instances that use different keyring components, use a local
manifest file in each data directory to load the correct keyring component for that instance.

.. note:: 

    Enable only one keyring plugin or one keyring component at a time for each server instance. Enabling multiple keyring plugins or keyring components or mixing keyring plugins or keyring components is not supported and may result in data loss.

The following example is a global manifest file that does not use local
manifests:

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

The configuration settings are either in a global configuration file or a local
configuration file. The settings are the same.

The KMS configuration file has the following options:

* read_local_config

* path - the location of the JSON keyring database file.

* read_only - if true, the keyring cannot be modified.

* kms_key - the identifier of an AWS KMS master key. This key must be created by
  the user before creating the manifest file. The identifier can be one of the
  following:

  * UUID
  * Alias
  * ARN

  For more information, see `Finding the key ID and key ARN
  <https://docs.aws.amazon.com/kms/latest/developerguide/find-cmk-id-arn.html>`__.

* region - the AWS where the KMS is stored. Any HTTP request connect to this
  region.

* auth_key - an AWS user authentication key. The user must have access to the
  KMS key.

* secret_access_key - the secret key (API "password") for the AWS user.


.. note::

    The configuration file contains authentication information. Only the
    MySQL process should be able to read this file.

The following **JSON** is an example of a configuration file:


.. code-block:: json

    {
     "read_local_config": "true/false",
     "path": "/usr/local/mysql/keyring-mysql/aws-keyring-data",
     "region": "eu-central-1",
     "kms_key": "UUID, alias or ARN as displayed by the KMS console",
     "auth_key": "AWS user key",
     "secret_access_key": "AWS user secret key"
    }


For more information, see `Keyring Component installation <https://dev.mysql.com/doc/refman/8.0/en/keyring-component-installation.html>`__ 

