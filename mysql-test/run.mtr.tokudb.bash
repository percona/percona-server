engine=tokudb
suites=tokudb.add_index,tokudb.alter_table,tokudb,tokudb.bugs,tokudb.parts,tokudb.rpl,tokudb.sys_vars,engines/funcs,engines/iuds
./mtr --suite=$suites --mysqld=--default-storage-engine=$engine --mysqld=--default-tmp-storage-engine=$engine --skip-test=fast_up.* --force --retry=0 --max-test-fail=0 --parallel=auto --no-warnings --testcase-timeout=60 --big-test
