The Tokutek hot backup library intercepts system calls that write files and duplicates the writes on backup files. It does this while copying files to the backup directory.  There are two technical issues to address to get hot backup working with MySQL.

First, the hot backup library must be loaded into the mysqld server so that it can intercept system calls.  We use LD_PRELOAD to solve the system call intercept problem.  Alternatively, the hot backup library can be linked into mysqld.

Second, there must be a user interface that can be used to start a backup, track its progress, and determine whether or not the backup succeeded.  We use a plugin that kicks off a backup as a side effect of setting a backup session variable to the name of the destination directory.

# Install the hot backup libraries
1 Extract the tarball

2 Copy tokudb_backup.so to MySQL's plugin directory

3 Copy libHotBackup.so to MySQL's lib directory

4 Init mysqld
```
scripts/mysql_install_db
```

5 Run mysqld with the hot backup library (should exist in the lib directory)
```
LD_PRELOAD=PATH_TO_MYSQL_BASE_DIR/lib/libHotBackup.so mysqld_safe
```
NOTE: The preload is NOT necessary for MySQL and MariaDB builds from Tokutek since we link the hot backup library into mysqld already.

6 Install the backup plugin (should exist in MySQL's plugin directory)
```
install plugin tokudb_backup soname 'tokudb_backup.so';
````

# Run a backup

Backup to the '/tmp/backup1047' directory.  This blocks until the backup is complete.
```
set tokudb_backup_dir='/tmp/backup1047';
```
The ```set tokudb_backup_dir``` statement will succeed if the backup was taken.  Otherwise, the ```tokudb_backup_last_error``` variable is set.

```
select @@tokudb_backup_last_error, @@tokudb_backup_last_error_string;
```

# Exclude source files
Lets suppose that you want to exclude all 'lost+found' directories from the backup.  The ```tokudb_backup_exclude``` session variable contains a regular expression that all source file names are compared with.  If the source file name matches the exclude regular expression, then the source file is excluded from the backup.
```
set tokudb_backup_exclude='/lost\\+found($|/)';
```
```
set tokudb_backup_dir='/tmp/backup105';
```

# Monitor progress
The Tokutek hot backup updates the processlist state with progress information while it is running.

# Throttle the backup write rate
The ```tokudb_backup_throttle``` variable imposes an upper bound on the write rate of the TokuDB backup.  Units are bytes per second.  Default is no upper bound.

# Variables
## tokudb_backup_plugin_version
* name:tokudb_backup_plugin_version
* readonly:true
* scope:system
* type:str
* comment:version of the tokudb backup plugin

## tokudb_backup_version
* name:tokudb_backup_version
* readonly:true
* scope:system
* type:str
* comment:version of the tokutek backup library

## tokudb_backup_allowed_prefix
* name:tokudb_backup_allowed_prefix
* readonly:true
* scope:system
* type:str
* comment:allowed prefix of the destination directory

## tokudb_backup_throttle
* name:tokudb_backup_throttle
* readonly:false
* scope:session
* type:ulonglong
* def_val:18446744073709551615
* min_val:0
* max_val:18446744073709551615
* comment:backup throttle on write rate in bytes per second

## tokudb_backup_dir
* name:tokudb_backup_dir
* readonly:false
* scope:session
* type:str
* comment:name of the directory where the backup is stored

## tokudb_backup_last_error
* name:tokudb_backup_last_error
* readonly:false
* scope:session
* type:ulong
* def_val:0
* min_val:0
* max_val:18446744073709551615
* comment:error from the last backup. 0 is success

## tokudb_backup_last_error_string
* name:tokudb_backup_last_error_string
* readonly:false
* scope:session
* type:str
* comment:error string of the last backup

## tokudb_backup_exclude
* name:tokudb_backup_exclude
* readonly:false
* scope:session
* type:str
* comment:exclude source file regular expression

# Build the hot backup plugin from source
1 Checkout the Percona Server source with tag Percona-Server-5.6.23-72.1
```
git clone -b 5.6 git@github.com/percona/percona-server
git checkout Percona-Server-5.6.23-72.1
mkdir percona-server-build
```

2 Checkout the tokudb backup plugin with tag 'tokudb-backup-0.17'
```
cd percona-server/plugin
git clone git@github.com:Tokutek/tokudb-backup-plugin
pushd tokudb-backup-plugin
git checkout tokudb-backup-0.17
popd
```

3 Checkout the tokudb hot backup library with tag 'tokudb-backup-0.17'
```
cd percona-server/plugin/tokudb-backup-plugin
git clone git@github.com:Tokutek/backup-enterprise
ln -s backup-enterprise/backup backup
pushd backup-enterprise
git checkout tokudb-backup-0.17
popd
```

4 Build
```
cd percona-server-build
cmake -DBUILD_CONFIG=mysql_release -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=../tokudb-backup-0.17-percona-server-5.6.23 -DTOKUDB_BACKUP_PLUGIN_VERSION=tokudb-backup-0.17 ../percona-server
cd plugin/tokudb-backup-plugin
make -j8 install
```

5 Make tarball
```
tar czf tokudb-backup-0.17-percona-server-5.6.23.tar.gz tokudb-backup-0.17-percona-server-5.6.23
```
