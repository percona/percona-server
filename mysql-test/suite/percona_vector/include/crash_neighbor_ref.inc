# Writer A runs $a_sql, writer B runs $b_sql, then the server is killed.
#
#   $a_park = 1  A stops at vec_node_row_written: its node row is written
#                but not committed, and its node is still LINKING.
#   $a_park = 0  A's statement finishes inside an open transaction: its
#                node rows are committed and its node is COMPLETE, but the
#                user transaction never commits.
#   $a_begin = 1 with $a_park = 1: A's statement runs inside BEGIN.

--connect (writer_a,localhost,root,,test)
if ($a_park) {
  if ($a_begin) {
    BEGIN;
  }
  SET DEBUG_SYNC = 'vec_node_row_written SIGNAL parked WAIT_FOR go';
  --send_eval $a_sql
  --connection default
  SET DEBUG_SYNC = 'now WAIT_FOR parked';
}
if (!$a_park) {
  BEGIN;
  --eval $a_sql
  --connection default
}

--eval $b_sql

--source include/kill_and_restart_mysqld.inc
--disconnect writer_a
