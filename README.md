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

Contribute
----------

Please report TokuDB bugs at https://tokutek.atlassian.net/browse/DB.

We have two publicly accessible mailing lists:

 - tokudb-user@googlegroups.com is for general and support related
   questions about the use of TokuDB.
 - tokudb-dev@googlegroups.com is for discussion of the development of
   TokuDB.

We are on IRC on freenode.net, in the #tokutek channel.


License
-------

TokuDB is available under the GPL version 2 and AGPL version 3.  See [COPYING][copying]

The PerconaFT component of TokuDB is available under the GPL version 2, and AGPL version 3 with
slight modifications.  See [README-TOKUDB][license].

[copying]: http://github.com/Percona/tokudb-engine/blob/master/COPYING
[license]: http://github.com/Percona/PerconaFT/blob/master/README-TOKUDB
