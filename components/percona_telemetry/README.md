To compile and run:
1. configure project with cmake `-DWITH_PERCONA_TELEMETRY=ON``
2. compile the server and the component
3. start mysqld
4. Percona Telemetry component is enabled by default
5. To disable without server restart execute `UNINSTALL COMPONENT "file://component_percona_telemetry";``
6. To make disable changes permanent set `percona_telemetry_disable=1` in `my.cnf`
```
mysql> show variables like '%percona_telemetry%';
+-----------------------------------------+---------------------------------+
| Variable_name                           | Value                           |
+-----------------------------------------+---------------------------------+
| percona_telemetry.grace_interval        | 86400                           |
| percona_telemetry.history_keep_interval | 604800                          |
| percona_telemetry.scrape_interval       | 86400                           |
| percona_telemetry.telemetry_root_dir    | /usr/local/percona/telemetry/ps |
| percona_telemetry_disable               | OFF                             |
+-----------------------------------------+---------------------------------+
```
Configurable in my.cnf:
```
percona_telemetry.grace_interval = 20
percona_telemetry.scrape_interval = 30
percona_telemetry.history_keep_interval = 70
percona_telemetry.telemetry_root_dir = /some/custom/dir
percona_telemetry_disable = ON/OFF
```
Note that `percona_telemetry.telemetry_root_dir` has to exist and be writable.

```
+-----------------------------------------+----------------------------------------------------------------------+
| Variable_name                           | Value                                                                |
+-----------------------------------------+----------------------------------------------------------------------+
| percona_telemetry.grace_interval        | 20                                                                   |
| percona_telemetry.history_keep_interval | 70                                                                   |
| percona_telemetry.scrape_interval       | 30                                                                   |
| percona_telemetry.telemetry_root_dir    | /usr/local/percona/telemetry/ps                                      |
| percona_telemetry_disable               | OFF                                                                  |
+-----------------------------------------+----------------------------------------------------------------------+

```
When the telemetry component is permanently disabled:
```
mysql> show variables like '%percona_telemetry%';
+---------------------------+-------+
| Variable_name             | Value |
+---------------------------+-------+
| percona_telemetry_disable | ON    |
+---------------------------+-------+
1 row in set (0,00 sec)
```


