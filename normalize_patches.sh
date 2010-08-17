#!/usr/bin/env bash
ls -1 *.patch | grep -v repair | xargs bzr revert
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
	patch_name_new=$patch_name.new;
	patch_name_split=$patch_name.split;
	rm $patch_name_new;
	rm -rf $patch_name_split;
	mkdir $patch_name_split;
	cd $patch_name_split;
	for filename in `splitdiff -a -d ../$patch_name | awk '{ print $2 }' | sed -e "s/>//g"`; do
	    echo $filename;
	    a_head=`cat $filename | head -n2 | head -n1`;
	    b_head=`cat $filename | head -n2 | tail -n1`;
	    echo "PATCH='$patch_name' FILE='$filename' A_HEAD='$a_head' B_HEAD='$b_head'";
	    a_path=`echo $a_head | awk '{ print $2 }'`;
	    b_path=`echo $b_head | awk '{ print $2 }'`;
	    echo "A_PATH='$a_path' B_PATH='$b_path'";
	    (cd ..; echo "diff -Nur $a_path $b_path" >> $patch_name_new);
	    (cat $filename | head -n2 | head -n1 >> ../$patch_name_new);
	    (cat $filename | head -n2 | tail -n1 >> ../$patch_name_new);
	    (cd ..; diff -Nur $a_path $b_path | tail -n+3 >> $patch_name_new);
	done;
	cd ..;
	cat $patch_name_new > $patch_name;
	rm $patch_name_new;
	rm -rf $patch_name_split;
    fi;
    patch -p1 -d Percona-Server < $patch_name > /dev/null;
    rm -rf a b;
done;
