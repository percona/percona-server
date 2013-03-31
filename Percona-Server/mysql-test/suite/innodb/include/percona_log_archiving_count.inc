
--source percona_log_archiving_stat.inc

let $lsn = $n_files * $LOG_FILE_LSN_STEP;

--echo $archived_logs_count

let $LAST_ARCHIVED_LSN = $last_log_file_lsn;

