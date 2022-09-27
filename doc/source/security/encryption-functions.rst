.. _encryption-functions:

===============================================================================
Encryption functions
===============================================================================

Percona Server for MySQL 8.0.28-20 adds encryption functions and variables to manage the encryption range. The functions may take an algorithm argument. Encryption converts plaintext into ciphertext using a key and an encryption algorithm.

You can also use the user-defined functions with the PEM format keys generated externally by the OpenSSL utility.

A digest uses plaintext and generates a hash value. This hash value can verify if the plaintext is unmodified. You can also sign or verify on digests to ensure that the original plaintext was not modified. You cannot decrypt the original text from the hash value.

When choosing key lengths, consider the following:

* Encryption strength increases with the key size and, also, the key generation time.

* If performance is important and the functions are frequently used, use symmetric encryption. Symmetric encryption functions are faster than asymmetric encryption functions. Moreover, asymmetric encryption has restrictions on the maximum length of a message being encrypted. For example, for `RSA` the algorithm maximum message size is the key length in bytes (key length in bits / 8) minus 11. 

The following table and sections describe the functions. For examples, see :ref:`function-examples`.

.. list-table::
   :header-rows: 1

   * - Function Name
   * - :ref:`asym_decrypt`
   * - :ref:`asym_derive`
   * - :ref:`asym_encrypt`
   * - :ref:`asym_sign`
   * - :ref:`asym_verify`
   * - :ref:`create_priv_key`
   * - :ref:`create_pub_key`
   * - :ref:`create_dh_param`
   * - :ref:`create_digest`

The following table describes the :ref:`encryption_variables` which can be used to set the maximum value for a key length based on the type of encryption.

.. list-table::
   :header-rows: 1

   * - Variable Name
   * - :ref:`dh_threshold`
   * - :ref:`dsa_threshold`
   * - :ref:`rsa_threshold`

.. _install_functions:

Install `component_encryption_udf`
-------------------------------------------------

Use the `Install Component Statement <https://dev.mysql.com/doc/refman/8.0/en/install-component.html>`__ to add the `component_encryption_udf` component. The functions and variables are available. The user-defined functions and the :ref:`encryption_variables` are auto-registered. There is no requirement to invoke ``CREATE FUNCTION ... SONAME ...``.

The ``INSERT`` privilege on the ``mysql.component`` system table is required to run the ``INSTALL COMPONENT`` statement. To register the component, the operation adds a row to this table.

The following is an example of the installation command:

.. code-block:: mysql

    mysql> INSTALL COMPONENT 'file://component_encryption_udf';

.. note::
  
    If you are :ref:`compile_from_source`, the Encryption UDF component is built by default when Percona Server for MySQL is built. Specify the ``-DWITH_ENCRYPTION_UDF=OFF`` cmake option to exclude it.


User-Defined Functions Described
----------------------------------------------------

.. _asym_decrypt:

asymmetric_decrypt(*algorithm, crypt_str, key_str*)
-----------------------------------------------------------

Decrypts an encrypted string using the algorithm and a key string.

.. rubric:: Returns

A plaintext as a string.

.. rubric:: Parameters

The following are the function's parameters:

* algorithm - the encryption algorithm supports `RSA` to decrypt the string.

* key_str - a string in the PEM format. The key string must have the following attributes:

  - Valid

  - Public or private key string that corresponds with the private or public key string used with the :ref:`asymmetric_encrypt<asym_encrypt>` function.

.. _asym_derive:

asymmetric_derive(*pub_key_str, priv_key_str*)
------------------------------------------------------------

Derives a symmetric key using a public key generated on one side and a private key generated on another.

.. rubric:: Returns

A key as a binary string.

.. rubric:: Parameters

The ``pub_key_str`` must be a public key in the PEM format and generated using the Diffie-Hellman (DH) algorithm.

The ``priv_key_str`` must be a private key in the PEM format and generated using the Diffie-Hellman (DH) algorithm.

.. _asym_encrypt:

asymmetric_encrypt(*algorithm, str, key_str*)
--------------------------------------------------------------

Encrypts a string using the algorithm and a key string.

.. rubric:: Returns

A ciphertext as a binary string.

.. rubric:: Parameters

The parameters are the following:

* algorithm - the encryption algorithm supports `RSA` to encrypt the string.

* str - measured in bytes. The length of the string must not be greater than the key_str modulus length in bytes - 11 (additional bytes used for PKCS1 padding)

* key_str - a key (either private or public) in the PEM format

.. _asym_sign:

asymmetric_sign(*algorithm, digest_str, priv_key_str, digest_type*)
------------------------------------------------------------------------

Signs a digest string using a private key string.

.. rubric:: Returns

A signature is a binary string.

.. rubric:: Parameters

The parameters are the following:

* algorithm - the encryption algorithm supports either `RSA` or `DSA` to encrypt the string.

* digest_str - the digest binary string that is signed. Invoking :ref:`create_digest<create_digest>` generates the digest.

