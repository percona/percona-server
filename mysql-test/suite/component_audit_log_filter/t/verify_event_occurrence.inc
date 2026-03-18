#
# Verify that the audit_log_filter component can observe audit event
# classes and subclasses by logging all events in one pass and then
# printing one pretty sample record per observed event type.
#
# The expected set of event types depends on the
# audit_log_filter.event_mode server variable:
#
#   FULL mode:
#     audit: startup, shutdown
#     general: log, error, result, status
#     connection: connect, disconnect, change_user, pre_authenticate
#     table_access: read, insert, update, delete
#     global_variable: get, set
#     command: start, end
#     query: start, nested_start, status_end, nested_status_end
#     stored_program: execute
#     authentication: flush, authid_create (currently missing),
#                     credential_change, authid_rename, authid_drop
#     message: internal, user
#     parse: preparse, postparse
#
#   REDUCED mode:
#     audit: startup, shutdown
#     general: status
#     connection: connect, disconnect, change_user
#     table_access: read, insert, update, delete
#     message: internal, user
#
--source include/have_component_audit_log.inc
--source ../inc/audit_tables_init.inc
--source ../inc/audit_comp_install.inc
--let $audit_filter_log_path = `SELECT @@global.datadir`
--let $audit_filter_log_name = `SELECT @@global.audit_log_filter.file`
--let $audit_event_mode = `SELECT @@global.audit_log_filter.event_mode`
--let audit_filter_log_path = $audit_filter_log_path
--let audit_filter_log_name = $audit_filter_log_name
--let audit_event_mode = $audit_event_mode

--disable_query_log
--disable_result_log

SET @saved_default_password_lifetime := @@global.default_password_lifetime;
SET GLOBAL default_password_lifetime = 100;

CREATE USER audit_user@localhost IDENTIFIED BY 'pass';
GRANT ALL PRIVILEGES ON *.* TO audit_user@localhost WITH GRANT OPTION;

INSTALL COMPONENT "file://component_test_audit_api_message";

--source suite/query_rewrite_plugins/include/install_ddl_rewriter.inc

connect (setup_conn,localhost,audit_user,pass,test);
connection setup_conn;

CREATE TABLE t_events (id INT PRIMARY KEY, val VARCHAR(20));
INSERT INTO t_events VALUES (1, 'one'), (2, 'two');

DELIMITER //;
CREATE PROCEDURE p_nested()
BEGIN
  DECLARE x INT DEFAULT 0;
  SET x = 1;
  SELECT x;
END//
DELIMITER ;//

CREATE TABLE trg_src (id INT);
CREATE TABLE trg_log (id INT, msg VARCHAR(50));

DELIMITER //;
CREATE TRIGGER trg_after_insert AFTER INSERT ON trg_src
FOR EACH ROW
BEGIN
  INSERT INTO trg_log VALUES (NEW.id, 'inserted');
END//
DELIMITER ;//

disconnect setup_conn;
--enable_query_log
--enable_result_log
connection default;

SELECT audit_log_filter_set_filter('log_all', '{"filter":{"log": true}}');
if ($audit_event_mode == 'FULL')
{
SELECT audit_log_filter_set_filter('log_pre_auth',
  '{"filter":{"class":{"name":"connection","event":{"name":"pre_authenticate","log":true}}}}');
SELECT audit_log_filter_set_user('%', 'log_pre_auth');
}
if ($audit_event_mode == 'REDUCED')
{
SELECT audit_log_filter_set_user('%', 'log_all');
}
SELECT audit_log_filter_set_user('audit_user@localhost', 'log_all');

--disable_query_log
--disable_result_log

connect (user_conn,localhost,audit_user,pass,test);

connection user_conn;
SELECT 1;
--error ER_NO_SUCH_TABLE
SELECT * FROM missing_table_for_audit;
SELECT * FROM t_events WHERE id = 1;
INSERT INTO t_events VALUES (3, 'three');
UPDATE t_events SET val = 'THREE' WHERE id = 3;
DELETE FROM t_events WHERE id = 3;
CALL p_nested();
INSERT INTO trg_src VALUES (1);
SELECT test_audit_api_message_internal();
SELECT test_audit_api_message_user();
# Keep an interesting preparse sample with rewritten_query populated.
# Keep an interesting postparse sample with the prepared-statement flag set.
CREATE TABLE t_parse_rewrite (id INT) ENGINE = InnoDB, ENCRYPTION = 'N';
DROP TABLE t_parse_rewrite;
PREPARE stmt_parse_ddl
  FROM 'CREATE TABLE t_parse_ps (id INT) ENGINE = InnoDB, ENCRYPTION = ''N''';
