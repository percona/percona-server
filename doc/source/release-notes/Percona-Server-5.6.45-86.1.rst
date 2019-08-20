.. rn:: 5.6.45-86.1

================================================================================
|Percona Server| |release|
================================================================================

Percona is glad to announce the release of |Percona Server| |release| on
|date| (Downloads are available `here
<https://www.percona.com/downloads/Percona-Server-5.6/LATEST/>`_
and from the :doc:`Percona Software Repositories </installation>`).

This release merges changes of `MySQL 5.6.45
<https://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-45.html>`_, including
all the bug fixes in it. Percona Server for MySQL 5.6.45-86.1 is now the current
GA release in the 5.6 series. All of Percona’s software is open-source and free.

Bugs Fixed
================================================================================

- The TokuDB hot backup library continually dumps TRACE information to the server error log. The user cannot enable or disable the dump of this information. Bug fixed :psbug:`4850`

- The TokuDBBackupPlugin is optional at cmake time. Bug fixed :psbug:`5748`.

Other Bugs Fixed
================================================================================

- :psbug:`5531`
- :psbug:`5146`
- :psbug:`5638`
- :psbug:`5645`
- :psbug:`5669`
- :psbug:`5749`
- :psbug:`5752`
- :psbug:`5780`
- :psbug:`5833`
- :psbug:`5725`
- :psbug:`5742`
- :psbug:`5743`
- :psbug:`5746`


Find the release notes for Percona Server for MySQL 5.6.45-86.1 in
`online documentation
<https://www.percona.com/doc/percona-server/5.6/index.html>`_. Report
bugs in the `Jira bug tracker
<https://jira.percona.com/projects/PS>`_.


.. |release| replace:: 5.6.45-86.1
.. |date| replace:: August 20, 2019

