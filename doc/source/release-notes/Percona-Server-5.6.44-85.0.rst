.. rn:: 5.6.44-85.0

================================================================================
|Percona Server| 5.6.44-85.0
================================================================================

Percona is glad to announce the release of |Percona Server| 5.6.44-85.0 on
May 17, 2019 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.44-85.0/>`_
and from the :doc:`Percona Software Repositories </installation>`).

This release merges changes of `MySQL 5.6.44
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-44.html>`_, including
all the bug fixes in it. Percona Server for MySQL 5.6.44-85.0 is now the current
GA release in the 5.6 series. All of Percona’s software is open-source and free.


New Features
================================================================================

- New :variable:`Audit_log_buffer_size_overflow` status variable has been
  implemented to track when an :ref:`audit_log_plugin` entry was either
  dropped or written directly to the file due to its size being bigger
  than :variable:`audit_log_buffer_size` variable.

Bugs Fixed
================================================================================

- TokuDB storage engine would assert on load when used with jemalloc 5.x.
  Bug fixed :psbug:`5406`.

- ``FLUSH`` commands written to the binary log could cause errors in case of
  replication. Bug fixed :psbug:`1827` (upstream :mysqlbug:`88720`).

- TokuDB storage engine couldn't be enabled on Docker images. Bug fixed :bug:`5283`.

- the ``ACCESS_DENIED`` field of the :table:`information_schema.user_statistics`
  table was not updated correctly. Bugs fixed :psbug:`3956`, :psbug:`4996`.

- postinst maintainer script in ``percona-server-server-5.6`` did not escape the
  password properly and as a result would fail to use it. Bug fixed :psbug:`4572`.

- the page cleaner could sleep for long time when the system clock was adjusted
  to an earlier point in time. Bug fixed :psbug:`5221` (upstream :mysqlbug:`93708`).

- executing ``SHOW BINLOG EVENT`` from an invalid position could result in a
  segmentation fault on 32bit machines. Bug fixed :psbug:`5243`.

- ``BLOB`` entries in the binary log could become corrupted
  in case when a database with ``Blackhole`` tables served as an
  intermediate binary log server in a replication chain. Bug fixed
  :psbug:`5353` (upstream :mysqlbug:`93917`).

- when :ref:`audit_log_plugin` was enabled, the server could use a lot of memory when
  handling large queries.  Bug fixed :psbug:`5395`.

- PerconaFT ``locktree`` library was re-licensed to Apache v2 license.
  Bug fixed :psbug:`5501`.

Other bugs fixed:
:psbug:`5512`,
:psbug:`5550`,
:psbug:`5578`,
:psbug:`5388` (upstream :mysqlbug:`94121`), and
:psbug:`5441`.

This release also contains the fixes for the following security issues:
CVE-2018-3123, CVE-2019-2683, CVE-2019-2627, and CVE-2019-2614.

Report bugs in the `Jira bug tracker <https://jira.percona.com/projects/PS>`_.
