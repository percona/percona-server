.. rn:: 5.6.37-82.2

==========================
Percona Server 5.6.37-82.2
==========================

Percona is glad to announce the release of Percona Server 5.6.37-82.2
on August 25, 2017.
Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.37-82.2/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.6.37
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-37.html>`_
and includes all the bug fixes in it.
Percona Server 5.6.37-82.2 is now the current stable release in the 5.6 series.
All software developed by Percona is open-source and free.
Details of the release can be found in the `5.6.37-82.2 milestone on Launchpad
<https://launchpad.net/percona-server/+milestone/5.6.37-82.2>`_.

.. note:: Red Hat Enterprise Linux 5 (including CentOS 5 and other derivatives),
   Ubuntu 12.04 and older versions are no longer supported by Percona software.
   The reason for this is that these platforms reached end of life,
   will not receive updates and are not recommended for use in production.

Bugs Fixed
==========

* :bug:`1703105`: Fixed overwriting of error log on server startup.

* :bug:`1705729`: Fixed the ``postinst`` script
  to correctly locate the ``datadir``.

* :bug:`1709834`: Fixed the ``mysqld_safe`` script
  to correctly locate the ``basedir``.

* Other fixes: :bug:`1706262`

TokuDB Changes
==============

* :jirabug:`TDB-72`: Fixed issue when renaming a table
  with non-alphanumeric characters in its name.

Platform Support
================

* Stopped providing packages for RHEL 5 (CentOS 5) and Ubuntu 12.04.

