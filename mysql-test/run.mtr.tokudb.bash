suites=tokudb.add_index,tokudb.alter_table,tokudb,tokudb.bugs,tokudb.parts,tokudb.rpl,tokudb.sys_vars
run="./mtr --suite=$suites --skip-test=fast_up.* --force --retry=0 --max-test-fail=0 --parallel=auto --no-warnings --testcase-timeout=60 --big-test"
$run &>tokudb.mtr.outs
