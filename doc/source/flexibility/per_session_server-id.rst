.. _per_session_server-id:

=========================
 Per-session server-id
=========================

Variable :variable:`server_id` is a global variable. In multi-master replication setups or for external replication, :variable:`server_id` can be useful as a session variable. In that case a session replaying a binary log from another server would set it to that server's id. That way binary log has the ultimate source server id attached to it no matter how many hosts it passes, and it would provide loop detection for multi-master replication. 

This was implemented by introducing the new :variable:`pseudo_server_id` variable. This variable, when set to non-zero value, will cause all binary log events in that session to have that :variable:`server_id` value. A new variable was introduced instead of converting :variable:`server_id` to have both global and session scope in order to preserve compatibility. 

Version Specific Information
============================

  * :rn:`5.6.26-74.0`
    Feature implemented and :variable:`pseudo_server_id` variable has been introduced

System Variables
================

.. variable:: pseudo_server_id

   :version 5.6.26-74.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :default: 0

When this variable is set to ``0`` (default), it will use the global :variable:`server_id` value. **Note:** this is different from the setting the global :variable:`server_id` to ``0`` which disables replication. Setting this variable to non-zero value will cause binary log events in that session to have it as :variable:`server_id` value. Setting this variable requires ``SUPER`` privileges.

Other Reading
=============

 * `MDEV-500 <https://mariadb.atlassian.net/browse/MDEV-500>`_ -  Session variable for server_id 
 * Upstream bug :mysqlbug:`35125` -  allow the ability to set the server_id for a connection for logging to binary log
