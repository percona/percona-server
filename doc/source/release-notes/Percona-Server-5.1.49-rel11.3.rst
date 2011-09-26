.. rn:: 5.1.49-rel11.3

===============================
|Percona Server| 5.1.49-rel11.3
===============================

New features
============

  * |Percona Server| 5.1.49-rel11.3 is based on |MySQL| 5.1.49.

  * A new variable was introduced: :variable:`innodb_use_sys_stats_table`. If ``ON``, the table's statistics are stored statically in the internal table ``SYS_STATS``. The table is populated with the ``ANALYZE TABLE`` command. (*Yasufumi Kinoshita*)

  * A new session variable was introduced: :variable:`innodb_flush_log_at_trx_commit_session`. (*Yasufumi Kinoshita*)

Fixed bugs
===========

  * Bug :bug:`576041` - Fixes long stalls while accessing the ``innodb_buffer_pool_pages_index`` table on systems with a large number of tables

  * Bug :bug:`592007` - More strictly enforces the maximum purge delay defined by ``innodb_max_purge_lag`` by removing the requirement that purge operations be delayed if an old consistent read view exists that could see the rows to be purged.

  * Bug :bug:`607449` - Fixes a crash during shutdown when userstat_running=1

  * Bug :bug:`612954` - Fixes a problem with ``SHOW PROCESSLIST`` displaying an incorrect time

  * Bug :bug:`610525` - Reduces the number of compile time errors when the server is rebuilt

  * Bug :bug:`569275` - Fixes a crash when |XtraDB| shuts down in "crash resistent mode"
