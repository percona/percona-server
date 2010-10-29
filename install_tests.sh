#!/usr/bin/env sh

install_path()
{
    echo "Installing mysql-test files: $2"
    install -m 644 $1/*.opt $1/*.test Percona-Server/mysql-test/t/
    install -m 644 $1/*.result Percona-Server/mysql-test/r/
    install -m 644 $1/*.inc Percona-Server/mysql-test/include/
}

install_path mysql-test "global"
for test_name in `(cd mysql-test; find . -type d | grep "./" | sed -e "s/.\///g")`; do
    install_path mysql-test/$test_name $test_name
done
echo "Done"
