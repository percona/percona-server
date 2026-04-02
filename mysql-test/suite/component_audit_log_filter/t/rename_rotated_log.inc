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
  use File::Spec;

  my $log_path = $ENV{'log_path'};
  my $log_name = $ENV{'log_name'};
  my $base_log_name = $ENV{'log_name'} =~ s/\.log$//r;
  my $new_file_name = $ENV{'new_file_name'} or die "Missing new_file_name\n";
  my @candidates;

  opendir(my $dh, $log_path) or die "Can't opendir $log_path: $!";

  while ((my $filename = readdir($dh))) {
    if ($filename =~ /^\Q$base_log_name\E\.\d{8}T\d{6}(?:-\d+)?\./) {
      my $full_path = File::Spec->catfile($log_path, $filename);
      my @stat = stat($full_path);
      push @candidates, [$filename, $stat[9] // 0];
    }
  }

  closedir $dh;

  die "No rotated audit log found for $base_log_name in $log_path\n"
      if (!@candidates);

  @candidates = sort {
    $b->[1] <=> $a->[1] || $b->[0] cmp $a->[0]
  } @candidates;

  my $source = File::Spec->catfile($log_path, $candidates[0]->[0]);
  my $target = File::Spec->catfile($log_path, $new_file_name);
  move($source, $target) or
      die "Failed to rename $source to $target: $!\n";
EOF
