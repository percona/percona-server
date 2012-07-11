# Perl module for parsing and generating the Subunit protocol
# Copyright (C) 2008-2009 Jelmer Vernooij <jelmer@samba.org>
#
#  Licensed under either the Apache License, Version 2.0 or the BSD 3-clause
#  license at the users choice. A copy of both licenses are available in the
#  project source as Apache-2.0 and BSD. You may not use this file except in
#  compliance with one of these two licences.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under these licenses is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
#  license you chose for the specific language governing permissions and
#  limitations under that license.

package Subunit;
use POSIX;

use vars qw ( $VERSION );

$VERSION = '0.0.2';

use strict;
my $SUBUNIT_OUT= 'test_results.subunit';
# reset the file
open(SUBUNITOUT, ">$SUBUNIT_OUT");
close(SUBUNITOUT);

sub subunit_start_test($)
{
	my ($testname) = @_;
        open(SUBUNITOUT, ">>$SUBUNIT_OUT");
	print SUBUNITOUT "test: $testname\n";
        close(SUBUNITOUT);
        return;
}

sub subunit_end_test($$;$)
{
	my $name = shift;
	my $result = shift;
	my $reason = shift;
        open(SUBUNITOUT, ">>$SUBUNIT_OUT");
	if ($reason) {
		print SUBUNITOUT "$result: $name [\n";
		print SUBUNITOUT "$reason\n";
		print SUBUNITOUT "]\n";
	} else {
		print SUBUNITOUT "$result: $name\n";
	}
        close(SUBUNITOUT);
        return;
}

sub subunit_skip_test($;$)
{
	my $name = shift;
	my $reason = shift;
	subunit_end_test($name, "skip", $reason);
}

sub subunit_fail_test($;$)
{
	my $name = shift;
	my $reason = shift;
	subunit_end_test($name, "failure", $reason);
}

sub subunit_pass_test($;$)
{
	my $name = shift;
	my $reason = shift;
	subunit_end_test($name, "success", $reason);
}

sub subunit_xfail_test($;$)
{
	my $name = shift;
	my $reason = shift;
	subunit_end_test($name, "xfail", $reason);
}

sub report_time($)
{
	my ($time) = @_;
	my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime($time);
        open(SUBUNITOUT, ">>$SUBUNIT_OUT");
	printf SUBUNITOUT "time: %04d-%02d-%02d %02d:%02d:%02dZ\n", $year+1900, ($mon+1), $mday, $hour, $min, $sec;
        close(SUBUNITOUT);
        return;
}



1;
