.. rn:: 5.6.21-70.1

==============================
 |Percona Server| 5.6.21-70.1 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.21-70.1 on November 24th, 2014 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.21-70.1/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.21 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-21.html>`_, including all the bug fixes in it, |Percona Server| 5.6.21-70.1 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.21-70.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.21-70.1>`_. 


Bugs Fixed
==========

 A slave replicating in RBR mode would crash, if a table definition between master and slave differs with an allowed conversion, and the binary log contains a table map event followed by two row log events. This bug is an upstream regression introduced by a fix for bug :mysqlbug:`72610`. Bug fixed :bug:`1380010`. 
 
 An incorrect source code function attribute would cause MySQL to crash on an InnoDB row write, if compiled with a recent GCC with certain compilation options.  Bug fixed :bug:`1390695` (upstream :mysqlbug:`74842`).

 MTR tests for :ref:`response_time_distribution` were not packaged in binary packages. Bug fixed :bug:`1387170`.

 The RPM packages provided for *CentOS* 5 were built using a debugging information format which is not supported in the ``gdb`` version included with *CentOS* 5.10. Bug fixed :bug:`1388972`.

 A session on a server in mixed mode binlogging would switch to row-based binlogging whenever a temporary table was created and then queried. This switch would last until the session end or until all temporary tables in the session were dropped. This was unnecessarily restrictive and has been fixed so that only the statements involving temporary tables were logged in the row-based format whereas the rest of the statements would continue to use the statement-based logging. Bug fixed :bug:`1313901` (upstream :mysqlbug:`72475`).

Other bugs fixed: :bug:`1387227`, and :bug:`1388001`.
