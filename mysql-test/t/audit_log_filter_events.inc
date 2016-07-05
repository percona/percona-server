SET GLOBAL audit_log_flush=ON;

connect (test,127.0.0.1,user1,password1,,$MASTER_PORT,);
connection test;
SELECT 'user1';
disconnect test;
--source include/wait_until_disconnected.inc

connect (test,localhost,user22,password1,,);
connection test;
SELECT 'user22';
disconnect test;
--source include/wait_until_disconnected.inc

connect (test,localhost,22user,password1,,);
connection test;
SELECT '22user';
change_user user22,password1;
SELECT 'user22';
disconnect test;
--source include/wait_until_disconnected.inc

connect (test,127.0.0.1,admin,password1,,$MASTER_PORT,);
connection test;
SELECT 'admin';
disconnect test;
--source include/wait_until_disconnected.inc

connect (test,localhost,"us,er1",password1,,);
connection test;
SELECT 'us,er1';
disconnect test;
--source include/wait_until_disconnected.inc

connection default;
SET GLOBAL audit_log_flush=ON;
