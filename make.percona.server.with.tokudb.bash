#!/bin/bash

function usage() {
    echo "make.percona.server.with.tokudb.bash percona-server-5.6.19-67.0-618-90 tokudb-7.1.7"
}

# download a github repo as a tarball and expand it in a local directory
# arg 1 is the github repo owner
# arg 2 is the github repo name
# arg 3 is the github commit reference
# the local directory name is the same as the github repo name
function get_repo() {
    local owner=$1; local repo=$2; local ref=$3

    curl -L https://api.github.com/repos/$owner/$repo/tarball/$ref --output $repo.tar.gz
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    mkdir $repo
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    tar --extract --gzip --directory $repo --strip-components 1 --file $repo.tar.gz
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    rm -rf $repo.tar.gz
}

function get_source_from_repos() {
    local perconaserver=$1; local tokudb=$2; local buildtype=$3

    # get percona server source
    get_repo Tokutek percona-server-5.6 $tokudb
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    mv percona-server-5.6 $perconaserver-$buildtype
 
    if [ ! -f $perconaserver-$buildtype/Docs/INFO_SRC ] ; then
        echo "revno: $tokudb" >$perconaserver-$buildtype/Docs/INFO_SRC
    else
        # append the tokudb tag to the revno string
        sed -i -e "1,\$s/\(revno:.*\)\$/\1-$tokudb/" $perconaserver-$buildtype/Docs/INFO_SRC
        if [ $? -ne 0 ] ; then test 1 = 0; return ; fi
    fi

    # make the tokudb source tarball
    bash -x make.tokudb.source.tarball.bash $perconaserver $tokudb
    if [ $? -ne 0 ] ; then test 1 = 0; return ; fi

    # extract the tokudb source tarball
    tar xzf $perconaserver.tokudb.tar.gz
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi

    # merge
    target=$PWD/$perconaserver-$buildtype
    pushd $perconaserver.tokudb
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    for d in mysql-test storage plugin; do
        if [ -d $d ] ; then
            for f in $(find $d -type f); do
                targetdir=$(dirname $target/$f)
                if [ ! -d $targetdir ] ; then 
                    mkdir -p $targetdir
                    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
                fi
                if [ $(basename $f) = disabled.def ] ; then
                    # append the tokudb disabled.def to the base disabled.def
                    cat $f >>$target/$f
                    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
                else
                    # replace the base file
                    cp $f $target/$f
                    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
                fi
            done
        fi      
    done
    popd

    # get jemalloc
    get_repo Tokutek jemalloc 3.6.0
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    mv jemalloc jemalloc-3.6.0
}

function build_tarballs_from_source() {
    local perconaserver=$1; local tokudb=$2; local buildtype=$3

    # build
    jemallocdir=$PWD/jemalloc-3.6.0
    pushd $perconaserver-$buildtype
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    buildargs="--with-jemalloc $jemallocdir"
    if [ $buildtype = "debug" ] ; then buildargs="-d $buildargs"; fi
    if [ $buildtype = "debug-valgrind" ] ; then buildargs="-d -v $buildargs"; fi
    bash -x build-ps/build-binary.sh $buildargs .
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    for x in *.gz; do
        md5sum $x >$x.md5
    done
    popd
}

function make_target() {
    local perconaserver=$1; local tokudb=$2; local buildtype=$3

    local builddir=$perconaserver-$tokudb-$buildtype
    rm -rf $builddir
    mkdir $builddir
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    pushd $builddir
    get_source_from_repos $perconaserver $tokudb $buildtype
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    build_tarballs_from_source $perconaserver $tokudb $buildtype
    if [ $? -ne 0 ] ; then test 1 = 0; return; fi
    popd
    mv $builddir/$perconaserver-$buildtype/Percona-Server*.gz* .
    # rm -rf build-$buildtype
}

if [ $# -lt 2 ] ; then usage; exit 1; fi
perconaserver=$1
tokudb=$2
buildtype=
if [ $# -eq 3 ] ;then buildtype=$3; fi

if [ -z "$buildtype" -o "$buildtype" = release ] ; then make_target $perconaserver $tokudb release; fi
if [ -z "$buildtype" -o "$buildtype" = debug ] ; then make_target $perconaserver $tokudb debug; fi
if [ -z "$buildtype" -o "$buildtype" = debug-valgrind ] ; then make_target $perconaserver $tokudb debug-valgrind; fi
