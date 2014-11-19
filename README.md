The Tokutek hot backup library intercepts system calls that write files and duplicates the writes on backup files. It does this while copying files to the backup directory.  There are two technical issues to address to get hot backup working with MySQL.

First, the hot backup library must be loaded into the mysqld server so that it can intercept system calls.  We use LD_PRELOAD to solve the system call intercept problem.  Alternatively, the hot backup library can be linked into mysqld.

Second, there must be a user interface that can be used to start a backup, track its progress, and determine whether or not the backup succeeded.  We use a plugin that kicks off a backup as a side effect of setting a backup session variable to the name of the destination directory.

# Install the hot backup libraries
1 Extract the tarball

2 Copy lib/mysql/plugin/tokudb_backup.so to MySQL's lib/mysql/plugin directory

3 Copy lib/libHotBackup.so to MySQL's lib directory

4 Init mysqld
```
scripts/mysql_install_db
```

5 Run mysqld with the hot backup library (should exist in the lib directory)
```
LD_PRELOAD=PATH_TO_WHERE_HOT_BACKUP_LIVES/libHotBackup.so ./mysqld_safe
```

6 Install the backup plugin (should exist in the lib/mysql/plugin directory)
```
mysql> install plugin tokudb_backup soname 'tokudb_backup.so';
````

# Run a backup

1 Backup to the '/tmp/backup1047' directory.  This blocks until the backup is complete.
```
mysql> set tokudb_backup_dir='/tmp/backup1047';
```

2 Check if the backup worked
```
mysql> select @@tokudb_backup_last_error, @@tokudb_backup_last_error_string;
```

# Monitor progress
The Tokutek hot backup updates the processlist state with progress information while it is running.

# Build the hot backup plugin from source
1 Checkout the Percona Server source

2 Checkout the tokudb backup plugin with tag 'tokudb-backup-0.9'
```
cd percona-server-5.6/plugin
git clone git@github.com:Tokutek/tokudb-backup-plugin
```

3 Checkout the tokudb hot backup library with tag 'tokudb-backup-0.9'
```
cd percona-server-5.6/plugin/tokudb-backup-plugin
git clone git@github.com:Tokutek/backup-enterprise
ln -s backup-enterprise/backup backup
```

4 Build
```
cmake -DBUILD_CONFIG=mysql_release -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=../tokudb-backup-plugin-0.9-percona-server-5.6 -DTOKUDB_BACKUP_PLUGIN_VERSION=tokudb-backup-0.9 ../percona-server-5.6
cd plugin/tokudb-backup-plugin
make -j8 install
```

5 Make tarball
```
tar czf tokudb-backup-plugin-0.9-percona-server-5.6.tar.gz tokudb-backup-plugin-0.9-percona-server-5.6
```

# Work to do
- Add versions to library names
- Hack mysqld_safe to auto preload the hot backup lib if it finds one
- Run with valgrind and fix memory leaks if any