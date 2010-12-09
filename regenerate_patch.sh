#!/usr/bin/env sh
a=$1
b=$2
patch_name=$3
prefix="[$patch_name]"
big_prefix="$prefix ===== "
echo "$big_prefix Regenerate patch $patch_name based thought $a and $b"
echo "$big_prefix Header is:"
cat $patch_name | head -n7;
echo "$big_prefix Ok, let's go:"
patch_name_new=$patch_name.new;
patch_name_split=$patch_name.split;
echo "$prefix Remove $patch_name_new if exists..."
rm -f $patch_name_new;
echo "$prefix Remove $patch_name_split if exists..."
rm -rf $patch_name_split;
echo "$prefix Create $patch_name_split"
mkdir $patch_name_split;
cd $patch_name_split;
echo "$prefix Add header"
cat ../$patch_name | head -n7 > ../$patch_name_new
echo "$prefix Split to separated files"
for filename in `splitdiff -a -d ../$patch_name | awk '{ print $2 }' | sed -e "s/>//g"`; do
    echo "$prefix extract diff metainformation from $filename"
    a_head=`cat $filename | head -n2 | head -n1`;
    b_head=`cat $filename | head -n2 | tail -n1`;
    a_path=`echo $a_head | awk '{ print $2 }'`;
    b_path=`echo $b_head | awk '{ print $2 }'`;
    diff_string="diff -ruN $a_path $b_path";
    echo "$big_prefix $filename metainformation is:";
    echo "$prefix $diff_string";
    echo "$prefix $a_head"
    echo "$prefix $b_head"
    echo "$big_prefix Update patch by $filename"    
    (cd ..; echo $diff_string >> $patch_name_new);
    (cat $filename | head -n2 | head -n1 >> ../$patch_name_new);
    (cat $filename | head -n2 | tail -n1 >> ../$patch_name_new);
    (cd ..; diff -ruN $a_path $b_path | tail -n+3 >> $patch_name_new);
done;
cd ..;
echo "$prefix Rename temporary patch to final"
cat $patch_name_new > $patch_name;
echo "$prefix Remove temporary files"
rm $patch_name_new;
rm -rf $patch_name_split;
echo "$big_prefix Complete"
