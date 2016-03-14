.. _known_issues:

============================
Known issues and limitations
============================

In |Percona Server| 5.7 `super_read_only <https://www.percona.com/doc/percona-server/5.6/management/super_read_only.html>`_ feature has been replaced with upstream implementation. There are currently two known issues compared to |Percona Server| 5.6 implementation:

  * Bug :mysqlbug:`78963`, :variable:`super_read_only` aborts ``STOP SLAVE`` if variable :variable:`relay_log_info_repository` is set to ``TABLE`` which could lead to a server crash in Debug builds.

  * Bug :mysqlbug:`79328`, :variable:`super_read_only` set as a server option has no effect.

|InnoDB| crash recovery might fail if :variable:`innodb_flush_method` is set to ``ALL_O_DIRECT``. The workaround is to set this variable to a different value before starting up the crashed instance (bug :bug:`1529885`).
