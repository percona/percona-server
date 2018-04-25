#!/bin/bash
#
# Execute this tool to test binary releases
#
#  possible jenkins vars:
#      CMAKE_BUILD_TYPE = (RelWithDebInfo Debug)
#      ANALYZER_OPTS = (-DWITH_ASAN=ON -DWITH_ASAN_SCOPE=ON -DWITH_MSAN=ON -DWITH_UBSAN=ON -DWITH_VALGRIND=ON)
#      DEFAULT_TESTING = (yes no)
#      HOTBACKUP_TESTING = (yes no)
#      TOKUDB_ENGINES_MTR = (yes no)
#      MTR_ARGS
#      MTR_REPEAT

set -o errexit
set -o xtrace

WORKDIR_ABS=$(cd ${1:-.}; pwd -P)
tar -C $WORKDIR_ABS -zxpf $(ls $WORKDIR_ABS/*.tar.gz | head -1)
cd $WORKDIR_ABS/Percona-Server-*/mysql-test

TOKUDB_PLUGIN=$(find $WORKDIR_ABS -type f -name 'ha_tokudb.so')
HOTBACKUP_LIB=$(find $WORKDIR_ABS -type f -name 'libHotBackup.so')
HOTBACKUP_PLUGIN=$(find $WORKDIR_ABS -type f -name 'tokudb_backup.so')
JEMALLOC=$(find /lib* /usr/lib* /usr/local/lib* -type f -name 'libjemalloc.so*' | head -n1)

if [[ ${CMAKE_BUILD_TYPE} == Debug ]]; then
    MTR_ARGS+=" --debug-server"
fi
if [[ ${ANALYZER_OPTS} == *WITH_VALGRIND=ON* ]]; then
    MTR_ARGS+=" --valgrind --valgrind-clients --valgrind-option=--leak-check=full --valgrind-option=--show-leak-kinds=all"
elif [[ ${ANALYZER_OPTS} == *WITH_ASAN=ON* ]]; then
    export ASAN_OPTIONS=allocator_may_return_null=true
else
    EATMYDATA=$(find /lib* /usr/lib* /usr/local/lib* -type f -name '*eatmyda*.so*' | head -n1)
    if [ "x${EATMYDATA}" = "x" ]; then
        echo "No libeatmydata.so lib found"
        exit 1
    fi
fi

if [ "x${MTR_REPEAT}" != "x" ]; then
    MTR_ARGS+=" --repeat=${MTR_REPEAT}"
fi

# force to use mecab everywhere (except RHEL)
if [ ! -e /etc/redhat-release ]; then
    MTR_ARGS+=" --mysqld=--loose-mecab-rc-file=/etc/mecabrc"
fi

status=0

# Running MTR test cases
if [ "x$DEFAULT_TESTING" != "xno" ]; then
	LD_PRELOAD="${EATMYDATA}" MTR_BUILD_THREAD=auto ./mysql-test-run.pl \
        --parallel=$(grep -c ^processor /proc/cpuinfo) \
        --result-file \
        ${MTR_ARGS} \
        --force \
        --max-test-fail=0 \
        --suite-timeout=9999 \
        --testcase-timeout=450 \
        | tee ${WORKDIR_ABS}/mtr.output \
        || status=$?
    awk 'BEGIN { print "<testsuite name=\"MySQL\">" } /^([[:alnum:]]+).*[\[] (retry-)?pass [\]](.*)/ { print "<testcase name=\""$1"\" time=\"" $6/1000 "\"/>" } /^(.*) .*\[ skipped \]/ { print "<testcase name=\""$1"\"><skipped/></testcase>" } /^(.*) .*\[ (retry-)?fail \]/ { print "<testcase name=\""$1"\"><failure/></testcase>" } /^MTR\47s internal check of the test case \47(.*)\47 failed/ { print "<testcase name=\"testcase-check-"$8"\"><failure/></testcase>" } END { print "</testsuite>" }' < ${WORKDIR_ABS}/mtr.output > $WORKDIR_ABS/junit.xml
fi


if [ "x$HOTBACKUP_TESTING" != "xno" -a "x${TOKUDB_PLUGIN}" != "x" -a "x${HOTBACKUP_LIB}" != "x" -a "x${HOTBACKUP_PLUGIN}" != "x" ]; then
    MYSQLD_ENV="${HOTBACKUP_LIB}:${JEMALLOC}:${EATMYDATA}"
    if [[ ${ANALYZER_OPTS} == *WITH_ASAN=ON* ]]; then
        MYSQLD_ENV="$(find /lib* /usr/lib* /usr/local/lib* -type f -name 'libasan.so*' | head -n1):${MYSQLD_ENV}"
    fi

    LD_PRELOAD="${MYSQLD_ENV//:/ }" MTR_BUILD_THREAD=auto ./mtr \
        --force \
        --max-test-fail=0 \
        --suite-timeout=9999 \
        --testcase-timeout=450 \
        --parallel=$(grep -c ^processor /proc/cpuinfo) \
        ${MTR_ARGS} \
        --mysqld-env="LD_PRELOAD=${MYSQLD_ENV}" \
        --suite tokudb.backup \
        | tee ${WORKDIR_ABS}/tokudb_hotbackup.output \
        || true
    awk 'BEGIN { print "<testsuite name=\"Percona Server - TokuDB HotBackup\">" } /^([[:alnum:]]+).*[\[] (retry-)?pass [\]](.*)/ { print "<testcase name=\""$1"\" time=\"" $6/1000 "\"/>" } /^(.*) .*\[ skipped \]/ { print "<testcase name=\""$1"\"><skipped/></testcase>" } /^(.*) .*\[ (retry-)?fail \]/ { print "<testcase name=\""$1"\"><failure/></testcase>" } /^MTR\47s internal check of the test case \47(.*)\47 failed/ { print "<testcase name=\"testcase-check-"$8"\"><failure/></testcase>" } END { print "</testsuite>" }' < ${WORKDIR_ABS}/tokudb_hotbackup.output > ${WORKDIR_ABS}/junit_tokudb_hotbackup.xml
fi

# if there's tokudb plugin compiled run tokudb tests else exit with previous status
if [ "x${TOKUDB_ENGINES_MTR}" = "xyes" -a "x${TOKUDB_PLUGIN}" != "x" ]; then
    # we test with system jemalloc
    if [ "x${JEMALLOC}" = "x" ]; then
        echo "No jemalloc lib found"
        exit 1
    fi

    # this is a workaround because mtr is looking for ha_tokudb.so in source_dir in lib/plugins/mysql when it's actually in builddir/storage/tokudb
    mkdir -p lib/mysql/plugin
    ln -s ${TOKUDB_PLUGIN}    lib/mysql/plugin/ha_tokudb.so
    ln -s ${HOTBACKUP_PLUGIN} lib/mysql/plugin/tokudb_backup.so

    LD_PRELOAD="${EATMYDATA} ${JEMALLOC}" MTR_BUILD_THREAD=auto \
        ./mtr --suite=engines/iuds,engines/funcs \
            --mysqld=--default-storage-engine=tokudb --mysqld=--default-tmp-storage-engine=tokudb \
            --suite-timeout=9999 --testcase-timeout=450 --parallel=$(grep -c ^processor /proc/cpuinfo) --big-test --max-test-fail=0 \
            --mysqld=--plugin-load=tokudb=ha_tokudb.so \
            --mysqld=--loose-tokudb_auto_analyze=0 --mysqld=--loose-tokudb_analyze_in_background=false \
            ${MTR_ARGS} \
            2>&1 | tee ${WORKDIR_ABS}/tokudb_engines.output || true
    awk 'BEGIN { print "<testsuite name=\"Percona Server - TokuDB\">" } /^([[:alnum:]]+).*[\[] (retry-)?pass [\]](.*)/ { print "<testcase name=\""$1"\" time=\"" $6/1000 "\"/>" } /^(.*) .*\[ skipped \]/ { print "<testcase name=\""$1"\"><skipped/></testcase>" } /^(.*) .*\[ (retry-)?fail \]/ { print "<testcase name=\""$1"\"><failure/></testcase>" } /^MTR\47s internal check of the test case \47(.*)\47 failed/ { print "<testcase name=\"testcase-check-"$8"\"><failure/></testcase>" } END { print "</testsuite>" }' < ${WORKDIR_ABS}/tokudb_engines.output > ${WORKDIR_ABS}/junit_tokudb.xml
fi

exit $status
