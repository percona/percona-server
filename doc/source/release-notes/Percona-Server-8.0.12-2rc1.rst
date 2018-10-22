.. rn:: 8.0.12-2rc1

================================================================================
|Percona Server| |release|
================================================================================

Following the `alpha release announced earlier
<https://www.percona.com/blog/2018/09/27/announcement-alpha-build-of-percona-server-8-0/>`_,
Percona announces the release candidate of Percona Server for MySQL |release| on
October 31, 2018. Download the latest version from the Percona web site or the
Percona Software Repositories.

This release is based on |MySQL| 8.0.12 and includes all bug fixes in it. It is
a *Release Candidate* quality release and it is not intended for
production. If you want a high quality, Generally Available release, use the
current Stable version (the most recent stable release at the time of writing in
the 5.7 series is 5.7.23-23).

This release removes the features marked as *deprecated* in |Percona Server| 5.7.

Percona provides completely open-source and free software.

Installation
================================================================================

As this is a release candidate, installation is performed by enabling the
testing repository and installing the software via your package manager.  For
Debian based distributions, see `apt installation instructions
<https://www.percona.com/doc/percona-server/8.0/installation/apt_repo.html>`_;
for RPM based distributions. see `yum installation instructions
<https://www.percona.com/doc/percona-server/8.0/installation/yum_repo.html>`_.
Note that in both cases after installing the current percona-release package,
you’ll need to enable the testing repository in order to install Percona Server
for MySQL 8.0.12-2rc1.  For manual installations you can download from the
`testing repository directly through our website
<https://www.percona.com/downloads/TESTING/>`_.

New Features
================================================================================

- :psbug:`4550`: Native Partitioning support for MyRocks storage engine
- :psbug:`3911`: Native Partitioning support for TokuDB storage engine
- :psbug:`275`: Remove the deprecated variables *max_binlog_files* and *binlog_space_limit*
- :psbug:`278`: Remove the deprecated *scalability metrics* plugin
- :psbug:`279`: Remove the CONCURRENT_CONNECTIONS column from the
  THREAD_STATISTICS table in INFORMATION_SCHEMA
- :psbug:`280`: Remove the compatibility *innodb_kill_idle_trx* alias
- :psbug:`4361`: Remove the deprecated *tokudb_support_xa* variable
- :psbug:`4946`: Add an option to prevent implicit creation of column family in MyRocks

Other new features:

- :psbug:`281`: Convert LP 1354988 fix to new "SHOW EFFECTIVE GRANTS"

Improvements
================================================================================

- :psbug:`3945`: Convert user statistics time columns to floating point values
- :psbug:`4749`: Fix gcc/clang warnings for 8.0 branch
- :psbug:`4843`: Add *relay_master_log_file* and *exec_master_log_position*
  channel properties to the *p_s.log_status*

Bugs Fixed
================================================================================

- :psbug:`4785`: Setting version_suffix to **NULL** could lead to *handle_fatal_signal* (sig=11) in *Sys_var_version::global_value_ptr*
- :psbug:`4788`: Setting *log_slow_verbosity* and enabling the *slow_query_log* could lead to a server crash
- :psbug:`4947`: Any index comment generated a new column family in MyRocks
- :psbug:`1107`: Binlog could be corrupted when *tmpdir* got full
- :psbug:`1549`: Server side prepared statements lead to a potential off-by-second timestamp on slaves
- :psbug:`4814`: TokuDB *fast* replace into was incompatible with 8.0 row replication
- :psbug:`4937`: *rocksdb_update_cf_options* was useless when specified in my.cnf or on command line.
- :psbug:`4705`: The server could crash on snapshot size check in RocksDB
- :psbug:`4736`: key buffer overrun
- :psbug:`4791`: SQL injection on slave due to non-quoting in binlogged ROLLBACK TO SAVEPOINT
- :psbug:`4834`: encrypted system tablespace had an empty uuid
- :psbug:`4953`: *rocksdb.truncate_table3* was unstable

Other bugs fixed: 

- :psbug:`4738`: Fix Travis-CI configuration for Trusty and 8.0
- :psbug:`4745`: Travis CI jobs timeout for 5.7 and 8.0
- :psbug:`4755`: MTR does not run with default suite run since the TokuDB push
- :psbug:`4818`: audit_null.audit_plugin_bugs test always failing
- :psbug:`4811`: 5.7 Merge and fixup for old DB-937 introduces possible regression
- :psbug:`4885`: Using ALTER ... ROW_FORMAT=TOKUDB_QUICKLZ leads to InnoDB: Assertion failure: ha_innodb.cc:12198:m_form->s->row_type == m_create_info->row_type

Upcoming Features
================================================================================

- `New encryption features <https://www.percona.com/doc/percona-server/8.0/management/data_at_rest_encryption.html>`_ in Percona Server for MySQL 5.7 will be ported forward to Percona Server for MySQL 8.0
- Adding back in `column compression with custom data dictionaries <https://www.percona.com/doc/percona-server/8.0/flexibility/compressed_columns.html>`_

.. |release| replace:: 8.0.12-2rc1
