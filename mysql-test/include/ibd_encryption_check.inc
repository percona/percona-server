perl;
use strict;
use warnings;
use Fcntl qw(SEEK_SET);

my $ibd_file = $ENV{'IBD_FILE'};

if (!$ibd_file) {
  print "ENV variable IBD_FILE is not set. Use --let IBD_FILE=some_filename_here in test\n";
  exit;
}

my $file_offset = 10390;

open(FILE, "<", $ibd_file) or die "could not open $ibd_file: $!";
binmode FILE;

seek(FILE, $file_offset, SEEK_SET) or die "could not seek: $!";
my $data;
read FILE, $data, 3;
my $error=0;
if ($data ne "lCC") {
   print "File:", $ibd_file, " Unexpected encryption magic: ", $data, " should be lCC\n";
   $error=1;
}


$file_offset = 10390 + 3;
seek(FILE, $file_offset, SEEK_SET) or die "could not seek: $!";
read FILE, $data, 4;
my $new_data = unpack "N", $data;
if ($new_data eq "0") {
   print "File:", $ibd_file, " Unexpected master key id: ", $new_data, " should be non-zero\n";
   $error=1;
}

$file_offset = 10390 + 3 + 4;
seek(FILE, $file_offset, SEEK_SET) or die "could not seek: $!";
read FILE, $data, 36;
$new_data = unpack("a*", $data);
#print "UUID read: ", $new_data;

if ($new_data !~ m/[a-zA-Z0-9]/ ) {
   print "File:", $ibd_file, " Empty UUID: ", $data, " should be non-empty\n";
   $error=1;
}

close FILE;
if ($error == 1) {
  die "File:", $ibd_file, " Encryption header is not proper\n";
}
EOF
