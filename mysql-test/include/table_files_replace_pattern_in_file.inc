--perl

my $filename = $ENV{'TABLE_FILES_REPLACE_FILE'};
my $data = read_file($filename);

$data =~ s/[a-z0-9]+_[a-z0-9]+_[a-z0-9]+(_[BP]_[a-z0-9]+){0,1}\./id./g;
$data =~ s/sqlx_[a-z0-9]+_[a-z0-9]+_/sqlx_nnnn_nnnn_/g;
$data =~ s/sqlx-[a-z0-9]+_[a-z0-9]+/sqlx-nnnn_nnnn/g;
$data =~ s/#p#/#P#/g;
$data =~ s/#sp#/#SP#/g;
$data =~ s/#tmp#/#TMP#/g;
$data =~ s/_([0-9]+)\.sdi/_id.sdi/g;

write_file($filename, $data);

sub read_file {
    my ($filename) = @_;

    open my $in, '<', $filename or die "Could not open '$filename' for reading $!";
    undef $/;
    my $all = <$in>;
    close $in;

    return $all;
}

sub write_file {
    my ($filename, $content) = @_;

    open my $out, '>', $filename or die "Could not open '$filename' for writing $!";;
    print $out $content;
    close $out;

    return;
}

EOF
