.. _innodb_dict_size_limit_page:

=====================================
 |InnoDB| Data Dictionary Size Limit
=====================================

This feature lets users limit the amount of memory used for |InnoDB| 's data dictionary. It was introduced in release 5.0.77-b13 of |Percona Server| with |XtraDB|.

The data dictionary is |InnoDB| 's internal catalog of tables. |InnoDB| stores the data dictionary on disk, and loads entries into memory while the server is running. This is somewhat analogous to |MySQL| 's table cache, but instead of operating at the server level, it is internal to the |InnoDB| storage engine. This feature permits you to control how |InnoDB| manages the data dictionary in memory, but does not modify on-disk storage.

In standard |InnoDB|, the size of the data dictionary depends on the number and size of tables opened in the server. Once a table is opened, it is never removed from the data dictionary unless you drop the table or you restart the server. In some cases, the data dictionary grows extremely large. If this consumes enough memory, the server will begin to use virtual memory. Use of virtual memory can cause swapping, and swapping can cause severe performance degradation. By providing a way to set an upper limit to the amount of memory the data dictionary can occupy, this feature provides users a way to create a more predictable and controllable situation.

If your data dictionary is taking up more than a gigabyte or so of memory, you may benefit from this feature. A data dictionary of this size normally occurs when you have many tens of thousands of tables. For servers on which tables are accessed little by little over a significant portion of time, memory usage will grow steadily over time, as if there is a memory leak. For servers that access every table fairly soon after being started, memory usage will increase quickly and then stabilize.

Please note that this variable only sets a soft limit on the memory consumed by the data dictionary. In some cases, memory usage will exceed the limit (see “Implementation Details” below for more).

You can see the actual size of the data dictionary by running the ``SHOW ENGINE INNODB STATUS`` command. Data dictionary size will be shown under ``BUFFER POOL AND MEMORY`` section: :: 
  
 ----------------------
 BUFFER POOL AND MEMORY
 ----------------------
 Total memory allocated 137756672; in additional pool allocated 0
 Total memory allocated by read views 88
 Internal hash tables (constant factor + variable factor)
    Adaptive hash index 2250352 	(2213368 + 36984)
    Page hash           139112 (buffer pool 0 only)
    Dictionary cache    612843 	(554768 + 58075)
    File system         83536 	(82672 + 864)
    Lock system         333248 	(332872 + 376)
    Recovery system     0 	(0 + 0)
 Dictionary memory allocated 58075
 Buffer pool size        8191


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


.. variable:: Innodb_dict_tables

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

Other reading
=============

  * `Limiting InnoDB data dictionary <http://www.mysqlperformanceblog.com/2009/02/11/limiting-innodb-data-dictionary/>`_

  * `How much memory InnoDB dictionary can take <http://www.mysqlperformanceblog.com/2010/05/06/how-much-memory-innodb-dictionary-can-take/>`_
