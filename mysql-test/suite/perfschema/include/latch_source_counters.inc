# Percona: show the events_waits_summary_by_source counters of
# SHOW ENGINE PERFORMANCE_SCHEMA STATUS.
# Usage: --source ../include/latch_source_counters.inc

--disable_query_log
let $latch_source_row = 1;
let $latch_source_name = query_get_value(SHOW ENGINE PERFORMANCE_SCHEMA STATUS, Name, $latch_source_row);
while ($latch_source_name != 'No such row')
{
  if ($latch_source_name == 'events_waits_summary_by_source.lost')
  {
    let $latch_source_lost = query_get_value(SHOW ENGINE PERFORMANCE_SCHEMA STATUS, Status, $latch_source_row);
  }
  if ($latch_source_name == 'events_waits_summary_by_source.holds_lost')
  {
    let $latch_source_holds_lost = query_get_value(SHOW ENGINE PERFORMANCE_SCHEMA STATUS, Status, $latch_source_row);
  }
  inc $latch_source_row;
  let $latch_source_name = query_get_value(SHOW ENGINE PERFORMANCE_SCHEMA STATUS, Name, $latch_source_row);
}
--enable_query_log
