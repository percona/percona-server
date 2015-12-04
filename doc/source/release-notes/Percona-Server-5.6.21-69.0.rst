.. rn:: 5.6.21-69.0

==============================
 |Percona Server| 5.6.21-69.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.21-69.0 on October 7th, 2014 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.21-69.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.21 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-21.html>`_, including all the bug fixes in it, |Percona Server| 5.6.21-69.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.21-69.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.21-69.0>`_. 


New Features
============

 :ref:`response_time_distribution` feature has been from |Percona Server| 5.5 as a plugin.

 TokuDB storage engine package has been updated to version `7.5.1 <http://docs.tokutek.com/tokudb/tokudb-release-notes.html#tokudb-7-5-0>`_. 

Bugs Fixed
==========

 :ref:`backup_locks` did not guarantee consistent ``SHOW SLAVE STATUS`` information with binary log disabled. Bug fixed :bug:`1358836`.

 :ref:`audit_log_plugin` would rotate the audit log in middle of an audit message. Bug fixed :bug:`1363370`.

 When the binary log is enabled on a replication slave, ``SHOW SLAVE STATUS`` performed under an active BINLOG lock could lead to a deadlock. Bug fixed :bug:`1372806`.
 
 Fixed a memory leak in :ref:`scalability_metrics_plugin`. Bug fixed :bug:`1334570`.

 Fixed a memory leak if :ref:`secure-file-priv <secure_file_priv_extended>` option was used with no argument. Bug fixed :bug:`1334719`.

 ``LOCK TABLES FOR BACKUP`` is now incompatible with ``LOCK TABLES``, ``FLUSH TABLES WITH READ LOCK``, and ``FLUSH TABLES FOR EXPORT`` in the same connection. Bug fixed :bug:`1360064`.

Other bugs fixed: :bug:`1361568`.


