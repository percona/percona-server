.. _fast_updates:

==========================
 Fast Updates with TokuDB
==========================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

Introduction
============

Update intensive applications can have their throughput limited by the random
read capacity of the storage system. The cause of the throughput limit is the
read-modify-write algorithm that *MySQL* uses to process update statements
(read a row from the storage engine, apply the updates to it, write the new row
back to the storage engine).

To address this throughput limit, *TokuDB* provides an experimental fast update
feature, which uses a different update algorithm. Update expressions of the SQL
statement are encoded into tiny programs that are stored in an update Fractal
Tree message. This update message is injected into the root of the Fractal Tree
index. Eventually, these update  messages reach a leaf node, where the update
programs are applied to the row. Since messages are moved between Fractal Tree
levels in batches, the cost of reading in the leaf node is amortized over many
update messages.

This feature is available for ``UPDATE`` and ``INSERT`` statements, and can be
turned ON/OFF separately for them with use of two variables. Variable
:ref:`tokudb_enable_fast_update` variable toggles fast updates for the
``UPDATE``, and  :ref:`tokudb_enable_fast_upsert` does the same  for
``INSERT``.

Limitations
===========

Fast updates are activated instead of normal MySQL read-modify-write updates
if the executed expression meets the number of conditions.

* fast updates can be activated for a statement or a mixed replication,

* a primary key must be defined for the involved table,

* both simple and compound primary keys are supported, and ``int``, ``char`` or
  ``varchar`` are the allowed data types for them,

* updated fields should have ``Integer`` or ``char`` data type,

* fields that are part of any key should be not updated,

* clustering keys are not allowed,

* triggers should be not involved,

* supported update expressions should belong to one of the following types:

  * ``x = constant``

  * ``x = x + constant``

  * ``x = x - constant``

  * ``x = if (x=0,0,x-1)``

  * ``x = x + values``

Usage Specifics and Examples
============================

Following example creates a table that associates event identifiers with their
count:

.. code-block:: sql

   CREATE TABLE t (
       event_id bigint unsigned NOT NULL PRIMARY KEY,
       event_count bigint unsigned NOT NULL
   );

Many graph applications that map onto relational tables can use duplicate key
inserts and updates to maintain the graph. For example, one can update the
meta-data associated with a link in the graph using duplicate key insertions.
If the affected rows is not used by the application, then the insertion or
update can be marked and executed as a fast insertion or a fast update.

Insertion example
-----------------

If it is not known if the event identifier (represented by `event_id`) already
exists in the table, then ``INSERT ... ON DUPLICATE KEY UPDATE ...`` statement
can insert it if not existing, or increment its `event_count` otherwise. Here
is an example with duplicate key insertion statement, where ``%id`` is some
specific `event_id` value:

.. code-block:: sql

   INSERT INTO t VALUES (%id, 1)
     ON DUPLICATE KEY UPDATE event_count=event_count+1;

Explanation
***********
If the event id’s are random, then the throughput of this application would be
limited by the random read capacity of the storage system since each ``INSERT``
statement has to determine if this `event_id` exists in the table.

*TokuDB* replaces the primary key existence check with an insertion of an
“upsert” message into the Fractal Tree index. This “upsert” message contains a
copy of the row and a program that increments event_count. As the Fractal Tree
buffer’s get filled, this “upsert” message is flushed down the tree.
Eventually, the message reaches a leaf node and gets executed there.
If the key exists in the leaf node, then the event_count is incremented.
Otherwise, the new row is inserted into the leaf node.

Update example
--------------

If `event_id` is known to exist in the table, then ``UPDATE`` statement can be
used to increment its `event_count` (once again, specific `event_id` value is
written here as ``%id``):

.. code-block:: sql

   UPDATE t SET event_count=event_count+1 
   WHERE event_id=%id;

Explanation
***********

TokuDB generates an “update” message from the ``UPDATE`` statement and its
update expression trees, and inserts this message into the Fractal Tree index.
When the message eventually reaches the leaf node, the increment program is
extracted from the message and executed.
