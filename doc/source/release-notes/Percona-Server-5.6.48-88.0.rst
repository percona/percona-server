.. _PS-5.6.48-88.0:

================================================================================
*Percona Server for MySQL* 5.6.48-88.0
================================================================================

:Date: May 13, 2020
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.6/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.6.48-88.0
includes all the features and bug fixes available in
`MySQL 5.6.48 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-48.html>`_
in addition to enterprise-grade features developed by Percona.

Improvements
================================================================================

* :jirabug:`PS-5391`: RedHat-based distributions may set service configurations in a file saved in the ‘/etc/sysconfig' directory. The init script did not read any file in that directory. We have added the ability for the init.d script to read files in the ‘etc/sysconfig’ directory.



Bugs Fixed
================================================================================

* :jirabug:`PS-6953`: If the LOAD DATA process found a line with an escape character followed by UTF8 characters, then the process failed to load the line and any additional lines. This issue was a regression. (Upstream :mysqlbug:`98089`)
* :jirabug:`PS-6945`: The tokubackup process exported an incorrect API. This API mismatch prevented large file backups.
* :jirabug:`PS-6946`: The address and thread sanitizers found memory use after free and other issues in the tokubackup software..
* :jirabug:`PS-7020`: In Ubuntu 20.04, only python3-mysqldb is available. We converted the MTR tests to support both Python2 and Python3. The scripts require Python2.6 or newer.
* :jirabug:`PS-6954`: In the tokudb-backup-plugin, there was a collision between -std=c++11 and -std=gnu++03.
* :jirabug:`PS-6899`: The tests, main.events_bugs and main.events_1, failed because 2020-01-01 was considered a future time. (Upstream :mysqlbug:`98860`)
* :jirabug:`PS-6110`: The libHotBackup.so was missing from Percona-Server-5.6.46-rel86.2-Linux.x86_64.ssl101.tar.gz.
* :jirabug:`PS-4649`: TokuDB: Documented PerconaFT, which is fractal tree indexing. This type of indexing enhances the B-tree data structure.
* :jirabug:`PS-6966`: In Percona Server compilation, fixed clang-10 compilation issues. (Upstream :mysqlbug:`99221`)
