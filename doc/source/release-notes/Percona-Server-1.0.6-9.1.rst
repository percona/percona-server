.. rn:1.0.6-9-1

==========================
|Percona Server| 1.0.6-9.1
==========================

New features in version 9.1
===========================
  * |MySQL| 5.1.43 is taken as the basis

  * packages name changed to ``Percona-XtraDB``

  * Enabled support of SSL

  * Enabled profiling

  * Added script to sort LRU dump

  * New supported platforts are added. The full list includes:

    * CentOS 5 (x86_64 and i386)

    * CenOS 4 (x86_64 and i386)

    * Debian lenny (x86_64 and i386)

    * Debian etch (x86_64 and i386)

    * Ubuntu  Jaunty (x86_64 and i386)

    * Ubuntu Intrepid (x86_64 and i386)

    * Ubuntu Hardy (x86_64 and i386)

    * FreeBSD 8 (x86_64 and i386)

    * OpenSolaris (x86_64)


Fixed bugs
==========

  * Bug :bug:`506894`: ``buf_flush_LRU_recommendation()`` is too optimistic

  * Fixed |MySQL|-tests:

    * mysqk

    * mysql_upgrade

    * ssl tests

    * enabled ``rpl_killed_ddl`` and ``innodb-autoinc`` tests

