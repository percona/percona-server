.. rn:: 5.5.57-38.9

==========================
Percona Server 5.5.57-38.9
==========================

Percona is glad to announce the release of Percona Server 5.5.57-38.9
on August 23, 2017.
Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.55-38.9/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.5.57
<http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-57.html>`_
and includes all the bug fixes in it.
Percona Server 5.5.57-38.9 is now the current stable release in the 5.5 series.
All software developed by Percona is open-source and free.
Details of this release can be found
in the `5.5.57-38.9 milestone on Launchpad
<https://launchpad.net/percona-server/+milestone/5.5.57-38.9>`_.

.. note:: Red Hat Enterprise Linux 5 (including CentOS 5 and other derivatives),
   Ubuntu 12.04 and older versions are no longer supported by Percona software.
   The reason for this is that these platforms reached end of life,
   will not receive updates and are not recommended for use in production.

New Features
============

* :bug:`1702903`: Added support of OpenSSL 1.1.

Platform Support
================

* Added support and packages for Debian 9 (stretch).
  Covers only the ``amd64`` architecture.

* Removed packages for RHEL 5 (CentOS 5) and Ubuntu 12.04.

Bugs Fixed
==========

* :bug:`1622985`: Downgraded diagnostic severity from warning to normal note
  for successful doublewrite buffer recovery.

* :bug:`1661488`: Fixed crash of debug server build when two clients connected,
  one of them enabled ``userstat``
  and ran either ``FLUSH CLIENT_STATISTICS`` or ``FLUSH USER_STATISTICS``,
  and then both clients exited.

* :bug:`1673656`: Added support of wildcards and Subject Alternative Names (SAN)
  in SSL certificates for ``--ssl-verify-server-cert``.
  For more information, see :ref:`compatibility-matrix`.

* :bug:`1705729`: Fixed the ``postinst`` script
  to correctly locate the ``datadir``.

* :bug:`1709834`: Fixed the ``mysqld_safe`` script
  to correctly locate the ``basedir``.

* Minor fixes: :bug:`1160986`, :bug:`1684601`, :bug:`1689998`,
  :bug:`1690012`.

.. _compatibility-matrix:

Compatibility Matrix
====================

=======================  =======  ==================  ====================
Feature                  YaSSL    OpenSSL < 1.0.2     OpenSSL >= 1.0.2
=======================  =======  ==================  ====================
'commonName' validation  Yes      Yes                 Yes       
SAN validation           No       Yes                 Yes       
Wildcards support        No       No                  Yes         
=======================  =======  ==================  ====================

