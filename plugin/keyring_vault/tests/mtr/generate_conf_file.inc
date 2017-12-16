let KEYRING_CONF_FILE_TO_GENERATE=$KEYRING_CONF_FILE_TO_GENERATE;
let KEYRING_CONF_TEMPLATE_FILE=$KEYRING_CONF_TEMPLATE_FILE;
let MYSQL_TEST_DIR=$MYSQL_TEST_DIR;

--perl
  use strict;
  my $mysql_test_dir= $ENV{MYSQL_TEST_DIR} or die "Need MYSQL_TEST_DIR";
  my $keyring_conf_template_file= $ENV{KEYRING_CONF_TEMPLATE_FILE} or die "Need KEYRING_CONF_TEMPLATE_FILE";
  my $keyring_conf_file_to_generate= $ENV{'KEYRING_CONF_FILE_TO_GENERATE'} or die("KEYRING_CONF_FILE_TO_GENERATE not set\n");
  open CONF_FILE, ">", "$keyring_conf_file_to_generate" or die "Could not open configuration file: ${keyring_conf_file_to_generate}.\n";
  open CONF_TEMPLATE_FILE, "<", "$keyring_conf_template_file" or die "Could not open configuration template file: ${keyring_conf_template_file}.\n";
  while (my $row = <CONF_TEMPLATE_FILE>)
  {
    $row =~ s/MYSQL_TEST_DIR/$mysql_test_dir/g;
    print CONF_FILE $row;
  }
  close(CONF_FILE);
  close(CONF_TEMPLATE_FILE);
EOF
