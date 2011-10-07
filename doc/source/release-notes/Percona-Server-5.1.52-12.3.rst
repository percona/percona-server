.. rn:: 5.1.52-12.3

============================
|Percona Server| 5.1.52-12.3
============================

Released on December 13, 2010 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.52-12.3/>`_ and from the `Release Candidates Repository <http://www.percona.com/docs/wiki/release:start#percona_release_candidates_repository>`_.)


Functionality Added or Changed
==============================

  * |Percona Server| 5.1.52-12.3 is now based on |MySQL| 5.1.52.

  * New Features Added:

    * :ref:`HandlerSocket plugin <handlersocket_page>` support has been added to all ``RPM`` and Debian packages. This is an experimental feature. (*Inada Naoki*, `DeNA Co., Ltd. <http://www.dena.jp/en/index.html>`_)

    * :ref:`show_slave_status_nolock` - Allows examination of slave status even when a lock prevents ``SHOW SLAVE STATUS`` from operating. (*Oleg Tsarev*)

    * :ref:`innodb_fast_shutdown` - Sleeping threads could cause a delay of up to 10 seconds during |InnoDB| shutdown. The sleeping state of threads was made interruptible to avoid this. (contributed by *Kristian Nielsen*) (*Alexey Kopytov*)

  * Other Changes: None

Bugs Fixed
==========

  * Bug :bug:`640576` - The false error "innodb_extra_undoslots option is disabled but it was enabled before" was sometimes reported after upgrading from |MySQL| 5.0. (*Yasufumi Kinoshita*)

  * Bug :bug:`643463` - Shutting down |XtraDB| could take up to 10 seconds, even when there was no actual work to do. (Fix contributed by *Kristian Nielsen*) (*Alexey Kopytov*)

  * Bug :bug:`663757` - On the FreeBSD platform, the "gcc atomic built-in" function isn't provided, so ``response_time_distribution`` now uses the native atomic API on FreeBSD. (*Oleg Tsarev*)

  * Bug :bug:`673426` - Use of some system variables as command-line options caused a crash or undefined behavior. (*Oleg Tsarev*)

  * Bug :bug:`673562` - Debug build was broken due a to failing compile-time assertion in ``mysqld.cc``. (*Alexey Kopytov*)

  * Bug :bug:`673567` - Compiler could produce spurious warnings when building on non-Linux platforms. A check is now made to see if ``clock_gettime()`` is present in ``librt`` at the configure stage. If yes, `` -lrt`` is added to ``LIBS``. (*Alexey Kopytov*)

  * Bug :bug:`673929` - Query cache misses were being reported for some queries when hits were actually occurring. (*Oleg Tsarev*)

  * Bug :bug:`676146` - The development environment test of ``log_slow_verbosity=innodb`` on a slave for row-based replication was not working correctly. (*Oleg Tsarev*)

  * Bug :bug:`676147` - The development environment test of option ``log_slow_slave_statements`` for row-based replication was not working correctly. (*Oleg Tsarev*)

  * Bug :bug:`676148` - Similar to Bug :bug:`676147`. A check is now made for the replication type to test. (*Oleg Tsarev*)
