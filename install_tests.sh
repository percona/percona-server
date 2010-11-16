#!/usr/bin/env sh
INSTALL_PATH=Percona-Server
echo INSTALL_PATH=$INSTALL_PATH
install_path()
{
    echo "Installing mysql-test files: $2"
    install -m 644 $1/*.opt $1/*.test $INSTALL_PATH/mysql-test/t/
    install -m 644 $1/*.result $INSTALL_PATH/mysql-test/r/
    install -m 644 $1/*.inc $INSTALL_PATH/mysql-test/include/
}

#install_path mysql-test "global"
for test_name in `cat series`; do
    install_path mysql-test/$test_name $test_name
done
echo "Done"
