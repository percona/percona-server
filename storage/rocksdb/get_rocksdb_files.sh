#!/bin/bash
if [ -z "$PATH" ]; then
    export PATH="/sbin:/usr/sbin:/bin:/usr/bin"
fi

MKFILE=`mktemp`
# create and run a simple makefile
# include rocksdb make file relative to the path of this script
echo "include rocksdb/src.mk
FOLLY_DIR = ./third-party/folly
all:" > $MKFILE

if [ -z $1 ]; then
  echo "	@echo \"\$(LIB_SOURCES)\"" >> $MKFILE
else
  echo "	@echo \"\$(LIB_SOURCES)\" \$(FOLLY_SOURCES)" >> $MKFILE
fi
for f in `make --makefile $MKFILE`
do
  echo ./rocksdb/$f
done
rm $MKFILE
