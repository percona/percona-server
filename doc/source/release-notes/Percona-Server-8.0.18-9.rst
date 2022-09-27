.. _8.0.18-9:

================================================================================
*Percona Server for MySQL* 8.0.18-9
================================================================================

|Percona| announces the release of |Percona Server| |release| on |date| (downloads are available `here <https://www.percona.com/downloads/Percona-Server-8.0/>`__ and from the `Percona Software Repositories <https://www.percona.com/doc/percona-server/8.0/installation.html#installing-from-binaries>`__).

This release includes fixes to bugs found in previous releases of |Percona
Server| 8.0.

|Percona Server| |release| is now the current GA release in the 8.0 series. All
of |Percona|’s software is open-source and free.

Percona Server for MySQL 8.0 includes all the `features available in MySQL
8.0.18 Community Edition
<https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-18.html>`__ in
addition to enterprise-grade features developed by Percona.

Bugs Fixed
================================================================================

- Setting the ``none`` value for :ref:`slow_query_log_use_global_control`
  generates an error. Bugs fixed :psbug:`5813`.

- If `pam_krb5
  <https://docs.oracle.com/cd/E88353_01/html/E37853/pam-krb5-7.html>`__ allows the
  user to change their password, and the password expired, a new password may
  cause a server exit. Bug fixed :psbug:`6023`.

- An incorrect assertion was triggered if any temporary tables should be logged
  to binlog. This event may cause a server exit. Bug fixed :psbug:`5181`.

- The Handler failed to trigger on Error 1049, SQLSTATE 42000, or plain
  sqlexception. Bug fixed :psbug:`6094`. (Upstream :mysqlbug:`97682`)

- When executing ``SHOW GLOBAL STATUS``, the variables may return incorrect
  values. Bug fixed :psbug:`5966`.

- The memory storage engine detected an incorrect ``full`` condition even 
  though the space contained reusable memory chunks released by deleted
  records and the space could be reused. Bug fixed :psbug:`1469`.

Other bugs fixed:

:psbug:`6051`,
:psbug:`5876`,
:psbug:`5996`,
:psbug:`6021`,
:psbug:`6052`,
:psbug:`4775`,
:psbug:`5836` (Upstream :mysqlbug:`96449`),
:psbug:`6123`,
:psbug:`5819`,
:psbug:`5836`,
:psbug:`6054`,
:psbug:`6056`,
:psbug:`6058`,
:psbug:`6078`,
:psbug:`6057`,
:psbug:`6111`, and
:psbug:`6073`.

.. |release| replace:: 8.0.18-9
.. |date| replace:: December 11, 2019
