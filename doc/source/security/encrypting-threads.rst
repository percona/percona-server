.. _encrypting-threads:

================================================================================
Working with Advanced Encryption Key Rotation
================================================================================

:Availability: This feature is tech preview.

The Advanced Encryption Key Rotation feature lets you perform specific encryption and
decryption tasks in real-time. 

The following table explains the benefits of Advanced Encryption Key Rotation:

.. list-table::
    :widths: 25 25
    :header-rows: 1
    
    * - Advanced Encryption Key Rotation
      - Master Key Encryption
    * - Encrypts any existing tablespaces in a single operation. Advanced
        Encryption Key Rotation
        allows encryption to be applied to all or selected existing
        tablespaces. You can exclude tablespaces.
      - Encrypts each existing tablespace as a separate operation.
    * - Encrypts tables with a key from a keyring.
      - Encrypts tables with a key that is then stored in the encryption header
        of the
        tablespace.
    * - Re-encrypts each tablespace page by page when the key is rotated.
      - Re-encrypts only the tablespace encryption header when the key is rotated.
   
If you enable Advanced Encryption Key Rotation with a Master key encrypted
tablespace, the tablespace is re-encrypted with the keyring key in a background
process. If the Advanced Encryption Key Rotation feature is enabled, you cannot
convert a tablespace to use Master key encryption. You must disable the feature
before you convert the tablespace.

:Availability: This feature is tech preview quality.

You must have the SYSTEM_VARIABLES_ADMIN privilege or the SUPER privilege to set
these variables.

.. _innodb_encryption_threads:

.. rubric:: ``innodb_encryption_threads``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb-encryption-threads``
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Numeric
   * - Default
     - 0

This variable works in combination with the
:ref:`default_table_encryption` variable set to ``ONLINE_TO_KEYRING``.
This variable
configures the number of threads for background encryption. For the online
encryption, the value must be greater than **zero**. 

.. _innodb_online_encryption_rotate_key_age:

.. rubric:: ``innodb_online_encryption_rotate_key_age``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb-online-encryption-rotate-key-age``
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Numeric
   * - Default
     - 1

Defines the rotation for the re-encryption of a table encrypted using KEYRING.
The value of this variable determines the how frequently the encrypted tables
are re-encrypted.

For example, the following values would trigger a re-encryption in the
following intervals:

*  The value is **1**, the table is re-encrypted on each key rotation. 
*  The value is **2**, the table is re-encrypted on every other key rotation.
*  The value is **10**, the table is re-encrypted on every tenth key rotation.

You should select the value which best fits your operational requirements.

.. _innodb_encryption_rotation_iops:

.. rubric:: ``innodb_encryption_rotation_iops``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb-encryption-rotation-iops``
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Numeric
   * - Default
     - 100

Defines the number of input/output operations per second (iops) available for
use by a key rotation processes.

.. _innodb_default_encryption_key_id:

.. rubric:: ``innodb_default_encryption_key_id``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb-default-encryption-key-id``
   * - Scope
     - Session
   * - Dynamic
     - Yes
   * - Data type
     - Numeric
   * - Default
     - 0
    
Defines the default encryption ID used to encrypt tablespaces.

Using Keyring Encryption
-------------------------------------------

:Availability: This feature is tech preview quality.

Keyring management is enabled for each table, per file table, separately when
you set encryption in the ``ENCRYPTION`` clause to ``KEYRING`` in the supported
SQL statement.

* CREATE TABLE ... ENCRYPTION='KEYRING'
* ALTER TABLE ... ENCRYPTION='KEYRING'

.. note::

    Running an ``ALTER TABLE ... ENCRYPTION='N'`` on a table created with
    ``ENCRYPTION='KEYRING'`` converts the table to the existing MySQL schema,
    tablespace, or table encryption state. 

.. seealso::

    :ref:`using-keyring-plugin`


