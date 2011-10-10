.. rn:: 5.5.13-20.4

============================
|Percona Server| 5.5.13-20.4
============================

|Percona| is glad to announce the release of |Percona Server| 5.5.13-20.4 on July 1, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.13-20.4/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on |MySQL| 5.5.13, |Percona Server| |Percona Server| 5.5.13-20.4 is now the current stable release in the 5.5 series. All of |Percona| 's software is open-source and free, all the details of the release can be found in the `5.5.13-20.4 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.13-20.4>`_.

Improvements
============

SHM Buffer Pool has been replaced with LRU Dump/Restore
-------------------------------------------------------

The :ref:`SHM buffer pool <innodb_buffer_pool_shm>` patch, which provided the ability to use a shared memory segment for the buffer pool to enable faster server restarts, has been removed. Instead, we recommend using the :ref:`LRU Dump/Restore <innodb_lru_dump_restore>` patch which provides similar improvements in restart performance.

Replacement is due to ``SHM`` buffer pool both being very invasive and not widely used. Improved restart times are better provided by the much safer ``LRU D/R`` patch which has the advantage of also persisting across machine restarts.

The configuration variables for :file:`my.cnf` have been kept for compatibility and warnings will be printed for the deprecated options (:variable:`innodb_buffer_pool_shm_key` and :variable:`innodb_buffer_pool_shm_checksum`) if used. 

Instructions for disabling the ``SHM`` buffer pool can be found :ref:`here <innodb_buffer_pool_shm>`.

Instructions on setting up ``LRU`` dump/restore can be found :ref:`here <innodb_lru_dump_restore>`.

Bug Fixes
=========

  * On a high concurrency environment with compressed tables, users may experience crashes due to improper mutex handling in ``buf_page_get_zip()``. Bug Fix: :bug:`802348` (*Yasufumi Kinoshita*).

  * |XtraDB| crashed when importing big tables (e.g. 350G) using the :ref:`Expand Table Import <innodb_expand_import_page>` feature due to a timeout. Bug Fix: :bug:`684829` (*Yasufumi Kinoshita*).

  * Partitioning adaptive hash index may leave to a hangup of the server in some scenarios. Bug Fix: :bug:`791030` (*Yasufumi Kinoshita*).

  * Statistics gathering for each record's update. Bug :bug:`791092` (*Yasufumi Kinoshita*)

Other Changes
=============

Improvements and fixes on the |Percona Server| Test Suite
---------------------------------------------------------

 * :bug:`693415`, :bug:`794840`, :bug:`800035`, :bug:`800559`, :bug:`782391`, :bug:`785566`, :bug:`790199` (*Oleg Tsarev*, *Yasufumi Kinoshita*, *Stewart Smith*).

Improvements and fixes on platform-specific distribution:
---------------------------------------------------------

  *  :bug:`737947`, :bug:`764038` (!), :bug:`656933` (*Ignacio Nin*, *Laurynas Biveinis*)
