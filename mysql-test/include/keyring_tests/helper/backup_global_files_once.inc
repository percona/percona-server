# ==== NOTE ====
#
# This file is NOT allowed to be used in any test directly.
# It is intended to be used in setup_component_customized.inc only.
# Other usage will confuse MTR and can make tests failure.
#
# ==== Purpose ====
#
# Backup 2 files:
#  - global configuration file for given keyring component (located in plugin dir)
#  - global manifest (located in mysqld install dir)
# The backup of the file has a *.global_backup extension added.
# The original files are left intact, so they are still available for the remaining part of test.
# If the file with *.global_backup already exists, no new backup is taken.
# Thus the script must allways be used in pair with succeding restore_global_file_once.inc.
# Therefore if the test uses setup_component_customized.inc,
# it must use a complementary succeding teardown_component_customized.inc.
#
# ==== Usage ====
#
# --let COMPONENT_DIR= <keyring_component_directory>
# --let COMPONENT_NAME= <keyring_component_name_without_extension>
# --let GLOBAL_BACKUP_SUFFIX= <global_backup_suffix>
# --source include/keyring_tests/backup_global_files_once.inc
#   <do the test stuff here>
# --source include/keyring_tests/restore_global_files_once.inc
#
# ==== Parameters ====
#
# $COMPONENT_DIR
#    Directory where keyring component is located
#
# $COMPONENT_NAME
#    Name of the keyring component. Configuration file name
#    would be <$COMPONENT_NAME>.cnf .
#
# $GLOBAL_BACKUP_SUFFIX
#    the extension suffix added to backup file
#    optional parameter, defaults to "global_backup"

if (! $GLOBAL_BACKUP_SUFFIX) {
  let GLOBAL_BACKUP_SUFFIX=global_backup;
}

--echo # Do global_backup of global manifest and global configuration file for: $component_name

# backup global config
--let $CONFIG_FILE_PATH = $COMPONENT_DIR/$COMPONENT_NAME.cnf
--let $BACKUP_CONFIG_FILE_PATH = $COMPONENT_DIR/$COMPONENT_NAME.cnf.$GLOBAL_BACKUP_SUFFIX
--copy_file $CONFIG_FILE_PATH $BACKUP_CONFIG_FILE_PATH

# backup global manifest
--perl
  use strict;
  use File::Basename;
  use File::Copy;
  my $manifest_file_ext = ".my";
  my $backup_suffix = $ENV{'GLOBAL_BACKUP_SUFFIX'} || "global_backup";
  my ($exename, $path, $suffix) = fileparse($ENV{'MYSQLD'}, qr/\.[^.]*/);
  my $manifest_file_path = $path.$exename.$manifest_file_ext;
  my $backup_manifest_file_path = $manifest_file_path.".".$backup_suffix;
  copy($manifest_file_path, $backup_manifest_file_path)
    or die "Could not copy $manifest_file_path to $backup_manifest_file_path: $!";
EOF
