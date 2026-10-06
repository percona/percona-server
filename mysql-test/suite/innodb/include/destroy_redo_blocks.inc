# Overwrite a range of redo log blocks with 0xff bytes, so that their
# header numbers do not match their positions and the encryption bit in
# their data length fields is set.  The server must be stopped.
#
# Parameters:
#   MYSQLD_DATADIR : the data directory
#   FIRST_LSN      : the first block to destroy is the one holding this LSN
#   LAST_LSN       : the last block to destroy is the one holding the byte
#                    before this LSN (the current LSN is where the next byte
#                    of redo goes; the byte before it is the last one written)
#
# The blocks are located through the start LSN in the header of the redo
# log file that covers them, and every one of them is checked to hold the
# header number of its position before it is destroyed.

--source include/wait_until_disconnected.inc

perl;
use strict;
use warnings;
use Fcntl qw(SEEK_SET);
use IO::Handle;
my $block = 512;
my $last = $ENV{'LAST_LSN'} - 1;
my $first = $ENV{'FIRST_LSN'};
$first = $last if $first > $last;
my $first_block = int($first / $block) * $block;
my $last_block = int($last / $block) * $block;
my $dir = "$ENV{'MYSQLD_DATADIR'}/#innodb_redo";
opendir(my $dh, $dir) or die "cannot open $dir: $!";
my @files = grep { /^#ib_redo\d+$/ } readdir($dh);
closedir($dh);
my $done = 0;
foreach my $f (@files) {
  my $fname = "$dir/$f";
  open(my $fh, "+<", $fname) or die "cannot open $fname: $!";
  binmode $fh;
  my $buf;
  # LOG_HEADER_START_LSN, 8 bytes at offset 8 of the first header block.
  seek($fh, 8, SEEK_SET);
  read($fh, $buf, 8) == 8 or die "short read of the header of $fname";
  my ($hi, $lo) = unpack("NN", $buf);
  my $start_lsn = $hi * 4294967296 + $lo;
  # The file covers [start_lsn, start_lsn + data size); LOG_FILE_HDR_SIZE
  # is four blocks.
  my $end_lsn = $start_lsn + (-s $fname) - 4 * $block;
  for (my $lsn = $first_block; $lsn <= $last_block; $lsn += $block) {
    next unless $lsn >= $start_lsn && $lsn < $end_lsn;
    my $off = 4 * $block + ($lsn - $start_lsn);
    seek($fh, $off, SEEK_SET);
    read($fh, $buf, 4) == 4 or die "short read of block at $off in $fname";
    # LOG_BLOCK_HDR_NO without LOG_BLOCK_FLUSH_BIT_MASK.
    my $hdr_no = unpack("N", $buf) & 0x7fffffff;
    my $expected = 1 + (int($lsn / $block) % 0x40000000);
    die "block at $off of $fname has header number $hdr_no, expected $expected"
      unless $hdr_no == $expected;
    seek($fh, $off, SEEK_SET);
    print $fh chr(0xff) x $block;
    $done++;
  }
  close $fh;
}
my $wanted = ($last_block - $first_block) / $block + 1;
die "destroyed $done redo log blocks, wanted $wanted" unless $done == $wanted;
print "destroyed the redo log blocks\n";
EOF
