#!/usr/bin/env sh
test -z ${PERCONA_SERVER} && export PERCONA_SERVER=Percona-Server
echo PERCONA_SERVER=$PERCONA_SERVER
install_path()
{
    echo "Installing mysql-test files: $2"
    if [ -d $1 ]; then
	install -m 644 $1/*.opt $1/*.test $PERCONA_SERVER/mysql-test/t/
	install -m 644 $1/*.result $PERCONA_SERVER/mysql-test/r/
	install -m 644 $1/*.inc $PERCONA_SERVER/mysql-test/include/
    fi
}

#install_path mysql-test "global"
for test_name in `cat series`; do
    install_path mysql-test/$test_name $test_name
done
echo "Done"
