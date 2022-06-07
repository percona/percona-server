#
# Test SELECT INTO OUTFILE/DUMPFILE with fifo and unix socket as destination
#

perl;
my $fn = "$ENV{'MYSQL_TMP_DIR'}/regular";
unlink($fn);
open(FILE, ">$fn");
print(FILE "1\n");
close(FILE);
EOF

--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR
--error 1086
--eval SELECT 1,2,3 INTO $select_into '$MYSQL_TMP_DIR/regular'

perl;
use POSIX qw(mkfifo);
my $fn = "$ENV{'MYSQL_TMP_DIR'}/fifo";
unlink($fn);
mkfifo($fn, 0666) or die("mkfifo: $!");
EOF

--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR
--send_eval SELECT 1,2,3 INTO $select_into '$MYSQL_TMP_DIR/fifo'

--cat_file $MYSQL_TMP_DIR/fifo

--reap

DELIMITER //;
# procedure spin waiting when file 'trigger' appears
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR
--eval CREATE PROCEDURE p1() BEGIN WHILE ISNULL(LOAD_FILE('$MYSQL_TMP_DIR/trigger')) DO SELECT SLEEP(1); END WHILE; SELECT 1,2,3 INTO $select_into '$MYSQL_TMP_DIR/socket'; END
DELIMITER ;//

--send CALL p1()

perl;
use Socket;
use IO::Handle;

my $fn = "$ENV{'MYSQL_TMP_DIR'}/socket";
my $trigger = "$ENV{'MYSQL_TMP_DIR'}/trigger";

unlink($fn);
socket(SERV, PF_UNIX, SOCK_STREAM, 0) or die("socket: $!");
bind(SERV, sockaddr_un($fn)) or die("bind $fn: $!");

# tell the p1 that we created the socket
open(FILE, ">$trigger");
close(FILE);

listen(SERV, 1) or die("listen: $!");

accept(CLIENT, SERV);
$content = <CLIENT>;
close(CLIENT);
close(SERV);
unlink($fn);
unlink($trigger);

EOF

--disable_result_log
--reap

DROP PROCEDURE p1;
