#!/bin/bash

# make the tokudb source tarball from the Tokutek github repositories

function usage() {
    echo "make.tokudb.source.tarball.bash percona-server-5.6.19-67.0-618-90 tokudb-7.1.7"
}

# download a github repo as a tarball and expand it in a local directory
# arg 1 is the github repo owner
# arg 2 is the github repo name
# arg 3 is the github commit reference
# the local directory name is the same as the github repo name
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
        rm -rf $repo.tar.gz
    fi
}

if [ $# -ne 2 ] ; then usage; exit 1; fi
staging=$1.tokudb
ref=$2

get_repo Tokutek tokudb-engine $ref
if [ $? -ne 0 ] ; then exit 1; fi

get_repo Tokutek ft-index $ref
if [ $? -ne 0 ] ; then exit 1; fi

get_repo Tokutek tokudb-percona-server-5.6 $ref
if [ $? -ne 0 ] ; then exit 1; fi

get_repo Tokutek tokudb-backup-plugin $ref
if [ $? -ne 0 ] ; then exit 1; fi

# merge the repos into the staging directory
if [ ! -d $staging ] ; then
    mkdir $staging
    if [ $? -ne 0 ] ; then exit 1; fi
    cp -r tokudb-engine/storage $staging
    cp -r tokudb-engine/mysql-test $staging
    mv ft-index $staging/storage/tokudb
    cp -r tokudb-percona-server-5.6/mysql-test $staging
    mkdir $staging/plugin
    mv tokudb-backup-plugin $staging/plugin

    # set the tokudb version to the github ref in the cmake file
    pushd $staging/storage/tokudb
    if [ $? -ne 0 ] ; then exit 1; fi
    echo "SET(TOKUDB_VERSION $ref)" >new.CMakeLists.txt
    cat CMakeLists.txt >>new.CMakeLists.txt
    mv new.CMakeLists.txt CMakeLists.txt
    popd
fi

# make the tokudb source tarball and md5 checksum file
tar czf $staging.tar.gz $staging
if [ $? -ne 0 ] ; then exit 1; fi
md5sum $staging.tar.gz >$staging.tar.gz.md5
if [ $? -ne 0 ] ; then exit 1; fi

#cleanup
rm -rf tokudb-engine* ft-index* tokudb-percona-server-5.6* $staging
