#!/usr/bin/env bash

if [ $# -ne 1 ] ; then
    exit 1
fi
target=$1

for d in mysql-test ; do
    if [ -d $d ] ; then
        for f in $(find $d -type f); do
            if [[ $f =~ disabled.def ]] ; then
                echo "cat $f >>$target/$f"
            else
                echo "cp $f $target/$f"
            fi
         done
    fi      
done