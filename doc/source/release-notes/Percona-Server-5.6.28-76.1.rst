.. rn:: 5.6.28-76.1

==============================
 |Percona Server| 5.6.28-76.1 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.28-76.1 on January 12th, 2016 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.28-76.1/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.28 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-28.html>`_, including all the bug fixes in it, |Percona Server| 5.6.28-76.1 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.28-76.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.28-76.1>`_.

Bugs Fixed
==========
 
 Clustering secondary index could not be created on a partitioned |TokuDB| table. Bug fixed :bug:`1527730` (:tokubug:`720`).

 When enabled, :ref:`super-read-only` could break statement-based replication while executing a multi-table update statement on a slave. Bug fixed :bug:`1441259`.

 Running ``OPTIMIZE TABLE`` or ``ALTER TABLE`` without the ``ENGINE`` clause would silently change table engine if :variable:`enforce_storage_engine` variable was active. This could also result in system tables being changed to incompatible storage engines, breaking server operation. Bug fixed :bug:`1488055`.

 Setting the :variable:`innodb_sched_priority_purge` (available only in debug builds) while purge threads were stopped would cause a server crash. Bug fixed :bug:`1368552`.

 Small buffer pool size could cause |XtraDB| buffer flush thread to spin at 100% CPU. Bug fixed :bug:`1433432`. 

 Enabling |TokuDB| with ``ps_tokudb_admin`` script inside the Docker container would cause an error due to insufficient privileges even when running as root. In order for this script to be used inside docker containers this error has be been changed to a warning that a check is impossible. Bug fixed :bug:`1520890`. 

 |InnoDB| status will start printing negative values for spin rounds per wait, if the wait number, even though being accounted as a signed 64-bit integer, will not fit into a signed 32-bit integer. Bug fixed :bug:`1527160` (upstream :mysqlbug:`79703`).

Other bugs fixed: :bug:`1384595` (upstream :mysqlbug:`74579`), :bug:`1384658` (upstream :mysqlbug:`74619`), :bug:`1471141` (upstream :mysqlbug:`77705`), :bug:`1179451`, :bug:`1524763` and :bug:`1530102`.
