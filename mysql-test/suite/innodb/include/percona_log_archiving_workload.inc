#
# Perform insert into table to produce some log records
#

insert into t (a) select t1.a from t t1, t t2, t t3 LIMIT 40000;
