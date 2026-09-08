#!/bin/bash

# Notes about this script:
# 1. This file defines the function set_suites(), which sets WORKER_x_MTR_SUITES for x=1..8.
# 2. The function check_suites(), defined in https://github.com/Percona-Lab/ps-build/blob/8.0/local/utils.inc.sh
#    checks for inconsistencies between the suites specified in mysql-test-run.pl and those defined in this script.
# 3. The default split is defined in https://github.com/Percona-Lab/ps-build/blob/8.0/jenkins/suites-groups.sh
# 4. The default split can be overridden by mysql-test/suites-groups.sh, if present,
#    allowing custom suite splits on development branches.
# 5. By default, the Jenkins pipeline fails if inconsistencies are detected (IGNORE_INCONSISTENCY=0).
# 6. If IGNORE_INCONSISTENCY=1 is set, the pipeline continues with a warning instead of failing.
# 7. Jenkins scripts support the following suite formats:
#
#    main       - all tests will be allowed to be executed (big and no-big). Note that the final decision belongs to --big-tests MTR parameter
#    main|nobig - only no-big tests are allowed
#    main|big   - only big tests are allowed
#
#    Such approach makes it possible to split the suite execution among two workers, where one woker executes no-big test
#    and another executes only bit tests.


# Uncomment to continue testing even if this file is inconsistent with DEFAULT_SUITES from mysql-test-run.pl
#IGNORE_INCONSISTENCY=1


