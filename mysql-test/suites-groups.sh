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
  # Comparing to 8.0 added in 8.4: component_audit_log_filter, percona, percona_innodb, component_js_lang
  # Comparing to 8.0 removed from 8.4: data_masking, binlog_57_decryption, audit_log_filter, audit_log
  if [[ "$1" == "Valgrind" ]]; then
    # Unit tests, KEYRING_VAULT tests, ps_protocol, ci_fs will be executed by worker 1
    echo "Setting WORKER_x_MTR_SUITES for PS 8.4 with Valgrind (a custom suite split)"
    # TO DO: Update, copied from 8.0 Valgrind + synchronized
    WORKER_1_MTR_SUITES="group_replication|big,engines/funcs|big,rocksdb|nobig,sysschema|big,sys_vars|big,parts|nobig,binlog_gtid|nobig,collations,innodb_zip|big,opt_trace|nobig,component_encryption_udf|nobig,procfs"
    WORKER_2_MTR_SUITES="innodb|nobig,innodb_gis|nobig,funcs_2|nobig,test_services,interactive_utilities,percona-pam-for-mysql"
    WORKER_3_MTR_SUITES="innodb|big,engines/funcs|nobig,sys_vars|nobig,rocksdb_rpl|big,funcs_2|big,funcs_1|nobig,jp,gcol|big,audit_null,rocksdb_stress"
    WORKER_4_MTR_SUITES="main|nobig,component_keyring_file|big,rpl_nogtid|big,component_audit_log_filter,binlog|big,innodb_fts|big,binlog_nogtid|nobig,auth_sec|big,gcol|nobig,binlog_gtid|big,federated|nobig,service_udf_registration,opt_trace|big"
    WORKER_5_MTR_SUITES="main|big,x|big,stress|big,rpl_nogtid|nobig,innodb_fts|nobig,parts|big,innodb_gis|big,rocksdb_rpl|nobig,json,gis|nobig,service_sys_var_registration,connection_control"
    WORKER_6_MTR_SUITES="rpl|big,rpl|nobig,rocksdb|big,rpl_gtid|nobig,innodb_zip|nobig,auth_sec|nobig,component_encryption_udf|big,encryption|nobig,sysschema|nobig,test_service_sql_api,perfschema|big,service_status_var_registration"
    WORKER_7_MTR_SUITES="group_replication|nobig,clone|nobig,innodb_undo|nobig,binlog|nobig,federated|big,component_keyring_file|nobig,rpl_encryption,funcs_1|big,query_rewrite_plugins,rocksdb_sys_vars,secondary_engine,information_schema"
    WORKER_8_MTR_SUITES="percona,percona_innodb,component_js_lang,rpl_gtid|big,clone|big,perfschema|nobig,innodb_undo|big,x|nobig,engines/iuds|big,binlog_nogtid|big,component_masking_functions,engines/iuds|nobig,stress|nobig,encryption|big,gis|big"
  elif [[ "$1" == "RelWithDebInfo" ]]; then
    # Unit tests, KEYRING_VAULT tests, ps_protocol, ci_fs will be executed by worker 1
    echo "Setting WORKER_x_MTR_SUITES for PS 8.4 with BUILD_TYPE=RelWithDebInfo (a custom suite split)"
    WORKER_1_MTR_SUITES="main|nobig,rpl_gtid,rpl_nogtid|big,innodb_gis,binlog_nogtid,binlog_gtid,json,information_schema,service_status_var_registration"
    WORKER_2_MTR_SUITES="group_replication|big,funcs_1,component_masking_functions,collations,gis,percona-pam-for-mysql"
    WORKER_3_MTR_SUITES="rocksdb|big,clone,x|nobig,rocksdb_stress,funcs_2,test_services,encryption,component_js_lang"
    WORKER_4_MTR_SUITES="rocksdb|nobig,innodb|nobig,sys_vars,component_keyring_file,innodb_zip,federated,rpl_encryption,secondary_engine"
    WORKER_5_MTR_SUITES="component_encryption_udf,main|big,parts,x|big,binlog,component_audit_log_filter,connection_control,rocksdb_sys_vars"
    WORKER_6_MTR_SUITES="innodb|big,percona_innodb,rocksdb_rpl,engines/funcs,innodb_fts,engines/iuds,service_sys_var_registration,audit_null,procfs"
    WORKER_7_MTR_SUITES="rpl|nobig,rpl_nogtid|nobig,auth_sec,perfschema,gcol,test_service_sql_api,interactive_utilities,jp,service_udf_registration"
    WORKER_8_MTR_SUITES="group_replication|nobig,rpl|big,sysschema,percona,innodb_undo,stress,query_rewrite_plugins,opt_trace"
  else # Debug (and everything different from "RelWithDebInfo" and "Valgrind")
    # Unit tests, KEYRING_VAULT tests, ps_protocol, ci_fs will be executed by worker 1
    echo "Setting WORKER_x_MTR_SUITES for PS 8.4 with BUILD_TYPE=Debug (a custom suite split)"
    WORKER_1_MTR_SUITES="innodb|nobig,rpl|big,perfschema|nobig,component_encryption_udf,clone|nobig,federated,rpl_encryption,collations,audit_null,component_js_lang"
    WORKER_2_MTR_SUITES="rocksdb|big,component_keyring_file|big,percona,x|nobig,component_keyring_file|nobig,engines/iuds,funcs_2,component_masking_functions,interactive_utilities,percona-pam-for-mysql"
    WORKER_3_MTR_SUITES="clone|big,percona_innodb|big,innodb_undo|big,component_audit_log_filter,rocksdb_rpl|big,innodb_zip|big,opt_trace,test_services,jp,service_udf_registration"
    WORKER_4_MTR_SUITES="group_replication|big,rpl_nogtid|nobig,innodb_undo|nobig,binlog|big,rpl_nogtid|big,perfschema|big,binlog_nogtid|nobig,innodb_zip|nobig,gis,information_schema"
    WORKER_5_MTR_SUITES="main|big,percona_innodb|nobig,rpl_gtid|nobig,gcol,sysschema|nobig,parts|nobig,rocksdb_stress,rocksdb_sys_vars,connection_control,procfs"
    WORKER_6_MTR_SUITES="main|nobig,group_replication|nobig,engines/funcs,rpl_gtid|big,innodb_fts|nobig,auth_sec|big,auth_sec|nobig,binlog_nogtid|big,service_sys_var_registration,service_status_var_registration"
    WORKER_7_MTR_SUITES="innodb|big,innodb_gis,innodb_fts|big,sys_vars,rocksdb_rpl|nobig,sysschema|big,stress,binlog_gtid|big,test_service_sql_api,secondary_engine"
    WORKER_8_MTR_SUITES="rpl|nobig,rocksdb|nobig,parts|big,binlog|nobig,encryption,funcs_1,x|big,binlog_gtid|nobig,json,query_rewrite_plugins"
  fi
}
