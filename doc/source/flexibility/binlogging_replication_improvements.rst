.. _binlogging_replication_improvements:

=======================================
Binlogging and replication improvements
=======================================

Due to continuous development, |Percona Server| incorporated a number of
improvements related to replication and binary logs handling. This resulted in
replication specifics, which distinguishes it from |MySQL|.

Temporary tables and mixed logging format
=========================================

Summary of the fix:
*******************

As soon as some statement involving temporary table was met when using mixed
binlog format, |MySQL| was switching to row-based logging of all statements the
end of the session or until all temporary tables used in this session are
dropped. It is inconvenient in case of long lasting connections, including
replication-related ones. |Percona Server| fixes the situation by switching
between statement-based and row-based logging as and when necessary.

Version Specific Information
****************************

  * :rn:`5.5.41-37.0`
    Fix backported from |Percona Server| 5.6.21-70.1

Details:
********

Mixed binary logging format supported by |Percona Server| means that
server runs in statement-based logging by default, but switches to row-based
logging when replication would be unpredictable - in the case of a
nondeterministic SQL statement that may cause data divergence if reproduced on
a slave server. The switch is done upon any condition from the long list, and
one of these conditions is the use of temporary tables.

Temporary tables are **never** logged using row-based format, but any
statement, that touches a temporary table, is logged in row mode. This way all
the side effects that temporary tables may produce on non-temporary ones are
intercepted.

There is no need to use row logging format for any other statements solely
because of the temp table presence. However |MySQL| was undertaking such an
excessive precaution: once some statement with temporary table had appeared and
the row-based logging was used, |MySQL| logged unconditionally all
subsequent statements in row format.

Percona Server have implemented more accurate behavior: instead of switching to
row-based logging until the last temporary table is closed, the usual rules of
row vs statement format apply, and presence of currently opened temporary
tables is no longer considered. This change was introduced with the fix of a
bug :psbug:`151` (upstream :mysqlbug:`72475`).

Safety of statements with a ``LIMIT`` clause
============================================

Summary of the fix:
*******************

|MySQL| considers all ``UPDATE/DELETE/INSERT ... SELECT`` statements with
``LIMIT`` clause to be unsafe, no matter wether they are really producing
non-deterministic result or not, and switches from statement-based logging
to row-based one. |Percona Server| is more accurate, it acknowledges such
instructions as safe when they include ``ORDER BY PK`` or ``WHERE``
condition. This fix has been ported from the upstream bug report
:mysqlbug:`42415` (:psbug:`44`).

Version Specific Information
****************************

  * :rn:`5.5.41-37.0`
    Fix implemented in |Percona Server| 5.5



