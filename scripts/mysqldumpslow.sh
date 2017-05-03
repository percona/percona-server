#!/usr/bin/env perl

# Copyright (c) 2000, 2012, Oracle and/or its affiliates. All rights reserved.
# Copyright (c) 2016, Percona Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; version 2
# of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
# MA 02110-1301, USA

# mysqldumpslow - parse and summarize the MySQL slow query log

# Original version by Tim Bunce, sometime in 2000.
# Further changes by Tim Bunce, 8th March 2001.
# Handling of strings with \ and double '' by Monty 11 Aug 2001.

usage();

sub usage {
    my $text= <<HERE;
"mysqldumpslow.sh" is not currently compatible with Percona extended slow query
log format. Please use "pt-query-digest" from Percona Toolkit instead
(https://www.percona.com/doc/percona-toolkit/2.2/pt-query-digest.html).

HERE
    print $text;
    exit 0;
}
