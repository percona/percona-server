.. _innodb_extra_rseg:

============================
 Multiple Rollback Segments
============================

In Percona Server 5.1, an improvement was provided for write-intensive
workloads that allowed multiple rollback segments to be used. It has
been removed from |Percona Server| 5.5.11-20.2 because an equivalent
variable, ``innodb_rollback_segment``, has been implemented in |MySQL|
5.5, and so the MySQL 5.5 implementation is available in Percona
Server 5.5.
