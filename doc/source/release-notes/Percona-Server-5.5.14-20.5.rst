.. rn:: 5.5.14-20.5

============================
|Percona Server| 5.5.14-20.5
============================

Percona is glad to announce the release of |Percona Server| 5.5.14-20.5 on August 12, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.14-20.5/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on |MySQL| 5.5.14, including all the bug fixes in it, |Percona Server| 5.5.14-20.5 is now the current stable release in the 5.5 series. All of Percona's software is open-source and free, all the details of the release can be found in the `5.5.14-20.5 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.14-20.5>`_.


Improvements
============

Performance Improvements
------------------------

``fsync()`` has been replaced with ``fdatasync()`` to improve perfomance where possible. The former is intended to sync the metadata of the file also (size, name, access time, etc.), but for the transaction log and the doublewrite buffer, such sync of metadata isn't needed. Bug Fixed: :bug:`803270` (*Yasufumi Kinoshita*).

Compatibility Collations
------------------------

Two new collations, ``utf8_general50_ci`` and ``ucs2_general50_ci``, have been to improve compatibility for those upgrading from |MySQL| 5.0 or 5.1 prior to version 5.1.24.

A fix for a |MySQL| bug (`#27877 <http://bugs.mysql.com/bug.php?id=27877>`_) introduced an incompatible change in collations in |MySQL| 5.1.24. If the following collations were used:

  * ``utf8_general_ci``

  * ``ucs2_general_ci``

and any of the indexes contained the German letter “U+00DF SHARP S” ``ß`` (which became equal to ``s``), when upgrading from 5.0 / 5.1.23 or lower:

  * any indexes on columns in that situation must be rebuilt after the upgrade, and

  * unique constrains may get broken after upgrade due to possible duplicates.

This problem is avoided when upgrading to |Percona Server| by converting the affected tables or columns to the collations introduced:

  * ``utf8_general_ci`` to ``utf8_general50_ci``, and

  * ``ucs2_general_ci`` to ``ucs2_general50_ci``.

Blueprint: `utf8-general50-ci-5.5 <https://blueprints.launchpad.net/percona-server/+spec/utf8-general50-ci-5.5>`_ (*Alexey Kopytov*).

Bug Fixes
=========

  * When adding a table to the cache, the server may evict and close another if the table cache is full. If the closed table was on the ``FEDERATED`` engine and a replication environment, its client connection to the remote server was closed leading to an unappropriated network error and stopping the Slave SQL thread. Bugs Fixed :bug:`813587` / `#51196 <http://bugs.mysql.com/bug.php?id=51196>`_ and `#61790 <http://bugs.mysql.com/bug.php?id=61790>`_ in |MySQL| (*Alexey Kopytov*).

  * Querying ``global_temporary_tables`` caused the server to crash in some scenarios due to insufficient locking. Fixed by introducing a new mutex to protect from race conditions. Bug Fixed: :bug:`745241` (*Alexey Kopytov*).

  * Using the ``innodb_lazy_drop_table`` option led to an assertion error when truncating a table in some scenarios. Bug Fixed: :bug:`798371` (*Yasufumi Kinoshita*).

Other Changes
=============

Improvements and fixes on platform-specific distribution:

  * The compilation of the :ref:`Response Time Distribution <response_time_distribution>` patch has been fixed on *Solaris* (supported platform) and *Windows* (experimental). Bug Fixed: :bug:`737947` (*Laurynas Biveinis*)

Improvements and fixes on general distribution: 

  * :bug:`766266`, :bug:`794837`, :bug:`806975` (*Laurynas Biveinis*, *Stewart Smith*, *Alexey Kopytov*)

Improvements and fixes on the |Percona Server| documentation: 

  * :bug:`803109`, :bug:`803106`, :bug:`803097` (*Rodrigo Gadea*)
