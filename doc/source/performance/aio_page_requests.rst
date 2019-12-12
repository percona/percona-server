.. _aio_page_requests:

=======================================
Multiple page asynchronous I/O requests
=======================================

|InnoDB| supports simulated asynchronous I/O (``aio``) or the Linux Native
asynchronous I/O subsystem. 

The |InnoDB| simulated system handles each aio request sequentially
when the trigger is set to Logical Read Ahead (LRA). The |InnoDB| unit
size is 16KB, the InnoDB page size, which makes this operation less
efficient than a larger aio unit.

By default, |InnoDB| uses the Linux Native subsystem, which buffers the
aio requests and then
submits the requests at one time to improve performance.

`On a HDD RAID 1+0 environment
<http://yoshinorimatsunobu.blogspot.hr/2013/10/making-full-table-scan-10x-faster-in.html>`_,
more than 1000MB/s disk reads can be achieved by submitting 64 consecutive pages
requests at once, while only
160MB/s disk reads is shown by submitting single page request.

Version Specific Information
============================

 * :rn:`5.6.38-83.0`- Feature ported from the *Facebook MySQL* patch.

Status Variables
================

.. variable:: Innodb_buffered_aio_submitted

   :version 5.6.38-83.0: Implemented
   :vartype: Numeric
   :scope: Global

This variable shows the number of asynchronous I/O requests buffered in
the Linux subsystem before submission.

The following example of a variable call:

.. code-block:: mysql

   mysql> SHOW GLOBAL STATUS like "innodb_buffered_aio_submitted";

   innodb_buffered_aio_submitted 12493

   
Other Reading
=============

 * `Making full table scan 10x faster in InnoDB
   <http://yoshinorimatsunobu.blogspot.hr/2013/10/making-full-table-scan-10x-faster-in.html>`_

 * `Bug #68659	InnoDB Linux native aio should submit more i/o requests at once
   <https://bugs.mysql.com/bug.php?id=68659>`_
