.. _data-masking:

==================================================================
Data Masking
==================================================================

This feature was implemented in |Percona Server| version :rn:`5.7.32-35`.

The Percona Data Masking plugin is a free and Open Source implementation of the
MySQL's data masking plugin. Data Masking provides a set of functions to hide
sensitive data with modified content.

Data masking can have either of the characteristics:

* Generation of random data, such as an email address

* De-identify data by transforming the data to hide content

.. rubric:: Installing the plugin

The following command installs the plugin:

.. code-block:: mysql

    mysql> INSTALL PLUGIN data_masking SONAME 'data_masking.so';

.. rubric:: Data Masking functions

The data masking functions have the following categories:

* General purpose

* Special purpose

* Generating Random Data with Defined characteristics

* Using Dictionaries to Generate Random Data

.. rubric:: General Purpose


The general purpose data masking functions are the following:

.. tabularcolumns:: |p{0.25\linewidth}|p{0.25\linewidth}|p{0.50\linewidth}|

.. list-table::
    :widths: 20 30 60
    :header-rows: 1

    * - Parameter
      - Description
      - Sample
    * - mask_inner(string, margin1, margin2 [, character])
      - Returns a result where only the inner part of a string is masked. An
        optional masking character can be specified.
      - .. code-block:: MySQL

            mysql> SELECT mask_inner('123456789', 1, 2);

            +-----------------------------------+
            | mask_inner("123456789", 1, 2)     |
            +-----------------------------------+
            |1XXXXXX89                          |
            +-----------------------------------+

    * - mask_outer(string, margin1, margin2 [, character])

      - Masks the outer part of the string. The inner section is not masked.
      - .. code-block:: MySQL

            mysql> SELECT mask_outer('123456789', 2, 2);

            +------------------------------------+
            | mask_outer("123456789", 2, 2).     |
            +------------------------------------+
            | XX34567XX                          |
            +------------------------------------+

.. rubric:: Special Purpose

The special purpose data masking functions are as follows:

.. tabularcolumns:: |p{0.25\linewidth}|p{0.25\linewidth}|p{0.50\linewidth}|

.. list-table::
    :widths: 20 30 60
    :header-rows: 1

    * - Parameter
      - Description
      - Sample

    * - mask_pan(string)

      - Masks the Primary Account Number (PAN) by replacing the
        string with an "X" except for the last four characters.

        .. note::

            The PAN string must be 15 characters or 16 characters in length.

      - .. code-block:: MySQL

            mysql> SELECT mask_pan ('123456789012345');

            +------------------------------------+
            | mask_pan(gen_rnd_pan()).           |
            +------------------------------------+
            | XXXXXXXXXXX2345                    |
            +------------------------------------+

    * - mask_pan_relaxed(string)
      - Returns the first six numbers and the last four numbers. The rest of
        the string is replaced by "X".

      - .. code-block:: MySQL

            mysql> SELECT mask_pan_relaxed(gen_rnd_pan());

            +------------------------------------------+
            | mask_pan_relaxed(gen_rnd_pan())          |
            +------------------------------------------+
            | 520754XXXXXX4848                         |
            +------------------------------------------+

   * - mask_ssn(string)
      - Returns a string with only the last four numbers visible. The rest
        of the string is replaced by "X".

      - .. code-block:: MySQL

            mysql> SELECT mask_ssn('555-55-5555');

            +-------------------------+
            | mask_ssn('555-55-5555') |
            +-------------------------+
            | XXX-XX-5555             |
            +-------------------------+

.. rubric:: Generating Random Data for Specific Requirements

The following functions generate random values for specific requirements:

.. tabularcolumns:: |p{0.25\linewidth}|p{0.25\linewidth}|p{0.50\linewidth}|