EXECUTE stmt_parse_ddl;
DEALLOCATE PREPARE stmt_parse_ddl;
DROP TABLE t_parse_ps;
SELECT @@global.default_password_lifetime;
SET GLOBAL default_password_lifetime = 101;
CREATE USER auth_evt_user@localhost IDENTIFIED BY 'pass';
ALTER USER auth_evt_user@localhost IDENTIFIED BY 'pass2';
RENAME USER auth_evt_user@localhost TO auth_evt_user2@localhost;
DROP USER auth_evt_user2@localhost;
FLUSH PRIVILEGES;

connection default;
--source include/count_sessions.inc
connect (change_conn,localhost,audit_user,pass,test);
connection change_conn;
--change_user audit_user,pass
connection default;
disconnect change_conn;
--source include/wait_until_count_sessions.inc

SELECT audit_log_filter_flush();

connection default;
SET GLOBAL default_password_lifetime = @saved_default_password_lifetime;
UNINSTALL PLUGIN ddl_rewriter;
UNINSTALL COMPONENT "file://component_test_audit_api_message";
disconnect user_conn;
DROP TRIGGER trg_after_insert;
DROP PROCEDURE p_nested;
DROP TABLE trg_log;
DROP TABLE trg_src;
DROP TABLE t_events;
DROP USER audit_user@localhost;
--source ../inc/audit_comp_uninstall.inc
--source ../inc/audit_tables_cleanup.inc

--enable_result_log
--enable_query_log

