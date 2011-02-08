#!/usr/bin/env bash
#ls -1 *.patch | grep -v repair | xargs bzr revert
MYSQL_VERSION=5.1.55
MYSQL_DIR=mysql-${MYSQL_VERSION}
MYSQL_TAR_GZ=${MYSQL_DIR}.tar.gz
echo "===== Prepare source code for patch's adaptation...";
echo "===== Remove 'a' copy...";
rm -rf a;
echo "===== Remove 'b' copy..."
rm -rf b;
echo "===== Remove 'Percona-Server' copy..."
rm -rf Percona-Server;
echo "===== Unpack ${MYSQL_DIR} to Percona-Server..."
tar zxf ${MYSQL_TAR_GZ};
mv ${MYSQL_DIR} Percona-Server;
echo "===== Prepare 'a' copy..."
cp -R Percona-Server a;
echo "===== Prepare 'b' copy..."
cp -R Percona-Server b;
echo "===== Ok, let's go patch adaptation..."
for patch_name in `cat series`; do
    echo "========================================================="
    echo "===== Check patch $patch_name";
    echo "===== Apply patch $patch_name...";
    patch -p1 -d b < $patch_name >result;
    fail=`cat result | grep FAIL | wc -l`;
    hunk=`cat result | grep Hunk | wc -l`;
    echo "===== Patch $patch_name FAILED: $fail";
    echo "===== Patch $patch_name HUNK:   $hunk";
    if [ $fail -ne 0 ]; then
	echo "===== Patch $patch_name are failed";
	exit 1;
    fi;
    if [ $hunk -ne 0 ]; then
	find b -name "*.orig" | xargs rm;
	./regenerate_patch.sh a b $patch_name
    fi;
    patch -p1 -d Percona-Server < $patch_name > /dev/null;
    patch -p1 -d a < $patch_name > /dev/null;
    echo "===== Patch $patch_name regenerated succesfully"
    if [ $hunk -ne 0 ]; then
	echo "===== Remove temporary 'b' version"
	rm -rf b;
	echo "===== Prepare 'b' copy..."
	cp -R Percona-Server b;
    fi;
done;
