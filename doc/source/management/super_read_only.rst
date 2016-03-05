.. _super-read-only:

============================
 ``super-read-only`` option 
============================

|Percona Server| has ported a new option, :variable:`super_read_only` from *WebScaleSQL*, which is like the `read-only <http://dev.mysql.com/doc/refman/5.6/en/server-system-variables.html#sysvar_read_only>`_ option, but affecting users with ``SUPER`` privileges as well. This means that server will not allow any updates even for the users that have ``SUPER`` privilege. If slave replication repositories are tables (when :variable:`master-info-repository` variable is set to ``TABLE`` and/or :variable:`relay-log-info-repository` variable is set to ``TABLE``), any implicit updates to them (as performed by ``CHANGE MASTER``, ``START SLAVE``, or ``STOP SLAVE``) are allowed regardless of :variable:`super_read_only` setting. Enabling :variable:`super_read_only` implies regular ``read-only`` as well.

Interaction with :variable:`read_only` variable:

 * Turning :variable:`read_only` off also turns off :variable:`super_read_only`.
 * Turning :variable:`super_read_only` on also turns :variable:`read_only` on.
 * All other changes to either one of these have no affect on the other.

System Variables
================

.. variable:: super_read_only

     :version 5.6.21-70.0: Variable ported from *WebScaleSQL*
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Boolean
     :default: OFF

When enabled the server will not allow any updates, beside updating the replication repositories if slave status logs are tables, even for the users that have ``SUPER`` privilege. 

