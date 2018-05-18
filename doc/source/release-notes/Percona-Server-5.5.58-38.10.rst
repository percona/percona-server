.. rn:: 5.5.58-38.10

=============================
|Percona Server| 5.5.58-38.10
=============================

Percona is glad to announce the release of |Percona Server| 5.5.58-38.10 on
December 7th, 2017. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.58-38.10/>`_
and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.58
<http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-58.html>`_, including
all the bug fixes in it, |Percona Server| 5.5.58-38.10 is now the current
stable release in the 5.5 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.5.58-38.10
milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.5.58-38.10>`_.

New Features
============

 |Percona Server| packages are now available for *Ubuntu 17.10 (Artful)*.

Bugs Fixed
==========

 If an I/O syscall returned an error during the server shutdown with
 :ref:`threadpool` enabled, a mutex could be left locked. Bug fixed
 :bug:`1702330` (*Daniel Black*).

 ``MEMORY`` storage engine incorrectly allowed ``BLOB`` columns before indexed
 columns. Bug fixed :bug:`1731483`.
 
Other bugs fixed: :bug:`1729241`.
