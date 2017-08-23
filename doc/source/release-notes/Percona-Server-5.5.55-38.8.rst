.. rn:: 5.5.55-38.8

============================
|Percona Server| 5.5.55-38.8
============================

Percona is glad to announce the release of |Percona Server| 5.5.55-38.8 on
May 10th, 2017. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.55-38.8/>`_
and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.55
<http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-55.html>`_, including
all the bug fixes in it, |Percona Server| 5.5.55-38.8 is now the current stable
release in the 5.5 series. All of |Percona|'s software is open-source and free,
all the details of the release can be found in the `5.5.55-38.8 milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.5.55-38.8>`_.

New Features
============

 |Percona Server| 5.5 packages are now available for Ubuntu 17.04 (*Zesty
 Zapus*).

Bugs Fixed
==========

 If a bitmap write I/O errors happened in the background log tracking thread
 while a ``FLUSH CHANGED_PAGE_BITMAPS`` is executing concurrently it could
 cause a server crash. Bug fixed :bug:`1651656`.

 Querying :table:`TABLE_STATISTICS` in combination with a stored function could
 lead to a server crash. Bug fixed :bug:`1659992`.

 Queries from the :table:`INNODB_CHANGED_PAGES` table would needlessly read
 potentially incomplete bitmap data past the needed LSN range. Bug fixed
 :bug:`1625466`.

 It was not possible to configure basedir as a symlink. Bug fixed
 :bug:`1639735`.

Other bugs fixed: :bug:`1688161`, :bug:`1683456`, :bug:`1670588` (upstream
:mysqlbug:`84173`), :bug:`1672389`, :bug:`1675623`, :bug:`1660243`,
:bug:`1677156`, :bug:`1680061`, :bug:`1680510` (upstream :mysqlbug:`85838`),
:bug:`1683993`, :bug:`1684012`, :bug:`1684025`, and :bug:`1674281`.
