# ==== Purpose ====
#
# Check if the openssl binary is of a particular version
#
# The condition has the same form as expressions evaluated by include/eval.inc
# ==== Usage ====
#
# --let $openssl_binary_version = 1\\.1\\..*
# --let $openssl_binary_version_do_not_skip = 1 (optional)
# --source include/have_openssl_binary_version.inc
#
# result (if $openssl_binary_version_do_not_skip was set to 1):
# $openssl_binary_version_match = 0 | 1

--disable_query_log
--disable_result_log

let OPENSSL_VERSION_INFO= $MYSQLTEST_VARDIR/log/openssl_binary_version_check.txt;
let OPENSSL_CONFIG_INC= $MYSQLTEST_VARDIR/log/openssl_binary_config_check.inc;
let OPENSSL_SEARCH_PATTERN=$openssl_binary_version;

--error 0,1
--remove_file $OPENSSL_VERSION_INFO
--error 0,1
--remove_file $OPENSSL_CONFIG_INC

--error 0,1, 127
--exec $OPENSSL_EXECUTABLE version > $OPENSSL_VERSION_INFO

perl;
 use strict;
 my $search_file= $ENV{'OPENSSL_VERSION_INFO'};
 my $search_pattern_1= $ENV{'OPENSSL_SEARCH_PATTERN'};
 my $content= "";
 my $dir= $ENV{'MYSQLTEST_VARDIR'};
 open(CONFIG_INC, ">$dir/log/openssl_binary_config_check.inc");
 open(FILE, "$search_file") or die("Unable to open '$search_file' : $!\n");
 read(FILE, $content, 100, 0);
 close(FILE);

 if ( $content =~ m{$search_pattern_1} ) {
    print CONFIG_INC "let \$STATUS_VAR = 0;\n";
 }
 else {
    print CONFIG_INC "let \$STATUS_VAR = 1;\n";
 }
 close(CONFIG_INC);
EOF

--source $OPENSSL_CONFIG_INC

--error 0,1
--remove_file $OPENSSL_VERSION_INFO
--error 0,1
--remove_file $OPENSSL_CONFIG_INC

if ($STATUS_VAR)
{
  if (!$openssl_binary_version_do_not_skip)
  {
    --skip Test requires openssl version to be $openssl_binary_version
  }
  --let $openssl_binary_version_match = 0
}

if (!$STATUS_VAR)
{
  --let $openssl_binary_version_match = 1
}

--enable_query_log
--enable_result_log

