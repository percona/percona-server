# Teardown

--echo # ----------------------------------------------------------------------
--echo # Teardown
# Remove keyring kmip
--source include/keyring_tests/helper/local_keyring_kmip_remove.inc

# Drop global keyring config
--source include/keyring_tests/helper/global_keyring_remove_config.inc

# Remove manifest file for mysqld binary
--source include/keyring_tests/helper/binary_remove_manifest.inc

# Restore the standard binary manifest so subsequent tests can read their
# instance manifests (MySQL only reads instance manifests when the binary
# manifest says read_local_manifest: true).
--perl
  use strict;
  use File::Basename;
  my $content = '{ "read_local_manifest": true }';
  my $manifest_file_ext = ".my";
  my ($exename, $path, $suffix) = fileparse($ENV{'MYSQLD'}, qr/\.[^.]*/);
  my $manifest_file_path = $path.$exename.$manifest_file_ext;
  open(my $mh, "> $manifest_file_path") or die "Cannot write $manifest_file_path: $!";
  print $mh $content or die;
  close($mh) or die;
EOF

# Restart server without manifest file
--source include/keyring_tests/helper/cleanup_server_with_manifest.inc
--echo # ----------------------------------------------------------------------
