.. rn:: 5.7.19-17

========================
Percona Server 5.7.19-17
========================

Percona is glad to announce the release of Percona Server 5.7.19-17
on August 17, 2017.
Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.19-17/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.19
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-19.html>`_
and includes all the bug fixes in it.
Percona Server 5.7.19-17 is now the current GA release in the 5.7 series.
All software developed by Percona is open-source and free.
Details of this release can be found in the `5.7.19-17 milestone on Launchpad
<https://launchpad.net/percona-server/+milestone/5.7.19-17>`_.

.. note:: Red Hat Enterprise Linux 5 (including CentOS 5 and other derivatives),
   Ubuntu 12.04 and older versions are no longer supported by Percona software.

New Features
============

* Included the MyRocks storage engine

  .. note:: MyRocks for Percona Server is currently experimental
     and not recommended for production deployments until further notice.
     You are encouraged to try it in a testing environment
     and provide feedback or report bugs.

* Implemented Performance Schema instrumentation for the TokuDB storage engine

* Removed packages for Ubuntu 12.04.

* :bug:`1708087`: Added the ``mysql-helpers`` script
  to handle checking for missing ``datadir`` during startup.
  Also fixes :bug:`1635364`.

Bugs Fixed
==========

* :bug:`1669414`: Fixed handling of failure to set ``O_DIRECT``
   on parallel doublewrite.

* :bug:`1705729`: Fixed the ``postinst`` script
  to correctly locate the ``datadir``.
  Also fixes :bug:`1698019`.

* :bug:`1709811`: Fixed ``yum upgrade`` to not enable the ``mysqld`` service
  if it was disabled before the upgrade.

* :bug:`1709834`: Fixed the ``mysqld_safe`` script
  to correctly locate the ``basedir``.

* Other fixes: :bug:`1698996`, :bug:`1706055`, :bug:`1706262`, :bug:`1706981`

TokuDB Changes
==============

* :jirabug:`TDB-70`: Prevented ``fsync`` of TokuDB redo log
  during binlog group commit flush stage.

