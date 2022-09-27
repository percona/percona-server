.. _rotating-master-key:

==============================================================================
Rotating the Master Key
==============================================================================

The Master key should be periodically rotated. You should rotate the key if you
believe the key has been compromised. The Master key rotation changes the Master
key and tablespace keys are re-encrypted and updated in the tablespace headers.
The operation does not affect tablespace data.

If the master key rotation is interrupted, the rotation operation is rolled
forward when the server restarts. InnoDB reads the encryption data from the
tablespace header, if certain tablespace keys have been encrypted with the prior
master key, InnoDB retrieves the master key from the keyring to decrypt the
tablespace key. InnoDB re-encrypts the tablespace key with the new Master key.

To allow for Master Key rotation, you can encrypt an already encrypted InnoDB
system tablespace with a new master key by running the following ``ALTER
INSTANCE`` statement:

.. code-block:: mysql

   mysql> ALTER INSTANCE ROTATE INNODB MASTER KEY;

The rotation operation must complete before any tablespace encryption operation
can begin.

.. note::

    The rotation re-encrypts each tablespace key. The tablespace key is not
    changed. If you want to change a tablespace key, you should disable and then
    re-enable encryption.