.. list-table::
    :widths: 20 30 60
    :header-rows: 1

    * - Parameter
      - Description
      - Sample
    * - gen_range(lower, upper)
      - Generates a random number based on a selected range and supports 
        negative numbers.

      - .. code-block:: MySQL

              mysql> SELECT gen_range(10, 100) AS result;

              +--------------------------------------+
              | result                               |
              +--------------------------------------+
              | 56                                   |
              +--------------------------------------+

              mysql> SELECT gen_range(100,80);

              +--------------------------------------+
              | gen_range(100,80)                    |
              +--------------------------------------+
              | 91                                   |
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

    * - gen_rnd_pan([size in integer])
      - Generates a random primary account number. This function should only
        be used for test purposes.

      - .. code-block:: MySQL

              mysql> SELECT mask_pan(gen_rnd_pan());

              +-------------------------------------+
              | mask_pan(gen_rnd_pan())             |
              +-------------------------------------+
              | XXXXXXXXXXXX4444                    |
              +-------------------------------------+

    * - gen_rnd_us_phone()
      - Generates a random U.S. phone number. The generated number adds the
        `1` dialing code and is in the `555` area code. The `555` area code
        is not valid for any U.S. phone number.
      - .. code-block:: MySQL

            mysql> SELECT gen_rnd_us_phone();

            +-------------------------------+
            | gen_rnd_us_phone()            |
            +-------------------------------+
            | 1-555635-5709                 |
            +-------------------------------+

    * - gen_rnd_ssn()
      - Generates a random, non-legitimate US Social Security Number in
        an ``AAA-BBB-CCCC`` format. This function should only be used for test
        purposes.
      - .. code-block:: MySQL

          mysql> SELECT gen_rnd_ssn()

          +-----------------------------+
          | gen_rnd_ssn()               |
          +-----------------------------+
          | 995-33-5656                 |
          +-----------------------------+

.. rubric:: Using Dictionaries to Generate Random Terms

Data masking returns a value from a range. To use a predefined file as the range to select a string value, load and use a dictionary. A dictionary supports only strings and is loaded from a file with the following characteristics:

* Plain text

* One term per line

* Must contain at least one entry

An example of a dictionary, which is a list of trees, located in /usr/local/mysql/dict-files/testdict

* Black Ash
* White Ash
* Bigtooth Aspen
* Quaking Aspen

The following table displays the commands for using dictionaries to generate random terms:

.. tabularcolumns:: |p{0.35\linewidth}|p{0.15\linewidth}|p{0.50\linewidth}|

.. list-table::
    :widths: 20 30 60
    :header-rows: 1

    * - Parameter
      - Description
      - Sample
    * - gen_dictionary_load(dictionary path, dictionary name)
      - Load a file into the dictionary registry and configures the dictionary
        name. The name can be used with any function. If the dictionary is
        edited, you must drop and then reload the dictionary to view the changes.
        Returns either success or failure.

      - .. code-block:: MySQL

             mysql> SELECT gen_dictionary_load('/usr/local/mysql/dict-files/testdict', 'testdict');

             +------------------------------------------------------------------------+
             | gen_dictionary_load('/usr/shared/mysql/dict-files/fndict', 'fndict')   |
             +========================================================================+
             | Dictionary load successfully                                           |
             +------------------------------------------------------------------------+
    * - gen_dictionary(dictionary_name)
      - Returns a random term from the selected dictionary.

      - .. code-block:: MySQL

            mysql> SELECT gen_dictionary('trees');

            +--------------------------------------------------+
            | gen_dictionary('trees')                          |
            +--------------------------------------------------+
            | Norway spruce                                    |
            +--------------------------------------------------+
    * - gen_blacklist(str, dictionary_name, replacement_dictionary_name)
      - Replaces a term with a term from a second dictionary.

      - .. code-block:: MySQL

            mysql> SELECT gen_blacklist('apple', 'fruit', 'nut');

            +-----------------------------------------+
            | gen_blacklist('apple', 'fruit', 'nut')  |
            +-----------------------------------------+
            | walnut                                  |
            +-----------------------------------------+

    * - gen_dictionary_drop(dictionary_name)
      - Removes the selected dictionary from the dictionary registry. Returns
        either success or failure.

      - .. code-block:: MySQL

          mysql> SELECT gen_dictionary_drop('mytestdict')

          +-------------------------------------+
          | gen_dictionary_drop('mytestdict')   |
          +-------------------------------------+
          | Dictionary removed                  |
          +-------------------------------------+



.. rubric:: Uninstalling the plugin

The `UNINSTALL PLUGIN <https://dev.mysql.com/doc/refman/5.7/en/uninstall-plugin.html>`_ statement disables and uninstalls the plugin.

