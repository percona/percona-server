.. _innodb_insert_buffer: 

==========================
Configurable Insert Buffer
==========================

Percona has implemented several changes related to MySQL's InnoDB Insert Buffer. These features enable adjusting the insert buffer to the different workloads and hardware configurations.

System variables:
=================

.. variable:: innodb_ibuf_active_contract

   :version 5.5.8-20.0: Introduced
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Numeric
   :default:  1
   :range: 0 - 1

This variable specifies whether the insert buffer can be processed before it reaches its maximum size. The following values are allowed:

  * 0:
    the insert buffer is not processed until it is full. This is the standard |InnoDB| behavior.

  * 1:
    the insert buffer can be processed even it is not full.

.. variable:: innodb_ibuf_accel_rate

   :version 5.5.8-20.0: Introduced
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :default: 100
   :range: 100 - 999999999

This variable allows better control of the background thread processing the insert buffer. Each time the thread is called, its activity is altered by the value of both ``innodb_io_capacity`` and ``innodb_ibuf_merge_rate`` this way: ::

  [real activity] = [default activity] * (innodb_io_capacity/100) * (innodb_ibuf_merge_rate/100)

By increasing the value of ``innodb_ibuf_merge_rate``, you will increase the insert buffer activity.

.. variable:: innodb_ibuf_max_size

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Numeric
     :default: Half the size of the |InnoDB| buffer pool
     :range: 0 - Half the size of the |InnoDB| buffer pool
     :unit: Bytes

This variable specifies the maximum size of the insert buffer. By default the insert buffer is half the size of the buffer pool so if you have a very large buffer pool, the insert buffer will be very large too and you may want to restrict its size with this variable.

Setting this variable to 0 is equivalent to disabling the insert buffer. But then all changes to secondary indexes will be performed synchronously which will probably cause performance degradation. Likewise a too small value can hurt performance.

If you have very fast storage (ie storage with RAM-level speed, not just a RAID with fast disks), a value of a few MB may be the best choice for maximum performance.

Other Reading
=============

* `Some little known facts about InnoDB Insert Buffer <http://www.mysqlperformanceblog.com/2009/01/13/some-little-known-facts-about-innodb-insert-buffer/>`_
* `5.0.75-build12 Percona binaries <http://www.mysqlperformanceblog.com/2009/01/23/5075-build12-percona-binaries/>`_
