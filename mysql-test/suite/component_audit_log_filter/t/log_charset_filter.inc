# Raw string matching, raw byte lengths, and conditional digest replacement.
--let charset_query = SELECT 'café' AS c
if ($charset_class == table_access) {
  --let charset_query = SELECT 'café' AS c FROM charset_filter_data
}
perl;
  use strict;
  use warnings;
  use Encode qw(decode encode);
  my $q = decode('UTF-8', $ENV{charset_query});
  for my $cs ('latin1', 'utf8mb4') {
    open my $f, '>:raw', "$ENV{charset_sql}.$cs" or die $!;
    print $f encode($cs eq 'latin1' ? 'iso-8859-1' : 'UTF-8', "$q;\n");
    close $f;
  }
EOF
# Use JSON_QUOTE so the filter contains literal SQL quotes and UTF-8 text.
--eval SET @query = "$charset_query"
--eval SET @class = '$charset_class', @event = '$charset_event', @field = '$charset_field'
SET @condition = JSON_OBJECT('field', JSON_OBJECT('name', CONCAT(@field,'.str'), 'value', @query));
SET @definition = JSON_OBJECT('filter', JSON_OBJECT('class', JSON_OBJECT('name', @class, 'event', JSON_OBJECT('name', @event, 'log', CAST(@condition AS JSON)))));
SELECT audit_log_filter_set_filter('charset_filter', @definition);
SELECT audit_log_filter_set_user('charset_filter@localhost', 'charset_filter');
SELECT audit_log_rotate();
--exec $MYSQL --default-character-set=latin1 -u charset_filter test < $charset_sql.latin1 > $MYSQLTEST_VARDIR/tmp/charset_filter.out
--exec $MYSQL --default-character-set=utf8mb4 -u charset_filter test < $charset_sql.utf8mb4 > $MYSQLTEST_VARDIR/tmp/charset_filter.out
--let charset_closed = `SELECT audit_log_rotate()`
--let charset_expected_count = 1
--let charset_expect_digest = 0
--source log_charset_filter_check.inc
SELECT audit_log_filter_remove_filter('charset_filter');

# Only the latin1 byte length matches; the UTF-8 statement is one byte longer.
SET @condition = JSON_OBJECT('field', JSON_OBJECT('name', CONCAT(@field,'.length'), 'value', CAST(LENGTH(CONVERT(@query USING latin1)) AS CHAR)));
SET @definition = JSON_OBJECT('filter', JSON_OBJECT('class', JSON_OBJECT('name', @class, 'event', JSON_OBJECT('name', @event, 'log', CAST(@condition AS JSON)))));
SELECT audit_log_filter_set_filter('charset_filter', @definition);
SELECT audit_log_filter_set_user('charset_filter@localhost', 'charset_filter');
SELECT audit_log_rotate();
--exec $MYSQL --default-character-set=latin1 -u charset_filter test < $charset_sql.latin1 > $MYSQLTEST_VARDIR/tmp/charset_filter.out
--exec $MYSQL --default-character-set=utf8mb4 -u charset_filter test < $charset_sql.utf8mb4 > $MYSQLTEST_VARDIR/tmp/charset_filter.out
--let charset_closed = `SELECT audit_log_rotate()`
--source log_charset_filter_check.inc
SELECT audit_log_filter_remove_filter('charset_filter');

# Both events are logged. Only UTF-8 matches the raw print condition; latin1
# gets the digest. A non-ASCII identifier in the digest must stay UTF-8.
--let charset_query = SELECT 'café' AS café FROM charset_filter_data
perl;
  use Encode qw(decode encode);
  my $q = decode('UTF-8', $ENV{charset_query});
  for my $cs ('latin1', 'utf8mb4') {
    open my $f, '>:raw', "$ENV{charset_sql}.$cs" or die $!;
    print $f encode($cs eq 'latin1' ? 'iso-8859-1' : 'UTF-8', "$q;\n");
    close $f;
  }
EOF
--eval SET @query = "$charset_query"
SET @print = JSON_OBJECT('field', JSON_OBJECT('name', CONCAT(@field,'.str'), 'value', @query));
SET @action = JSON_OBJECT('field', JSON_OBJECT('name', CONCAT(@field,'.str'), 'print', CAST(@print AS JSON), 'replace', JSON_OBJECT('function', JSON_OBJECT('name', 'query_digest'))));
SET @definition = JSON_OBJECT('filter', JSON_OBJECT('class', JSON_OBJECT('name', @class, 'event', JSON_OBJECT('name', @event, 'log', CAST('true' AS JSON), 'print', CAST(@action AS JSON)))));
SELECT audit_log_filter_set_filter('charset_filter', @definition);
SELECT audit_log_filter_set_user('charset_filter@localhost', 'charset_filter');
SELECT audit_log_rotate();
--exec $MYSQL --default-character-set=latin1 -u charset_filter test < $charset_sql.latin1 > $MYSQLTEST_VARDIR/tmp/charset_filter.out
--exec $MYSQL --default-character-set=utf8mb4 -u charset_filter test < $charset_sql.utf8mb4 > $MYSQLTEST_VARDIR/tmp/charset_filter.out
--let charset_closed = `SELECT audit_log_rotate()`
--let charset_expected_count = 2
--let charset_expect_digest = 1
--source log_charset_filter_check.inc
SELECT audit_log_filter_remove_filter('charset_filter');
