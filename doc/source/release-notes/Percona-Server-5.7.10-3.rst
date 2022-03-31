.. rn:: 5.7.10-3

=========================
|Percona Server| 5.7.10-3
=========================

Percona is glad to announce the first GA (Generally Available) release of
|Percona Server| 5.7.10-3 on February 23rd, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.10-3/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.10
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-10.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.10-3 is the current Generally
Available release in the |Percona Server| 5.7 series. All of Percona's
software is open-source and free, all the details of the release can be found
in the `5.7.10-3 milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.7.10-3>`_

New Features
============

 Complete list of changes between |Percona Server| 5.6 and 5.7 can be seen in
 :ref:`changed_in_57`.

 |Percona Server| has implemented :ref:`Multi-threaded asynchronous LRU flusher
 <lru_manager_threads>`. This work also allows to safely use ``backoff`` value
 for the :variable:`innodb_empty_free_list_algorithm` server system variable,
 and its default has been changed accordingly.

Known Issues
============

 In |Percona Server| 5.7 `super_read_only
 <https://www.percona.com/doc/percona-server/5.6/management/super_read_only.html>`_
 feature has been replaced with upstream implementation. There are currently
 two known issues compared to |Percona Server| 5.6 implementation:

   * Bug :mysqlbug:`78963`, :variable:`super_read_only` aborts ``STOP SLAVE``
     if variable :variable:`relay_log_info_repository` is set to ``TABLE``
     which could lead to a server crash in Debug builds.

   * Bug :mysqlbug:`79328`, :variable:`super_read_only` set as a server option
     has no effect.

 InnoDB crash recovery might fail if :variable:`innodb_flush_method` is set
 to ``ALL_O_DIRECT``. The workaround is to set this variable to a different
 value before starting up the crashed instance (bug :bug:`1529885`).

Bugs Fixed
==========

 |Percona Server| :rn:`5.7.10-1` didn't write the initial root password into
 the log file :file:`/var/log/mysqld.log` during the installation on
 *CentOS 6*. Bug fixed :bug:`1541769`.

 Cardinality of partitioned TokuDB tables became inaccurate after the changes
 introduced by :ref:`tokudb_background_analyze_table` feature in |Percona
 Server| :rn:`5.7.10-1`. Bug fixed :tokubug:`925`.

 Running the ``TRUNCATE TABLE`` while :ref:`tokudb_background_analyze_table` is
 enabled could lead to a server crash once analyze job tries to access the
 truncated table. Bug fixed :tokubug:`938`.

 :ref:`toku_backup` would fail with an unclear error if backup process found
 :file:`mysqld_safe.pid` file (owned by root) inside the :variable:`datadir`.
 Fixed by excluding the ``pid`` file by default. Bug fixed :backupbug:`125`.

 :ref:`pam_plugin` build warning has been fixed. Bug fixed :bug:`1541601`.
