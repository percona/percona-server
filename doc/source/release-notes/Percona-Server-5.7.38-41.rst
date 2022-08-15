.. _5.7.38-41:

====================================================
*Percona Server for MySQL* 5.7.38-41 (2022-06-02)
====================================================

.. include:: ../_res/rn/ps-mysql-blurb.txt

.. contents::
   :local:

.. include:: ../_res/rn/5.7.38-highlights.txt

.. include:: ../_res/rn/deprecated.txt

Bugs Fixed
=================================================

* :jirabug:`PS-6029`: Data masking gen_rnd_us_phone() function had a different format compared to MySQL upstream version.
* :jirabug:`PS-8129`: A fix for when mutex hangs in thread_pool_unix.
* :jirabug:`PS-8136`: ``LOCK TABLES FOR BACKUP`` did not prevent InnoDB key rotation. Due to this behavior, Percona Xtrabackup couldn't fetch the key in case the key was rotated after starting the backup.
* :jirabug:`PS-8143`: Fixed the memory leak in ``File_query_log::set_rotated_name()``. 
* :jirabug:`PS-8204`: When the ``audit_log_format`` was set to XML, logged queries were truncated after a newline character.

.. include:: ../_res/rn/useful-links.txt