perl;
  use strict;
  use warnings;

  use B ();
  use JSON::PP ();

  die "Log path is not set\n" if (!$ENV{audit_filter_log_path});
  die "Log name is not set\n" if (!$ENV{audit_filter_log_name});

  my $log_dir = $ENV{audit_filter_log_path};
  my $log_name = $ENV{audit_filter_log_name};
  my $full_log_name = "$log_dir/$log_name";

  if (!-e $full_log_name) {
    my ($log_prefix, $log_suffix) = split(/\./, $log_name, 2);
    $log_suffix = ".$log_suffix";

    opendir(my $dh, $log_dir) or die "Can't opendir '$log_dir': $!";
    my @log_files = sort grep {
      $_ eq $log_name ||
      /^\Q$log_prefix\E\.\d{8}T\d{6}\Q$log_suffix\E$/
    } readdir($dh);
    closedir($dh);

    die "No audit log files found\n" if !@log_files;
    $full_log_name = "$log_dir/$log_files[-1]";
  }

  open(my $fh, '<:encoding(UTF-8)', $full_log_name)
    or die "Could not open '$full_log_name': $!";

  local $/;
  my $json_text = <$fh>;
  close($fh);

  $json_text =~ s/\s+$//;
  unless ($json_text =~ /\]\s*$/) {
    $json_text =~ s/,\s*$//;
    $json_text .= "\n]\n";
  }

  my $events = JSON::PP::decode_json($json_text);

  sub is_number {
    my ($val) = @_;
    return 0 unless defined $val;
    return 0 if ref $val;
    my $flags = B::svref_2object(\$val)->FLAGS;
    return ($flags & (B::SVf_IOK | B::SVf_NOK)) ? 1 : 0;
  }

  sub hidden_raw { return bless {}, 'MtrHiddenRaw'; }
  sub hidden_str { return bless {}, 'MtrHiddenStr'; }
  sub is_hidden_raw { return ref($_[0]) eq 'MtrHiddenRaw'; }
  sub is_hidden_str { return ref($_[0]) eq 'MtrHiddenStr'; }

  sub clone_and_mask {
    my ($value, $path) = @_;

    if (!ref $value) {
      return hidden_str() if $path eq 'timestamp';
      return hidden_raw() if $path eq 'time' ||
                              $path eq 'id' ||
                              $path eq 'connection_id' ||
                              $path eq 'startup_data.server_id' ||
                              $path eq 'shutdown_data.server_id';
      return hidden_str() if $path eq 'startup_data.os_version' ||
                              $path eq 'startup_data.mysql_version';
      return $value;
    }

    if (ref $value eq 'HASH') {
      if ($path eq 'connection_attributes') {
        my %hidden = map { $_ => hidden_str() } keys %{$value};
        return \%hidden;
      }

      my %copy;
      for my $key (keys %{$value}) {
        my $child_path = $path eq '' ? $key : "$path.$key";

        if ($child_path eq 'startup_data.args' &&
            ref $value->{$key} eq 'ARRAY' &&
            @{$value->{$key}} > 0) {
          $copy{$key} = ['<hidden>'];
          next;
        }

        $copy{$key} = clone_and_mask($value->{$key}, $child_path);
      }

      return \%copy;
    }

    if (ref $value eq 'ARRAY') {
      return [map { clone_and_mask($_, $path) } @{$value}];
    }

    return $value;
  }

  my @key_priority = qw(
    timestamp time id class event connection_id
    account login
    user host os ip proxy
    general_data connection_data table_access_data global_variable_data
    command_data query_data stored_program_data authentication_data
    message_data parse_data startup_data shutdown_data
    connection_attributes
    command sql_command query status
    connection_type db table name value rewritten_query flags
    component producer message message_attributes
    args
    _pid _platform _os _client_name _client_version program_name
  );

  sub ordered_keys {
    my ($hashref) = @_;
    my %seen;
    my @keys = grep { exists $hashref->{$_} && !$seen{$_}++ } @key_priority;
    push @keys, grep { !$seen{$_}++ } sort keys %{$hashref};
    return @keys;
  }

  sub fmt {
    my ($val, $depth) = @_;
    $depth //= 0;

    return '<hidden>' if is_hidden_raw($val);
    return '"<hidden>"' if is_hidden_str($val);
    return 'null' if !defined $val;

    if (!ref $val) {
      return "$val" if is_number($val);

      my $str = $val;
      $str =~ s/\\/\\\\/g;
      $str =~ s/"/\\"/g;
      $str =~ s/\n/\\n/g;
      return qq("$str");
    }

    my $indent = "  " x $depth;
    my $next_indent = "  " x ($depth + 1);

    if (ref $val eq 'ARRAY') {
      return "[]" if !@{$val};

      return "[\n" .
             join(",\n", map { $next_indent . fmt($_, $depth + 1) } @{$val}) .
             "\n$indent]";
    }

    if (ref $val eq 'HASH') {
      my @keys = ordered_keys($val);
      return "{}" if !@keys;

      return "{\n" .
             join(",\n",
                  map { $next_indent . qq("$_": ) . fmt($val->{$_}, $depth + 1) }
                  @keys) .
             "\n$indent}";
    }

    return 'null';
  }

  my %samples;

  sub sample_score {
    my ($key, $event) = @_;
    return 0 if !defined $event;
    return 0 if $key ne 'parse/preparse' &&
                $key ne 'parse/postparse';

    my $parse_data = $event->{parse_data};
    return 0 if ref $parse_data ne 'HASH';

    my $score = 0;
    my $rewritten_query = $parse_data->{rewritten_query} // '';
    my $query = $parse_data->{query} // '';
    my $flags = $parse_data->{flags};

    $score += 100 if $rewritten_query ne '';
    $score += 10 if $query =~ /^CREATE TABLE /i;
    $score += 5 if $query !~ /^PREPARE /i;
    $score += $flags if defined $flags && !ref $flags;

    return $score;
  }

  for my $ev (@{$events}) {
    next if !exists $ev->{class} || !exists $ev->{event};

    my $key = "$ev->{class}/$ev->{event}";
    next if exists $samples{$key} &&
            sample_score($key, $ev) <= sample_score($key, $samples{$key});

    $samples{$key} = clone_and_mask($ev, '');
  }

  my $mode = $ENV{audit_event_mode} // 'FULL';

  my @expected = (
    'audit/startup',
    'general/log',
    'general/error',
    'general/result',
    'general/status',
    'connection/connect',
    'connection/disconnect',
    'connection/change_user',
    'connection/pre_authenticate',
    'table_access/read',
    'table_access/insert',
    'table_access/update',
    'table_access/delete',
    'global_variable/variable_get',
    'global_variable/variable_set',
    'command/command_start',
    'command/command_end',
    'query/query_start',
    'query/query_nested_start',
    'query/query_status_end',
    'query/query_nested_status_end',
    'stored_program/execute',
    'authentication/auth_flush',
    'authentication/auth_authid_create',
    'authentication/auth_credential_change',
    'authentication/auth_authid_rename',
    'authentication/auth_authid_drop',
    'message/internal',
    'message/user',
    'parse/preparse',
    'parse/postparse',
    'audit/shutdown',
  );

  # auth_authid_create is expected but the current tree does not emit it.
  my %known_missing = ('authentication/auth_authid_create' => 1);

  if ($mode eq 'REDUCED') {
    my @disabled = qw(
      general/log
      general/error
      general/result
      connection/pre_authenticate
      global_variable/variable_get
      global_variable/variable_set
      command/command_start
      command/command_end
      query/query_start
      query/query_nested_start
      query/query_status_end
      query/query_nested_status_end
      stored_program/execute
      authentication/auth_flush
      authentication/auth_authid_create
      authentication/auth_credential_change
      authentication/auth_authid_rename
      authentication/auth_authid_drop
      parse/preparse
      parse/postparse
    );
    $known_missing{$_} = 1 for @disabled;
  }

  my $all_ok = 1;

  for my $key (@expected) {
    print "--- $key ---\n";
    if (exists $samples{$key}) {
      print fmt($samples{$key}), "\n";
    } else {
      print "MISSING: $key\n";
      $all_ok = 0 if !$known_missing{$key};
    }
    print "\n";
  }

  die "Some expected event classes/subclasses were not observed\n"
    if !$all_ok;
EOF
