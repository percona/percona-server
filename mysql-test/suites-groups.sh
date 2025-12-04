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
  if [[ "$1" == "Valgrind" ]]; then
    # Unit tests, KEYRING_VAULT tests, ps_protocol, ci_fs will be executed by worker 1
    echo "Setting WORKER_x_MTR_SUITES for PS 8.0 with Valgrind (a custom suite split)"
    WORKER_1_MTR_SUITES="rpl_gtid|big,clone|nobig,rocksdb|big,rpl_gtid|nobig,perfschema|big,innodb_zip|nobig,binlog_nogtid|nobig,funcs_1|big,jp,service_sys_var_registration,encryption|big,service_udf_registration"
    WORKER_2_MTR_SUITES="innodb|nobig,parts|big,innodb_fts|big,funcs_2|big,audit_log,query_rewrite_plugins,engines/iuds|nobig,stress|nobig,service_status_var_registration"
    WORKER_3_MTR_SUITES="innodb|big,rpl_nogtid|big,innodb_undo|big,sys_vars|nobig,component_keyring_file|nobig,binlog_gtid|nobig,component_masking_functions,sysschema|nobig,test_service_sql_api,connection_control,gis|big"
    WORKER_4_MTR_SUITES="main|big,rocksdb|nobig,perfschema|nobig,x|nobig,rocksdb_rpl|big,sys_vars|big,gcol|nobig,innodb_gis|nobig,secondary_engine,component_encryption_udf|nobig,procfs,percona-pam-for-mysql"
    WORKER_5_MTR_SUITES="main|nobig,x|big,binlog|nobig,engines/funcs|nobig,binlog|big,rpl_encryption,binlog_nogtid|big,auth_sec|big,gis|nobig,funcs_2|nobig,data_masking,interactive_utilities"
    WORKER_6_MTR_SUITES="group_replication|nobig,engines/funcs|big,stress|big,audit_log_filter,innodb_fts|nobig,parts|nobig,innodb_gis|big,encryption|nobig,gcol|big,test_services,federated|nobig,opt_trace|big,rocksdb_stress"
    WORKER_7_MTR_SUITES="rpl|big,rpl|nobig,innodb_undo|nobig,sysschema|big,federated|big,engines/iuds|big,rocksdb_rpl|nobig,funcs_1|nobig,json,opt_trace|nobig,binlog_57_decryption,information_schema"
    WORKER_8_MTR_SUITES="group_replication|big,clone|big,component_keyring_file|big,rpl_nogtid|nobig,auth_sec|nobig,component_encryption_udf|big,collations,innodb_zip|big,binlog_gtid|big,rocksdb_sys_vars,audit_null,percona"
  elif [[ "$1" == "RelWithDebInfo" ]]; then
    # Unit tests, KEYRING_VAULT tests, ps_protocol, ci_fs will be executed by worker 1
    echo "Setting WORKER_x_MTR_SUITES for PS 8.0 with BUILD_TYPE=RelWithDebInfo (a custom suite split)"
    WORKER_1_MTR_SUITES="rpl_nogtid|nobig,auth_sec,sys_vars,gcol,information_schema,json,binlog_57_decryption,percona,percona-pam-for-mysql"
    WORKER_2_MTR_SUITES="group_replication|big,binlog_nogtid,audit_log_filter,funcs_2,query_rewrite_plugins,audit_null"
    WORKER_3_MTR_SUITES="rocksdb|big,rocksdb_rpl,rpl_nogtid|big,innodb_fts,audit_log,stress,collations,service_sys_var_registration,encryption"
    WORKER_4_MTR_SUITES="rocksdb|nobig,parts,innodb_gis,perfschema,innodb_undo,federated,jp,secondary_engine"
    WORKER_5_MTR_SUITES="component_encryption_udf,rpl_gtid,sysschema,x|big,rocksdb_stress,engines/iuds,test_services,data_masking,procfs"
    WORKER_6_MTR_SUITES="rpl|nobig,main|big,innodb|big,component_keyring_file,innodb_zip,binlog_gtid,interactive_utilities,opt_trace,service_udf_registration"
    WORKER_7_MTR_SUITES="rpl|big,innodb|nobig,clone,engines/funcs,test_service_sql_api,connection_control,rocksdb_sys_vars,service_status_var_registration"
    WORKER_8_MTR_SUITES="group_replication|nobig,main|nobig,binlog,x|nobig,funcs_1,component_masking_functions,rpl_encryption,gis"
  else # Debug (and everything different from "RelWithDebInfo" and "Valgrind")
    # Unit tests, KEYRING_VAULT tests, ps_protocol, ci_fs will be executed by worker 1
    echo "Setting WORKER_x_MTR_SUITES for PS 8.0 with BUILD_TYPE=Debug (a custom suite split)"
    WORKER_1_MTR_SUITES="rocksdb|nobig,innodb_undo|nobig,clone|nobig,x|nobig,component_keyring_file|nobig,federated,innodb_undo|big,collations,binlog_57_decryption,procfs"
    WORKER_2_MTR_SUITES="rocksdb|big,rpl_gtid|nobig,binlog|big,rocksdb_rpl|big,auth_sec|nobig,rpl_encryption,audit_log,service_sys_var_registration,secondary_engine"
    WORKER_3_MTR_SUITES="main|big,parts|big,gcol,rpl_nogtid|big,perfschema|big,engines/iuds,binlog_gtid|big,json,gis"
    WORKER_4_MTR_SUITES="group_replication|big,rpl_gtid|big,engines/funcs,component_encryption_udf,binlog_nogtid,x|big,rocksdb_stress,component_masking_functions,query_rewrite_plugins,jp"
    WORKER_5_MTR_SUITES="innodb|nobig,component_keyring_file|big,binlog|nobig,audit_log_filter,innodb_zip|big,clone|big,funcs_2,rocksdb_sys_vars,data_masking,service_udf_registration"
    WORKER_6_MTR_SUITES="main|nobig,group_replication|nobig,perfschema|nobig,rocksdb_rpl|nobig,sysschema|big,stress,binlog_gtid|nobig,innodb_gis|nobig,test_services,audit_null,service_status_var_registration"
    WORKER_7_MTR_SUITES="innodb|big,innodb_gis|big,rpl_nogtid|nobig,sys_vars,innodb_fts|nobig,auth_sec|big,parts|nobig,innodb_zip|nobig,opt_trace,interactive_utilities,percona,percona-pam-for-mysql"
    WORKER_8_MTR_SUITES="rpl|nobig,rpl|big,innodb_fts|big,encryption,sysschema|nobig,information_schema,funcs_1|big,funcs_1|nobig,test_service_sql_api,connection_control"
  fi
}
