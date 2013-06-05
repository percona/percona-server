.. rn:: 5.1.68-14.6

=============================
 |Percona Server| 5.1.68-14.6
=============================

Percona is glad to announce the release of |Percona Server| 5.1.68-14.6 on April 19th, 2013 (Downloads are available from `Percona Server 5.1.68-14.6 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.68-14.6/>`_ and from the `Percona Software Repositories <http://http://www.percona.com/doc/percona-server/5.1/installation.html>`_).

Based on `MySQL 5.1.68 <http://dev.mysql.com/doc/refman/5.1/en/news-5.1.68.html>`_, this release will include all the bug fixes in it. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.68-14.6 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.68-14.6>`_.

Bug Fixes
=========

 Fixed yum dependencies that were causing conflicts in ``CentOS`` 6.3 during installation. Bugs fixed :bug:`1031427` and :bug:`1051874`.

 When **mysqldump** was used with :option:`--innodb-optimize-keys` option it produced invalid SQL for cases when there was an explicitly named foreign key constraint which implied an implicit secondary index with the same name. Fixed by detecting such cases and omitting the corresponding secondary keys from deferred key creation optimization. Bugs fixed :bug:`1081016`.

 When **mysqldump** was used with :option:`--innodb-optimize-keys` and :option:`--no-data` options, all secondary key definitions would be lost. Bug fixed :bug:`989253`.

 |Percona Server| was built with YaSSL which could cause some of the programs that use it to crash. Fixed by building packages with OpenSSL support rather than the bundled YaSSL library. Bug fixed :bug:`1104977`.

 Fix for bug :bug:`1070856` introduced a regression in |Percona Server| :rn:`5.1.66-14.2` which could cause a server to hang when binary log is enabled. Bug fixed :bug:`1162085`.

 |Percona Server| would re-create the test database when using ``rpm`` on server upgrade, even if the database was previously removed. Bug fixed :bug:`710799`.

 *Debian* packages included the old version of **innotop**. Fixed by removing **innotop** and its ``InnoDBParser`` Perl package from source and *Debian* installation. Bug fixed :bug:`1032139`.

 |Percona Server| was missing help texts in the |MySQL| client because the help tables were missing. Bug fixed :bug:`1041981`.
 
Other bug fixes: bug fixed :bug:`1154962`, bug fixed :bug:`1154959`, bug fixed :bug:`1154957`, bug fixed :bug:`1154954`, bug fixed :bug:`1144059`, bug fixed :bug:`1050536`.
