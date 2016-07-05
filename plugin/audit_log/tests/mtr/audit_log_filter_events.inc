--source include/count_sessions.inc

connect (test,127.0.0.1,user1,password1,,$MASTER_PORT,);
connection test;
SELECT 'user1';
disconnect test;
connection default;
--source include/wait_until_count_sessions.inc

connect (test,localhost,user22,password1,,);
connection test;
SELECT 'user22';
disconnect test;
connection default;
--source include/wait_until_count_sessions.inc

connect (test,localhost,22user,password1,,);
connection test;
SELECT '22user';
change_user user22,password1;
SELECT 'user22';
disconnect test;
connection default;
--source include/wait_until_count_sessions.inc

connect (test,127.0.0.1,admin,password1,,$MASTER_PORT,);
connection test;
SELECT 'admin';
disconnect test;
connection default;
--source include/wait_until_count_sessions.inc

connect (test,localhost,"us,er1",password1,,);
connection test;
SELECT 'us,er1';
disconnect test;
connection default;
--source include/wait_until_count_sessions.inc

connection default;

connection default;
--source include/wait_until_count_sessions.inc
