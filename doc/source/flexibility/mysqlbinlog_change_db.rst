.. _mysqlbinlog_change_db:

==========================================
Ability to change database for mysqlbinlog
==========================================

Sometimes there is a need to take a binary log and apply it to a database with 
a different name than the original name of the database on binlog producer.

New option rewrite-db has been added to the mysqlbinlog utility that allows the changing names of the used databases in both Row-Based and Statement-Based replication. This was possible before by using tools like grep, awk and sed but only for SBR, because with RBR database name is encoded within the BINLOG ‘....’ statement.

Option :option:`rewrite-db` of **mysqlbinlog** utility allows to setup rewriting rule "from->"to".

Example
=======

**mysqlbinlog** output before rewrite-db :: 

 $ mysqlbinlog mysql-bin.000005
 ...
 # at 175
 #120517 13:10:00 server id 2  end_log_pos 203   Intvar
 SET INSERT_ID=4083/*!*/;
 # at 203
 #120517 13:10:00 server id 2  end_log_pos 367   Query   thread_id=88    exec_time=0     error_code=0
 use world/*!*/;
 SET TIMESTAMP=1337253000/*!*/;
 insert into City (Name, CountryCode, District, Population) values ("New City", "ZMB", "TEX", 111000)
 /*!*/;
 # at 367
 #120517 13:10:00 server id 2  end_log_pos 394   Xid = 1414
 COMMIT/*!*/;
 DELIMITER ;

**mysqlbinlog** output when the new option is used:  :: 

 $ mysqlbinlog --rewrite-db='world->new_world' mysql-bin.000005
 ...
 # at 106
 use new_world/*!*/;
 #120517 13:10:00 server id 2  end_log_pos 175   Query   thread_id=88    exec_time=0     error_code=0
 SET TIMESTAMP=1337253000/*!*/;
 SET @@session.pseudo_thread_id=88/*!*/;
 SET @@session.foreign_key_checks=1, @@session.sql_auto_is_null=1, @@session.unique_checks=1, @@session.autocommit=1/*!*/;
 SET @@session.sql_mode=0/*!*/;
 SET @@session.auto_increment_increment=1, @@session.auto_increment_offset=1/*!*/;
 /*!\C latin1 *//*!*/;
 SET @@session.character_set_client=8,@@session.collation_connection=8,@@session.collation_server=8/*!*/;
 SET @@session.lc_time_names=0/*!*/;
 SET @@session.collation_database=DEFAULT/*!*/;
 BEGIN
 /*!*/;
 # at 175
 #120517 13:10:00 server id 2  end_log_pos 203   Intvar
 SET INSERT_ID=4083/*!*/;
 # at 203
 #120517 13:10:00 server id 2  end_log_pos 367   Query   thread_id=88    exec_time=0     error_code=0
 SET TIMESTAMP=1337253000/*!*/;
 insert into City (Name, CountryCode, District, Population) values ("New City", "ZMB", "TEX", 111000)
 /*!*/;
 # at 367
 #120517 13:10:00 server id 2  end_log_pos 394   Xid = 1414
 COMMIT/*!*/;


Version Specific Information
============================

  * :rn:`5.6.16-64.0`
    Full functionality.

Client Command Line Parameter
=============================

.. option:: rewrite-db

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: String
     :default: Off


Related Reading
===============

  * `WL #36 <http://askmonty.org/worklog/Server-Sprint/?tid=36>`_

