# ==== Purpose ====
#
# Try to create a filter with a single 'field' condition used as 'log'
# condition of an event subclass, show the UDF result and the number of
# rows stored under the filter name.
#
# ==== Usage ====
#
# --let $rx_name = filter name, must not exist yet
# --let $rx_field = field object JSON text, '~' stands for a backslash
# [--let $rx_class = event class, table_access by default]
# [--let $rx_event = event subclass, insert by default]
# --source filter_definition_regex_set.inc

--let $_rx_class = table_access
if ($rx_class != '') {
  --let $_rx_class = $rx_class
}
--let $_rx_event = insert
if ($rx_event != '') {
  --let $_rx_event = $rx_event
}

--echo # $rx_name: $rx_field
--disable_query_log
eval SET @rx_definition = REPLACE('{"filter":{"class":{"name":"$_rx_class","event":{"name":"$_rx_event","log":{"field":$rx_field}}}}}', '~', CHAR(92));
--enable_query_log
eval SELECT audit_log_filter_set_filter('$rx_name', @rx_definition) AS result;
eval SELECT COUNT(*) AS stored_rows FROM mysql.audit_log_filter WHERE name = '$rx_name';

--let $rx_class =
--let $rx_event =
