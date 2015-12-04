.. _multiple_user_level_locks:

==========================================
 Multiple user level locks per connection
==========================================

|Percona Server| supports multiple user level locks per connection as of version :rn:`5.6.19-67.0`. The ``GET_LOCK()`` function in upstream |MySQL| allows a connection to hold at most one user level lock. Taking a new lock automatically releases the old lock, if any.

The limit of one lock per session existed since early versions of |MySQL| didn't have a deadlock detector for SQL locks. `Metadata Locking <http://dev.mysql.com/doc/refman/5.5/en/metadata-locking.html>`_ introduced in |MySQL| 5.5 added a deadlock detector, so starting from 5.5 it became possible to take multiple locks in any order. Deadlock would be detected when occurred, and an error would returned to the client which closed the wait chain.

Version Specific Information
============================
 * :rn:`5.6.19-67.0` - Feature implemented in |Percona Server| 5.6

Other Information
=================

 * Author / Origin:
   Kostja Osipov

Other Reading 
=============
* `MySQL: multiple user level locks per connection <http://kostja-osipov.livejournal.com/46410.html>`_ 
