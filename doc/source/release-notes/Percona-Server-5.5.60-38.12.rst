.. rn:: 5.5.60-38.12

=============================
|Percona Server| 5.5.60-38.12
=============================

Percona is glad to announce the release of |Percona Server| 5.5.60-38.12 on
May 18th, 2018. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.60-38.12/>`_
and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.60
<http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-60.html>`_, including
all the bug fixes in it, |Percona Server| 5.5.60-38.12 is now the current
stable release in the 5.5 series. All of |Percona|'s software is open-source
and free.

Bugs Fixed
==========

* ``mysqldump`` utility with ``--innodb-optimize-keys`` option was incorrectly
  working with foreign keys pointing on the same table, producing invalid SQL
  statements. Bugs fixed :psbug:`1125` and :psbug:`3863`.

* A typo in ``plugin.cmake`` file prevented to compile plugins statically
  into the server. Bug fixed :psbug:`3871` (upstream :mysqlbug:`89766`).

* Using ``-DWITHOUT_<PLUGIN>=ON`` CMake variable to exclude a plugin from the
  build didn't work for some plugins, including a number of storage engines.
  Bug fixed :psbug:`3901`.

* A fix was introduced to remove GCC 8 compilation warnings for the
  |Percona Server| build. Bug fixed :psbug:`3950`.

* A code clean-up was done to fix compilation warnings and errors specific
  for clang 6. Bug fixed :psbug:`3893` (upstream :mysqlbug:`90111`).

* |Percona Server| Debian packages description included reference to
  ``/etc/mysql/my.cnf`` file, which is not actually present in these packages.
  Bug fixed :psbug:`2046`.

* A clean-up in |Percona Server| binlog-related code was made to avoid
  uninitialized memory comparison. Bug fixed :psbug:`3925` (upstream
  :mysqlbug:`90238`).

* Some ``IO_CACHE`` file operations were not instrumented for Performance
  Schema causing inaccurate statistics generated. Bug fixed :psbug:`3937`
  (upstream :mysqlbug:`90264`).

* A ``key_block_size`` value was set automatically by the Improved MEMORY
  Storage Engine, which resulted in warnings when changing the engine type to
  |InnoDB|, and constantly growing ``key_block_size`` during alter operations.
  Bugs fixed :psbug:`3936`, :psbug:`3940`, and :psbug:`3943`.


Other bugs fixed: :psbug:`3767` "Fix compilation warnings/errors with clang",
:psbug:`3778` "5.5 Tree received Percona-TokuBackup submodule where it should
not", :psbug:`3794` "MTR test main.percona_show_temp_tables_stress does not
wait for events to start", :psbug:`3798` "MTR test
innodb.percona_extended_innodb_status fails if InnoDB status contains unquoted
special characters", and :psbug:`3926` "Potentially truncated bitmap file name
in log_online_open_bitmap_file_read_only()
(storage/innobase/log/log0online.cc)".
