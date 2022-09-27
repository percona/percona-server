.. _undo-tablespace-encryption:

================================================================================
Encrypting the Undo Tablespace
================================================================================

The undo data may contain sensitive information about the database operations.

You can encrypt the data in an undo log using the
:ref:`innodb_undo_log_encrypt` option. You can change the setting for this variable
in the configuration file, as a startup parameter, or during runtime as a global
variable. The undo data encryption must be enabled; the feature
is disabled by default.

.. _innodb_undo_log_encrypt:

.. rubric:: ``innodb_undo_log_encrypt``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb_undo-log_encrypt``
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Boolean
   * - Default
     - OFF

Defines if an undo log data is encrypted. The default for the undo log is
"OFF", which disables the encryption.

You can create up to 127 undo tablespaces and you can, with the server
running, add or reduce the number of undo tablespaces.

.. note::

    If you disable encryption, any encrypted undo data remains encrypted. To
    remove this data, truncate the undo tablespace.

.. seealso::

  *MySQL* Documentation
  
    `innodb_undo_log_encrypt
    <https://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html#sysvar_innodb_undo_log_encrypt>`__

How to Enable Encryption on an Undo Log
----------------------------------------

You enable encryption for an undo log by adding the following to the my.cnf
file:

.. code-block:: MySQL

  [mysqld]
  innodb_undo_log_encrypt=ON

.. seealso::

    :ref:`encrypting-redo-log`
