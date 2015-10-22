# Tests basic replication interoperability between tokudb and other storage
# engines, specifically innodb.
# Credit to Sveta Smirnova for the basic idea.
connection slave;
stop slave;
start slave;
connection master;

-- disable_query_log
set sql_log_bin=0;
eval create table numbers(
id int not null auto_increment primary key,
f1 int,
f2 bigint,
f3 tinyint,
f4 smallint,
f5 int unsigned,
f6 bigint unsigned,
f7 tinyint unsigned,
f8 smallint unsigned,
f9 bit(2),
f10 bool,
f11 mediumint,
f12 decimal,
f13 decimal(5,3),
f14 float,
f15 double
) engine=$master_engine;

#Dates
eval create table dates(
id int not null auto_increment primary key,
f1 timestamp,
f2 timestamp,
f3 date,
f4 time,
f5 datetime,
f6 year
) engine=$master_engine;

#Strings
eval create table strings(
id int not null auto_increment primary key,
f1 char(10),
f2 varchar(20),
f3 text,
f4 blob,
f5 binary(10),
f6 varbinary(20),
f7 tinyblob,
f8 tinytext,
f9 mediumblob,
f10 mediumtext,
f11 longblob,
f12 longtext,
f13 enum('Света','Валерий','MySQL'),
f14 set('Света','Валерий','MySQL')
) character set utf8, engine=$master_engine;

-- enable_query_log

show create table numbers;
show create table dates;
show create table strings;

-- disable_query_log

connection slave;
SET NAMES UTF8;

#Numbers
eval create table numbers(
id int not null auto_increment primary key,
f1 int,
f2 bigint,
f3 tinyint,
f4 smallint,
f5 int unsigned,
f6 bigint unsigned,
f7 tinyint unsigned,
f8 smallint unsigned,
f9 bit(2),
f10 bool,
f11 mediumint,
f12 decimal,
f13 decimal(5,3),
f14 float,
f15 double
) engine=$slave_engine;

#Dates
eval create table dates(
id int not null auto_increment primary key,
f1 timestamp,
f2 date,
f3 time,
f4 datetime,
f5 year
)  engine=$slave_engine;

#Strings
eval create table strings(
id int not null auto_increment primary key,
f1 char(10),
f2 varchar(20),
f3 text,
f4 blob,
f5 binary(10),
f6 varbinary(20),
f7 tinyblob,
f8 tinytext,
f9 mediumblob,
f10 mediumtext,
f11 longblob,
f12 longtext,
f13 enum('Света','Валерий','MySQL'),
f14 set('Света','Валерий','MySQL')
) character set utf8, engine=$slave_engine;

-- enable_query_log

show create table numbers;
show create table dates;
show create table strings;

-- disable_query_log

-- enable_query_log

connection master;

#Testing numbers
insert into numbers(f1,f2,f3,f4,f5,f6,f7,f8) values(1,2,3,4,5,6,7,8);
insert into numbers(f1,f2,f3,f4,f5,f6,f7,f8) values(-1,-2,-3,-4,-5,-6,-7,-8);
insert into numbers(f1,f2,f3,f4,f5,f6,f7,f8, f9, f10, f11, f12, f13, f14, f15) values(2147483647, 9223372036854775807, 127, 32767, 4294967295, 18446744073709551615, 255, 65535, b'11', true, 8388607, 1043.4332, 3232.432, 32432.432423, 432.42342);
insert into numbers(f1,f2,f3,f4,f5,f6,f7,f8, f9, f10, f11, f12, f13, f14, f15) values(-2147483648, -9223372036854775808, -128, -32768, -2147483648, -9223372036854775808, -128, -32768, b'00', false, -8388608, -18.43, 32.432, 32432.432423, 432.42342);

#Dates
insert into dates(f1,f2,f3,f4,f5) values('1970-01-01 00:00:01.0001', '1971-08-27', '12:01:02', '1941-12-07 00:00:01.0001', '2014');
insert into dates(f1,f2,f3,f4,f5) values('1970-01-01 00:00:02.0002', '1971-07-21', '12:01:03', '1941-12-07 00:00:02.0002', '89');

#Strings
SET NAMES UTF8;
insert into strings(f1, f2, f3, f4) values('Света', 'Света', 'Смирнова', 'Смирнова');
insert into strings(f1, f2, f3, f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14) values('электричество', 'Шла Саша по шоссе и сосала сушки', 'Смирнова', 'Смирнова', 'электричество', 'Шла Саша по шоссе и сосала сушки', 'Смирнова', 'Смирнова', 'Смирнова', 'Смирнова', 'Смирнова', 'Смирнова', 'Света', 'Света,Валерий');
insert into strings(f1, f2, f3, f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14) values('электричество', 'Шла Саша по шоссе и сосала сушки', 'Смирнова', 'Смирнова', 'электричество', 'Шла Саша по шоссе и сосала сушки', 'Смирнова', 'Смирнова', 'Смирнова', 'Смирнова', 'Смирнова', 'Смирнова', 'MySQL', 'MySQL');

select * from numbers;
select * from dates;
select * from strings;

-- sync_slave_with_master

select * from numbers;
select * from dates;
select * from strings;

connection master;

#Numbers
update numbers set f1=15 where f5=5;

#Dates
update dates set f4=now() where f1=1;
update dates set f6='2015' where f3=curdate();

#Strings
update strings set f2='газ' where f2 like 'Света%';
update strings set f2='Валерий' where f13='Света';
update strings set f2='Валерий' where f14='Света';
update strings set f2='Валерий' where f13='MySQL';
update strings set f2='Валерий' where f14='MySQL';

select * from numbers;
select * from dates;
select * from strings;
select * from strings where id=2;

-- sync_slave_with_master

select * from numbers;
select * from dates;
select * from strings;
select * from strings where id=2;

connection master;
#Deletes
#Numbers
delete from numbers where f4 < 0;

#Dates
delete from dates where f1 < now();

#Strings
delete from strings where f1='Света';
        
select * from numbers;
select * from dates;
select * from strings;

-- sync_slave_with_master
select * from numbers;
select * from dates;
select * from strings;

connection master;
drop table numbers, dates, strings;

-- sync_slave_with_master

connection slave;
drop table if exists numbers, dates, strings;
-- source include/rpl_end.inc
