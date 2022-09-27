.. _tokudb_fractal_tree_indexing:

=============================
TokuDB Fractal Tree Indexing
=============================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

Fractal Tree indexing is the technology behind TokuDB and is
protected by multiple patents. This type of index enhances the tradional B-tree
data structure used in other database engines, and optimizes performance for
modern hardware and data sets.

Background
-----------------

The B-tree data structure was optimized for large blocks of data but the
performance is limited by I/O bandwidth. The size of a production database
generally exceeds available main memory. Most leaves in a tree are stored on
disk, not in RAM. If a leaf is not in main memory inserting information requires
a disk I/O operation. Continually adding RAM to keep pace with data's
growth is too expensive.

Buffers
-----------

Like a B-tree structure, a fractal tree index is a tree data structure, but each
node has buffers that allow messages to be stored. Insertions, deletions, and
updates are inserted into the buffers as messages.
Buffers let each disk operation be more efficient by writing large amounts of
data. Buffers also avoid the common B-tree scenario when disk writes change only
a small amount of data.

In fractal tree indexes, non-leaf (internal) nodes have child nodes. The
number of child nodes is variable and based on a pre-defined range. When data is
inserted or deleted from a node, the number of child nodes changes. Internal nodes may
join or split to maintain the defined range. When the buffer is full, the
mesages are flushed to children nodes.

Fractal tree index data structure involves the same algorithmic complexity as
B-tree queries. There is no data loss because the queries follow the path from
the root to leaf and pass through all messages. A query knows the current state
of data even if changes have not been propagated to the corresponding leaves.

Each message is stamped with a unique message sequence number (MSN) when the
message is stored in a non-leaf node message buffer. The MSN maintains the order
of messages and ensures the messages are only applied once to leaf nodes when
the leaf node is updated by messages.

Buffers are also serialized to disk, messages in internal nodes are not lost in
the case of a crash or outage. If a write happened after a checkpoint, but
before a crash, recovery replays the operation from the log.
