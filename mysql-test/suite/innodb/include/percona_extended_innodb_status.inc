# starting a transaction in the "default" connection
connection default;
START TRANSACTION;
# execute a statement which locks a record
UPDATE t SET value = 1 WHERE id = 3;

# creating another connection
connect (con1,localhost,root,,);
--echo connection con1

# identify this connection ID
--let $blocked_connection_id = `SELECT CONNECTION_ID()`

# make sure the upcoming blocked update will not time out
SET innodb_lock_wait_timeout = 1073741824;

# starting a transaction in this connection
START TRANSACTION;
# execute a statement which tries to update the same record
# this statement will hang until the privious transaction finishes
--send UPDATE t SET value = 1 WHERE id = 3

connection default;
--echo connection default

# wait until the second transaction switches to "LOCK WAIT" state
let $wait_condition = SELECT trx_state = 'LOCK WAIT'
  FROM information_schema.innodb_trx
  WHERE trx_mysql_thread_id = $blocked_connection_id;
--source include/wait_condition.inc

# executing "SHOW ENGINE INNODB STATUS" in the "default" connection
# saving the output into "STATUS" environment variable
--let STATUS = query_get_value(SHOW ENGINE INNODB STATUS, Status, 1)

# The output should also include detailed lock info, i.e.
#
# Record lock, heap no 2 PHYSICAL RECORD: n_fields 4; compact format; info bits 0
#  0: len 4; hex 80000001; asc     ;;
#  1: len 6; hex 000000000d07; asc       ;;
#  2: len 7; hex 14000001bc0110; asc        ;;
#  3: len 4; hex 80000001; asc     ;;

# using perl script to analyze the ouput from the "STATUS" environment
# variable
perl;
  use strict;
  use warnings;

  my $expected_locks = $ENV{'expected_locks'};
  my $output = $ENV{'STATUS'};
  # Only consider the part after "TRANSACTIONS", or we could end up counting
  # "LAST FOREIGN KEY ERROR" record locks too
  my @output = split "\n", $output;
  my $index = 0;
  $index++ until @output[$index] eq 'TRANSACTIONS';
  splice @output, 0, $index;
  $output = join("\n", @output);
  if ($output =~ /^Record lock, heap no \d+ PHYSICAL RECORD: n_fields \d+; compact format; info bits \d+$/m)
  {
    print "\"Record lock\" found\n";
    my $records = () = $output =~ /^\s*\d+: len \d+; hex [[:xdigit:]]+; asc .*;;$/gm;
    print "n = $records\n";
    if ($expected_locks != $records)
    {
       print "Expected $expected_locks, found $records. Dumping status\n";
       print $output;
    }
  }
  else
  {
    print "\"Record lock\" not found\n";
    if ($expected_locks != 0)
    {
       print "Expected $expected_locks, found 0. Dumping status\n";
       print $output;
    }
  }
  if($output =~ /^TOO MANY LOCKS PRINTED FOR THIS TRX: SUPPRESSING FURTHER PRINTS$/m)
  {
    print "too many locks printed\n";
  }
EOF

# rolling back the first transaction
ROLLBACK;

# rolling back the second transaction
connection con1;
--echo connection con1
--reap
ROLLBACK;

# closing auxiliary connection
connection default;
--echo connection default
disconnect con1;
