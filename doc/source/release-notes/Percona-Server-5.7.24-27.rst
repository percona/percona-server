.. rn:: 5.7.24-27
	
==========================
|Percona Server| 5.7.24-27
==========================
	
Percona is glad to announce the release of |Percona Server| 5.7.24-27 on
December 18, 2018. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.24-27/>`_
and from the :doc:`Percona Software Repositories </installation>`.
	
This release is based on `MySQL 5.7.24
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-24.html>`_
and includes all the bug fixes in it. |Percona Server| 5.7.24-27 is
now the current GA (Generally Available) release in the 5.7 series.
	
All software developed by Percona is open-source and free.
	
.. note:: 

   If you're currently using |Percona Server| 5.7, Percona recommends
   upgrading to this version of 5.7 prior to upgrading to |Percona Server|
   8.0.

Bugs Fixed
==========
	
* When uninstalling |Percona Server| packages on *CentOS 7* default
  configuration file :file:`my.cnf` would get removed as well. This fix
  makes the backup of the configuration file instead of removing it.
  Bug fixed :psbug:`5092`.
	

.. 5.7.24-27 replace:: 5.7.24-27

