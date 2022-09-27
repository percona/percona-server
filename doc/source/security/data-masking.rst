.. _data-masking:

.. highlight:: mysql

==================================================================
Data Masking
==================================================================

This feature was implemented in *Percona Server for MySQL* version :ref:`8.0.17-8`.

The Percona Data Masking plugin is a free and Open Source implementation of the
*MySQL*'s data masking plugin. Data Masking provides a set of functions to hide
sensitive data with modified content.

Data masking can have either of the characteristics:

* Generation of random data, such as an email address

* De-identify data by transforming the data to hide content

.. rubric:: Installing the plugin

The following command installs the plugin::

    $ INSTALL PLUGIN data_masking SONAME 'data_masking.so';

.. rubric:: Data Masking functions

The data masking functions have the following categories:

* General purpose

* Special purpose

* Generating Random Data with Defined characteristics

* Using Dictionaries to Generate Random Data

.. rubric:: General Purpose

The general purpose data masking functions are the following:

.. tabularcolumns:: |p{3cm}|p{3cm}|p{8.5cm}|
.. list-table::
    :widths: 2 2 6
    :header-rows: 1

    * - Parameter
      - Description
      - Sample
    * - mask_inner(string, margin1, margin2 [, character])
      - Returns a result where only the inner part of a string is masked. An optional masking character can be specified.
      - ::

            mysql> SELECT mask_inner('123456789', 1, 2);
            +-----------------------------------+
            | mask_inner('123456789', 1, 2)     |
            +-----------------------------------+
            |1XXXXXX89                          |
            +-----------------------------------+

    * - mask_outer(string, margin1, margin2 [, character])
      - Masks the outer part of the string. The inner section is not masked.
      - ::

            mysql> SELECT mask_outer('123456789', 2, 2);
            +------------------------------------+
            | mask_outer('123456789', 2, 2).     |
            +------------------------------------+
            | XX34567XX                          |
            +------------------------------------+

.. rubric:: Special Purpose

The special purpose data masking functions are as follows:

.. tabularcolumns:: |p{3cm}|p{3cm}|p{8.5cm}|
.. list-table::
    :class: longtable
    :widths: 2 3 6
    :header-rows: 1

    * - Parameter
      - Description
      - Sample
    * - mask_pan(string)
      - Masks the Primary Account Number (PAN) by replacing the
        string with an "X" except for the last four characters. The PAN string must be 15 characters or 16 characters in length.
      - ::

            mysql> SELECT mask_pan (gen_rnd_pan());
            +------------------------------------+
            | mask_pan(gen_rnd_pan()).           |
            +------------------------------------+
            | XXXXXXXXXXX2345                    |
            +------------------------------------+

    * - mask_pan_relaxed(string)
      - Returns the first six numbers and the last four numbers. The rest of
        the string is replaced by "X".
      - ::

            mysql> SELECT mask_pan_relaxed(gen_rnd_pan());
            +------------------------------------------+
            | mask_pan_relaxed(gen_rnd_pan())          |
            +------------------------------------------+
            | 520754XXXXXX4848                         |
            +------------------------------------------+

    * - mask_ssn(string)
      - Returns a string with only the last four numbers visible. The rest
        of the string is replaced by "X".
      - ::

            mysql> SELECT mask_ssn('555-55-5555');
            +-------------------------+
            | mask_ssn('555-55-5555') |
            +-------------------------+
            | XXX-XX-5555             |
            +-------------------------+

.. rubric:: Generating Random Data for Specific Requirements

These functions generate random values for specific requirements.

