.. rn:: 5.5.19-24.0

==============================
 |Percona Server| 5.5.19-24.0
==============================

Percona is glad to announce the release of |Percona Server| 5.5.19-24.0 on January 13th, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.19-24.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.19 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-19.html>`_, including all the bug fixes in it, |Percona Server| 5.5.19-24.0 is now the current stable release in the 5.5 series. All of |Percona| 's software is open-source and free, all the details of the release can be found in the `5.5.19-24.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.19-24.0>`_.


New Features
============

  * Variable :variable:`innodb_flush_neighbor_pages` can be now set to
    a new value ``cont``.  The previously-available option values 0
    and 1 now have more descriptive names ``none`` and ``area``.  The
    value of ``none`` disables the neighbor page flush and ``area``
    matches the default |InnoDB| behavior: any dirty pages in the
    vicinity of the page selected for flushing may be flushed too.
    The new option value ``cont`` improves the neighbor flushing by
    considering only contiguous blocks of neighbor pages, thus
    performing the flush by sequential instead of random
    I/O. (*Yasufumi Kinoshita*, *Laurynas Biveinis*)

  * Improvements to the XtraDB's sync flush algorithm.  If the XtraDB
    checkpoint age grows dangerously close to its limit and XtraDB is
    forced to perform a sync flush, these changes should slightly
    improve the user query performance instead of completely blocking
    them. (*Yasufumi Kinoshita*, *Laurynas Biveinis*)

Bug Fixes
=========

  * Minor MEMORY engine test suite fix: :bug:`849921` (*Laurynas
    Biveinis*)
  * A fix for testsuite integration into Jenkins: :bug:`911237`
    (*Oleg Tsarev*)
