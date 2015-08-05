TokuDB
======

TokuDB is a high-performance, write optimized, transactional storage engine for Percona Server and MySQL.
For more details, see our [product page][products].

This repository contains the MySQL plugin that uses the [PerconaFT][perconaft] core.

[products]: https://www.percona.com/software/percona-tokudb
[perconaft]: http://github.com/Percona/PerconaFT

Download
--------

* [Percona Server 5.6 + TokuDB](http://www.percona.com/downloads/)

Build
-----

Before you start, make sure you have a C++11-compatible compiler (GCC >=
4.7 is recommended), as well as CMake >=2.8.8, and the libraries and
header files for valgrind,zlib, and Berkeley DB.  We are using the gcc 4.7
in devtoolset-1.1.

On CentOS, `yum install valgrind-devel zlib-devel libdb-devel`

On Ubuntu, `apt-get install valgrind zlib1g-dev libdb-dev`

You can set the compiler by passing `--cc` and `--cxx` to the script, to
select one that's new enough.  The default is `scripts/make.mysql.bash
--cc=gcc47 --cxx=g++47`, which may not exist on your system.

We use gcc from devtoolset-1.1 on CentOS 5.9 for builds.

To build a complete set of Percona Server and TokuDB, follow the instructions at
[build a debug environment][howtobuild].

[howtobuild]: https://github.com/percona/tokudb-percona-server-5.6/wiki/Build-a-debug-environment

Contribute
----------

Please report TokuDB bugs to the [issue tracker][jira].

We have two publicly accessible mailing lists:

 - tokudb-user@googlegroups.com is for general and support related
   questions about the use of TokuDB.
 - tokudb-dev@googlegroups.com is for discussion of the development of
   TokuDB.

All source code and test contributions must be provided under a [BSD 2-Clause][bsd-2] license. For any small change set, the license text may be contained within the commit comment and the pull request. For larger contributions, the license must be presented in a COPYING.<feature_name> file in the root of the tokudb-engine project. Please see the [BSD 2-Clause license template][bsd-2] for the content of the license text.

[jira]: https://tokutek.atlassian.net/browse/DB/
[bsd-2]: http://opensource.org/licenses/BSD-2-Clause/

License
-------

TokuDB is available under the GPL version 2 and AGPL version 3.  See [COPYING][copying]

PerconaFT is a part of TokuDB and is available under the GPL version 2,
and AGPL version 3, with slight modifications. See [COPYING.AGPLv3][agpllicense],
[COPYING.GPLv2][gpllicense], and
[PATENTS][patents].

[agpllicense]: http://github.com/Perona/PerconaFT/blob/master/COPYING.AGPLv3
[gpllicense]: http://github.com/Perona/PerconaFT/blob/master/COPYING.GPLv2
[patents]: http://github.com/Perona/PerconaFT/blob/master/PATENTS
