#!/usr/bin/env sh
text -z ${PERCONA_SERVER} && PERCONA_SERVER=Percona-Server
echo PERCONA_SERVER=$PERCONA_SERVER
uninstall_files()
{
    for file in `(cd $1; ls *.$2)`; do
	echo "uninstall $1/$file from ${PERCONA_SERVER}/mysql-test/$3/$file"
	rm ${PERCONA_SERVER}/mysql-test/$3/$file
    done;
}
uninstall_path()
{
    echo "Installing mysql-test files: $2"
    uninstall_files $1 opt t;
    uninstall_files $1 test t;
    uninstall_files $1 result r;
    uninstall_files $1 inc include;
}

uninstall_path mysql-test "global"
for test_name in `cat series`; do
    uninstall_path mysql-test/$test_name $test_name
done
echo "Done"
