.. _sequence_table:

===============================================================================
SEQUENCE_TABLE(n) Function
===============================================================================

*Percona Server for MySQL* 8.0.20-11 adds the SEQUENCE_TABLE() function.

A sequence of numbers can be defined as an arithmetic progression when the common difference between two consecutive terms is always the same.

The function is an inline table-valued function. A single SELECT statement generates a multi-row result set. In contrast, a scalar function (like `EXP(x) <https://dev.mysql.com/doc/refman/8.0/en/mathematical-functions.html#function_exp>`_ or `LOWER(str) <https://dev.mysql.com/doc/refman/8.0/en/string-functions.html#function_lower>`_ always  returns a single value of a specific data type.

The `JSON_TABLE() <https://dev.mysql.com/doc/refman/8.0/en/json-table-functions.html>`_ is the only table function available in Oracle MySQL Server. ``JSON_TABLE`` and ``SEQUENCE_TABLE()`` are the only table functions available in Percona Server.

The basic syntax is the following:

* SEQUENCE_TABLE(n) [AS] alias
 
  *Usage*:
  SELECT ... FROM SEQUENCE_TABLE(n) [AS] alias

    .. code-block:: mysql

        SEQUENCE_TABLE(
            n
        ) [AS] alias
    
``n:`` The number of generated values.

As with any `derived tables <https://dev.mysql.com/doc/refman/8.0/en/derived-tables.html>`_, a table function requires an `alias <https://dev.mysql.com/doc/refman/8.0/en/identifiers.html>`_ in the ``SELECT`` statement. 

The result set is a single column with the predefined column name ``value`` of type ``BIGINT UNSIGNED``. You can reference the ``value`` column in ``SELECT`` statements. The following statements are valid. ::

    SELECT * FROM SEQUENCE_TABLE(n) AS tt;
    SELECT <expr(value)> FROM SEQUENCE_TABLE(n) AS tt;

The first number in the series, the initial term, is defined as ``0`` and the series ends with a value less then ``n``. In this example, enter the following statement to generate a sequence:

..  code-block:: mysql

    mysql> SELECT * FROM SEQUENCE_TABLE(3) AS tt;
    +-------+
    | value |
    +-------+
    |     0 |
    |     1 |
    |     2 |
    +-------+
    
You can define  the initial term using the ``WHERE`` clause. The following example starts the sequence with ``4``.

.. code-block:: mysql

    SELECT value AS result FROM SEQUENCE_TABLE(8) AS tt WHERE value >= 4;
    +--------+
    | result |
    +--------+
    |      4 |
    |      5 |
    |      6 |
    |      7 |
    +--------+
    
Consecutive terms increase or decrease by a common difference. The default common difference value is ``1``. However, it is possible to filter the results using the WHERE clause to simulate common differences greater than 1.

The following example prints only even numbers from the 0..7 range:

.. code-block:: mysql

    SELECT value AS result FROM SEQUENCE_TABLE(8) AS tt WHERE value % 2 = 0;
    +--------+
    | result |
    +--------+
    |      0 |
    |      2 |
    |      4 |
    |      6 |
    +--------+

The following is an example of using the function to populate a table with a set of random numbers:

.. code-block:: mysql

    mysql> SELECT FLOOR(RAND()) * 100) AS result FROM SEQUENCE_TABLE(4) AS tt;
    +--------+
    | result |
    +--------+
    |     24 |
    |     56 |
    |     70 |
    |     25 |
    +--------+

You can populate a table with a set of pseudo-random strings with the following statement:

.. code-block:: mysql

    mysql> SELECT MD5(value) AS result FROM SEQUENCE_TABLE(4) AS tt;
    +----------------------------------+
    | result                           |
    +----------------------------------+
    | f17d9c990f40f8ac215f2ecdfd7d0451 |
    | 2e5751b7cfd7f053cd29e946fb2649a4 |
    | b026324c6904b2a9cb4b88d6d61c81d1 |
    | 26ab0db90d72e28ad0ba1e22ee510510 |
    +----------------------------------+

You can add the sequence as a column to a new table or an existing table, as shown in this example:

.. code-block:: mysql

    mysql> CREATE TABLE t1 AS SELECT * FROM SEQUENCE_TABLE(4) AS tt;
    
    mysql> SELECT * FROM t1;
    +-------+
    | value |
    +-------+
    |     0 |
    |     1 |
    |     2 |
    |     3 |
    +-------+

There are many uses for a sequence when populating tables.
