#!/bin/bash

function usage() {
    echo "make.percona.server.with.tokudb.bash percona-server-5.6.19-67.0-618-90 tokudb-7.1.7"
}

function get_repo() {
    local owner=$1; local repo=$2; local ref=$3
    if [ ! -f $repo.tar.gz ] ; then
        rm -rf $repo
        curl -L https://api.github.com/repos/$owner/$repo/tarball/$ref --output $repo.tar.gz
        if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    fi
    if [ ! -d $repo ] ; then
        mkdir $repo
        if [ $? -ne 0 ] ; then test 1 = 0; return; fi
        tar --extract --gzip --directory $repo --strip-components 1 --file $repo.tar.gz
        if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    fi
}

function get_source_from_repos() {
    local perconaserver=$1; local tokudb=$2; local buildtype=$3

    # clean jemalloc
    pushd $HOME/jemalloc-3.6.0
    if [ $? -ne 0 ] ; then exit 1; fi
    make clean
    popd

    # get percona server source
    if [ ! -d $perconaserver-$buildtype ] ; then
        get_repo Tokutek percona-server-5.6 $perconaserver
        if [ $? -ne 0 ] ; then test 1 = 0; return; fi
        mv percona-server-5.6 $perconaserver-$buildtype
        rm -rf percona-server-5.6.tar.gz
    fi

    # make the tokudb source tarball
    if [ ! -f $perconaserver.tokudb.tar.gz ] ; then
        bash -x make.tokudb.source.tarball.bash $perconaserver $tokudb
        if [ $? -ne 0 ] ; then test 1 = 0; return ; fi
    fi

    # merge
    if [ ! -d $perconaserver.tokudb ] ; then
        tar xzf $perconaserver.tokudb.tar.gz
        if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    fi
    target=$PWD/$perconaserver-$buildtype
    pushd $perconaserver.tokudb
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    for d in mysql-test storage; do
        if [ -d $d ] ; then
            for f in $(find $d -type f); do
                targetdir=$(dirname $target/$f)
                if [ ! -d $targetdir ] ; then 
                    mkdir -p $targetdir
                    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
                fi
                if [[ $f =~ disabled.def ]] ; then
                    cat $f >>$target/$f
                    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
                else
                    cp $f $target/$f
                    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
                fi
            done
        fi      
    done
    popd
}

function build_tarballs_from_source() {
    local perconaserver=$1; local tokudb=$2; local buildtype=$3

    # build
    pushd $perconaserver-$buildtype
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    buildargs="--with-jemalloc $HOME/jemalloc-3.6.0"
    if [ $buildtype = "debug" ] ; then buildargs="-d $buildargs"; fi
    if [ $buildtype = "debug-valgrind" ] ; then buildargs="-d -v $buildargs"; fi
    bash -x build-ps/build-binary.sh $buildargs . >build.out 2>&1
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    for x in *.gz; do
        md5sum $x >$x.md5
    done
    popd

    # cleanup
    cp $perconaserver-$buildtype/Percona-Server*.gz* .
    rm -rf ${perconaserver}*
}

if [ $# -ne 2 ] ; then usage; exit 1; fi
perconaserver=$1
tokudb=$2

# release
get_source_from_repos $perconaserver $tokudb release
if [ $? -ne 0 ] ; then exit 1; fi
build_tarballs_from_source $perconaserver $tokudb release
if [ $? -ne 0 ] ; then exit 1; fi

# debug
get_source_from_repos $perconaserver $tokudb debug
if [ $? -ne 0 ] ; then exit 1; fi
build_tarballs_from_source $perconaserver $tokudb debug
if [ $? -ne 0 ] ; then exit 1; fi

# debug valgrind
get_source_from_repos $perconaserver $tokudb debug-valgrind
if [ $? -ne 0 ] ; then exit 1; fi
build_tarballs_from_source $perconaserver $tokudb debug-valgrind
if [ $? -ne 0 ] ; then exit 1; fi
