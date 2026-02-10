# ==== NOTE ====
#
# This file is NOT allowed to be used in any test directly.
# It is intended to be used in teardown_component_customized.inc only.
# Other usage will confuse MTR and can make tests failure.
#
# ==== Purpose ====
#
# Restores 2 files from backup:
#  - global configuration file for given keyring component (located in plugin dir)
#  - global manifest (located in mysqld install dir)
# The backup of the file has a *.global_backup extension expected.
# If the files to be restored already exist, they are deleted beforehand,
# then the original content is restored from the backup.
# If the file with *.global_backup does not exist, the script fails.
# Thus the script must allways be used in pair with preceding backup_global_file_once.inc.
# Therefore if the test uses teardown_component_customized.inc,
# it must use a complementary preceding setup_component_customized.inc.
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

--echo # Restore global manifest and global configuration file from global_backup for: $component_name

# restore global config
--let $CONFIG_FILE_PATH = $COMPONENT_DIR/$COMPONENT_NAME.cnf
--let $BACKUP_CONFIG_FILE_PATH = $COMPONENT_DIR/$COMPONENT_NAME.cnf.$GLOBAL_BACKUP_SUFFIX
--move_file $BACKUP_CONFIG_FILE_PATH $CONFIG_FILE_PATH

# restore global manifest
--perl
  use strict;
  use File::Basename;
  my $manifest_file_ext = ".my";
  my $backup_suffix = $ENV{'GLOBAL_BACKUP_SUFFIX'} || "global_backup";
  my ($exename, $path, $suffix) = fileparse($ENV{'MYSQLD'}, qr/\.[^.]*/);
  my $manifest_file_path = $path.$exename.$manifest_file_ext;
  my $backup_manifest_file_path = $manifest_file_path.".".$backup_suffix;
  rename($backup_manifest_file_path, $manifest_file_path)
    or die "Could not move $backup_manifest_file_path to $manifest_file_path: $!";
EOF
