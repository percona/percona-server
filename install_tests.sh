#!/usr/bin/env bash

set -u

MYSQL_VERSION="$(grep ^MYSQL_VERSION= "Makefile" \
    | cut -d = -f 2)"
export PERCONA_SERVER="Percona-Server-$MYSQL_VERSION"

install_file_type()
{
    for file in `ls $1/*.$2 2>/dev/null`; do
	test -f $file && install -m 644 $file ${PERCONA_SERVER}/mysql-test/$3
    done;
}
do_install_path()
{
    install_file_type $1 test t
    install_file_type $1 opt t
    install_file_type $1 result r
    install_file_type $1 require r
    install_file_type $1 inc include
}
install_path()
{
    echo "[$3/$4] Installing mysql-test files: $2"
    test -d $1 && do_install_path $1 $2
}
let current=1;
count=`wc -l series`;
install_path mysql-test "global" $current $count
for test_name in `cat series`; do
    let current=$current+1;
    install_path mysql-test/$test_name $test_name $current $count
done
echo "Done"
