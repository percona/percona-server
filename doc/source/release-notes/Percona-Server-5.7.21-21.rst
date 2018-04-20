.. rn:: 5.7.21-21

========================
Percona Server 5.7.21-21
========================

Percona is glad to announce the release of Percona Server 5.7.21-21
on April 23, 2018. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.21-21/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.21
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-21.html>`_
and includes all the bug fixes in it.
Percona Server 5.7.21-21 is now the current GA (Generally Available) release
in the 5.7 series. All software developed by Percona is open-source and free.

New Features
============

* A new variable :variable:`innodb_temp_tablespace_encrypt` is introduced to
  turn encryption of temporary tablespace and temporary InnoDB file-per-table
  tablespaces on/off. Bug fixed :psbug:`3821`.

* A new variable :variable:`innodb_encrypt_online_alter_logs` simultaneously
  turns on encryption of files used by InnoDB for merge sort, online DDL logs,
  and temporary tables created by InnoDB for online DDL. Bug fixed :psbug:`3819`.

* A new variable :variable:`innodb_encrypt_tables` can be set to ``ON``, making
  |InnoDB| tables encrypted by default, to ``FORCE``, disabling creation of
  unencrypted tables, or ``OFF``, restoring the like-before behavior. Bug fixed
  :psbug:`1525`.

* Query response time plugin now can be disabled at session level.

Bugs Fixed
==========

* Query response time plugin installation was able to cause server crash.
  Bug fixed :psbug:`3959`.

* There was a server crash caused by a materialized temporary table from
  semi-join optimization with key length larger than 1000 bytes. Bug fixed
  :psbug:`296`.

* A regression was causing integer overflow with
  :variable:`thread_pool_stall_limit` variable values bigger than 2 seconds.
  Bug fixed :psbug:`1095`.

* A memory leak took place in |Percona Server| when performance schema is used
  in conjunction with thread pooling. Bug fixed :psbug:`1096`.

* A code clean-up was done to fix compilation with clang, as general warnings 
  (bug fixed :psbug:`3814`, upstream :mysqlbug:`89646`) so clang 6 specific
  warnings and errors (bug fixed :psbug:`3893`, upstream :mysqlbug:`98111`).

* Compilation warning was fixed for statically linked QRT. Bug fixed
  :psbug:`3841`.

* |Percona Server| returned empty result for ``SELECT`` query if number of
  connections exceeded 65535. Bug fixed :psbug:`314`.

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

* A fix introduced in |Percona Server| :rn:`5.7.10-2` which made busy server
  not to prefer LRU flushing over flush list flushing too strongly was reverted
  back because of the MT LRU flusher introduction, which removed LRU flushing
  from the cleaner loop, Bug fixed :psbug:`3812`.

* The possibility of a truncated bitmap file name was fixed in |InnoDB| logging
  subsystem. Bug fixed :psbug:`3926`.

* Non-instrumented functions were used in cached reads and writes of files in
  fixed-size units. Bug fixed :psbug:`3937` (upstream :mysqlbug:`90264`).

* A crash in the unsafe query warning checks with views took place for
  ``UPDATE`` statement in case of statement binlogging format. Bug fixed
  :psbug:`290`.

MyRocks Changes
===============

* A re-implemented variable :variable:`rpl_skip_tx_api` allows to turn on simple
  |RocksDB| write batches functionality, increasing replication performance
  by the transaction api skip. Bug fixed :jirabug:`MYR-47`.

* Unpack info bytes indicating the trailing space length in a padded varchar
  were not read/skipped for value-less fields, causing data corruption and/or
  server crash. Bug fixed :jirabug:`MYR-232`.

TokuDB Changes
===============

* Two new variables introduced for the |TokuDB| fast updates feature,
  :variable:`tokudb_enable_fast_update` and
  :variable:`tokudb_enable_fast_upsert` should be now used instead of the 
  ``NOAR`` keyword, which is now optional at compile time and off by default.
  Bugs fixed :tdbbug:`63` and :tdbbug:`148`.

* A set of compilation fixes as introduced to make |TokuDB| successfully
  build in 8.0. Bugs fixed :tdbbug:`84`, :tdbbug:`85`, :tdbbug:`114`,
  :tdbbug:`115`, :tdbbug:`118`, :tdbbug:`128`, :tdbbug:`139`, :tdbbug:`141`,
  and :tdbbug:`172`.

* Conditional compilation code dependent on version ID in the TokuDB tree was
  separated and arranged to specific version branches. Bugs fixed
  :tdbbug:`133`, :tdbbug:`134`, :tdbbug:`135`, and :tdbbug:`136`.

* An additional code clean-up was made in bounds of 8.0 transition to remove
  MariaDB-specific constructions. Bugs fixed :tdbbug:`180`, :tdbbug:`181`,
  and :tdbbug:`182`.

* Alter table comment caused |TokuDB| to rebuild the whole table, which is not
  needed, as only FRM metadata should be changed. Bug fixed :tdbbug:`130`,
  and :tdbbug:`137`.

* Data race on the cache table pair attributes was fixed.

Other bugs fixed: :psbug:`3793`, :psbug:`3813`, :psbug:`3815`, :psbug:`3818`,
:psbug:`3835`, :psbug:`3875` (upstream :mysqlbug:`89916`), :psbug:`3843`
(upstream :mysqlbug:`89822`), :psbug:`3848`, :psbug:`3856`, :psbug:`3887`,
:jirabug:`MYR-160`, :jirabug:`MYR-245`, :tdbbug:`109`, :tdbbug:`111`,
and :tdbbug:`188`.



