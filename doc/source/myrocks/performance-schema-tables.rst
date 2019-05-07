.. _performance_schema_tables:

-------------------------------------------------------------------------------
Performance Schema MyRocks changes
-------------------------------------------------------------------------------

.. _log_status:

RocksDB WAL file information can be seen in the
`performance_schema.log_status <https://dev.mysql.com/doc/mysql-perfschema-excerpt/8.0/en/log-status-table.html>`__
table in the ``STORAGE ENGINE`` column.

This feature has been implemented in Percona Server :rn:`8.0.15-6` release.

Example
-------------------------------------------------------------------------------

.. code-block:: mysql

  mysql> select * from performance_schema.log_status\G
  *************************** 1. row ***************************
    SERVER_UUID: f593b4f8-6fde-11e9-ad90-080027c2be11
          LOCAL: {"gtid_executed": "", "binary_log_file": "binlog.000004", "binary_log_position": 1698222}
    REPLICATION: {"channels": []}
    STORAGE_ENGINES: {"InnoDB": {"LSN": 36810235, "LSN_checkpoint": 36810235}, "RocksDB": {"wal_files": [{"path_name": "/000026.log", "log_number": 26, "size_file_bytes": 371869}]}}
    1 row in set (0.00 sec)
