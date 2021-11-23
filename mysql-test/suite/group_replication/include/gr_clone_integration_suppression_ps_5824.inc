# MySQL 8.0.27 sets the default `replica-parallel-workers` to 4,
# which causes some errors due to Percona PS-5824

call mtr.add_suppression("The slave coordinator and worker threads are stopped, possibly leaving data in inconsistent state");
call mtr.add_suppression("The applier thread execution was aborted. Unable to process more transactions, this member will now leave the group.");
call mtr.add_suppression("Fatal error during execution on the Applier process of Group Replication. The server will now leave the group.");
call mtr.add_suppression("Skipping leave operation: concurrent attempt to leave the group is on-going.");
call mtr.add_suppression("Skipping leave operation: member already left the group.");
call mtr.add_suppression("The server was automatically set into read only mode after an error was detected.");
call mtr.add_suppression("Error, maximum number of retries exceeded when waiting for the internal server session state to be operating");
call mtr.add_suppression("Failed to establish an internal server connection to execute plugin operations");
