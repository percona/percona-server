.. rn:: 5.7.20-18

========================
Percona Server 5.7.20-18
========================

Percona is glad to announce the release of Percona Server 5.7.20-18
on December 14.
Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.20-18/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.20
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-20.html>`_
and includes all the bug fixes in it.
Percona Server 5.7.20-18 is now the current GA release in the 5.7 series.
All software developed by Percona is open-source and free.
Details of this release can be found in the `5.7.20-18 milestone on Launchpad
<https://launchpad.net/percona-server/+milestone/5.7.20-18>`_.


New Features
============

* |Percona Server| packages are now available for *Ubuntu 17.10 (Artful)*.

* As part of :ref:`innodb_fts_improvements` a new
  :variable:`innodb_ft_ignore_stopwords` variable has been implemented which
  controls whether InnoDB Full-Text Search should ignore stopword list
  when building/updating an FTS index. This feature is also fixing bug
  :bug:`1679135` (upstream :mysqlbug:`84420`).

* |Percona Server| has implemented :ref:`innodb_fragmentation_count`.

* |Percona Server| has implemented support for :ref:`aio_page_requests`.
  This feature was ported from a *Facebook MySQL* patch.

* |Percona Server| has implemented TokuDB :ref:`integration
  <tokudb_performance_schema>` with ``PERFORMANCE_SCHEMA``.

* As part of :ref:`data_at_rest_encryption`, |Percona Server| has implemented
  support for `innodb_general_tablespace_encryption` and
  :ref:`keyring_vault_plugin`. This feature is considered **BETA** quality.

Bugs Fixed
==========

* |Percona Server| 5.7 docker images did not include TokuDB. Bugs fixed
  :bug:`1682419` and :bug:`1699241`.

* If an I/O syscall returned an error during the server shutdown with
  :ref:`threadpool` enabled, a mutex could be left locked. Bug fixed
  :bug:`1702330` (*Daniel Black*).

* Dynamic row format feature to support ``BLOB/VARCHAR`` in ``MEMORY`` tables
  requires all the key columns to come before any ``BLOB`` columns. This
  requirement however was not enforced, allowing creating MEMORY tables in
  unsupported column configurations, which then crashed or lose data in usage.
  Bug fixed :bug:`1731483`.

* After fixing bug :bug:`1668602`, bug :bug:`1539504`, and bug :bug:`1313901`,
  ``CREATE/DROP TEMPORARY TABLE`` statements were forbidden incorrectly in
  transactional contexts, including function and trigger calls, even when
  they required no binary logging at all. Bug fixed :bug:`1711781`.

* Running ``ANALYZE TABLE`` while a long-running query is accessing the same
  table in parallel could lead to a situation where new queries on the same
  table are blocked in a ``Waiting for table flush`` state.
  Fixed by stopping ``ANALYZE TABLE`` flushing affected InnoDB and TokuDB
  tables from the table definition cache. Bug fixed :bug:`1704195`
  (upstream :mysqlbug:`87065`).

* ``CREATE TABLE ... LIKE ...`` did not use source ``row_format`` on target
  TokuDB table. Bug fixed :tdbbug:`76`.

* TokuDB would encode already encoded database name for a directory name.
  Bug fixed :tdbbug:`74`.

* Optimizer would pick wrong index for TokuDB tables having a hot created
  index, unless ``ALTER TABLE`` was run. Bug fixed :tdbbug:`35`.

Other bugs fixed: :bug:`1720810`, :tdbbug:`83`, :tdbbug:`80`, and :tdbbug:`75`.

MyRocks Changes
===============

* RocksDB has implemented a FlushWAL API which improves upon the performance of
  MySQL 2-phase-commit during binary log group commit flush stage. This
  feature adds support for using the FlushWAL API in MyRocks and also matches
  :variable:`rocksdb_flush_log_at_trx_commit` variable with
  :variable:`innodb_flush_log_at_trx_commit` behavior. Two implement this
  feature :variable:`rocksdb_manual_wal_flush` and
  :variable:`rocksdb_concurrent_prepare` variables have been implemented.

* New :variable:`rocksdb_force_compute_memtable_stats_cachetime` variable has
  been implemented that cane be used to specify how long the cached value of
  memtable statistics should be used instead of computing it every time during
  the query plan analysis.

* New :variable:`rocksdb_large_prefix` variable has been implemented which,
  when enabled, allows index key prefixes longer than 767 bytes (up to 3072
  bytes). This option mirrors the `innodb_large_prefix
  <https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_large_prefix>`_
  The values for this variable should be the same between master and slave.

* New :variable:`rocksdb_max_background_jobs` variable has been implemented
  to replace :variable:`rocksdb_base_background_compactions`,
  :variable:`rocksdb_max_background_compactions`, and
  :variable:`rocksdb_max_background_flushes` variables. This variable specifies
  the maximum number of background jobs. It automatically decides
  how many threads to allocate towards flush/compaction. It was implemented to
  reduce the number of (confusing) options users and can tweak and push the
  responsibility down to RocksDB level.

* New :variable:`rocksdb_sim_cache_size` variable has been implemented to
  enable the simulated cache. This can be used to figure out the hit/miss rate
  with a specific cache size without changing the real block cache.

* Input can be now sorted by the Primary Key during the bulkload by enababling
  the :variable:`rocksdb_bulk_load_allow_unsorted` variable.

* New :variable:`rocksdb_ignore_unknown_options` variable has been implemented,
  which when enabled (default) allows RocksDB to receive unknown options and
  not exit.
