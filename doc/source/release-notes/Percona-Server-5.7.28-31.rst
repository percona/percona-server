.. rn:: 5.7.28-31:

===============================================================================
|Percona Server| 5.7.28-31
===============================================================================

Percona is glad to announce the release of |Percona Server| 5.7.28-31 on November 13, 2019. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.28-31/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.28 <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-28.html>`_ and includes all the bug fixes in it. |Percona Server| 5.7.28-31 is now the current GA
(Generally Available) release in the 5.7 series.

All software developed by Percona is open-source and free.

.. note::

   If you're currently using |Percona Server| 5.7, Percona recommends upgrading to this version of 5.7 prior to upgrading to |Percona Server| 8.0.

Bugs Fixed
===============================================================================

- When using `skip-innodb_doublewrite` in my.cnf, a parallel doublewrite buffer is still created. Bugs fixed :psbug:`3411`.

- During a binlogging replication event, if the master crashes after the multi-threaded slave has begun copying to the slave's relay log and before the process has completed, a `STOP SLAVE` on the slave takes longer than expected. Bug fixed :psbug:`5824`.

- If `pam_krb5 <https://docs.oracle.com/cd/E88353_01/html/E37853/pam-krb5-7.html>`__ is configured to allow the user to change their password, and the password expired, the server crashed after receiving the new password. Bug fixed :psbug:`6023`.


Other bugs fixed:
:psbug:`5859`,
:psbug:`5910`,
:psbug:`5966`,
:psbug:`4784`,
:psbug:`5216`,
:psbug:`5327`,
:psbug:`5584`,
:psbug:`5642`,
:psbug:`5659`,
:psbug:`5754`,
:psbug:`5761`,
:psbug:`5797`,
:psbug:`5875`,
:psbug:`5933`,
:psbug:`5941`,
:psbug:`5997`,
:psbug:`6050`,
:psbug:`6052`,
:psbug:`3345`, and
:psbug:`5585`

Known Issues
===============================================================================

- :psbug:`5783`: The length of time and resources required for a MySQL query execution increased with a large number of table partitions. :ref:`query-limit-estimates` describes the experimental options added to prevent index scans on the partitions and return a specified number of values.


.. November 13, 2019 replace:: November 13, 2019
.. 5.7.28-31 replace:: 5.7.28-31
