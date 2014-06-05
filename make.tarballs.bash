#!/bin/bash
if [ $# -ne 1 ] ; then exit 1; fi
branch=$1

# release
pushd $HOME/jemalloc-3.6.0
if [ $? -ne 0 ] ; then exit 1; fi
make clean
popd

git clone -b $branch git@github.com:Tokutek/percona-server-5.6 $branch-release
if [ $? -ne 0 ] ; then exit 1; fi
pushd $branch-release
if [ $? -ne 0 ] ; then exit 1; fi
nohup bash -x build-ps/build-binary.sh --with-jemalloc $HOME/jemalloc-3.6.0 . >build.out
if [ $? -ne 0 ] ; then exit 1; fi
for x in *.gz; do
    md5sum $x >$x.md5
done
popd

# debug
pushd $HOME/jemalloc-3.6.0
if [ $? -ne 0 ] ; then exit 1; fi
make clean
popd

git clone -b $branch git@github.com:Tokutek/percona-server-5.6 $branch-debug
if [ $? -ne 0 ] ; then exit 1; fi
pushd $branch-debug
if [ $? -ne 0 ] ; then exit 1; fi
nohup bash -x build-ps/build-binary.sh -d --with-jemalloc $HOME/jemalloc-3.6.0 . >build.out
if [ $? -ne 0 ] ; then exit 1; fi
for x in *.gz; do
    md5sum $x >$x.md5
done
popd

# debug valgrind
pushd $HOME/jemalloc-3.6.0
if [ $? -ne 0 ] ; then exit 1; fi
make clean
popd

git clone -b $branch git@github.com:Tokutek/percona-server-5.6 $branch-debug-valgrind
if [ $? -ne 0 ] ; then exit 1; fi
pushd $branch-debug-valgrind
if [ $? -ne 0 ] ; then exit 1; fi
nohup bash -x build-ps/build-binary.sh -d -v --with-jemalloc $HOME/jemalloc-3.6.0 . >build.out
if [ $? -ne 0 ] ; then exit 1; fi
for x in *.gz; do
    md5sum $x >$x.md5
done
popd

