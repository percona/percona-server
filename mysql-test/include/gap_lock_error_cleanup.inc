set global gap_lock_raise_error= off;

drop table gap1, gap2, gap3, gap5, gap6;
if (!$gap_lock_error_skip_partitioning) {
  drop table gap4;
}
