.. rn:: 5.1.49-rel12.0

===============================
|Percona Server| 5.1.49-rel12.0
===============================

New features
============

  * |Percona Server| 5.1.49-rel12.0 is based on |MySQL| 5.1.49.

  * New features added:

    * :ref:`error_pad` - Implements error code compatibilities with |MySQL| 5.5. (*Oleg Tsarev*)

    * ``query_cache_totally_disable`` - Allows the user to disable use of the query cache. (*Oleg Tsarev*, backport from |MySQL| 5.5)

    * ``show_engines`` - Changes ``SHOW STORAGE ENGINES`` to report |XtraDB| when appropriate. (*Oleg Tsarev*)

    * :ref:`remove_fcntl_excessive_calls` - Removes excessive fcntl calls. (*Oleg Tsarev*, port from *FaceBook* tree)

    * :ref:`sql_no_fcache` - Prevents blocks of data from being cached to FlashCache during a query. (*Oleg Tsarev*, port from *FaceBook* tree)

    * ``status_wait_query_cache_mutex`` - Provides a new thread state - "Waiting on query cache mutex". (*Oleg Tsarev*)

    * :ref:`log_connection_error` - Issues the warning “Too many connection” if log_warnings is enabled. (*Oleg Tsarev*)

    * :ref:`response_time_distribution` - Counts queries with very short execution times and groups them by time interval. (*Oleg Tsarev*)

    * :ref:`innodb_buffer_pool_shm` - Allows storing the buffer pool in a shared memory segment between restarts of the server. (*Yasufumi Kinoshita*)

    * Option :ref:`mysql_syslog` was added to the |MySQL| client. If enabled, all commands run on the client are logged to syslog. (*Oleg Tsarev*)

  * New variables introduced:

    * :ref:`innodb_io_page` - Implements a session-level version of the |MySQL| global system variable ``innodb_flush_log_at_trx_commit``. (*Yasufumi Kinoshita*)

    * :ref:`innodb_fast_index_creation` - Allows disabling of fast index creation. (*Yasufumi Kinoshita*)

    * :ref:`innodb_stats`- If ON, the table's statistics are stored statically in the internal table ``SYS_STATS``. The table is populated with the ``ANALYZE TABLE`` command. (*Yasufumi Kinoshita*)
 

Fixed bugs
==========

  * Bug :bug:`576041` - Fixes long stalls while accessing the ``innodb_buffer_pool_pages_index`` table on systems with a large number of tables.

  * Bug :bug:`592007` - More strictly enforces the maximum purge delay defined by ``innodb_max_purge_lag`` by removing the requirement that purge operations be delayed if an old consistent read view exists that could see the rows to be purged.

  * Bug :bug:`607449` - Fixes a crash during shutdown when ``userstat_running=1``.

  * Bug :bug:`612954` - Fixes a problem with ``SHOW PROCESSLIST`` displaying an incorrect time.

  * Bug :bug:`610525` - Reduces the number of compile time errors when the server is rebuilt.

  * Bug :bug:`569275` - Fixes a crash when |XtraDB| shuts down in "crash resistent mode".

  * Bug :bug:`589484` - Adds reporting of the query cache mutex status in ``SHOW PROCESSLIST``.

  * Bug :bug:`606965` - Allows preventing data caching to flash storage during a query.

  * Bug :bug:`606810` - Ports a fix from to remove excessive ``fcntl`` calls.

  * Bug :bug:`609027` - Allows query cache use to be completely disabled

  * Bug :bug:`600352` - Fixes ``SHOW STORAGE ENGINES`` to correctly report "Percona-XtraDB" rather than "InnoDB"
