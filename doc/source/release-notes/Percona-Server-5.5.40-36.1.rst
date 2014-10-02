.. rn:: 5.5.40-36.1

==============================
 |Percona Server| 5.5.40-36.1
==============================

Percona is glad to announce the release of |Percona Server| 5.5.40-36.1 on October 7th, 2014. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.40-36.1/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.40 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-40.html>`_, including all the bug fixes in it, |Percona Server| 5.5.40-36.1 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.40-36.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.40-36.1>`_. 

Bugs Fixed
==========

 :ref:`audit_log_plugin` would rotate the audit log in middle of an audit message. Bug fixed :bug:`1363370`.
 
 Fixed a memory leak in :ref:`scalability_metrics_plugin`. Bug fixed :bug:`1334570`.

 Fixed a memory leak if :ref:`secure-file-priv <secure_file_priv_extended>` option was used with no argument. Bug fixed :bug:`1334719`.

