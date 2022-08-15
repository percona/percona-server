.. _5.7.39-42:

====================================================
*Percona Server for MySQL* 5.7.39-42 (2022-08-15)
====================================================

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.39-42
includes all the features and bug fixes available in `MySQL 5.7.39 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-39.html>`__ in addition to enterprise-grade features developed by Percona.

.. include:: ../_res/rn/ps-mysql-blurb.txt

.. contents::
   :local:

.. include:: ../_res/rn/5.7.39-highlights.txt

.. include:: ../_res/rn/deprecated-5.7.39.txt

Improvements
=================================================

The ``SHOW PROCESSLIST`` statement now displays an extra field ``TIME_MS``. The ``TIME_MS`` field provides the information about the time in milliseconds that the thread has been in its current state. 

Bugs Fixed
=================================================

* :jirabug:`PS-8205`: ``DICT_TF2_FLAG_SET`` was used instead of ``DICT_TF2_FLAG_IS_SET``.
* :jirabug:`PS-8174`: MySQL crashed at shutdown with ``buf0flu.cc:3567:UT_LIST_GET_LEN(buf_pool->flush_list) == 0`` assertion.

.. include:: ../_res/rn/useful-links.txt

