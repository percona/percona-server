.. rn:: 8.0.15-6

================================================================================
|Percona Server| |release|
================================================================================

|Percona| announces the release of |Percona Server| |release| on |date|
(downloads are available `here
<https://www.percona.com/downloads/Percona-Server-8.0/>`__ and from the `Percona
Software Repositories
<https://www.percona.com/doc/percona-server/8.0/installation.html#installing-from-binaries>`__).

This release includes fixes to bugs found in previous releases of |Percona
Server| 8.0.

|Percona Server| |release| is now the current GA release in the 8.0
series. All of |Percona|’s software is open-source and free.

Percona Server for MySQL 8.0 includes all the `features available in MySQL 8.0
Community Edition
<https://dev.mysql.com/doc/refman/8.0/en/mysql-nutshell.html>`__ in addition to
enterprise-grade features developed by Percona.  For a list of highlighted
features from both MySQL 8.0 and Percona Server for MySQL 8.0, please see the
`GA release announcement
<https://www.percona.com/blog/2018/12/21/announcing-general-availability-of-percona-server-for-mysql-8-0/>`__.

.. note::

   If you are upgrading from 5.7 to 8.0, please ensure that you read the
   `upgrade guide
   <https://www.percona.com/doc/percona-server/8.0/upgrading_guide.html>`__ and
   the document `Changed in Percona Server for MySQL 8.0
   <https://www.percona.com/doc/percona-server/8.0/changed_in_version.html>`__.

New Features
================================================================================

- The server part of MyRocks cross-engine consistent physical backups has been
  implemented by introducing :variable:`rocksdb_disable_file_deletions` and 
  :variable:`rocksdb_create_temporary_checkpoint` session variables. These
  variables are intended to be used by backup tools. Prolonged use or
  other misuse can have serious side effects to the server instance.

- RocksDB WAL file information can now be seen in the
  :table:`performance_schema.log_status` :ref:`table <log_status>`.

- New :variable:`Audit_log_buffer_size_overflow` status variable has been
  implemented to track when an :ref:`audit_log_plugin` entry was either
  dropped or written directly to the file due to its size being bigger
  than :variable:`audit_log_buffer_size` variable.
  

Bugs Fixed
================================================================================

- TokuDB and MyRocks native partitioning handler objects were allocated from a 
  wrong memory allocator. Memory was released only on shutdown and concurrent
  access to global memory allocator caused memory corruptions and therefore
  crashes. Bug fixed :psbug:`5508`.

- using TokuDB or MyRocks native partitioning and ``index_merge`` could lead to
  a server crash. Bugs fixed :psbug:`5206`, :psbug:`5562`.

- upgrade from |Percona Server| 5.7.24 to :rn:`8.0.13-3` wasn't working with
  encrypted undo tablespaces. Bug fixed :psbug:`5223`.

- :ref:`keyring_vault_plugin` couldn't be initialized on *Ubuntu Cosmic 17.10*.
  Bug fixed :psbug:`5453`.

- rotated key encryption did not register ``encryption_key_id`` as a valid
  table option. Bug fixed :psbug:`5482`.

- :table:`INFORMATION_SCHEMA.GLOBAL_TEMPORARY_TABLES` queries could crash if
  online ``ALTER TABLE`` was running in parallel. Bug fixed :psbug:`5566`.

- setting the :variable:`log_slow_verbosity` to include ``innodb`` value and
  enabling the :variable:`slow_query_log` could lead to a server crash.
  Bug fixed :psbug:`4933`.

- :ref:`compression_dictionary` operations were not allowed under
  :variable:`innodb-force-recovery`. Now they work correctly when
  :variable:`innodb_force_recovery` is <= ``2``, and are forbidden when
  :variable:`innodb_force_recovery` is >= ``3``.
  Bug fixed :psbug:`5148`.

- ``BLOB`` entries in the binary log could become corrupted
  in case when a database with ``Blackhole`` tables served as an
  intermediate binary log server in a replication chain. Bug fixed
  :psbug:`5353`.

- ``FLUSH CHANGED_PAGE_BITMAPS`` would leave gaps between the last written
  bitmap LSN and the |InnoDB| checkpoint LSN. Bug fixed :psbug:`5446`.

- :ref:`changed_page_tracking` was missing pages changed by the in-place DDL.
  Bug fixed :psbug:`5447`.

- ``innodb_system`` tablespace information was missing from the 
  :table:`INFORMATION_SCHEMA.innodb_tablespaces` view.
  Bug fixed :psbug:`5473`.

- undo log tablespace encryption status is now available through 
  :table:`INFORMATION_SCHEMA.innodb_tablespaces` view.
  Bug fixed :psbug:`5485` (upstream :mysqlbug:`94665`).

- enabling temporay tablespace encryption didn't mark the 
  ``innodb_temporary`` tablespace with the encryption flag. Bug fixed
  :psbug:`5490`.

- server would crash during bootstrap if :variable:`innodb_encrypt_tables`
  was set to ``1``. Bug fixed :psbug:`5492`.

- fixed intermittent shutdown crashes that were happening if :ref:`threadpool`
  was enabled. Bug fixed :psbug:`5510`.

- compression dictionary ``INFORMATION_SCHEMA`` views were missing when 
  :term:`datadir` was upgraded from 8.0.13 to 8.0.15. Bug fixed :psbug:`5529`.

- :variable:`innodb_encrypt_tables` variable accepted ``FORCE`` option only
  as a string. Bug fixed :psbug:`5538`. 

- ``ibd2sdi`` utility was missing in Debian/Ubuntu packages. Bug fixed
  :psbug:`5549`.

- Docker image is now ignoring password that is set in the configuration 
  file when first initializing. Bug fixed :psbug:`5573`.

- long running ``ALTER TABLE ADD INDEX`` could cause a ``semaphore wait > 600``
  assertion. Bug fixed :psbug:`3410` (upstream :mysqlbug:`82940`).

- system keyring keys initialization wasn't thread safe. Bugs fixed
  :psbug:`5554`.

- :ref:`backup_locks` was blocking DML for RocksDB. Bug fixed :psbug:`5583`.

- PerconaFT ``locktree`` library was re-licensed to Apache v2 license.
  Bug fixed :psbug:`5501`.

Other bugs fixed:
:psbug:`5537`,
:psbug:`5243`,
:psbug:`5371`,
:psbug:`5475`,
:psbug:`5484`,
:psbug:`5512`,
:psbug:`5514`,
:psbug:`5523`,
:psbug:`5528`,
:psbug:`5536`,
:psbug:`5550`,
:psbug:`5570`,
:psbug:`5578`,
:psbug:`5441`,
:psbug:`5442`,
:psbug:`5456`,
:psbug:`5462`,
:psbug:`5487`,
:psbug:`5489`,
:psbug:`5520`, and
:psbug:`5560`.

.. |release| replace:: 8.0.15-6
.. |date| replace:: May 07, 2019
