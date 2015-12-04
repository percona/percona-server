.. rn:: 5.6.11-60.3

==============================
 |Percona Server| 5.6.11-60.3
==============================

Percona is glad to announce the first Release Candidate release of |Percona Server| 5.6.11-60.3 on June 3rd, 2013 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.11-60.3/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.6.11 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-11.html>`_, including all the bug fixes in it, |Percona Server| 5.6.11-60.3 is the  first RC release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.11-60.3 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.11-60.3>`_.

This release contains all the bug fixes from latest |Percona Server| 5.5 release (currently |Percona Server| `5.5.31-30.3 <http://www.percona.com/doc/percona-server/5.5/release-notes/Percona-Server-5.5.31-30.3.html>`_). 

New Features
============

 |Percona Server| has implemented :ref:`log_archiving`. Currently this feature implementation is considered *ALPHA*.


Ported Features
===============

 |Percona Server| has ported priority connection scheduling for the :ref:`threadpool` from |Percona Server| 5.5.

 |Percona Server| has ported the :ref:`atomic_fio` patch from |MariaDB|. This feature adds atomic write support for ``directFS`` filesystem on ``Fusion-io`` devices. This feature implementation is considered BETA quality. 

 |Percona Server| has ported :variable:`innodb_read_views_memory` and :variable:`innodb_descriptors_memory` status variables in the :ref:`innodb_show_status` to improve |InnoDB| memory usage diagnostics.

 :ref:`innodb_io_page` has been ported from |Percona Server| 5.5

 :ref:`innodb_numa_support` has been ported from |Percona Server| 5.5

 :ref:`log_warning_suppress` has been ported from |Percona Server| 5.5

 :ref:`improved_memory_engine` has been ported from |Percona Server| 5.5

 :ref:`maximum_binlog_files` has been ported from |Percona Server| 5.5

 :ref:`log_connection_error` has been ported from |Percona Server| 5.5

 :ref:`error_pad` has been ported from |Percona Server| 5.5

 :ref:`show_slave_status_nolock` has been ported from |Percona Server| 5.5

 :ref:`udf_percona_toolkit` has been ported from |Percona Server| 5.5

 :ref:`innodb_fake_changes_page` has been ported from |Percona Server| 5.5

 :ref:`innodb_kill_idle_trx` has been ported from |Percona Server| 5.5

 :ref:`enforce_engine` has been ported from |Percona Server| 5.5

 :ref:`psaas_utility_user` has been ported from |Percona Server| 5.5

 :ref:`secure_file_priv_extended` has been ported from |Percona Server| 5.5

 :ref:`expanded_option_modifiers` has been ported from |Percona Server| 5.5

 :ref:`changed_page_tracking` has been ported from |Percona Server| 5.5

 :ref:`pam_plugin` has been ported from |Percona Server| 5.5

 :ref:`user_stats` has been ported from |Percona Server| 5.5

 :ref:`slow_extended` has been ported from |Percona Server| 5.5

 :ref:`innodb_show_status` has been ported from |Percona Server| 5.5

 :ref:`innodb_deadlocks_page` has been ported from |Percona Server| 5.5

 :ref:`mysql_syslog` has been ported from |Percona Server| 5.5

 :ref:`show_engines` has been ported from |Percona Server| 5.5

 :ref:`thread_based_profiling` has been ported from |Percona Server| 5.5


Bug Fixes
==========

 Transaction objects are now allocated calling ``calloc()`` directly instead of using |InnoDB| heap allocation. This may improve write performance for high levels of concurrency. Bug fixed :bug:`1185686`.

 Under very rare circumstances, deleting a zero-size bitmap file at the right moment would make server stop with an I/O error if changed page tracking is enabled. Bug fixed :bug:`1184517`.

 Missing path separator between the directory and file name components in a bitmap file name could stop the server starting if the :variable:`innodb_data_home_dir` variable didn't have the path separator at the end. Bug fixed :bug:`1181887`.

 Changed page tracking used to hold the log system mutex for the log reads needlessly, potentially limiting performance on write-intensive workloads. Bug fixed :bug:`1171699`.

 Incorrect schema definition for the :ref:`user_stats` tables in ``INFORMATION_SCHEMA`` (:table:`CLIENT_STATISTICS`, :table:`INDEX_STATISTICS`, :table:`TABLE_STATISTICS`, :table:`THREAD_STATISTICS`, and :table:`USER_STATISTICS`) led to the maximum counter values being limited to 32-bit signed integers. Fixed so that these values can be 64-bit unsigned integers now. Bug fixed :bug:`714925`.

 Server would crash if an :table:`INNODB_CHANGED_PAGES` query is issued that has an empty LSN range and thus does not need to read any bitmap files. Bug fixed :bug:`1184427`.

 Query to the :table:`INNODB_CHANGED_PAGES` table would cause server to stop with an I/O error if a bitmap file in the middle of requested LSN range was missing. Bug fixed :bug:`1179974`.

 A warning is now returned if a bitmap file I/O error occurs after an :table:`INNODB_CHANGED_PAGES` query started returning data to indicate an incomplete result set. Bug fixed :bug:`1185040`.
 
 The :table:`INNODB_CHANGED_PAGES` table couldn't be queried if the log tracker wasn't running. Bug fixed :bug:`1185304`.

 Fixed the upstream bug :mysqlbug:`68970` that, in |Percona Server|, would cause small tablespaces to expand too fast around 500KB tablespace size. Bug fixed :bug:`1169494`.

 Fixed the ``RPM`` package dependencies issues. Bug fixed :bug:`1186831`.

 Reduced the overhead from :ref:`innodb_corrupt_table_action_page` check as it was missing branch predictor annotations. Bug fixed :bug:`1176864`.

Other bugs fixed: bug fixed :bug:`1184695`, bug fixed :bug:`1184512`, bug fixed :bug:`1183585`, bug fixed :bug:`1178606`, bug fixed :bug:`1177356`, bug fixed :bug:`1160895`, bug fixed :bug:`1182876`, bug fixed :bug:`1180481`, bug fixed :bug:`1163135`, bug fixed :bug:`1157078`, bug fixed :bug:`1182889`, bug fixed :bug:`1133926`, bug fixed :bug:`1165098`, bug fixed :bug:`1182793`, bug fixed :bug:`1157075`, bug fixed :bug:`1183625`, bug fixed :bug:`1155475`, bug fixed :bug:`1157037`, bug fixed :bug:`1182065`, bug fixed :bug:`1182837`, bug fixed :bug:`1177780`, bug fixed :bug:`1154954`.

