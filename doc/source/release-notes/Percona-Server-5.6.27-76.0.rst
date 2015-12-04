.. rn:: 5.6.27-76.0

==============================
 |Percona Server| 5.6.27-76.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.27-76.0 on December 4th, 2015 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.27-76.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.27 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-27.html>`_, including all the bug fixes in it, |Percona Server| 5.6.27-76.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.27-76.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.27-76.0>`_.

New Features
============

 ``SHOW SLAVE STATUS NOLOCK`` syntax in 5.6 has been undeprecated. Both ``SHOW SLAVE STATUS NOLOCK`` and ``SHOW SLAVE STATUS NONBLOCKING`` are now supported. |Percona Server| originally used ``SHOW SLAVE STATUS NOLOCK`` syntax for this feature. As of :rn:`5.6.20-68.0` release, |Percona Server| implements ``SHOW SLAVE STATUS NONBLOCKING`` syntax, which comes from early |MySQL| 5.7. Current |MySQL| 5.7 does not have this syntax and regular ``SHOW SLAVE STATUS`` is non-blocking.

 |TokuDB| tables can now be automatically :ref:`analyzed in the background <tokudb_background_analyze_table>` based on a measured change in data.
 
 |Percona Server| has implemented new :variable:`tokudb_strip_frm_data` variable which can be used to assist in |TokuDB| data recovery. **WARNING:** Use this variable only if you know what you're doing otherwise it could lead to data loss. 

Bugs Fixed
==========
 
 Setting the :variable:`tokudb_backup_last_error_string` and :variable:`tokudb_backup_last_error` values manually could cause server assertion. Bug fixed :bug:`1512464`.

 Fixed invalid memory accesses when :program:`mysqldump` was running with ``--innodb-optimize-keys`` option. Bug fixed :bug:`1517444`.

 Fixed incorrect filename specified in :file:`storage/tokudb/PerconaFT/buildheader/CMakeLists.txt`  which could cause subsequent builds to fail. Bug fixed :bug:`1510085` (*Sergei Golubchik*).

 Fixed multiple issues with |TokuDB| CMake scripts. Bugs fixed :bug:`1510092`, :bug:`1509219` and :bug:`1510081` (*Sergei Golubchik*).

 An upstream fix for upstream bug :mysqlbug:`76135` might cause server to stall or hang. Bug fixed :bug:`1519094` (upstream :mysqlbug:`79185`).

 :program:`ps_tokudb_admin` now prevents :ref:`toku_backup` activation if there is no |TokuDB| storage engine on the server. Bug fixed :bug:`1520099`.

 :ref:`toku_backup` plugin now gets removed during the |TokuDB| storage engine uninstall process. Bug fixed :bug:`1520472`.

 New :option:`--defaults-file` option has been implemented for :program:`ps_tokudb_admin` to specify the |MySQL| configuration file if it's not in the default location. Bug fixed :bug:`1517021`.

Other bugs fixed: :bug:`1425480`, :bug:`1517523`, :bug:`1515741` and :bug:`1513178`. 
