.. _data-masking:

==================================================================
Data Masking
==================================================================

This feature is **Experimental** quality.

This feature was implemented in |Percona Server| version :rn:`8.0.17-8`.

The Percona Data Masking plugin is a free and Open Source implementation of the
|MySQL|'s data masking plugin. Data Masking provides a set of functions to hide
sensitive data with modified content.

.. rubric:: Installing the plugin

The following command installs the plugin:

.. code-block:: bash

    INSTALL PLUGIN data_masking SONAME 'data_masking.so';

The data masking functions are the following:

.. tabularcolumns:: |p{2cm}|p{2cm}|p{2cm}|

.. list-table::
    :widths: 2 3 6
    :header-rows: 1

    * - Type
      - Description
      - Sample
    * - mask_inner()
      - Masks the inner part of a string. The string ends are not masked.
      - .. code-block:: MySQL

            mysql> SELECT mask_inner('123456789', 1, 1);

            +-----------------------------------+
            | mask_inner("123456789", 1, 1)     |
            +-----------------------------------+
            |1XXXXXXX9                          |
            +-----------------------------------+
    * - mask_outer()
      - Masks the outer part of the string. The inner section is not masked.
      - .. code-block:: MySQL

            mysql> SELECT mask_outer('123456789', 2, 2);

            +------------------------------------+
            | mask_outer("123456789", 2, 2).     |
            +------------------------------------+
            | XX34567XX                          |
            +------------------------------------+
    * - mask_pan()
      - Masks the Primary Account Number (PAN) by replacing the
        string with an "X" except for the last four characters.

        .. note::

            The PAN string must be 15 characters or 16 characters in length.

      - .. code-block:: MySQL

            mysql> SELECT mask_pan ('123456789012345');

            +------------------------------------+
            | mask_pan("123456789012345").             |
            +------------------------------------+
            | XXXXXXXXXXX2345                         |
            +------------------------------------+
    * - mask_pan_relaxed()
      - Returns the first six numbers and the last four numbers. The rest of
        the string is replaced by "X".
      - .. code-block:: MySQL

            mysql> SELECT mask_pan_relaxed(gen_rnd_pan());

            +------------------------------------------+
            | mask_pan_relaxed(gen_rnd_pan())          |
            +------------------------------------------+
            | 520754XXXXXX4848                         |
            +------------------------------------------+
    * - mask_ssn()
      - Returns a  string with only the last four numbers visible. The rest
        of the string is
        replaced by "X".

      - .. code-block:: MySQL

            mysql> SELECT mask_ssn('555-55-5555');

            +-------------------------+
            | mask_ssn('555-55-5555') |
            +-------------------------+
            | XXX-XX-5555             |
            +-------------------------+
    * - gen_range()
      - Generates a random number based on a selected range.
      - .. code-block:: MySQL

              mysql> SELECT gen_range(10, 100);

              +--------------------------------------+
              | gen_range(10,100)                    |
              +--------------------------------------+
              | 56                                   |
              +--------------------------------------+
    * - gen_rnd_email()
      - Generates a random email address. The domain is ``example.com``.
      - .. code-block:: MySQL

             mysql> SELECT gen_rnd_email();

             +---------------------------------------+
             | gen_rnd_email()                       |
             +---------------------------------------+
             | sma.jrts@example.com                  |
             +---------------------------------------+
    * - gen_rnd_pan()
      - Generates a random primary account number.
      - .. code-block:: MySQL

              mysql> SELECT mask_pan(gen_rnd_pan());

              +-------------------------------------+
              | mask_pan(gen_rnd_pan())             |
              +-------------------------------------+
              | XXXXXXXXXXXX4444                    |
              +-------------------------------------+
    * - gen_rnd_us_phone()
      - Generates a random U.S. phone number. The generated number adds the
        `1` dialiing code and is in the `555` area code. The `555` area code
        is not valid for any U.S. phone number.
      - .. code-block:: MySQL

            mysql> SELECT gen_rnd_us_phone();

            +-------------------------------+
            | gen_rnd_us_phone()            |
            +-------------------------------+
            | 1-555635-5709                 |
            +-------------------------------+
    * - gen_blacklist(str, dictionary_name, replacement_dictionary_name)
      - Replaces a value with a value from a second dictionary.
          * str: Value to be replaced
          * dictionary_name: Contains the dictionary
          * replacement_dictionary_name: Select a value from this dictionary
      - .. code-block:: MySQL

            mysql> SELECT gen_blacklist('apple', 'fruit', 'nut');

            +-----------------------------------------+
            | gen_blacklist('apple', 'fruit', 'nut')  |
            +-----------------------------------------+
            | walnut                                  |
            +-----------------------------------------+
    * - gen_dictionary(dictionary_name)
      - Returns a random term from the selected dictionary.
      - .. code-block:: MySQL

            mysql> SELECT gen_dictionary(dictionary_name);

            +--------------------------------------------------+
            | gen_dictionary('trees')                          |
            +--------------------------------------------------+
            | Norway spruce                                    |
            +--------------------------------------------------+

.. rubric:: Uninstalling the plugin

The `UNINSTALL PLUGIN <https://dev.mysql.com/doc/refman/8.0/en/uninstall-plugin.html>`_ statement disables and uninstalls the plugin.

.. seealso::
    |MySQL| Documentation
    https://dev.mysql.com/doc/refman/8.0/en/data-masking-reference.html
