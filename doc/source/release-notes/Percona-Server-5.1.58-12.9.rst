.. rn:: 5.1.58-12.9

==============================
 |Percona Server| 5.1.58-12.9
==============================

Percona is glad to announce the release of |Percona Server| 5.1.58-12.9 on August 12, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.58-12.9/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.1.58 <http://dev.mysql.com/doc/refman/5.1/en/news-5-1-58.html>`_, including all the bug fixes in it, |Percona Server| 5.1.58-12.9 is now the current stable release in the 5.1 series. All of Percona`s software is open-source and free, all the details of the release can be found in the `5.1.58-12.9 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.58-12.9>`_.

Highlights
==========

Performance Improvements
------------------------

  * ``fsync()`` has been replaced with ``fdatasync()`` to improve perfomance where possible. The former is intended to sync the metadata of the file also (size, name, access time, etc.), but for the transaction log and the doublewrite buffer, such sync of metadata isn't needed. Bug Fixed: :bug:`803270` (*Yasufumi Kinoshita*).

  * A remaining loop from an unimplemented feature degraded the performance when using compressed tables and has been removed (``buf_LRU_insert_zip_clean``). Bugs Fixed: :bug:`802825` / `#61341 <http://bugs.mysql.com/bug.php?id=61341>`_ in |MySQL| (*Yasufumi Kinoshita*).

Compatibility Collations
------------------------

Two new collations, ``utf8_general50_ci`` and ``ucs2_general50_ci``, have been to improve compatibility for those upgrading from |MySQL| 5.0 or 5.1 prior to version 5.1.24.

A fix for a |MySQL| bug ([[`#27877 <http://bugs.mysql.com/bug.php?id=27877>`) introduced an incompatible change in collations in |MySQL| 5.1.24. If the following collations were used:

  * ``utf8_general_ci`` 
  * ``ucs2_general_ci``

and any of the indexes contained the German letter "U+00DF SHARP S" ``ß`` (which became equal to ``s``), when upgrading from 5.0 / 5.1.23 or lower:

  * any indexes on columns in that situation must be rebuilt after the upgrade, and
  * unique constrains may get broken after upgrade due to possible duplicates.

This problem is avoided when upgrading to |Percona Server| by converting the affected tables or columns to the collations introduced: 
  * ``utf8_general_ci`` to ``utf8_general50_ci``, and 
  * ``ucs2_general_ci`` to ``ucs2_general50_ci``.

Blueprint:[[https://blueprints.launchpad.net/percona-server/+spec/utf8-general50-ci-5.1|utf8-general50-ci-5.1]] (*Alexey Kopytov*).

SHM Buffer Pool has been removed
--------------------------------

The :ref:`innodb_buffer_pool_shm` has been removed due to being both invasive and not widely used.

Instead, we recommend using the much safer :ref:`LRU Dump/Restore <innodb_lru_dump_restore>` patch, which provides similar improvements in restart performance and has the advantage of persisting across machine restarts.

The configuration variables for ``my.cnf`` have been kept for compatibility and warnings will be printed for the deprecated options (:variable:`innodb_buffer_pool_shm_key` and :variable:`innodb_buffer_pool_shm_checksum`) if used. 

Instructions for disabling the ``SHM`` buffer pool can be found :ref:`innodb_buffer_pool_shm` and for setting up LRU dump/restore :ref:`innodb_lru_dump_restore`.

Bug Fixes
==========

  * When adding a table to the cache, the server may evict and close another if the table cache is full. If the closed table was on the ``FEDERATED`` engine and a replication environment, its client connection to the remote server was closed leading to an unappropriated network error and stopping the Slave SQL thread. Bugs Fixed :bug:`813587` / `#51196 <http://bugs.mysql.com/bug.php?id=51196>`_ and `#61790 <http://bugs.mysql.com/bug.php?id=61790>`_ in |MySQL| (*Alexey Kopytov*).

  * Uninitialized values in the :ref:`Slow Query Log <slow_extended>` patch. Bug Fixed: :bug:`794774` (*Oleg Tsarev*).

  * Querying ``global_temporary_tables`` caused the server to crash in some scenarios due to insufficient locking. Fixed by introducing a new mutex to protect from race conditions. Bugs Fixed: :bug:`745241` (*Alexey Kopytov*).

  * As the option ``ignore-builtin-innodb`` is incompatible with |Percona Server| with |XtraDB|, the server will not start and print the corresponding error instead. Bug Fixed: :bug:`704216` (Laurynas Biveinis).

  * Querying ``INNODB_SYS_TABLES`` after an ``ALTER TABLE`` statement leaded to a server crash. Bug Fixed: :bug:`627189` (*Yasufumi Kinoshita*).

  * The 64-bit CAS implementation may lead to a server crash on IA32 systems. Bug Fixed: :bug:`803865` (Laurynas Biveinis).

  * Using the ``innodb_lazy_drop_table`` option led to an assertion error when truncating a table in some scenarios. Bug Fixed: :bug:`798371` (*Yasufumi Kinoshita*).

Other Changes
==============

  * Improvements and fixes on platform-specific distribution:

     * The compilation of the :ref:`response_time_distribution` patch has been fixed on Solaris  (supported platform) and Windows (experimental). Bug Fixed: :bug:`737947` (Laurynas Biveinis)

  * Improvements and fixes on general distribution: 

    * :bug:`806975`, :bug:`790199`, :bug:`782391`, :bug:`802829`, :bug:`700965`, :bug:`794840`, :bug:`766266`, (*Alexey Kopytov*, *Oleg Tsarev*, Stewart Smith, Laurynas Biveinis)

  * Improvements and fixes on the |Percona Server| Test Suite: :bug:`790199`, :bug:`785566`, :bug:`782391`, :bug:`800559`, :bug:`794790`, :bug:`794780`, :bug:`800035`, :bug:`684250`, :bug:`803140`, :bug:`803137`, :bug:`803124`, :bug:`803110`, :bug:`803100`, :bug:`803093`, :bug:`803088`, :bug:`803076`, :bug:`803071` (*Oleg Tsarev*, *Yasufumi Kinoshita*, Stewart Smith, *Alexey Kopytov*).

