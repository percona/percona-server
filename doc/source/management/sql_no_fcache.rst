.. _sql_no_fcache:

==============================
Prevent Caching to FlashCache
==============================

*FlashCache* increases performance by caching data in SSDs. It works even better when only hot data is cached. This feature prevents unwanted blocks of data to be cached.

Better utilization of *FlashCache* partitions is achieved when caching of rarely used data is avoided. Use of this feature prevents blocks of data from being cached to *FlashCache* during a query.

Usage of the feature is as follows: ::

  SELECT /* sql_no_fcache */ ... 

The :command:`mysqldump` binary was changed to use this option.


Version-Specific Information
============================

  * 5.1.49-12.0:
    Full functionality available.

Other Information
=================

The feature is a port of the original *Facebook* change.

Other reading
=============

  * `Releasing Flashcache <http://www.facebook.com/note.php?note_id=388112370932>`_

  * `Level 2 Flash cache is there <http://www.mysqlperformanceblog.com/2010/04/27/level-2-flash-cache-is-there/>`_
