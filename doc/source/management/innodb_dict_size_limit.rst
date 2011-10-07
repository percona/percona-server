.. _innodb_dict_size_limit_page:

=====================================
 |InnoDB| Data Dictionary Size Limit
=====================================

This feature lets users limit the amount of memory used for |InnoDB| 's data dictionary. It was introduced in release 5.0.77-b13 of |Percona Server| with |XtraDB|.

The data dictionary is |InnoDB| 's internal catalog of tables. |InnoDB| stores the data dictionary on disk, and loads entries into memory while the server is running. This is somewhat analogous to |MySQL| 's table cache, but instead of operating at the server level, it is internal to the |InnoDB| storage engine. This feature permits you to control how |InnoDB| manages the data dictionary in memory, but does not modify on-disk storage.

In standard |InnoDB|, the size of the data dictionary depends on the number and size of tables opened in the server. Once a table is opened, it is never removed from the data dictionary unless you drop the table or you restart the server. In some cases, the data dictionary grows extremely large. If this consumes enough memory, the server will begin to use virtual memory. Use of virtual memory can cause swapping, and swapping can cause severe performance degradation. By providing a way to set an upper limit to the amount of memory the data dictionary can occupy, this feature provides users a way to create a more predictable and controllable situation.

If your data dictionary is taking up more than a gigabyte or so of memory, you may benefit from this feature. A data dictionary of this size normally occurs when you have many tens of thousands of tables. For servers on which tables are accessed little by little over a significant portion of time, memory usage will grow steadily over time, as if there is a memory leak. For servers that access every table fairly soon after being started, memory usage will increase quickly and then stabilize.

If you;re using |Percona Server|, you can determine the actual size of the data dictionary. (See Show Hashed Memory.) However, if you're not using |Percona Server|, you can still make an estimate of the data dictionary's size. (See “Estimating the Data Dictionary Size” below.)

Please note that this variable only sets a soft limit on the memory consumed by the data dictionary. In some cases, memory usage will exceed the limit (see “Implementation Details” below for more).


System Variables
================

The following system variable was introduced by this feature.

.. variable:: innodb_dict_size_limit

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :default: 0
     :range: 0-LONG_MAX
     :unit: Bytes

This variable places a soft upper bound on the amount of memory used by tables in the data dictionary. When the allocated memory exceeds this amount, |InnoDB| tries to remove some unused entries, if possible. The default value of 0 indicates an unlimited amount of memory and results in the same behavior as standard |InnoDB|.


Status Variables
================

The following status variable was introduced by this feature:


.. variable:: innodb_dict_tables

     :vartype: LONG
     :scope: Global

This status variable shows the number of entries in the |InnoDB| data dictionary cache.


Choosing a Good Value
=====================

As a rough guide, a server that is likely to run into problems with an oversized data dictionary is probably a powerful machine with a lot of memory, perhaps 48GB or more. A gigabyte seems like a comfortable upper limit on the data dictionary for such a server, but this is a matter of opinion and you should choose a value that makes sense to you.

You might find it helpful to understand how much memory each table requires in the dictionary. Quick tests on a 32-bit server show that the data dictionary requires a minimum of about 1712 bytes per table, plus 288 bytes per column, and about 570 bytes for each index. The number might be higher on a 64-bit server due to the increased size of pointers.

Please do not rely on these rules of thumb as absolute truth. We do not know an exact formula for the memory consumption, and would appreciate your input if you investigate it more deeply.


Implementation Details
======================

This feature tries to remove the least recently used |InnoDB| tables from the data dictionary. To achieve this, we need to sort entries in the dictionary in a LRU fashion and to know whether the table is used by the server. The first part is provided by an existing LRU algorithm in |InnoDB|. To determine whether the server is using a table, we check the server``s table cache for the second part. If a table is in the table cache, it is considered to be in use by the server, and is kept in the dictionary. If it is not in the table cache, it can be removed from the dictionary.

Unfortunately, the table cache is not always an accurate way to know whether the table is used by |MySQL| or not. Tables that are in the table cache might not really be in use, so if you have a big table cache, the algorithm will only be able to remove some of the items in the dictionary, which means that the memory consumed by the dictionary may exceed the value of ``innodb_dict_size_limit``. This is why we said this variable sets a soft limit on the size of the dictionary, not an absolute limit.

Estimating the Data Dictionary Size
-----------------------------------

|Percona Server| provides instrumentation to show the data dictionary size directly, but if you``re not using |Percona Server|, you can estimate the size of the data dictionary. By calculating how much memory |InnoDB| has allocated that is not attributable to the buffer pool, etc., you will have an idea of how much allocated memory is not accounted for. This will not be the exact size of the data dictionary, but it will be a reasonable estimate.

To make this estimate, first locate the following lines in the output of ``SHOW INNODB STATUS``: ::

  ----------------------
  BUFFER POOL AND MEMORY
  ----------------------
  Total memory allocated 13563931762; in additional pool allocated 1048576
  Buffer pool size   524288

The line beginning ``Total memory allocated`` shows the total memory |InnoDB| has allocated. The next line shows the buffer pool size in pages; you can either multiply that by the page size to get a value in bytes, or determine the size of the |InnoDB| buffer pool by executing the command ``SHOW VARIABLES LIKE 'innodb_buffer_pool_size'`` . The latter is easier, if you use |MySQL| 5.0 or later, so for the purpose of this example, assume we use that and it gives the value 8589934592. Finally, subtract the |InnoDB| buffer pool size from the total memory allocated: ::

  13563931762 - 8589934592 = 4973997170

So, there is a little over 4.6 GB of memory that |InnoDB| has allocated and which is unaccounted for. This is a pretty large amount of extra memory usage; quite a bit more than the gigabyte or so suggested as a maximum. So, you may benefit from using this feature.


Other reading
=============

  * `Limiting InnoDB data dictionary <http://www.mysqlperformanceblog.com/2009/02/11/limiting-innodb-data-dictionary/>`_

  * `How much memory InnoDB dictionary can take <http://www.mysqlperformanceblog.com/2010/05/06/how-much-memory-innodb-dictionary-can-take/>`_
