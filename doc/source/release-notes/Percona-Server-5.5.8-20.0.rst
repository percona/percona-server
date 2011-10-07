.. rn:: 5.5.8-20.0

===========================
|Percona Server| 5.5.8-20.0
===========================

Released on February 16, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.8-beta20.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_.)

|Percona Server| 5.5.8-20.0 is a beta release.

New Features
============

  * |InnoDB| adaptive hash function searches can now be spread across multiple partitions (see :ref:`innodb_adaptive_hash_partitions_page`). Bug fixed: :bug:`688866`. (*Yasufumi Kinoshita*)

  * Information from ``SHOW INNODB STATUS`` was made available in new status variables in |InnoDB| Show Status. Bug fixed: :bug:`698797`.

Variable Changes
================

  * New variable :variable:`innodb_adaptive_flushing_method` was added.

  * New variable :variable:`innodb_use_global_flush_log_at_trx_commit` was added. Bug fixed: :bug:`635399`. (*Yasufumi Kinoshita*)

  * New variable :variable:`log_warnings_silence` replaced old variable :variable:`suppress_log_warning_1592`. Bug fixed: :bug:`692413`. (*Oleg Tsarev*)

  * Old variable :variable:`innodb_adaptive_checkpoint` was deleted. Bug fixed: :bug:`689450`. (*Yasufumi Kinoshita*)

  * Old variable :variable:`innodb_flush_log_at_trx_commit_session` was deleted. Bug fixed: :bug:`635399`. (*Yasufumi Kinoshita*)

  * Old variable :variable:`use_global_long_query_time` was deleted. Bug fixed: :bug:`692415`.  (*Oleg Tsarev*)

  * Old variable ``innodb_ibuf_accel_rate`` was renamed to ``innodb_ibuf_merge_rate``. Bug fixed: :bug:`695906` (*Yasufumi Kinoshita*)

  * Old variable ``innodb_ibuf_active_contract`` was renamed to ``innodb_ibuf_active_merge``. Bug fixed: :bug:`695906` (*Yasufumi Kinoshita*)

  * Old variable enable_query_response_time_stats was renamed to :variable:`query_response_time_stats`. (Oleg Tsarev)

  * Existing variable :variable:`log_slow_verbosity` had two new values added: ``profiling`` and ``profiling_use_getrusage``. (Oleg Tsarev)

  * Existing variables :variable:`profiling_server` and :variable:`profiling_use_getrusage` were merged into the Slow Query Log page. (*Oleg Tsarev*)

Other Changes
=============

  * Additional information was added to the ``LOG`` section of the ``SHOW STATUS`` command. Bug fixed: :bug:`693269`. (*Yasufumi Kinoshita*)

  * The ``SHOW PATCHES`` command was removed. (*Vadim Tkachenko*)

  * The ``INFORMATION_SCHEMA`` table ``XTRADB_ENHANCEMENTS`` was removed. (*Yasufumi Kinoshita*)

  * Several fields in the ``INFORMATION_SCHEMA`` table ``INNODB_INDEX_STATS`` were renamed. Bug fixed: :bug:`691777`. (Yasufumi Kinoshita)

  * The |XtraDB| version was set to 20.0. (*Aleksandr Kuzminsky*)

  * Many |InnoDB| compilation warnings were fixed. Bug fixed: :bug:`695273`. (*Yasufumi Kinoshita*)

  * An *Amazon* OS repository was created. Bug fixed: :bug:`691996`. (*Aleksandr Kuzminsky*)
