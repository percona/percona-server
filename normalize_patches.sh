#!/usr/bin/env bash

MYSQL_VERSION="$(grep ^MYSQL_VERSION= "Makefile" \
    | cut -d = -f 2)"
PERCONA_SERVER_VERSION="$(grep ^PERCONA_SERVER_VERSION= "Makefile" \
    | cut -d = -f 2)"
PERCONA_SERVER="Percona-Server-${MYSQL_VERSION}-${PERCONA_SERVER_VERSION}"
MYSQL_DIR=mysql-${MYSQL_VERSION}
MYSQL_TAR_GZ=${MYSQL_DIR}.tar.gz

echo "===== Prepare source code for patch's adaptation...";
echo "===== Remove 'a' copy...";
rm -rf a;
echo "===== Remove 'b' copy..."
rm -rf b;
echo "===== Remove '${PERCONA_SERVER}' copy..."
rm -rf ${PERCONA_SERVER};
echo "===== Unpack ${MYSQL_DIR} to ${PERCONA_SERVER}..."
tar zxf ${MYSQL_TAR_GZ};
mv ${MYSQL_DIR} ${PERCONA_SERVER};
echo "===== Prepare 'a' copy..."
cp -R ${PERCONA_SERVER} a;
echo "===== Prepare 'b' copy..."
cp -R ${PERCONA_SERVER} b;
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
	./regenerate_patch.sh a b $patch_name
    fi;
    patch -p1 -d ${PERCONA_SERVER} < $patch_name > /dev/null;
    patch -p1 -d a < $patch_name > /dev/null;
    echo "===== Patch $patch_name regenerated succesfully"
    if [ $hunk -ne 0 ]; then
	echo "===== Remove temporary 'b' version"
	rm -rf b;
	echo "===== Prepare 'b' copy..."
	cp -R ${PERCONA_SERVER} b;
    fi;
done;
