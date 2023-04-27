# ==== Purpose ====
#
# Find and rename rotated log file. Only one rotated log file is expected.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = path to dir containing audit filter log
# --let $audit_filter_log_name = audit filter log file name
# --let $new_file_name = new file name
# --source rename_rotated_log.inc
#
# Parameters:
#
# $audit_filter_log_path
#   Path to dir containing audit filter log.
# $audit_filter_log_name
#   Audit filter log file name.
# $new_file_name
#   New file name.

--let log_path = $audit_filter_log_path
--let log_name = $audit_filter_log_name
--let new_file_name = $new_file_name

perl;
  use strict;
  use warnings;
  use File::Copy qw(move);

  my $log_path = $ENV{'log_path'};
  my $log_name = $ENV{'log_name'};
  my $base_log_name = $ENV{'log_name'} =~ s/\.log$//r;
  my $new_file_name = $ENV{'new_file_name'} or die "Missing new_file_name\n";

  opendir(my $dh, $log_path) or die "Can't opendir $log_path: $!";

  while ((my $filename = readdir($dh))) {
    if ($filename =~ /^$base_log_name\.\d{8}T\d{6}\./) {
      move $log_path.$filename, $log_path.$new_file_name;
      last;
    }
  }

  closedir $dh;
EOF
