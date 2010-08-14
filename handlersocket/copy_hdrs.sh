#!/bin/bash

if [ "$2" == "" ]; then
	echo "Usage: $0 MYSQL_DIR VERSION";
	exit 1;
fi
major=`echo $2 | cut -d '.' -f 1`
subdirs="include regex sql"
if [ "$major" == "6" ]; then
	subdirs="include regex sql storage"
fi
rm -rf "mysql-hdrs-$2"
mkdir "mysql-hdrs-$2"
for i in $subdirs; do
	mkdir "mysql-hdrs-$2/$i"
	wd=`pwd`
	pushd "$1/$i" && tar c `find -name '*.h'` | tar x -C "$wd/mysql-hdrs-$2/$i/" && popd
done

