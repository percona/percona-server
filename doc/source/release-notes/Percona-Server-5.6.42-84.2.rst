.. rn:: 5.6.42-84.2

================================================================================
|Percona Server| |release|
================================================================================

Percona is glad to announce the release of |Percona Server| |release| on
November 29, 2018 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.42-84.2/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.42
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-42.html>`_, including
all the bug fixes in it, |Percona Server| |release| is the current GA release in
the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and
free.

Improvements
================================================================================

- :psbug:`4790`: Improve user statistics accuracy

.. rubric:: Other improvements

- :psbug:`4881`: Add LLVM/clang 7 to Travis-CI


Bugs Fixed
================================================================================

- Slave replication could break if upstream bug :mysqlbug:`74145`
  (*FLUSH LOGS improperly disables the logging if the log file cannot
  be accessed*) occurred in master. Bug fixed :psbug:`1017` (Upstream
  :mysqlbug:`83232`).
- The binary log could be corrupted when the disk partition used for temporary. 
  files (``tmpdir`` system variable) had little free space. Bug fixed :psbug:`1107` (Upstream :mysqlbug:`72457`).
- PURGE CHANGED_PAGE_BITMAPS did not work when the ``innodb_data_home_dir`` system
  variable was used. Bug fixed :psbug:`4723`.
- Setting the ``tokudb_last_lock_timeout`` variable via command
  line could cause server to stop working when the actual timeout took place. Bug fixed :psbug:`4943`.
- Dropping TokuDB table with non-alphanumeric characters could lead to a crash. Bug fixed :psbug:`4979`.
 
.. rubric:: Other bugs fixed

- :psbug:`4781`: sql_yacc.yy uses SQLCOM_SELECT instead of SQLCOM_SHOW_XXXX_STATS
- :psbug:`4529`: MTR: index_merge_rocksdb2 inadvertently tests InnoDB instead of MyRocks
- :psbug:`4746`: Revert our fix for PS-3851 (Percona Ver 5.6.39-83.1 Failing assertion: sym_node->table != NULL)
- :psbug:`4773`: Percona Server sources can't be compiled without server
- :psbug:`4785`: Setting version_suffix to NULL leads to handle_fatal_signal (sig=11) in Sys_var_version::global_value_ptr
- :psbug:`4813`: Using flush_caches leads to SELinux denial errors

Find the release notes for Percona Server for MySQL 5.6.42-84.2 in our
`online documentation
<https://www.percona.com/doc/percona-server/5.6/index.html>`_. Report
bugs in the `Jira bug tracker <https://jira.percona.com/projects/PS>`_.

.. |release| replace:: 5.6.42-84.2
