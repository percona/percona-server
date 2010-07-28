#!/usr/bin/env sh
rm -rf a b Percona-Server;
tar zxf mysql-5.1.49.tar.gz;
mv mysql-5.1.49 Percona-Server;
for patch_name in `cat series`; do
    echo "========================================================="
    echo "===== Check patch $patch_name";
    cp -R Percona-Server a;
    cp -R Percona-Server b;
    echo "===== Apply patch $patch_name...";
    patch -p1 -d b < $patch_name >result;
    fail=`cat result | grep FAIL | wc -l`;
    hunk=`cat result | grep Hunk | wc -l`;
    echo "===== Patch $patch_name FAILED: $fail";
    echo "===== Patch $patch_name HUNK:   $hunk";
    if [ $fail -ne 0 ]; then
	echo "==== Patch $patch_name are failed";
	exit 1;
    fi;
    if [ $hunk -ne 0 ]; then
	echo "==== Patch $patch_name need adaptation...";
	find b -name "*.orig" | xargs rm;
	diff -Nur a b > $patch_name;
    fi;
    patch -p1 -d Percona-Server < $patch_name > /dev/null;
    rm -rf a b;
done;
