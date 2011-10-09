.. rn:: 5.5.11-20.2

==============================
 |Percona Server| 5.5.11-20.2
==============================

Released on April 28, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/LATEST/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_. An `experimental build for MacOS is available <http://www.percona.com/downloads/TESTING/Percona-Server-55/Percona-Server-5.5.11-20.2/release-5.5.11-20.2/114/MacOSX/binary/>`_.)

|Percona Server| 5.5.11-20.2 is a stable release.

New Features
============

  * ``HandlerSocket``, a ``NoSQL`` plugin for |MySQL|, has been updated to the latest stable version as April 11th, 2011.

  * |InnoDB| :ref:`innodb_fast_index_creation` now works with :command:`mysqldump`, ``ALTER TABLE`` and ``OPTIMIZE TABLE``. (*Alexey Kopytov*)

Variable Changes
================

Variable :variable:`innodb_extra_rsegments` was removed because the equivalent, :variable:`innodb_rollback_segments`, has been implemented in |MySQL| 5.5. (*Yasufumi Kinoshita*)

Bug Fixes
=========

  * Bug :bug:`757749` - Using ``ALTER TABLE`` to convert an |InnoDB| table to a ``MEMORY`` table could fail due to a bug in the :ref:`innodb_fast_index_creation` patch. (Alexey Kopytov)

  * Bug :bug:`764395` - |InnoDB| crashed with an assertion failure when receiving a signal on ``pwrite()``. The problem was that |InnoDB| I/O code was not fully conforming to the standard on ``POSIX`` systems. Calls to ``fsync()``, ``pread()``, and ``pwrite()`` can be interrupted by a signal. In such cases, |InnoDB| would crash with an assertion failure, rather than just restarting the interrupted call. (*Alexey Kopytov*)

  * Bug :bug:`766236` - A crash was occurring in some cases when ``innodb_lazy_drop_table`` was enabled with very large buffer pools. (*Yasufumi Kinoshita*)

  * Bug :bug:`733317` - ``SYS_STATS`` internal table of |XtraDB| has been expanded for supporting ``innodb_stats_method`` from |InnoDB| -plugin. (*Yasufumi Kinoshita*)

Known Bugs
==========

The version of Percona |XtraDB| shown in logs is not correct. The actual version is 1.1.6-20.2 (instead of 1.1.6-20.1).
