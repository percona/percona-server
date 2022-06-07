# Purpose:
# To get name and size of the latest file written
# by online log tracking. In additional it returns
# number of bitmap files found in the provided directory.
#
# Parameters:
# --let $bmp_data_dir = path/to/bmp/files/dir
#
# $bmp_data_dir
# Path to the directory where bitmap files will be searched
#
# Usage:
# --let $bmp_data_dir = path/to/pmp/files/dir
# --source include/percona_get_last_bmp_file_name_size.inc
#
# Return Value:
# $last_bmp_file_name
# $last_bmp_file_size
# $bmp_files_count
#

--let _BMP_DATA_DIR = $bmp_data_dir
--perl
    use strict;
    use warnings;
    use File::stat;

    my $tmp_files_dir = $ENV{'MYSQLTEST_VARDIR'};
    my $bmp_data_dir = $ENV{'_BMP_DATA_DIR'};

    open(OUTPUT, ">$tmp_files_dir/tmp/last_bmp_file.inc")
        or die "Could not open $tmp_files_dir/tmp/active_bmp_file.inc";
    opendir(DATADIR, $bmp_data_dir)
        or die "Could not open $bmp_data_dir\n";

    my $last_bmp_file_name = '';
    my $last_bmp_file_size = 0;
    my $files = {};

    while (my $filename = readdir(DATADIR)) {
        next if ($filename !~ m/xdb/);

        $files->{$filename} = 1;
        my $fs = stat($bmp_data_dir . '/' . $filename)->size;
        if ($filename gt $last_bmp_file_name) {
            $last_bmp_file_name = $filename;
            $last_bmp_file_size = $fs;
        }
    }

    my $bmp_files_count = keys(%$files);

    print OUTPUT "--let \$last_bmp_file_name = $last_bmp_file_name\n";
    print OUTPUT "--let \$last_bmp_file_size = $last_bmp_file_size\n";
    print OUTPUT "--let \$bmp_files_count = $bmp_files_count\n";

    close(OUTPUT);
    closedir(DATADIR);
EOF

--source  $MYSQLTEST_VARDIR/tmp/last_bmp_file.inc
--remove_file $MYSQLTEST_VARDIR/tmp/last_bmp_file.inc
