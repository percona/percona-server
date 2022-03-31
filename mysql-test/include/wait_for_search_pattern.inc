
# Purpose:
#    Simple search with Perl for a pattern in some file that repeats the
#    search until a timeout occurs.  Bulk of search logic copied from
#    include/search_pattern.inc
#
#    The environment variables SEARCH_FILE and SEARCH_PATTERN must be set
#    before sourcing this routine.  The environment variable SEARCH_WAIT
#    may be set to specify the time to keep waiting for the desired
#    pattern to appear.  The default timeout is 60 seconds.
#
#    In case of
#    - SEARCH_FILE and/or SEARCH_PATTERN is not set
#    - SEARCH_FILE cannot be opened
#    the test will abort immediate.

perl;
    use strict;
    my @search_files =         glob($ENV{'SEARCH_FILE'})     or die "SEARCH_FILE not set";
    my $search_pattern=        $ENV{'SEARCH_PATTERN'}        or die "SEARCH_PATTERN not set";
    my $search_wait=           $ENV{'SEARCH_WAIT'}     || 60;
    my $search_echo=           $ENV{'SEARCH_ECHO'}     || "NONE";
    my $ignore_pattern=        $ENV{'IGNORE_PATTERN'}  || "";
    my $search_echo_ctx=       $ENV{'SEARCH_ECHO_CTX'} || 0;

    my $flag= 0;
    my $counter= $search_wait;

    # We include the current line and 'n' previous lines in its context
    # as a FIFO queue i.e., oldest entries in the queue are removed when
    # new lines are read from the file.
    my @context= (undef) x ($search_echo_ctx + 1);

    my $line;

    while ($counter)
    {
        foreach my $search_file (@search_files) {
            open(FILE, "< $search_file") or die("Unable to open '$search_file': $!\n");

            # Read the file line by line
            while ($line= <FILE>)
            {
                my $cur_lineno= $.;
                my $file_pos= tell(FILE);

                # Context stored as a sliding window of size 'n' lines
                shift @context;
                push  @context, $line;

                if ($line =~ /$search_pattern/)
                {
                    if (!(length $ignore_pattern) or !($line =~ /$ignore_pattern/))
                    {
                        # Line matches the specified pattern
                        $flag= 1;

                        if ($search_echo eq 'FIRST')
                        {
                            print_current_and_previous_n_lines($search_echo_ctx,
                                                               $cur_lineno);
                            print_succeeding_n_lines($search_echo_ctx, $cur_lineno)
                              if ($search_echo_ctx > 0);
                            last;
                        }
                        elsif ($search_echo eq 'ALL')
                        {
                            print_current_and_previous_n_lines($search_echo_ctx,
                                                               $cur_lineno);

                            if ($search_echo_ctx > 0)
                            {
                                print_succeeding_n_lines($search_echo_ctx, $cur_lineno);

                                # Return to current file position after reading 'n'
                                # succeeding lines
                                seek(FILE, $file_pos, 0);
                                $.= $cur_lineno;

                                # Separate each matching line
                                print "---\n";
                            }
                        }
                    }
                }
            }
            close(FILE);
        }

        if ($flag == 1)
        {
            print "Pattern \"$search_pattern\" found\n";
            last;
        }
        sleep 1;
        $counter--;
    }
    if ($flag == 0)
    {
        print "Pattern \"$search_pattern\" not found\n";
    }

    sub print_current_and_previous_n_lines()
    {
        my ($search_echo_ctx, $cur_lineno)= @_;
        my $start_lineno= $cur_lineno - $search_echo_ctx;
        $cur_lineno= $start_lineno > 0 ? $start_lineno: 1;

        for my $ctx_line (@context)
        {
            next if not defined $ctx_line;
            print "$cur_lineno: $ctx_line";
            $cur_lineno++;
        }
    }

    sub print_succeeding_n_lines()
    {
        # Read the next 'n' lines from the file (excluding the current
        # line) and print them

        my ($search_echo_ctx, $cur_lineno)= @_;
        my $end_lineno= $cur_lineno + $search_echo_ctx;

        while ($line= <FILE>)
        {
            last if ($. > $end_lineno);
            print "$.: $line";
        }
    }

EOF

# Setting the env vars to default value
let SEARCH_ECHO=NONE;
let IGNORE_PATTERN=;
let SEARCH_ECHO_CTX=0;
