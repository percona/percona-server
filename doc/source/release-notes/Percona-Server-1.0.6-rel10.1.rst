.. rn:: 1.0.6-rel10.1

==============================
|Percona Server| 1.0.6-rel10.1
==============================

Percona is glad to announce the release of |Percona Server| 1.0.6-rel10.1 on May 25th, 2010.

New Features:
=============

  * Variable ``innodb_trx_id`` is added to slow log statistics

Fixed bugs
===========

  * Fixed MySQL bug `#53371 <http://bugs.mysql.com/bug.php?id=53371>`_ Parent directory entry ("..") can be abused to bypass table level grants.

  * Fixed MySQL bug `#53237 <http://bugs.mysql.com/bug.php?id=53237>`_  ``mysql_list_fields/COM_FIELD_LIST`` stack smashing - remote execution possible.

  * Fixed MySQL bug `#50974 <http://bugs.mysql.com/bug.php?id=50974>`_ Server keeps receiving big (``> max_allowed_packet``) packets indefinitely.
