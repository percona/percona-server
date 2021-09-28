.. _csv_engine_mode:

================================================================
 CSV engine mode for standard-compliant quote and comma parsing
================================================================

`MySQL CSV Storage Engine <https://dev.mysql.com/doc/refman/5.7/en/csv-storage-engine.html>`_ is non-standard with respect to embedded ``"`` and ``,`` character parsing. Fixing this issue unconditionally would break MySQL CSV format compatibility for any pre-existing user tables and for data exchange with other MySQL instances, but it would improve compatibility with other CSV producing/consuming tools.

To keep both MySQL and other tool compatibility, a new dynamic, global/session server variable :variable:`csv_mode` has been implemented. This variable allows an empty value (the default), and ``IETF_QUOTES``. 

If ``IETF_QUOTES`` is set, then embedded commas are accepted in quoted fields as-is, and a quote character is quoted by doubling it. In legacy mode embedded commas terminate the field, and quotes are quoted with a backslash.

Example
=======

Table: 

.. code-block:: mysql

      > CREATE TABLE albums (
      `artist` text NOT NULL,
      `album` text NOT NULL
      ) ENGINE=CSV DEFAULT CHARSET=utf8
      ;

Following example shows the difference in parsing for default and ``IETF_QUOTES`` :variable:`csv_quotes`. 

.. code-block:: mysql

    > INSERT INTO albums VALUES ("Great Artist", "Old Album"),
    ("Great Artist", "Old Album \"Limited Edition\"");  

If the variable :variable:`csv_mode` is set to empty value (default) parsed data will look like: :: 

  "Great Artist","Old Album"
  "Great Artist","\"Limited Edition\",Old Album"

If the variable :variable:`csv_mode` is set to ``IETF_QUOTES`` parsed data will look like as described in `CSV rules <http://en.wikipedia.org/wiki/Comma-separated_values#Basic_rules_and_examples>`_: :: 

   "Great Artist","Old Album"
   "Great Artist","""Limited Edition"",Old Album"

Parsing the CSV file which has the proper quotes (shown in the previous example) can show different results:

With :variable:`csv_mode` set to empty value, parsed data will look like:

.. code-block:: mysql

   > SELECT * FROM albums;
   +--------------+--------------------+
   | artist       | album              |
   +--------------+--------------------+
   | Great Artist | Old Album          |
   | Great Artist | ""Limited Edition" |
   +--------------+--------------------+
   2 rows in set (0.02 sec)

With :variable:`csv_mode` set to ``IETF_QUOTES`` parsed data will look like: 

.. code-block:: mysql

   mysql> SET csv_mode = 'IETF_QUOTES';
   Query OK, 0 rows affected (0.00 sec)

.. code-block:: mysql

   > SELECT * FROM albums;
   +--------------+-----------------------------+
   | artist       | album                       |
   +--------------+-----------------------------+
   | Great Artist | Old Album                   |
   | Great Artist | "Limited Edition",Old Album |
   +--------------+-----------------------------+


Version Specific Information
============================

  * :rn:`5.7.10-1`:
    Feature ported from |Percona Server| 5.6

System Variables
================

.. variable:: csv_mode

     :cli: Yes
     :conf: Yes
     :scope: Global, Session
     :dyn: Yes
     :vartype: SET
     :default: ``(empty string)``
     :range: ``(empty string)``, ``IETF_QUOTES``

Setting this variable is to ``IETF_QUOTES`` will enable the standard-compliant quote parsing: commas are accepted in quoted fields as-is, and quoting of ``"`` is changed from ``\"`` to ``""``. If the variable is set to empty value (the default), then the old parsing behavior is kept.

Related Reading
===============

  * `MySQL bug #71091 <http://bugs.mysql.com/bug.php?id=71091>`_

