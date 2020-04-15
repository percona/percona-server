.. _encrypting-redo-log:

================================================================================
Encrypting the Redo Log
================================================================================

The Redo log can be encrypted with the :variable: `innodb_redo_log_encrypt`
variable. The default value for the variable is ``OFF``. The Redo log uses the
tablespace encryption key.

.. variable:: innodb_redo_log_encrypt

    :cli:  ``--innodb-redo-log-encrypt``
    :dyn: Yes
    :scope: Global
    :vartype: Text
    :default: OFF

Determines the encryption for redo log data for tables. The encryption of redo
log data, by default, is 'OFF'.  

When you enable `innodb_redo_log_encrypt` any existing redo log pages stay
unencrypted, and new pages are encrypted when they are written to disk. If you
disable `innodb_redo_log_encrypt`, any encrypted pages remain encrypted, but
new pages are unencrypted.

As implemented in :rn:`8.0.16-7`, the supported values for :variable:
`innodb_redo_log_encrypt` are the following:

* ON

* OFF

* master_key

* keyring_key

The keyring_key is an **Experimental** value.

.. seealso::

   For more information on the keyring_key - :ref:`encrypting-threads`

.. note::

    For `innodb_redo_log_encrypt`, the "ON" value is a compatibility alias for
    master_key.

After starting the server, an attempt to encrypt the redo log fails in the
following conditions:

    * Server started with no keyring specified

    * Server started with a keyring, but you have specified a different redo
      log encryption method that what the same server previously used.

.. seealso::

    :ref:`encrypting-tables`

    :ref:`encrypting-tablespaces`