.. tabularcolumns:: |p{3cm}|p{3cm}|p{8.5cm}|
.. list-table::
    :class: longtable
    :widths: 2 3 6
    :header-rows: 1

    * - Parameter
      - Description
      - Sample
    * - gen_range(lower, upper)
      - Generates a random number based on a selected range and supports negative numbers.
      - ::

              mysql> SELECT gen_range(10, 100);
              +--------------------------------------+
              | gen_range(10,100)                    |
              +--------------------------------------+
              | 56                                   |
              +--------------------------------------+

              mysql> SELECT gen_range(-100,-80);
              +--------------------------------------+
              | gen_range(-100,-80)                    |
              +--------------------------------------+
              | -91                             |
              +--------------------------------------+

    * - gen_rnd_email()
      - Generates a random email address. The domain is ``example.com``.
      - ::

             mysql> SELECT gen_rnd_email();
             +---------------------------------------+
             | gen_rnd_email()                       |
             +---------------------------------------+
             | sma.jrts@example.com                  |
             +---------------------------------------+

    * - gen_rnd_pan([size in integer])
      - Generates a random primary account number. This function should only be used for test purposes.
      - ::

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
      - ::

            mysql> SELECT gen_rnd_us_phone();
            +-------------------------------+
            | gen_rnd_us_phone()            |
            +-------------------------------+
            | 1-555-635-5709                |
            +-------------------------------+

    * - gen_rnd_ssn()
      - Generates a random, non-legitimate US Social Security Number in an ``AAA-BBB-CCCC`` format. This function should only be used for test purposes.
      - ::

          mysql> SELECT gen_rnd_ssn()
          +-----------------------------+
          | gen_rnd_ssn()               |
          +-----------------------------+
          | 995-33-5656                 |
          +-----------------------------+

.. rubric:: Using Dictionaries to Generate Random Terms

Use a selected dictionary to generate random terms. The dictionary must be loaded from a file with the following characteristics:

* Plain text

* One term per line

* Must contain at least one entry

Copy the dictionary files to a directory accessible to MySQL. The `secure-file-priv <https://dev.mysql.com/doc/refman/8.0/en/server-system-variables.html#sysvar_secure_file_priv>`_ option defines the directories where gen_dictionary_load() loads the dictionary files.

.. note::

    |Percona Server| 8.0.21-12 enabled using the ``secure-file-priv`` option for `gen_dictionary_load()`.
    
    
.. tabularcolumns:: |p{4cm}|p{6cm}|p{6cm}|p{9cm}|
.. list-table::
    :class: longtable
    :widths: 2 3 3 6
    :header-rows: 1

    * - Parameter
      - Description
      - Returns
      - Sample
    * - gen_blacklist(str, dictionary_name, replacement_dictionary_name)
      - Replaces a term with a term from a second dictionary.
      - A dictionary term
      - ::

            mysql> SELECT gen_blacklist('apple', 'fruit', 'nut');
            +-----------------------------------------+
            | gen_blacklist('apple', 'fruit', 'nut')  |
            +-----------------------------------------+
            | walnut                                  |
            +-----------------------------------------+

    * - gen_dictionary(dictionary_name)
      - Randomizes the dictionary terms
      - A random term from the selected dictionary.
      - ::

            mysql> SELECT gen_dictionary('trees');
            +--------------------------------------------------+
            | gen_dictionary('trees')                          |
            +--------------------------------------------------+
            | Norway spruce                                    |
            +--------------------------------------------------+

    * - gen_dictionary_drop(dictionary_name)
      - Removes the selected dictionary from the dictionary registry. 
      - Either success or failure
      - ::

          mysql> SELECT gen_dictionary_drop('mytestdict')
          +-------------------------------------+
          | gen_dictionary_drop('mytestdict')   |
          +-------------------------------------+
          | Dictionary removed                  |
          +-------------------------------------+

    * - gen_dictionary_load(dictionary path, dictionary name)
      - Loads a file into the dictionary registry and configures the dictionary name. The name can be used with any function. If the dictionary is edited, you must drop and then reload the dictionary to view the changes.
      - Either success or failure
      - ::

          mysql> SELECT gen_dictionary_load('/usr/local/mysql/dict-files/testdict', 'testdict');
          +-------------------------------------------------------------------------------+
          | gen_dictionary_load('/usr/local/mysql/mysql/dict-files/testdict', 'testdict') |
          +-------------------------------------------------------------------------------+
          | Dictionary load successfully                                                  |
          +-------------------------------------------------------------------------------+

.. rubric:: Uninstalling the plugin

The `UNINSTALL PLUGIN <https://dev.mysql.com/doc/refman/8.0/en/uninstall-plugin.html>`_ statement disables and uninstalls the plugin.

.. seealso::
    *MySQL* Documentation
    https://dev.mysql.com/doc/refman/8.0/en/data-masking-reference.html
    https://dev.mysql.com/doc/refman/8.0/en/data-masking-functions.html
