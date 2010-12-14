#!/bin/sh

test -z ${PERCONA_SERVER} && export PERCONA_SERVER=Percona-Server
echo PERCONA_SERVER=${PERCONA_SERVER}

install_path()
{
    echo "Installing mysql-test files: $2"
    find $1 -iname '*.test' -exec install -m 644 {} ${PERCONA_SERVER}/mysql-test/t/ ';'
    find $1 -iname '*.opt' -exec install -m 644 {} ${PERCONA_SERVER}/mysql-test/t/ ';'
    find $1 -iname '*.result' -exec install -m 644 {} ${PERCONA_SERVER}/mysql-test/r/ ';'
    find $1 -iname '*.require' -exec install -m 644 {} ${PERCONA_SERVER}/mysql-test/r/ ';'
    find $1 -iname '*.inc' -exec install -m 644 {} ${PERCONA_SERVER}/mysql-test/include/ ';'
}

install_path mysql-test "global"
for test_name in `cat series`; do
    test -d mysql-test/$test_name && install_path mysql-test/$test_name $test_name
done
echo "Done"
