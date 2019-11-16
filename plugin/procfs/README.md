# procfs
procfs mysql `information_schema` plugin allows to get access to the row, unformatted contents of /proc and /sys files with SQL queries.
It could be useful for agent-less monitoring systems, for providing access to disk, networking and cpu statistics in cloud systems just with standard mysql protocol.
```
mysql> SELECT contents FROM information_schema.procfs WHERE file LIKE '/proc/irq/9/spurious'\G
*************************** 1. row ***************************
contents: count 22009
unhandled 1
last_unhandled 4294667480 ms

1 row in set (0.01 sec)
```

## Installation
1. put procfs directory inside plugin directory of mysql 5.7 or 8.0 source directory
2. configure mysql with cmake and build
3. If you have existing installation copy `procfs.so` to mysql `plugin_dir`
4. create or copy example `procfs.cnf` file to mysql data directory
5. In mysql-cli run install plugin sql command: `INSTALL PLUGIN procfs SONAME 'procfs.so';`

## Configuration
The plugin reads datadir/procfs.cnf file and caches files satisfying to all patterns for next 60 seconds.
procfs.cnf should contain paths to /sys or /proc files, one file per line. In addition you can use shell glob(7) syntax to specify multiple files:
```
/proc/cpuinfo
/sys/block/?d[a-z]/stat
/proc/irq/*/*
```
It's safe to specify path non-existent on this system or directories. The plugin silently skips invalid entries or matched directories.
```
/proc/net/sockstat*
```
glob expansion happens only during mysqld startup or plugin installation, you will not see the file appeared after mysqld was started without mysqld service restart.

## Usage
You can read one or multiple /sys or /proc files with single sql statement:
```
SELECT * FROM information_schema.procfs;
SELECT * FROM information_schema.procfs WHERE file = '/proc/irq/9/spurious';
SELECT * FROM information_schema.procfs WHERE file IN('/proc/irq/9/spurious', '/proc/irq/8/spurious');
SELECT contents FROM information_schema.procfs WHERE file LIKE '/proc/irq/_/spurious';
SELECT contents FROM information_schema.procfs WHERE file LIKE '/proc/irq/%/spurious';
```

All statements above will cause file reads only for matched files.
For more complex conditions the plugin will read all files and as a second step rows will be filtered by mysql runtime.
E.g.
```
SELECT * FROM information_schema.procfs WHERE file='/proc/irq/9/spurious' OR file='/proc/irq/8/spurious';
```
Returns same rows as a corresponding IN statement, but reads all configured /proc and /sys files.

## Limitations
1. Only first 60k of /proc/ /sys/ files returned
2. file name size is limited to 1k
3. the plugin can't read files if path is not starting from /proc or /sys
4. Complex WHERE conditions force plugin to read all configured files.