* priv_key_str - the private key used to sign the digest string. The key must be in the PEM format.

* digest_type - the supported values are listed in the digest type table of :ref:`create_digest<create_digest>`.

.. _asym_verify:

asymmetric_verify(*algorithm, digest_str, sig_str, pub_key_str, digest_type*)
--------------------------------------------------------------------------------

Verifies whether the signature string matches the digest string.

.. rubric:: Returns

A ``1`` (success) or a ``0`` (failure).

.. rubric:: Parameters

The parameters are the following:

* algorithm - supports either 'RSA' or 'DSA'.

* digest_str - invoking :ref:`create_digest<create_digest>` generates this digest binary string.

* sig_str - the signature binary string. Invoking :ref:`asymmetric_sign<asym_sign>` generates this string.

* pub_key_str - the signer's public key string. This string must correspond to the private key passed to :ref:`asymmetric_sign<asym_sign>` to generate the signature string. The string must be in the PEM format.

* digest_type - the supported values are listed in the digest type table of :ref:`create_digest<create_digest>`

.. _create_priv_key:

create_asymmetric_priv_key(*algorithm, (key_len | dh_parameters)*)
--------------------------------------------------------------------------------

Generates a private key using the given algorithm and key length for RSA or DSA
or Diffie-Hellman parameters for DH. For RSA or DSA, if needed, execute ``KILL
[QUERY|CONNECTION] <id>`` to terminate a long-lasting key generation. The
DH key generation from existing parameters is a quick operation. Therefore, it
does not make sense to terminate that operation with ``KILL``.

.. rubric:: Returns

The key as a string in the PEM format.

.. rubric:: Parameters

The parameters are the following:

* algorithm - the supported values are 'RSA', 'DSA', or 'DH'.

* key_len - the supported key length values are the following:

  - RSA - the minimum length is 1,024. The maximum length is 16,384. 

  - DSA - the minimum length is 1,024. The maximum length is 9,984. 

    .. note:: The key length limits are defined by OpenSSL. To change the maximum key length, use either :ref:`rsa_threshold` or :ref:`dsa_threshold`.

* dh_parameters - Diffie-Hellman (DH) parameters. Invoking :ref:`create_dh_parameter<create_dh_param>` creates the DH parameters.

.. _create_pub_key:

create_asymmetric_pub_key(*algorithm, priv_key_str*)
--------------------------------------------------------------------------

Derives a public key from the given private key using the given algorithm.

.. rubric:: Returns

The key as a string in the PEM format.

.. rubric:: Parameters

The parameters are the following:

* algorithm - the supported values are 'RSA', 'DSA', or 'DH'.

* priv_key_str - must be a valid key string in the PEM format.

.. _create_dh_param:

create_dh_parameters(*key_len*)
--------------------------------------

Creates parameters for generating a Diffie-Hellman (DH) private/public key pair.
If needed, execute ``KILL [QUERY|CONNECTION] <id>`` to terminate the generation of long-lasting parameters.

Generating the DH parameters can take more time than generating the RSA keys or
the DSA keys.
OpenSSL defines the parameter length limits. To change the
maximum parameter length, use :ref:`dh_threshold`.

.. rubric:: Returns

A string in the PEM format and can be passed to :ref:`create_asymmetric_private_key<create_priv_key>`.

.. rubric:: Parameters

The parameters are the following:

* key_len - the range for the key length is from 1024 to 10,000. The default value is 10,000.

.. _create_digest:

create_digest(*digest_type, str*)
--------------------------------------------

Creates a digest from the given string using the given digest type. The digest string can be used with :ref:`asymmetric_sign<asym_sign>` and :ref:`asymmetric_verify<asym_verify>`.

.. rubric:: Returns

The digest of the given string as a binary string

.. rubric:: Parameters

The parameters are the following:

* digest_type - the supported values are the following (based on the OpenSSL version):

  .. list-table::
      :header-rows: 1

      * - Value Name for OpenSSL 1.0.2
        - Value Name for OpenSSL 1.1.x addition
      * - 'MD5'
        - 'BLAKE2B512'
      * - 'SHA1'
        - 'BLAKE2S256'
      * - 'SHA224'
        - 'RIPEMD'
      * - 'SHA256'
        - 'RMD160'
      * - 'SHA384'
        - 'SHAKE128'
      * - 'SHA512'
        - 'SHAKE256'
      * - 'MD4'
        - 'SM3'
      * - 'RIPEMD160'
        - 'WHIRLPOOL'

* str - String used to generate the digest string.

.. _encryption_variables:

.. rubric:: Encryption threshold variables

The maximum key length limits are defined by OpenSSL. Server administrators can limit the maximum key length using the encryption threshold variables. 

The variables are automatically registered when `component_encryption_udf` is installed.


.. list-table::
   :header-rows: 1

   * - Variable Name
   * - :ref:`dh_threshold`

.. _dh_threshold:

.. rubric:: ``encryption_udf.dh_bits_threshold``

