.. rn:: 5.5.28-29.3

===============================
 |Percona Server| 5.5.28-29.3
===============================

Percona is glad to announce the release of |Percona Server| 5.5.28-29.3 on January 8th, 2013 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.28-29.3/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.28 <http://dev.mysql.com/doc/refman/5.5/en/news-5.5.28.html>`_, including all the bug fixes in it, |Percona Server| 5.5.28-29.3 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.28-29.3 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.28-29.3>`_. 

Bug Fixes
=========

  Fixed the upstream bug :mysqlbug:`66550` and the security vulnerability `CVE-2012-4414 <http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2012-4414>`_. This was caused because user-supplied identifiers (table names, field names, etc.) are not always properly quoted, so authorized users that have privileges to modify a table (any non-temporary table) can inject arbitrary SQL into the binary log and that could cause multiple SQL injection like vulnerabilities. This bug fix comes originally from MariaDB (see `MDEV-382 <https://mariadb.atlassian.net/browse/MDEV-382>`_). Bug fixed :bug:`1049871` (*Vlad Lesin*).

  Fixed the upstream bug :mysqlbug:`67685` and the security vulnerability `CVE-2012-5611 <http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2012-5611>`_. This vulnerability allowed remote authenticated users to execute arbitrary code via a long argument to the ``GRANT FILE`` command. This bug fix comes originally from MariaDB (see `MDEV-3884 <https://mariadb.atlassian.net/browse/MDEV-3884>`_). Bug fixed :bug:`1083377` (*Vlad Lesin*).

  ``Rows_read`` was calculated in a way which lead to a negative value being printed in the slow query log. Fixed by making ``Rows_read`` to be a synonym for ``Rows_examined`` in the slow query log. Bug fixed :bug:`830286` (*Alexey Kopytov*).

  Fixed the upstream bug :mysqlbug:`66237`. Temporary files created by binary log cache were not purged after transaction commit. Fixed by truncating the temporary file, if used for a binary log transaction cache, when committing or rolling back a statement or a transaction. Bug fixed :bug:`1070856` (*Alexey Kopytov*).

  Values for ``Rows_sent`` and ``Rows_read`` would be identical in the :ref:`slow_extended_55`. This bug was introduced when slow_extended.patch was ported to |Percona Server| 5.5. Fixed by making ``Rows_read`` identical to ``Rows_examined`` instead. Bug fixed :bug:`721176` (*Alexey Kopytov*).

  Fixed unsigned math error in ``fsp_reserve_free_extents`` that in some specific cases would cause the function to believe that billions more extents have been reserved than have actually been reserved. Bug fixed :bug:`1083700` (*George Ormond Lorch III*).

  When :command:`mysqldump` was used with :option:`--innodb-optimize-keys`, it  did not handle composite indexes correctly when verifying if the optimization is applicable with respect to ``AUTO_INCREMENT`` columns. Bug fixed :bug:`1039536` (*Alexey Kopytov*).

  Upstream bug :mysqlbug:`67606` would cause |Percona Server| to crash with segmentation fault when disk quota was reached. Bug fixed :bug:`1079596` (*George Ormond Lorch III*).

  In cases where indexes with ``AUTO_INCREMENT`` columns where correctly detected, :command:`mysqldump` prevented all such keys from optimization, even though it is sufficient to skip just one (e.g. the first one). Bug fixed :bug:`1081003` (*Alexey Kopytov*).

Other bug fixes: bug fixed :bug:`1071986` (*Alexey Kopytov*), bug fixed :bug:`901060` (*Laurynas Biveinis*), bug fixed :bug:`1090596` (*Stewart Smith*), bug fixed :bug:`1087202` (*Vladislav Vaintroub, Laurynas Biveinis*) and bug fixed :bug:`1087218` (*Vladislav Vaintroub, Laurynas Biveinis*).
