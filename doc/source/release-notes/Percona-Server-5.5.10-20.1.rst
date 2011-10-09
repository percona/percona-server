.. rn:: 5.5.10-20.1

============================
|Percona Server| 5.5.10-20.1
============================

Released on April 4, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.10-rc20.1/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_.)

|Percona Server| 5.5.10-20.1 is a release candidate.

New Features
============

  * Added columns ``ROWS_EXAMINED``, ``ROWS_SENT``, and ``ROWS_READ`` to table ``PROCESSLIST`` and to the output of ``SHOW PROCESSLIST``. (*Laurynas Biveinis*)

Variable Changes
================

  * Old status variable innodb_row_lock_numbers was renamed to innodb_current_row_locks. (*Yasufumi Kinoshita*)

  * Old system variable :variable:`innodb_enable_unsafe_group_commit` was deleted. The existing |MySQL| variable :variable:`innodb_support_xa` can be used instead. (*Yasufumi Kinoshita*)

  * Old system variable :variable:`log_warnings_silence` was renamed to :variable:`log_warnings_suppress`. (*Oleg Tsarev*)

  * Old system variable :variable:`log_slow_timestamp_every` was renamed to :variable:`slow_query_log_timestamp_always`. (*Oleg Tsarev*)

  * Old system variable :variable:`slow_query_log_microseconds_timestamp` was renamed to :variable:`slow_query_log_timestamp_precision`. (*Oleg Tsarev*)

  * Old system variable :variable:`use_global_log_slow_control` was renamed to :variable:`slow_query_log_use_global_control`. (*Oleg Tsarev*)

  * Old system variable :variable:`userstat_running` was renamed to :variable:`userstat`. (*Oleg Tsarev*)

  * Old system variable :variable:`innodb_expand_import_page` was renamed to :variable:`innodb_import_table_from_xtrabackup`. (*Yasufumi Kinoshita*)

  * Old system variable :variable:`innodb_auto_lru_dump` was renamed to :variable:`innodb_buffer_pool_restore_at_startup`. (*Yasufumi Kinoshita*)

  * Old system variable :variable:`innodb_overwrite_relay_log_info` was renamed to :variable:`innodb_recovery_update_relay_log`. (*Yasufumi Kinoshita*)

  * Old system variable :variable:`innodb_pass_corrupt_table` was renamed to :variable:`innodb_corrupt_table_action`. (*Yasufumi Kinoshita*)

Bug Fixes
=========

  * Bug :bug:`724674` - Ported an updated version of the original implementation of the :ref:`remove_fcntl_excessive_calls` feature, which removes some ``fcntl`` calls to improve performance. (*Oleg Tsarev*)

  * Bug :bug:`727704` - When using the :ref:`innodb_expand_import_page` feature, importing .ibd files created on |MySQL| 5.0 or |Percona Server| versions prior to 5.1.7 could crash the server. (*Yasufumi Kinoshita*)

  * |MySQL| bugs `56433 <http://bugs.mysql.com/56433>`_ and `51325 <http://bugs.mysql.com/51325>`_ - These |MySQL| bugs have been fixed in |Percona Server|. (*Yasufumi Kinoshita*)
