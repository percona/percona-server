.. rn:: 1.0.3-6

========================
|Percona Server| 1.0.3-6
========================

Released on July 20, 2009.

The release includes following new features:

  * |MySQL| 5.1.36 as a base release

  * :ref:`innodb_recovery_patches`

  * Experimental adaptive checkpoint method estimate

  * :ref:`innodb_stats` - the implementation of the fix for |MySQL| Bug #30423

  * :ref:`innodb_expand_import_page` - Support of import |InnoDB| / |XtraDB| tables from another server

  * New patch to split buffer pool mutex

  * ``g-style-io-thread``: Google's fixes to |InnoDB| IO threads

  * ``innodb_dict_size_limit`` - Limit of internal data dictionary

Fixed bugs
==========

  * |MySQL| Bug: ``#39793 <http://bugs.mysql.com/39793>`_: Foreign keys not constructed when column has a ``#`` in a comment or default value

  * Bug :bug:`395784` in Percona-XtraDB: ``main.innodb_xtradb_bug317074 internal check fails on 5.1.36``

  * Bug :bug:`395778` in Percona-XtraDB: ``main.innodb-analyze internal check failed on 5.1.36``

  * Bug :bug:`391189` in Percona-XtraDB: ``main.innodb_bug36172 mysq test fails with innodb_stat.patch applied``

  * Bug :bug:`388884` in Percona-XtraDB: ``patching fails on 5.1.35``

