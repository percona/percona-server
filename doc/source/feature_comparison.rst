=====================================
 |Percona Server| Feature Comparison
=====================================

|Percona Server| is an enhanced drop-in replacement for |MySQL|. With |Percona Server|,

  * Your queries will run faster and more consistently.

  * You will consolidate servers on powerful hardware.

  * You will delay sharding, or avoid it entirely.

  * You will save money on hosting fees and power.

  * You will spend less time tuning and administering.

  * You will achieve higher uptime.

  * You will troubleshoot without guesswork.

We provide these benefits by significantly enhancing |Percona Server| as compared to the standard |MySQL| database server:

.. raw:: html

   <table class="datatable">
   <tr><th class="label">Feature</th><th>Percona Server 5.5.8</th><th>MySQL 5.5.8</th></tr>  
   <tr><td class="label">Open source</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">ACID Compliance</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Multi-Version Concurrency Control</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Row-Level Locking</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Automatic Crash Recovery</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Table Partitioning</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Views</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Subqueries</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Triggers</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Stored Procedures</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Foreign Keys</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><th class="label">Extra Features for Developers</th><th>Percona Server 5.5.8</th><th>MySQL 5.5.8</th></tr>
   <tr><td class="label">NoSQL Socket-Level Interface</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Extra Hash/Digest Functions</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><th class="label">Extra Diagnostic Features</th><th>Percona Server 5.5.8</th><th>MySQL 5.5.8</th></tr>
   <tr><td class="label">PERFORMANCE_SCHEMA Diagnostics Tables</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">INFORMATION_SCHEMA Tables</td><td>60</td><td>37</td></tr>
   <tr><td class="label">Global Performance and Status Counters</td><td>366</td><td>308</td></tr>
   <tr><td class="label">Per-Table Performance Counters</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Per-Index Performance Counters</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Per-User Performance Counters</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Per-Client Performance Counters</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">High-Resolution Process List Timing</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Detailed Query Execution and Plan Log</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Global Query Response Time Statistics</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">InnoDB Data Dictionary as I_S Tables</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Access to InnoDB Data Statistics</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Enhanced SHOW INNODB STATUS</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Enhanced Mutex Diagnostics</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><th class="label">Durability and Reliability Enhancements</th><th>Percona Server 5.5.8</th><th>MySQL 5.5.8</th></tr>
   <tr><td class="label">Transactional Replication State</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Handles Corrupted Tables Gracefully</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><th class="label">Performance &amp; Scalability Enhancements</th><th>Percona Server 5.5.8</th><th>MySQL 5.5.8</th></tr>
   <tr><td class="label">Support for &gt; 1024 Concurrent Transactions</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Support for Multiple I/O Threads</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Dedicated Purge Threads</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td></tr>
   <tr><td class="label">Self-Tuning Checkpoint Algorithm</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Fine-Grained Mutex Locking</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Lock-Free Algorithms</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Partitioned Adaptive Hash Search</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Separate Doublewrite File</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Fast Checksum Algorithm</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Buffer Pool Pre-Load</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Fast Shut-Down</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Support for FlashCache</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Read-Ahead Improvements</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>  
   <tr><th class="label">Extra Features for DBA/Operations Staff</th><th>Percona Server 5.5.8</th><th>MySQL 5.5.8</th></tr>
   <tr><td class="label">Configurable Page Sizes</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Shared-Memory Buffer Pool</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Import Tables From Different Servers</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Configurable Data Dictionary Size</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Configurable Insert Buffer Size</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Active Change Buffer Purging</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Error/Warning Logging Enhancements</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Configurable Fast Index Creation</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   <tr><td class="label">Fast Index Renaming</td><td><img src="http://s0.percona.com/check-yes.png" alt="Yes" width="24" height="24" /></td><td></td></tr>
   </table>

