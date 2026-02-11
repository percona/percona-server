# ==== Purpose ====
#
# Check if the provided library ('.so') can be successfully loaded with 'dlopen(..., RTLD_NOW)'
#
# ==== Usage ====
#
# --let $DLOPEN_CHECKER_LIBRARY_PATH = <path_to_the_library>
# --source include/keyring_tests/mats/dynamic_loading.inc
#
# ==== Parameters ====
#
# DLOPEN_CHECKER_LIBRARY_PATH
#    Full path to the library that needs to be checked for unresolved symbols ('.so')
#

--let $dlopen_checker_source = $MYSQL_TEST_DIR/std_data/dlopen_checker.cpp
--let $dlopen_checker_binary = $MYSQL_TMP_DIR/dlopen_checker

--echo *** Building dlopen_checker utility
--exec g++ -std=c++17 -o $dlopen_checker_binary $dlopen_checker_source -ldl

--echo *** Checking for unresolved symbols
--replace_result $DLOPEN_CHECKER_LIBRARY_PATH <LIBRARY_PATH>
--exec $dlopen_checker_binary $DLOPEN_CHECKER_LIBRARY_PATH

--echo *** Deleting dlopen_checker utility
--remove_file $dlopen_checker_binary
