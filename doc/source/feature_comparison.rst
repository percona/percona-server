=====================================
 |Percona Server| Feature Comparison
=====================================

|Percona Server| is an enhanced drop-in replacement for MySQL. With |Percona Server|,

  * Your queries will run faster and more consistently.

  * You will consolidate servers on powerful hardware.

  * You will delay sharding, or avoid it entirely.

  * You will save money on hosting fees and power.

  * You will spend less time tuning and administering.

  * You will achieve higher uptime.

  * You will troubleshoot without guesswork.

We provide these benefits by significantly enhancing |Percona Server| as compared to the standard MySQL database server:

.. tabularcolumns:: |p{5cm}|p{5cm}|p{5cm}|

.. list-table::
    :header-rows: 1

    * - Features
      - Percona Server 5.7.27
      - MySQL 5.7.27
    * - Open source
      - Yes
      - Yes
    * - ACID Compliance
      - Yes
      - Yes
    * - Multi-Version Concurrency Control
      - Yes
      - Yes
    * - Row-Level Locking
      - Yes
      - Yes
    * - Automatic Crash Recovery
      - Yes
      - Yes
    * - Table Partitioning
      - Yes
      - Yes
    * - Views
      - Yes
      - Yes
    * - Subqueries
      - Yes
      - Yes
    * - Triggers
      - Yes
      - Yes
    * - Stored Procedures
      - Yes
      - Yes
    * - Foreign Keys
      - Yes
      - Yes
    * - GTID Replication
      - Yes
      - Yes
    * - Group Replication
      - Yes
      - Yes
    * - MyRocks Storage Engine
      - Yes
      - No
    * - TokuDB Storage Engine
      - Yes
      - No

.. tabularcolumns:: |p{5cm}|p{5cm}|p{5cm}|

.. list-table::
   :header-rows: 1

   * - Extra Features for Developers
     - Percona Server 5.7.27
     - MySQL 5.7.27
   * - NoSQL Socket-Level Interface
     - Yes
     - Yes
   * - X API Support
     - Yes
     - Yes
   * - InnoDB Full-Text Search Improvements
     - Yes
     - No
   * - Extra Hash/Digest Functions
     - Yes
     - No

.. tabularcolumns:: |p{5cm}|p{5cm}|p{5cm}|

.. list-table::
   :header-rows: 1

   * - Instrumentation and Troubleshooting Features
     - Percona Server 5.7.27
     - MySQL 5.7.27
   * - INFORMATION_SCHEMA Tables
     - 71
     - 61
   * - Global Performance and Status Counters
     - 432
     - 357
   * - Per-Table Performance Counters
     - Yes
     - No
   * - Per-Index Performance Counters
     - Yes
     - No
   * - Per-User Performance Counters
     - Yes
     - No
   * - Per-Client Performance Counters
     - Yes
     - No
   * - Per-Thread Performance Counters
     - Yes
     - No
   * - Global Query Reponse Time Statistics
     - Yes
     - No
   * - Enhanced SHOW ENGINE INNODB STATUS
     - Yes
     - No
   * - Undo Segment Information
     - Yes
     - No
   * - Temporary Tables Information
     - Yes
     - No
   * - Extended Slow Query Logging
     - Yes
     - No
   * - User Statistics
     - Yes
     - No

.. tabularcolumns:: |p{5cm}|p{5cm}|p{5cm}|

.. list-table::
   :header-rows: 1

   * - Performance and Scalability Features
     - Percona Server 5.7.27
     - MySQL 5.7.27
   * - Improved scalability by splitting mutexes
     - Yes
     - No
   * - Improved MEMORY Storage Engine
     - Yes
     - No
   * - Improved Flushing
     - Yes
     - No
   * - Parallel Doublewrite Buffer
     - Yes
     - No
   * - Configurable Page Sizes
     - Yes
     - Yes
   * - Configurable Fast Index Creation
     - Yes
     - No
   * - Per-column Compression for VARCHAR/BLOB and JSON
     - Yes
     - No
   * - Compressed columns with Dictionaries
     - Yes
     - No

.. tabularcolumns:: |p{5cm}|p{5cm}|p{5cm}|

.. list-table::
   :header-rows: 1

   * - Security Features
     - Percona Server 5.7.27
     - MySQL 5.7.27
   * - PAM Authentication Plugin
     - Yes
     - Enterprise-Only
   * - Audit Logging Plugin
     - Yes
     - Enterprise-only

.. tabularcolumns:: |p{5cm}|p{5cm}|p{5cm}|

.. list-table::
   :header-rows: 1

   * - Encryption Features
     - Percona Server 5.7.27
     - MySQL 5.7.27
   * - Encrypt InnodDB data
     - Yes
     - Yes
   * - Encrypt InnoDB tablespaces
     - Experimental
     - No
   * - Encrypt InnoDB logs
     - Experimental
     - No
   * - Encrypt Binary logs
     - Experimental
     - No
   * - Encrypt temporary files
     - Experimental
     - No
   * - Key Rotation
     - Experimental
     - No
   * - Scrubbing
     - Experimental
     - No
   * - Enforce Encryption
     - Experimental
     - No
   * - Storing Keyring in a file
     - Yes
     - Yes
   * - Storing Keyring in a Hashicorp Vault
     - Yes
     - No

.. tabularcolumns:: |p{5cm}|p{5cm}|p{5cm}|

.. list-table::
   :header-rows: 1

   * - Operational Improvements
     - Percona Server 5.7.27
     - MySQL 5.7.27
   * - Changed Page Tracking
     - Yes
     - Yes
   * - Threadpool
     - Yes
     - Enterprise-only
   * - Backup Locks
     - Yes
     - No
   * - Extended SHOW GRANTS
     - Yes
     - No
   * - Improved Handling of Corrupted Tables
     - Yes
     - No
   * - Ability to kill Idle Transactions
     - Yes
     - No
   * - Improvements to START TRANSACTION WITH CONSISTENT SNAPSHOT
     - Yes
     - No

.. tabularcolumns:: |p{5cm}|p{5cm}|p{5cm}|

.. list-table::
   :header-rows: 1

   * - Features for Running Database as a Service (DBaaS)
     - Percona Server 5.7.27
     - MySQL 5.7.27
   * - Special Utility User
     - Yes
     - No
   * - Enforce a Specific Storage Engine
     - Yes
     - No
   * - Expanded Program Option Modifiers
     - Yes
     - No