# usage: set_suites <BUILD_TYPE>
function set_suites() {
  # Comparing to 8.4 added in 9.4: component_connection_control, jdv
  if [[ "$1" == "Valgrind" ]]; then
    # Unit tests, KEYRING_VAULT tests, ps_protocol, ci_fs will be executed by worker 1
    echo "Setting WORKER_x_MTR_SUITES for PS 9.x with Valgrind (a custom suite split)"
    WORKER_1_MTR_SUITES="router,rpl_gtid|big,percona_rpl_gtid|big,percona|nobig,component_audit_log_filter,rpl_nogtid|nobig,sysschema|big,innodb_zip|big,rpl_encryption,rocksdb_rpl|nobig,jp,service_sys_var_registration,audit_null,rocksdb_stress,component_js_lang|big"
    WORKER_2_MTR_SUITES="innodb|nobig,innodb_fts|big,funcs_1|big,encryption|nobig,component_masking_functions,component_encryption_udf|nobig,component_connection_control|nobig,procfs,encryption|big"
    WORKER_3_MTR_SUITES="main|big,binlog|nobig,percona_binlog|nobig,innodb_zip|nobig,engines/funcs|nobig,innodb_gis|big,parts|big,binlog_nogtid|nobig,collations,sysschema|nobig,rocksdb_sys_vars,connection_control|nobig,jdv"
    WORKER_4_MTR_SUITES="innodb|big,group_replication|nobig,rpl_gtid|nobig,percona_rpl_gtid|nobig,binlog|big,percona_binlog|big,innodb_fts|nobig,binlog_nogtid|big,funcs_1|nobig,innodb_gis|nobig,perfschema|big,federated|nobig,gis|big"
    WORKER_5_MTR_SUITES="main|nobig,rpl|big,percona_rpl|big,rpl_nogtid|big,component_encryption_udf|big,x|nobig,binlog_gtid|nobig,component_keyring_file|nobig,funcs_2|big,test_services,binlog_gtid|big,stress|nobig,information_schema,component_js_lang|nobig,component_connection_control|big"
    WORKER_6_MTR_SUITES="group_replication|big,percona_innodb|nobig,rocksdb|big,innodb_undo|nobig,stress|big,x|big,auth_sec|nobig,rocksdb_rpl|big,gcol|nobig,query_rewrite_plugins,secondary_engine,interactive_utilities,connection_control|big,percona|big"
    WORKER_7_MTR_SUITES="rpl|nobig,percona_rpl|nobig,innodb_undo|big,perfschema|nobig,component_keyring_file|big,percona_innodb|big,sys_vars|nobig,auth_sec|big,opt_trace|nobig,json,engines/iuds|nobig,test_service_sql_api,service_status_var_registration,opt_trace|big"
    WORKER_8_MTR_SUITES="engines/funcs|big,clone|big,rocksdb|nobig,clone|nobig,parts|nobig,federated|big,engines/iuds|big,gis|nobig,sys_vars|big,gcol|big,funcs_2|nobig,service_udf_registration,percona-pam-for-mysql"
  elif [[ "$1" == "RelWithDebInfo" ]]; then
    # Unit tests, KEYRING_VAULT tests, ps_protocol, ci_fs will be executed by worker 1
    echo "Setting WORKER_x_MTR_SUITES for PS 9.x with BUILD_TYPE=RelWithDebInfo (a custom suite split)"
    WORKER_1_MTR_SUITES="main|nobig,main|big,parts,percona,component_audit_log_filter,engines/iuds,component_connection_control,opt_trace,information_schema"
    WORKER_2_MTR_SUITES="component_encryption_udf,innodb_zip,gis,json,component_js_lang,router"
    WORKER_3_MTR_SUITES="group_replication|big,binlog,percona_binlog,test_service_sql_api,component_masking_functions,connection_control,service_sys_var_registration,jdv"
    WORKER_4_MTR_SUITES="rocksdb|big,rpl_gtid,percona_rpl_gtid,auth_sec,engines/funcs,innodb_undo,interactive_utilities,query_rewrite_plugins,audit_null"
    WORKER_5_MTR_SUITES="rocksdb|nobig,percona_innodb,sys_vars,perfschema,innodb_fts,binlog_gtid,rpl_encryption,service_udf_registration,procfs"
    WORKER_6_MTR_SUITES="rpl|nobig,percona_rpl|nobig,rpl_nogtid|big,rocksdb_rpl,clone,innodb_gis,funcs_1,funcs_2,collations,jp,percona-pam-for-mysql"
    WORKER_7_MTR_SUITES="group_replication|nobig,rpl_nogtid|nobig,innodb|nobig,component_keyring_file,rocksdb_stress,gcol,stress,test_services,secondary_engine,service_status_var_registration"
    WORKER_8_MTR_SUITES="innodb|big,rpl|big,percona_rpl|big,sysschema,x|big,x|nobig,binlog_nogtid,federated,rocksdb_sys_vars,encryption"
  else # Debug (and everything different from "RelWithDebInfo" and "Valgrind")
    # Unit tests, KEYRING_VAULT tests, ps_protocol, ci_fs will be executed by worker 1
    echo "Setting WORKER_x_MTR_SUITES for PS 9.x with BUILD_TYPE=Debug (a custom suite split)"
    WORKER_1_MTR_SUITES="rocksdb|nobig,rpl|big,percona_rpl|big,innodb_undo|nobig,percona,sysschema|nobig,x|big,binlog_nogtid|nobig,encryption,service_sys_var_registration,audit_null,router"
    WORKER_2_MTR_SUITES="clone|big,component_keyring_file|big,perfschema|nobig,rocksdb_rpl|nobig,clone|nobig,innodb_zip|big,rocksdb_stress,binlog_nogtid|big,query_rewrite_plugins,jdv"
    WORKER_3_MTR_SUITES="rocksdb|big,rpl_nogtid|nobig,percona_innodb|big,component_audit_log_filter,sysschema|big,parts|nobig,rpl_encryption,component_masking_functions,jp,information_schema"
    WORKER_4_MTR_SUITES="group_replication|big,percona_innodb|nobig,binlog|nobig,percona_binlog|nobig,rpl_nogtid|big,rocksdb_rpl|big,federated,innodb_gis|nobig,collations,test_services,component_js_lang"
    WORKER_5_MTR_SUITES="main|big,innodb_gis|big,rpl_gtid|big,percona_rpl_gtid|big,binlog|big,percona_binlog|big,funcs_1,auth_sec|big,binlog_gtid|nobig,opt_trace,secondary_engine,service_status_var_registration"
    WORKER_6_MTR_SUITES="main|nobig,group_replication|nobig,parts|big,innodb_undo|big,innodb_fts|nobig,perfschema|big,engines/iuds,innodb_zip|nobig,rocksdb_sys_vars,component_connection_control,service_udf_registration"
    WORKER_7_MTR_SUITES="innodb|big,rpl|nobig,percona_rpl|nobig,engines/funcs,sys_vars,x|nobig,component_keyring_file|nobig,stress,funcs_2,json,interactive_utilities,procfs"
    WORKER_8_MTR_SUITES="component_encryption_udf,innodb|nobig,innodb_fts|big,rpl_gtid|nobig,percona_rpl_gtid|nobig,gcol,gis,auth_sec|nobig,binlog_gtid|big,test_service_sql_api,connection_control,percona-pam-for-mysql"
  fi
}
