.. rn:: 5.5.18-23.0

==============================
 |Percona Server| 5.5.18-23.0
==============================

Percona is glad to announce the release of |Percona Server| 5.5.18-23.0 on December 17th, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.18-23.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.18 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-18.html>`_, including all the bug fixes in it, |Percona Server| 5.5.18-23.0 is now the current stable release in the 5.5 series. All of |Percona| 's software is open-source and free, all the details of the release can be found in the `5.5.18-23.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.18-23.0>`_.


New Features
============

|Percona Server| now supports group commit between |XtraDB| and the replication binlog. Percona has imported the group commit patch from MariaDB and is making the performance improvements that group commit brings available to users of |Percona Server| 5.5. See the `Testing the Group Commit Fix <http://www.mysqlperformanceblog.com/2011/07/13/testing-the-group-commit-fix/>`_ blog post for the kind of performance improments that can be expected.


Bug Fixes
=========

  * Several crashes were reported when using the ``--query-cache-strip-comments`` feature of |Percona Server|. We have fixed several causes for crashes, especially around the handling of escaped characters. Bugs fixed: :bug:`856404`, :bug:`705688` (*Oleg Tsarev*)

  * The :ref:`Expand Table Import <innodb_expand_import_page>` was improved not to hold the InnoDB data dictionary mutex for the full duration of the import operation.  This allows queries accessing other InnoDB tables to proceed normally and not be blocked until the import completes.  Bug fixed: :bug:`901775` (*Alexey Kopytov*)
  
  * As a follow-up to the already-fixed :bug:`803865`, further fixes were made to the implementation of atomic operations which is used on 32-bit systems when compiled without i686+ support.  There were no observed issues with the previous implementation, the fixes were made proactively for benign issues.  Additionally, the :ref:`Response Time Distribution <response_time_distribution>`, which uses those operations, was made slightly more efficient.  Bug fixed: :bug:`878022` (*Laurynas Biveinis*)

  * An output buffer truncation check in :ref:`Response Time Distribution <response_time_distribution>` was fixed.  Bug fixed: :bug:`810272` (*Laurynas Biveinis*)

  * The compilation warnings, produced by GCC versions up to and including 4.6, were audited and fixed.  Bug fixed: :bug:`878164` (*Laurynas Biveinis*)

  * Testsuite stability fix for the percona_status_wait_query_cache_mutex test.  Bug fixed: :bug:`878709` (*Oleg Tsarev*)

  * A missing link was added to the Percona Server upgrade documentation.  Bug fixed: :bug:`885633` (*Alexey Kopytov*)
