#
# Cleanup after log archiving testing
#

DROP TABLE t;

--source include/restart_mysqld.inc

--remove_files_wildcard $MYSQLD_ARCHIVEDIR ib_log_archive_*
--rmdir $MYSQLD_ARCHIVEDIR
