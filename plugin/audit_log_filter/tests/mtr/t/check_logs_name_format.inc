# ==== Purpose ====
#
# Check currently active and rotated logs have correct name format.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = path to dir containing audit filter log
# --let $audit_filter_log_name = audit filter log file name
# --let $with_compression = [0|1] indicates if log compression is enabled
# --let $with_encryption = [0|1] indicates if log encryption is enabled
# --source check_logs_name_format.inc
#
# Parameters:
#
# $audit_filter_log_path
#   Path to dir containing audit filter log.
# $audit_filter_log_name
#   Audit filter log file name.
# $with_compression
#   Indicates if audit log compression is enabled. Defaults to 0.
# $with_encryption
#   Indicates if audit log encryption is enabled. Defaults to 0.

--let audit_filter_log_path = $audit_filter_log_path
--let audit_filter_log_name = $audit_filter_log_name
--let with_compression = $with_compression
--let with_encryption = $with_encryption

perl;
  use strict;
  use warnings;

  my $audit_filter_log_path = $ENV{'audit_filter_log_path'} or die "audit_filter_log_path not set";
  my $audit_filter_log_name = $ENV{'audit_filter_log_name'} or die "audit_filter_log_name not set";
  my $with_compression = $ENV{'with_compression'} || 0;
  my $with_encryption = $ENV{'with_encryption'} || 0;

  my $log_base_name = "";
  my $log_extensions = "";
  my $logs_count = 0;

  if ($ENV{'audit_filter_log_name'} =~ m/(.*?)\.(.*)/) {
    $log_base_name = $1;
    $log_extensions = "\.$2";
  }
  else {
    $log_base_name = $ENV{'audit_filter_log_name'};
  }

  opendir(my $dh, $audit_filter_log_path) or die "Can't opendir $audit_filter_log_path: $!";

  while ((my $filename = readdir($dh))) {
    if ($filename =~ /^$log_base_name/) {
      $logs_count++;

      if ($with_compression && $with_encryption) {
        die "Bad log name format: $filename"
            if ($filename !~ /^$audit_filter_log_name\.gz\.(\d{8}T\d{6})-\d\.enc$/ &&
                $filename !~ /^$log_base_name\.(\d{8}T\d{6})$log_extensions\.gz\.(\d{8}T\d{6})-\d\.enc/);
      }
      elsif ($with_compression) {
        die "Bad log name format: $filename"
            if ($filename !~ /^$audit_filter_log_name\.gz$/ &&
                $filename !~ /^$log_base_name\.(\d{8}T\d{6})$log_extensions\.gz/);
      }
      elsif ($with_encryption) {
        die "Bad log name format: $filename"
            if ($filename !~ /^$audit_filter_log_name\.(\d{8}T\d{6})-\d\.enc$/ &&
                $filename !~ /^$log_base_name\.(\d{8}T\d{6})$log_extensions\.(\d{8}T\d{6})-\d\.enc/);
      }
      else {
        die "Bad log name format: $filename"
            if ($filename !~ /^$audit_filter_log_name$/ &&
                $filename !~ /^$log_base_name\.(\d{8}T\d{6})$log_extensions/);
      }
    }
  }

  closedir $dh;

  die "No log files found" if (!$logs_count);
EOF