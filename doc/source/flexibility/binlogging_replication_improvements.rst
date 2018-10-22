.. _binlogging_replication_improvements:

=======================================
Binlogging and replication improvements
=======================================

Due to continuous development, |Percona Server| incorporated a number of
improvements related to replication and binary logs handling. This resulted in
replication specifics, which distinguishes it from |MySQL|.

Safety of statements with a ``LIMIT`` clause
============================================

Summary of the Fix
*******************

|MySQL| considers all ``UPDATE/DELETE/INSERT ... SELECT`` statements with
``LIMIT`` clause to be unsafe, no matter wether they are really producing
non-deterministic result or not, and switches from statement-based logging
to row-based one. |Percona Server| is more accurate, it acknowledges such
instructions as safe when they include ``ORDER BY PK`` or ``WHERE``
condition. This fix has been ported from the upstream bug report
:mysqlbug:`42415` (:psbug:`44`).

Performance improvement on relay log position update
====================================================

Summary of the Fix
*******************

|MySQL| always updated relay log position in multi-source replications setups
regardless of whether the committed transaction has already been executed or
not. Percona Server omitts relay log position updates for the already logged
GTIDs.

Details
*******

Particularly, such unconditional relay log position updates caused additional
fsync operations in case of ``relay-log-info-repository=TABLE``, and with the
higher number of channels transmitting such duplicate (already executed)
transactions the situation became proportionally worse. Bug fixed :psbug:`1786`
(upstream :mysqlbug:`85141`).

Performance improvement on master and connection status updates
===============================================================

Summary of the Fix
*******************

Slave nodes configured to update master status and connection information
only on log file rotation did not experience the expected reduction in load.
|MySQL| was additionaly updating this information in case of multi-source
replication when slave had to skip the already executed GTID event.

Details
*******

The configuration with ``master_info_repository=TABLE`` and
``sync_master_info=0`` makes slave to update master status and connection
information in this table on log file rotation and not after each
sync_master_info event, but it didn't work on multi-source replication setups.
Heartbeats sent to the slave to skip GTID events which it had already executed
previously, were evaluated as relay log rotation events and reacted with
``mysql.slave_master_info`` table sync. This inaccuracy could produce huge (up
to 5 times on some setups) increase in write load on the slave, before this
problem was fixed in |Percona Server|. Bug fixed :psbug:`1812` (upstream
:mysqlbug:`85158`).




