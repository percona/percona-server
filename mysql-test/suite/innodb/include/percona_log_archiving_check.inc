# Check that there are no gaps in archived files

--source percona_log_archiving_stat.inc

let $lsn = `SELECT (($archived_logs_count - 1) * $LOG_FILE_LSN_STEP + $first_log_file_lsn)`;

if ($last_log_file_lsn != $lsn)
{
  echo Expected last file lsn is $lsn but got $last_log_file_lsn;
  die Test failed;
}

echo Check OK;

let $LAST_ARCHIVED_LSN = $last_log_file_lsn;