The variable sets the maximum limit for the :ref:`create_dh_parameters <create_dh_param>` user-defined function and takes precedence over the OpenSSL maximum length value.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - command-line
     - Yes
   * - scope
     - Global
   * - data type
     - unsigned integer
   * - default
     - 10000

The range for this variable is from 1024 to 10,000. The default value is 10,000.

.. _dsa_threshold:

.. rubric:: `encryption_udf.dsa_bits_threshold`

The variable sets the threshold limits for :ref:`create_asymmetric_priv_key <create_priv_key>` user-defined function when the function is invoked with the `DSA` parameter and takes precedence over the OpenSSL maximum length value.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - command-line
     - Yes
   * - scope
     - Global
   * - data type
     - unsigned integer
   * - default
     - 9984


The range for this variable is from 1,024 to 9,984. The default value is 9,984.

.. _rsa_threshold:

.. rubric:: `encryption_udf.rsa_bits_threshold`

The variable sets the threshold limits for the :ref:`create_asymmetric_priv_key <create_priv_key>` user-defined function when the function is invoked with the `RSA` parameter and takes precedence over the OpenSSL maximum length value.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - command-line
     - Yes
   * - scope
     - Global
   * - data type
     - unsigned integer
   * - default
     - 16384

The range for this variable is from 1,024 to 16,384. The default value is 16,384.

.. _function_examples:

.. rubric:: Examples

Code examples for the following operations: 

* set the threshold variables
* create a private key 
* create a public key
* encrypt data
* decrypt data


.. code-block:: mysql

    -- Set Global variable
    mysql> SET GLOBAL encryption_udf.dh_bits_threshold = 4096;

    -- Set Global variable
    mysql> SET GLOBAL encryption_udf.rsa_bits_threshold = 4096;

.. code-block:: mysql

    -- Create private key
    mysql> SET @private_key = create_asymmetric_priv_key('RSA', 3072);

    -- Create public key
    mysql> SET @public_key = create_asymmetric_pub_key('RSA', @private_key);

    -- Encrypt data using the private key (you can also use the public key)
    mysql> SET @ciphertext = asymmetric_encrypt('RSA', 'This text is secret', @private_key);

    -- Decrypt data using the public key (you can also use the private key)
    -- The decrypted value @plaintext should be identical to the original 'This text is secret'
    mysql> SET @plaintext = asymmetric_decrypt('RSA', @ciphertext, @public_key);

Code examples for the following operations: 

* generate a digest string

* generate a digest signature

* verify the signature against the digest


.. code-block:: mysql

   -- Generate a digest string
   mysql> SET @digest = create_digest('SHA256', 'This is the text for digest');

   -- Generate a digest signature
   mysql> SET @signature = asymmetric_sign('RSA', @digest, @private_key, 'SHA256');

   -- Verify the signature against the digest
   -- The @verify_signature must be equal to 1
   mysql> SET @verify_signature = asymmetric_verify('RSA', @digest, @signature, @public_key, 'SHA256');

Code examples for the following operations:

* generate a DH parameter
* generates two DH key pairs
* generate a symmetric key using the public_1 and the private_2
* generate a symmetric key using the public_2 and the private_1

.. code-block:: mysql

   -- Generate a DH parameter
   mysql> SET @dh_parameter = create_dh_parameters(3072);

   -- Generate DH key pairs
   mysql> SET @private_1 = create_asymmetric_priv_key('DH', @dh_parameter);
   mysql> SET @public_1 = create_asymmetric_pub_key('DH', @private_1);
   mysql> SET @private_2 = create_asymmetric_priv_key('DH', @dh_parameter);
   mysql> SET @public_2 = create_asymmetric_pub_key('DH', @private_2);

  -- Generate a symmetric key using the public_1 and private_2
  -- The @symmetric_1 must be identical to @symmetric_2
  mysql> SET symmetric_1 = asymmetric_derive(@public_1, @private_2);

  -- Generate a symmetric key using the public_2 and private_1
  -- The @symmetric_2 must be identical to @symmetric_1
  mysql> SET symmetric_2 = asymmetric_derive(@public_2, @private_1);

Code examples for the following operations: 

* create a private key using a ``SET`` statement
* create a private key using a  ``SELECT`` statement
* create a private key using an ``INSERT`` statement

.. code-block:: mysql

  mysql> SET @private_key1 = create_asymmetric_priv_key('RSA', 3072);
  mysql> SELECT create_asymmetric_priv_key('RSA', 3072) INTO @private_key2;
  mysql> INSERT INTO key_table VALUES(create_asymmetric_priv_key('RSA', 3072));


.. _uninstall_functions:

Uninstall `component_encryption_udf`
---------------------------------------

You can deactivate and uninstall the component using the `Uninstall Component statement <https://dev.mysql.com/doc/refman/8.0/en/uninstall-component.html>`__.

.. code-block:: mysql

  mysql> UNINSTALL COMPONENT 'file://component_encryption_udf';
