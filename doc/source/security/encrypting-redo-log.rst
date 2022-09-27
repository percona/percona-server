.. _encrypting-redo-log:

================================================================================
Encrypting the Redo Log files
================================================================================

MySQL uses the redo log files to apply changes during data recovery.

Encrypt the redo log files by enabling the :ref:`innodb_redo_log_encrypt`
variable. The default value for the variable is ``OFF``.

The Redo log files uses the tablespace encryption key.

.. _innodb_redo_log_encrypt:

.. rubric:: ``innodb_redo_log_encrypt``
 
.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb-redo-log-encrypt``
   * - Dynamic
     - Yes
   * - Scope
     - Global
   * - Data type
     - Text
   * - Default
     - OFF

Determines the encryption for redo log data for tables.

When you enable `innodb_redo_log_encrypt` any existing redo log pages stay
unencrypted, and new pages are encrypted when they are written to disk. If you
disable `innodb_redo_log_encrypt` after enabling the variable, any encrypted pages remain encrypted, but the new pages are unencrypted.

As implemented in :ref:`8.0.16-7`, the supported values for
:ref:`innodb_redo_log_encrypt` are the following:

* ON

* OFF

* master_key

* keyring_key

The ``keyring_key`` value is in tech preview.

.. seealso::

   For more information on the keyring_key - :ref:`encrypting-threads`

.. note::

    For `innodb_redo_log_encrypt`, the "ON" value is a compatibility alias for
    master_key.

After starting the server, an attempt to encrypt the redo log files fails
if you have the following conditions:

    * Server started with no keyring specified

    * Server started with a keyring, but you specified a redo
      log encryption method that is different then previously used
      method on the server.
      
.. seealso::

    :ref:`encrypting-tables`

    :ref:`encrypting-tablespaces`

