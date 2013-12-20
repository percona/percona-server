.. rn:: 5.6.14-62.0

==============================
 |Percona Server| 5.6.14-62.0
==============================

Percona is glad to announce the release of |Percona Server| 5.6.14-62.0 on October 24th, 2013 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.14-62.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.6.14 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-14.html>`_, including all the bug fixes in it, |Percona Server| 5.6.14-62.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.14-62.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.14-62.0>`_.

New Features
============
 
 |Percona Server| has implemented more efficient log block checksums with new :variable:`innodb_log_checksum_algorithm` variable.

 |Percona Server| has implemented support for :ref:`per_query_variable_statement`.

Bugs Fixed
==========

 |Percona Server| could crash while accessing ``BLOB`` or ``TEXT`` columns in |InnoDB| tables if :ref:`innodb_fake_changes_page` was enabled. Bug fixed :bug:`1188168`.

 :ref:`expanded_option_modifiers` did not deallocate memory correctly. Bug fixed :bug:`1167487`. 

 Some :ref:`expanded_option_modifiers` didn't have an effect if they were specified in non-normalized way (:variable:`innodb_io_capacity` vs :variable:`innodb-io-capacity`). Bug fixed :bug:`1233294`.

 Building |Percona Server| with ``-DHAVE_PURIFY`` option would result in an error. Fixed by porting the ``close_socket`` function from |MariaDB|. Bug fixed :bug:`1203567`.

 Enabling :ref:`enforce_engine` feature could lead to error on |Percona Server| shutdown. Bug fixed :bug:`1233354`.

 Storage engine enforcement (:variable:`enforce_storage_engine`) is now ignored when the server is started in either bootstrap or skip-grant-tables mode. Bug fixed :bug:`1236938`.

 When installed |Percona Server| 5.6 GA release was still showing ``RC`` instead of ``GA`` on Debian-based systems. Bug fixed :bug:`1239418`.

Other bugs fixed: bug fixed :bug:`1238008`, bug fixed :bug:`1190604`, bug fixed :bug:`1200162`, bug fixed :bug:`1188172`,  and bug fixed :bug:`1214727`.
