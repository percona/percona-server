.. rn:: 5.7.26-29

==========================
|Percona Server| 5.7.26-29
==========================

Percona is glad to announce the release of |Percona Server| 5.7.26-29 on
May 27, 2019. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.26-29/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.26
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-26.html>`_
and includes all the bug fixes in it. |Percona Server| 5.7.26-29 is
now the current GA (Generally Available) release in the 5.7 series.

New Features
============

- New :variable:`Audit_log_buffer_size_overflow` status variable has been
  implemented to track when an :ref:`audit_log_plugin` entry was either
  dropped or written directly to the file due to its size being bigger
  than :variable:`audit_log_buffer_size` variable.

Bugs Fixed
==========

- TokuDB storage engine would assert on load when used with jemalloc 5.x.
  Bug fixed :psbug:`5406`.

- a read-write workload on compressed InnoDB tables could cause an assertion
  error. Bug fixed :psbug:`3581`.

- using TokuDB or MyRocks native partitioning and ``index_merge`` access method
  could lead to a server crash. Bugs fixed :psbug:`5206`, :psbug:`5562`.

- a stack buffer overrun could happen if the redo log encryption with
  key rotation was enabled. Bug fixed :psbug:`5305`.

- TokuDB and MyRocks native partitioning handler objects were allocated from a
  wrong memory allocator. Memory was released only on shutdown and concurrent
  access to global memory allocator caused memory corruptions and therefore
  crashes. Bugs fixed :psbug:`5508`, :psbug:`5525`.

- enabling redo log encryption resulted in redo log being written unencrypted.
  Bug fixed :psbug:`5547`.

- if there are multiple row versions in InnoDB, reading one row from PK may
  have O(N) complexity and reading from secondary keys may have O(N^2)
  complexity. Bugs fixed :psbug:`4712`, :psbug:`5450` (upstream :mysqlbug:`84958`).

- setting the :variable:`log_slow_verbosity` to include ``innodb`` value and
  enabling the :variable:`slow_query_log` could lead to a server crash.
  Bug fixed :psbug:`4933`.

- the page cleaner could sleep for long time when the system clock was adjusted
  to an earlier point in time. Bug fixed :psbug:`5221` (upstream :mysqlbug:`93708`).

- executing ``SHOW BINLOG EVENT`` from an invalid position could result in a
  segmentation fault on 32bit machines. Bug fixed :psbug:`5243`.

- ``BLOB`` entries in the binary log could become corrupted
  in case when a database with ``Blackhole`` tables served as an
  intermediate binary log server in a replication chain. Bug fixed
  :psbug:`5353` (upstream :mysqlbug:`93917`).

- when :ref:`audit_log_plugin` was enabled, the server could use a lot of
  memory when handling large queries.  Bug fixed :psbug:`5395`.

- :ref:`changed_page_tracking` was missing pages changed by the in-place DDL.
  Bug fixed :psbug:`5447`.

- :variable:`innodb_encrypt_tables` variable accepted ``FORCE`` option only
  inside quotes as a string. Bug fixed :psbug:`5538`.

- enabling redo log encryption and :ref:`changed_page_tracking` together would
  result in the error log flooded with decryption errors.
  Bug fixed :psbug:`5541`.

- system keyring keys initialization wasn't thread safe. Bugs fixed
  :psbug:`5554`.

- when using the Docker image, if the root passwords set in the mounted
  ``.cnf`` file and the one specified with ``MYSQL_ROOT_PASSWORD``
  are different, password from the ``MYSQL_ROOT_PASSWORD`` will be used.
  Bug fixed :psbug:`5573`.

- long running ``ALTER TABLE ADD INDEX`` could cause a ``semaphore wait > 600``
  assertion. Bug fixed :psbug:`3410` (upstream :mysqlbug:`82940`).

Other bugs fixed:
:psbug:`5007` (upstream :mysqlbug:`93164`),
:psbug:`5018`,
:psbug:`5561`,
:psbug:`5570`,
:psbug:`5578`,
:psbug:`5610`,
:psbug:`5441`, and
:psbug:`5442`.

This release also contains the fixes for the following security issues:
CVE-2019-2632, CVE-2019-1559, CVE-2019-2628, CVE-2019-2581, CVE-2019-2683,
CVE-2019-2592, CVE-2019-262, and CVE-2019-2614.

.. 5.7.26-29 replace:: 5.7.26-29
