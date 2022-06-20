.. _8.0.28-19:

================================================================================
*Percona Server for MySQL* 8.0.28-19 (2022-05-12)
================================================================================

.. include:: ../_res/rn/ps-mysql-blurb.txt

.. contents::
   :local:

.. include:: ../_res/rn/8.0.28-highlights.txt

.. include:: ../_res/rn/deprecated.txt

Improvement
=================================================

* :jirabug:`PS-7871`: Using the ``SET_VAR`` syntax, MyRocks variables can be set dynamically.
* :jirabug:`PS-8064`: The ability to change log file locations dynamically is restricted.

Bugs fixed
=================================================

* :jirabug:`PS-7999`: The ``FEDERATED`` storage engine would not reconnect when a ``wait_timeout`` was exceeded. (Thanks to Sami Ahlroos for reporting this issue) (Upstream :mysqlbug:`105878`)
* :jirabug:`PS-7856`: Fixed for a sever exit caused by an update query on a partition tables.
* :jirabug:`PS-8032`: An Inplace index build with ``lock=exclusive`` did not generate an ``MLOG_ADD_INDEX`` redo.
* :jirabug:`PS-8050`: An upgrade from *Percona Server for MySQL* 5.7 to *MySQL* 8.0.26, caused a server exit with an assertion failure.






.. include:: ../_res/rn/useful-links.txt