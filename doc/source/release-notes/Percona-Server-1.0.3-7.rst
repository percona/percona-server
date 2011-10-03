.. rn:: 1.0.3-7

========================
|Percona Server| 1.0.3-7
========================

The release includes following new features:

    * |MySQL| 5.1.37 as a base release

    * speed hack for ``buf_flush_insert_sorted_into_flush_list`` controlled by the new variable ``innodb_fast_recovery``

Fixed bugs
==========

    * MySQL bug `#39793 <http://bugs.mysql.com/39793>`_,  Bugs :bug:`45357`: 5.1.35 crashes with ``Failing assertion: index->type & DICT_CLUSTERED``

    * Bug :bug:`405714` in Percona-XtraDB: ``During page flush it may be enqueued for flush again when innodb_flush_neighbours=0``

    * Bug :bug:`395783` in Percona-XtraDB: ``main.innodb_bug42101 fails on 5.1.36``

    * This fixes also `#45749 <http://bugs.mysql.com/bug.php?id=45749>`_ and `#42101 <http://bugs.mysql.com/bug.php?id=42101>`_
