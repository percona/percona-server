.. _innodb_recovery_patches: 

================================
 |InnoDB| Recovery Stats
================================

When the variable :variable:`innodb_recovery_stats` is enabled and XtraDB has to do recovery on startup, server will write detailed recovery statistics information to the error log. This info will be written after the recovery process is finished.

Example of output statistics for recovery process:  ::

  -------------------
  RECOVERY STATISTICS
  -------------------
  Recovery time: 18 sec. (1 turns)
  
  Data page IO statistics
    Requested pages: 9126
    Read pages:      9126
    Written pages:   7957
    (Dirty blocks):  1156
    Grouping IO [times]:
          number of pages,
                  read request neighbors (in 32 pages chunk),
                          combined read IO,
                                  combined write IO
            1,    32,     335,    548
            2,    0,      121,    97
            3,    7,      49,     44
            4,    4,      43,     26
  ....
           64,    0,      2,      25
  
  Recovery process statistics
    Checked pages by doublewrite buffer: 128
    Overwritten pages from doublewrite:  0
    Recovered pages by io_thread:        9145
    Recovered pages by main thread:      0
    Parsed log records to apply:         2572491
              Sum of the length:         71274689
    Applied log records:                 2376356
              Sum of the length:         68098300
    Pages which are already new enough:  93
    Oldest page's LSN:                   926917970
    Newest page's LSN:                   1526578232


System Variables
================

.. variable:: innodb_recovery_stats

     :cli: No
     :conf: Yes
     :dyn: No
     :vartype: BOOL
     :default: FALSE
     :range: TRUE/FALSE



Other reading
=============

  * `How to estimate time it takes InnoDB to recover? <http://www.mysqlperformanceblog.com/2007/05/09/how-to-estimate-time-it-takes-innodb-to-recover/>`_

  * `InnoDB recovery - is large buffer pool always better? <http://www.mysqlperformanceblog.com/2007/07/17/innodb-recovery-is-large-buffer-pool-always-better/>`_

  * `What is the longest part of InnoDB recovery process? <http://www.mysqlperformanceblog.com/2007/12/20/what-is-the-longest-part-of-innodb-recovery-process/>`_

  * `Improving InnoDB recovery time <http://www.mysqlperformanceblog.com/2009/07/07/improving-innodb-recovery-time/>`_

  * `How long is recovery from 8G innodb_log_file <http://www.mysqlperformanceblog.com/2010/12/22/how-long-is-recovery-from-8g-innodb_log_file/>`_
