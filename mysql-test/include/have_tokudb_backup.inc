--source include/have_tokudb.inc

if (!$TOKUDB_BACKUP) {
  --skip TokuDB Backup requires the environment variable \$TOKUDB_BACKUP to be set (normally done by mtr)
}
