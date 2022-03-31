.. rn:: 5.7.21-21

========================
Percona Server 5.7.21-21
========================

Percona is glad to announce the release of Percona Server 5.7.21-21
on April 24, 2018. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.21-21/>`_ and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.21 <http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-21.html>`_ and includes all the bug fixes in it.
Percona Server 5.7.21-21 is now the current GA (Generally Available) release
in the 5.7 series.
This version of Percona Server for MySQL marks the following encryption
features, previously available as beta, as GA: `Vault keyring plugin <https://www.percona.com/doc/percona-server/5.7/management/data_at_rest_encryption.html#id13>`_, `encryption for InnoDB general tablespaces <https://www.percona.com/doc/percona-server/5.7/management/data_at_rest_encryption.html#id7>`_, and `encryption for binary log files <https://www.percona.com/doc/percona-server/5.7/management/data_at_rest_encryption.html#id14>`_.

All software developed by Percona is open-source and free.

New Features
============

* A new variable :variable:`innodb_temp_tablespace_encrypt` is introduced to
  turn encryption of temporary tablespace and temporary InnoDB file-per-table
  tablespaces on/off. Bug fixed :psbug:`3821`.

* A new variable :variable:`innodb_encrypt_online_alter_logs` simultaneously
  turns on encryption of files used by InnoDB for merge sort, online DDL logs,
  and temporary tables created by InnoDB for online DDL. Bug fixed
  :psbug:`3819`.

* A new variable :variable:`innodb_encrypt_tables` can be set to ``ON``, making
  InnoDB tables encrypted by default, to ``FORCE``, disabling creation of
  unencrypted tables, or ``OFF``, restoring the like-before behavior. Bug fixed
  :psbug:`1525`.

* Query response time plugin now can be disabled at session level with use
  of a new variable :variable:`query_response_time_session_stats`.

Bugs Fixed
==========

* Attempting to use a partially-installed query response time plugin could have
  caused server crash. Bug fixed :psbug:`3959`.

* There was a server crash caused by a materialized temporary table from
  semi-join optimization with key length larger than 1000 bytes. Bug fixed
  :psbug:`296`.

* A regression in the original 5.7 port was causing integer overflow with
  :variable:`thread_pool_stall_limit` variable values bigger than 2 seconds.
  Bug fixed :psbug:`1095`.

* A memory leak took place in |Percona Server| when performance schema is used
  in conjunction with thread pooling. Bug fixed :psbug:`1096`.

* A code clean-up was done to fix compilation with clang, both general warnings
  (bug fixed :psbug:`3814`, upstream :mysqlbug:`89646`) and clang 6 specific
  warnings and errors (bug fixed :psbug:`3893`, upstream :mysqlbug:`90111`).

* Compilation warning was fixed for `-DWITH_QUERY_RESPONSE_TIME=ON` CMake
  compilation option, which makes QRT to be linked statically. Bug fixed
  :psbug:`3841`.

* |Percona Server| returned empty result for ``SELECT`` query if number of
  connections exceeded 65535. Bug fixed :psbug:`314` (upstream
  :mysqlbug:`89313`).

* A clean-up in |Percona Server| binlog-related code was made to avoid
  uninitialized memory comparison. Bug fixed :psbug:`3925` (upstream
  :mysqlbug:`90238`).

* ``mysqldump`` utility with ``--innodb-optimize-keys`` option was incorrectly
  working with foreign keys on the same table, producing invalid SQL
  statements. Bugs fixed :psbug:`1125` and :psbug:`3863`.

* A fix of the mysqld startup script failed to detect jemalloc library
  location for preloading, thus not starting on systemd based machines,
  introduced in |Percona Server| :rn:`5.7.21-20`, was improved to take into
  account previously created configuration file. Bug fixed :psbug:`3850`.

* The possibility of a truncated bitmap file name was fixed in InnoDB logging
  subsystem. Bug fixed :psbug:`3926`.

* Temporary file I/O was not instrumented for Performance Schema. Bug fixed
  :psbug:`3937` (upstream :mysqlbug:`90264`).

* A crash in the unsafe query warning checks with views took place for
  ``UPDATE`` statement in case of statement binlogging format. Bug fixed
  :psbug:`290`.

MyRocks Changes
===============

* A re-implemented variable :variable:`rpl_skip_tx_api` allows to turn on
  simple RocksDB write batches functionality, increasing replication
  performance by the transaction API skip. Bug fixed :jirabug:`MYR-47`.

* Decoding value-less padded varchar fields could under some circumstances
  cause assertion and/or data corruption. Bug fixed :jirabug:`MYR-232`.

TokuDB Changes
===============

* Two new variables introduced to facilitate the TokuDB fast updates feature,
  :variable:`tokudb_enable_fast_update` and
  :variable:`tokudb_enable_fast_upsert`. Bugs fixed :tdbbug:`63` and
  :tdbbug:`148`.

* A set of compilation fixes was introduced to make TokuDB successfully
  build in MySQL / |Percona Server| 8.0. Bugs fixed :tdbbug:`84`,
  :tdbbug:`85`, :tdbbug:`114`, :tdbbug:`115`, :tdbbug:`118`, :tdbbug:`128`,
  :tdbbug:`139`, :tdbbug:`141`, and :tdbbug:`172`.

* Conditional compilation code dependent on version ID in the TokuDB tree was
  separated and arranged to specific version branches. Bugs fixed
  :tdbbug:`133`, :tdbbug:`134`, :tdbbug:`135`, and :tdbbug:`136`.

* ``ALTER TABLE ... COMMENT = ...`` statement caused TokuDB to rebuild the
  whole table, which is not needed, as only FRM metadata should be changed.
  Bugs fixed :tdbbug:`130` and :tdbbug:`137`.

* Data race on the cache table pair attributes was fixed. Bug fixed
  :tdbbug:`109`.

Other bugs fixed: :psbug:`3793`, :psbug:`3812`, :psbug:`3813`, :psbug:`3815`,
:psbug:`3818`, :psbug:`3835`, :psbug:`3875` (upstream :mysqlbug:`89916`),
:psbug:`3843` (upstream :mysqlbug:`89822`), :psbug:`3848`, :psbug:`3856`,
:psbug:`3887`, :jirabug:`MYR-160`, :jirabug:`MYR-245`, :tdbbug:`109`,
:tdbbug:`111`, :tdbbug:`180`, :tdbbug:`181`, :tdbbug:`182`, and :tdbbug:`188`.



