for suite in tokudb.add_index tokudb.alter_table tokudb tokudb.bugs ; do
    ./mtr --suite=$suite --skip-test=fast_up.* --mysqld='--plugin-load=tokudb=ha_tokudb.so;tokudb_trx=ha_tokudb.so;tokudb_locks=ha_tokudb.so;tokudb_lock_waits=ha_tokudb.so;tokudb_fractal_tree_info=ha_tokudb.so' --mysqld=--loose-tokudb-check-jemalloc=0 --force --retry=0 --max-test-fail=0 --parallel=auto --no-warnings --testcase-timeout=60 --big-test >$suite.out 2>&1 
done

for suite in funcs iuds ; do
    ./mtr --suite=engines/$suite --mysqld=--default-storage-engine=tokudb --mysqld=--default-tmp-storage-engine=tokudb --mysqld=--plugin-load=tokudb=ha_tokudb.so --mysqld=--loose-tokudb-check-jemalloc=0 --parallel=auto --force --retry=0 --max-test-fail=0 --big-test >$suite.out 2>&1
done

./mtr  --suite=parts --do-test=.*tokudb.* --mysqld=--plugin-load=tokudb=ha_tokudb.so --mysqld=--loose-tokudb-check-jemalloc=0 --force --retry=0 --max-test-fail=0 --parallel=auto --no-warnings --testcase-timeout=60 --big-test >parts.out 2>&1

./mtr  --suite=rpl --do-test=.*tokudb.* --mysqld=--plugin-load=tokudb=ha_tokudb.so --mysqld=--loose-tokudb-check-jemalloc=0 --force --retry=0 --max-test-fail=0 --parallel=auto --no-warnings --testcase-timeout=60 --big-test >rpl.out 2>&1
