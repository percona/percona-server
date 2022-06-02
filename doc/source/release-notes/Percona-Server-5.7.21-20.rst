.. _5.7.21-20:

========================
Percona Server 5.7.21-20
========================

Percona is glad to announce the release of Percona Server 5.7.21-20
on February 19, 2018. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.21-20/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.21
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-21.html>`_
and includes all the bug fixes in it.
Percona Server 5.7.21-20 is now the current GA (Generally Available) release
in the 5.7 series. All software developed by Percona is open-source and free.

New Features
============

* A new string variable :variable:`version_suffix` allows to change suffix
  for the |Percona Server| version string returned by the read-only
  :variable:`version` variable. Also :variable:`version_comment` is converted
  from a global read-only to a global read-write variable.

* A new :variable:`keyring_vault_timeout` variable allows to set the amount
  of seconds for the Vault server connection timeout. Bug fixed :psbug:`298`.

Bugs Fixed
==========

* mysqld startup script was unable to detect jemalloc library location for
  preloading, and that prevented starting |Percona Server| on systemd based
  machines. Bugs fixed :psbug:`3784` and :psbug:`3791`.

* There was a problem with fulltext search, which could find a word with
  punctuation marks in natural language mode only, but not in boolean mode.
  Bugs fixed :psbug:`258`, :psbug:`2501` (upstream :mysqlbug:`86164`).

* Build errors were present on FreeBSD (caused by fixing the bug
  :psbug:`255` in |Percona Server| :rn:`5.6.38-83.0`) and on MacOS (caused
  by fixing the bug :psbug:`264` in |Percona Server| :rn:`5.7.20-19`). Bugs
  fixed :psbug:`2284` and :psbug:`2286`.

* A bunch of fixes was introduced to remove GCC 7 compilation warnings for
  the |Percona Server| build. Bugs fixed :psbug:`3780` (upstream
  :mysqlbug:`89420`, :mysqlbug:`89421`, and :mysqlbug:`89422`).

* CMake error took place at compilation with bundled zlib. Bug fixed
  :psbug:`302`.

* A GCC 7 warning fix introduced regression in |Percona Server| that led to
  a wrong SQL query built to access the remote server when Federated storage
  engine was used. Bug fixed :psbug:`1134`.

* It was possible to enable :variable:`encrypt_binlog` with no binary or relay
  logging enabled. Bug fixed :psbug:`287`.

* Long buffer wait times where occurring on busy servers in case of the
  ``IMPORT TABLESPACE`` command. Bug fixed :psbug:`276`.

* Server queries that contained JSON special characters and were logged by
  :ref:`audit_log_plugin` in JSON format caused invalid output due to lack of
  escaping. Bug fixed :psbug:`1115`.

* Percona Server now uses *Travis CI*  for additional tests. Bug fixed
  :psbug:`3777`.

Other bugs fixed: :psbug:`257`, :psbug:`264`, :psbug:`1090`
(upstream :mysqlbug:`78048`), :psbug:`1109`, :psbug:`1127`, :psbug:`2204`,
:psbug:`2414`, :psbug:`2415`, :psbug:`3767`, :psbug:`3794`, and :psbug:`3804`
(upstream :mysqlbug:`89598`).

This release also contains fixes for the following CVE issues: CVE-2018-2565,
CVE-2018-2573, CVE-2018-2576, CVE-2018-2583, CVE-2018-2586, CVE-2018-2590,
CVE-2018-2612, CVE-2018-2600, CVE-2018-2622, CVE-2018-2640, CVE-2018-2645,
CVE-2018-2646, CVE-2018-2647, CVE-2018-2665, CVE-2018-2667, CVE-2018-2668,
CVE-2018-2696, CVE-2018-2703, CVE-2017-3737.

MyRocks Changes
===============

* A new behavior makes |Percona Server| fail to restart on detected data
  corruption; :variable:`rocksdb_allow_to_start_after_corruption` variable can
  be passed to ``mysqld`` as a command line parameter to switch off this
  restart failure.

* A new cmake option ``ALLOW_NO_SSE42`` was introduced to allow MyRocks build
  on hosts not supporting SSE 4.2 instructions set, which makes MyRocks usable
  without FastCRC32-capable hardware. Bug fixed :jirabug:`MYR-207`.

* :variable:`rocksdb_bytes_per_sync` and :variable:`rocksdb_wal_bytes_per_sync`
  variables were turned into dynamic ones.

* :variable:`rocksdb_flush_memtable_on_analyze` variable has been removed.

* :variable:`rocksdb_concurrent_prepare` is now deprecated, as it has been
  renamed in upstream to :variable:`rocksdb_two_write_queues`.

* :variable:`rocksdb_row_lock_deadlocks` and
  :variable:`rocksdb_row_lock_wait_timeouts` global status counters were added
  to track the number of deadlocks and the number of row lock wait timeouts.

* Creating table with string indexed column to non-binary collation now
  generates warning about using inefficient collation instead of error. Bug
  fixed :jirabug:`MYR-223`.

TokuDB Changes
===============

* A memory leak was fixed in the PerconaFT library, caused by not destroying
  PFS key objects on shutdown. Bug fixed :jirabug:`TDB-98`.

* A clang-format configuration was added to PerconaFT and TokuDB. Bug fixed
  :jirabug:`TDB-104`.

* A data race was fixed in minicron utility of the PerconaFT. Bug fixed
  :jirabug:`TDB-107`.

* Row count and cardinality decrease to zero took place after long-running
  ``REPLACE`` load.

Other bugs fixed: :jirabug:`TDB-48`, :jirabug:`TDB-78`, :jirabug:`TDB-93`,
and :jirabug:`TDB-99`.


