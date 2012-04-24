.. _mysqlbinlog_change_db:

==========================================
Ability to change database for mysqlbinlog
==========================================

Sometimes there is a need to take a binary log and apply it to a database with 
a different name than the original name of the database on binlog producer.

If one is using statement-based replication, he can achieve this by grepping
out "USE dbname" statements out of the output of mysqlbinlog(*).  With
row-based replication this is no longer possible, as database name is encoded
within the the BINLOG '....' statement.

This task is about adding an option to mysqlbinlog that would allow to change 
the names of used databases in both RBR and SBR events.

Varible :variable:`rewrite-db` of **mysqlbinlog** utility allows to setup rewriting rule "from->"to".

Version Specific Information
============================

  * 5.1.62-13.3
    Full functionality.

Client Command Line Parameter
=============================

.. variable:: rewrite-db

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: String
     :default: Off


Related Reading
===============

  * `WL #36 <http://askmonty.org/worklog/Server-Sprint/?tid=36>`_

