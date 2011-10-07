.. _innodb_doublewrite_path:

=========================================
 Configuration of the Doublewrite Buffer
=========================================

|InnoDB| and |XtraDB| use a special feature called the doublewrite buffer to provide a strong guarantee against data corruption. The idea is to write the data to a sequential log in the main tablespace before writing to the data files. If a partial page write happens (in other words, a corrupted write), |InnoDB| and |XtraDB| will use the buffer to recover the data. Even if the data is written twice the performance impact is usually small, but in some heavy workloads the doublewrite buffer becomes a bottleneck. Now we have an option to put the buffer on a dedicated disk in order to parallelize I/O activity on the buffer and on the tablespace.

This feature allows you to move the doublewrite buffer from the main tablespace to a separate location.

This option is for advanced users only. See the discussion below to fully understand whether you really need to use it.


Detailed Information
====================

The following discussion will clarify the improvements made possible by this feature.

Goal of the Doublewrite Buffer
------------------------------

|InnoDB| and |XtraDB| use many structures, some on disk and others in memory, to manage data as efficiently as possible. To have an overview of the different components see this post. Let``s now focus on the doublewrite buffer.

|InnoDB| / |XtraDB| uses a reserved area in its main tablespace, called the doublewrite buffer, to prevent data corruption that could occur with partial page writes. When the data in the buffer pool is flushed to disk, |InnoDB| / |XtraDB| will flush whole pages at a time (by default 16KB pages) and not just the records that have changed within a page. It means that, if anything unexpected happens during the write, the page can be partially written leading to corrupt data.

With the doublewrite buffer feature, |InnoDB| / |XtraDB| first writes the page in the doublewrite buffer and then to the data files.

If a partial page write occurs in the data files, |InnoDB| / |XtraDB| will check on recovery if the checksum of the page in the data file is different from the checksum of the page in the doublewrite buffer and thus will know if the page is corrupt or not. If it is corrupt, the recovery process will use the page stored in the doublewrite buffer to restore the correct data.

If a partial write occurs in the doublewrite buffer, the original page is untouched and can be used with the redo logs to recover the data. For further information on the doublewrite buffer, you can see this post.

Performance Impact of the Doublewrite Buffer
--------------------------------------------

In usual workloads the performance impact is low-5% or so. As a consequence, you should always enable the doublewrite buffer because the strong guarantee against data corruption is worth the small performance drop.

But if you experience a heavy workload, especially if your data does not fit in the buffer pool, the writes in the doublewrite buffer will compete against the random reads to access the disk. In this case, you can see a sharp performance drop compared to the same workload without the doublewrite buffer-a 30% performance degradation is not uncommon.

Another case when you can see a big performance impact is when the doublewrite buffer is full. Then new writes must wait until entries in the doublewrite buffer are freed.

What's New with This Feature
----------------------------

In a standard |InnoDB| / |XtraDB| installation, the doublewrite buffer is located in the main tablespace (whether you activate the ``innodb_file_per_table`` or not) and you have no option to control anything about it.

The feature adds an option (``innodb_doublewrite_file``) to have a dedicated location for the doublewrite buffer.

How to Choose a Good Location for the Doublewrite Buffer
--------------------------------------------------------

Basically if you want to improve the I/O activity, you will put the doublewrite buffer on a different disk. But is it better on an SSD or a more traditional HDD? First you should note that pages are written in a circular fashion in the doublewrite buffer and only read on recovery. So the doublewrite buffer performs mostly sequential writes and a few sequential reads. Second HDDs are very good at sequential write if a write cache is enabled, which is not the case of SSDs. Therefore you should choose a fast HDD if you want to see performance benefits from this option. For instance, you could place the redo logs (also written in a sequential way) and the doublewrite buffer on the same disk.

Prior to release 5.1.53-12.4, it was necessary to recreate your database and |InnoDB| system files when a dedicated file to contain the doublewrite buffer was specified. Beginning with release 5.1.53-12.4, you no longer need to do this.


Version Specific Information
============================

  * 5.1.47-11.0	 
    Full functionality available.

  * 5.1.53-12.4
    Rebuild of database and system files no longer necessary.

System Variables
================

The following system variable was introduced.


.. variable:: innodb_doublewrite_file

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: No
   :vartype: STR
   :def: NULL

Use this option to create a dedicated tablespace for the doublewrite buffer.

This option expects a filename which can be specified either with an absolute or a relative path. A relative path is relative to the data directory.


Related Reading
===============

  * `XtraDB / InnoDB internals in drawing <http://www.mysqlperformanceblog.com/2010/04/26/xtradb-innodb-internals-in-drawing/>`_

  * `InnoDB Double Write <http://www.mysqlperformanceblog.com/2006/08/04/innodb-double-write/>`_

  * `SSD and HDD for InnoDB <http://yoshinorimatsunobu.blogspot.com/2009/05/tables-on-ssd-redobinlogsystem.html>`_
