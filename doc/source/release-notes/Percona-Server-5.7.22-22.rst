.. rn:: 5.7.22-22

========================
Percona Server 5.7.22-22
========================

Percona is glad to announce the release of Percona Server 5.7.22-22
on May 31, 2018. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.22-22/>`_ and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.22 <http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-22.html>`_ and includes all the bug fixes in it. |Percona Server| 5.7.22-22 is now the current GA
(Generally Available) release in the 5.7 series.

All software developed by Percona is open-source and free.

New Features
============

* A new ``--encrypt-tmp-files`` option turns on encryption for the temporary
  files which  |Percona Server| may create on disk for filesort, binary log
  transactional caches and Group Replication caches.

Bugs Fixed
==========

* Executing the ``SHOW GLOBAL STATUS`` expression could cause "data drift" on
  global status variables in case of a query rollback: the variable, being by
  its nature a counter and allowing only an increase, could return to its
  previous value. Bug fixed :psbug:`3951` (upstream :mysqlbug:`90351`).

* NUMA support was improved in |Percona Server|, reverting upstream
  implementation back to the original one, due to upstream variant
  being less effective in memory allocation. Now
  `innodb_numa_interleave
  <http://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_numa_interleave>`_
  variable not only enables NUMA interleave memory policy for the
  InnoDB buffer pool allocation, but forces NUMA interleaved
  allocation at the buffer pool initialization time. Bug fixed
  :psbug:`3967`.

* :variable:`audit_log_include_accounts` variable did not take effect if
  placed in ``my.cnf`` configuration file, while still working as intended if
  set dynamically. Bug fixed :psbug:`3867`.

* A ``key_block_size`` value was set automatically by the Improved MEMORY
  Storage Engine, which resulted in warnings when changing the engine type to
  InnoDB, and constantly growing ``key_block_size`` during alter operations.
  Bugs fixed :psbug:`3936`, :psbug:`3940`, and :psbug:`3943`.

* Fixes were introduced to remove GCC 8 compilation warnings for the
  |Percona Server| build. Bug fixed :psbug:`3950`.

* An InnoDB Memcached Plugin code clean-up was backported from MySQL 8.0. Bug
  fixed :psbug:`4506`.

* |Percona Server| could not be built with ``-DWITH_LZ4=system`` option on
  Ubuntu 14.04 (Trusty) because of too old LZ4 packages. Bug fixed
  :psbug:`3842`.

* A regression brought during TokuDB code clean-up in :rn:`5.7.21-21` was
  causing assertion in cases when the FT layer returns an error during an alter
  table operation. Bug fixed :psbug:`4294`.

MyRocks Changes and Fixes
=========================

* ``UPDATE`` statements were returning incorrect results because of not making
  a full table scan on tables with unique secondary index. Bug fixed
  :psbug:`4495` (upstream `facebook/mysql-5.6#830 <https://github.com/facebook/mysql-5.6/issues/830>`_).

Other Bugs Fixed
================

* :psbug:`4451` \"Implement better compression algo testing\"

* :psbug:`4469` \"variable use out of scope bug in get_last_key test detected by
  ASAN in clang 6\"

* :psbug:`4470` \"the cachetable-simple-pin-nonblocking-cheap test occasionally
  fails due to a locking conflict with the cachetable evictor\"

* :psbug:`4488` \"\`-Werror\` is always disabled for \`innodb_memcached\`\"

* :psbug:`1114` \"Assertion \`inited \=\= INDEX\' failed\"

* :psbug:`1130` \"RBR Replication with concurrent XA in READ-COMMITTED takes
  supremum pseudo-records and breaks replication\"

