--source include/have_tokudb.inc

if (!$TOKUDB_BACKUP) {
  --skip TokuDB Backup requires the environment variable \$TOKUDB_BACKUP to be set (normally done by mtr)
}

#
# Check if --plugin-dir was setup for TokuDB Backup
#
if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) != '$TOKUDB_BACKUP_OPT/'`) {
  --skip TokuDB Backup requires that --plugin-dir is set to the TokuDB Backup plugin dir (either the .opt file does not contain \$TOKUDB_BACKUP_OPT or another plugin is in use)
}
