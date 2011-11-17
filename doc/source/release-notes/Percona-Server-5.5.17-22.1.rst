.. rn:: 5.5.17-22.1

==============================
 |Percona Server| 5.5.17-22.1
==============================

Percona is glad to announce the release of |Percona Server| 5.5.17-22.1 on November 19th, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.17-22.1/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.17 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-17.html>`_, including all the bug fixes in it, |Percona Server| 5.5.17-22.1 is now the current stable release in the 5.5 series. All of |Percona| 's software is open-source and free, all the details of the release can be found in the `5.5.17-22.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.17-22.1>`_.



Bug Fixes
=========

  * MyISAM repair-by-sort buffer could not be greater than 4GB even on 64bit architectures. Bug Fixed: :bug:`878404` (*Alexey Kopytov*).

  * The kill idle transactions feature in |XtraDB| (if enabled) could sometimes cause the server to crash. Bug Fixed: :bug:`871722` (*Yasufumi Kinoshita*).

  * In a master-master setup when using SET user variables it was possible to have `SHOW SLAVE STATUS` give incorrect output due to a corrupted relay log. Bug Fixed: :bug:`860910` (*Alexey Kopytov*).
