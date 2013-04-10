.. _sql_no_fcache:

==============================
Prevent Caching to FlashCache
==============================

`FlashCache <https://github.com/facebook/flashcache/blob/master/doc/flashcache-doc.txt>`_ increases performance by caching data on SSDs. It works even better when only hot data is cached. This feature prevents the caching of the unwanted blocks of data.

Better utilization of *FlashCache* partitions is achieved when caching of rarely used data is avoided. Use of this feature prevents blocks of data from being cached to *FlashCache* during a query.

Usage of the feature is as follows: ::

  SELECT /* sql_no_fcache */ ... 

The :command:`mysqldump` binary was changed to use this option.


Version-Specific Information
============================

  * :rn:`5.1.49-rel12.0`:
    Full functionality available.

  * :rn:`5.1.66-14.1`:
    Variable :variable:`have_flashcache` introduced.

System Variables
================
.. variable:: have_flashcache

   :version 5.1.66-14.1: Variable introduced
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: No
   :vartype: Boolean
   :range: Yes/No

This variable shows if the server was compiled with Flashcache support.

Status Variables
================
.. variable:: Flashcache_enabled

   :scope: Global
   :vartype: Boolean
   :range: OFF/ON

This status variable shows if the Flashcache support has been enabled.

Other Information
=================

The feature is a port of the original *Facebook* change.

Other reading
=============

  * `Releasing Flashcache <http://www.facebook.com/note.php?note_id=388112370932>`_

  * `Level 2 Flash cache is there <http://www.mysqlperformanceblog.com/2010/04/27/level-2-flash-cache-is-there/>`_
 
