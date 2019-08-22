#!/bin/sh

shell_quote_string() {
  echo "$1" | sed -e 's,\([^a-zA-Z0-9/_.=-]\),\\\1,g'
}

usage () {
    cat <<EOF
Usage: $0 [OPTIONS]
    The following options may be given :
        --builddir=DIR      Absolute path to the dir where all actions will be performed
        --get_sources       Source will be downloaded from github
        --build_src_rpm     If it is 1 src rpm will be built
        --build_source_deb  If it is 1 source deb package will be built
        --build_rpm         If it is 1 rpm will be built
        --build_deb         If it is 1 deb will be built
        --build_tarball     If it is 1 tarball will be built
        --install_deps      Install build dependencies(root previlages are required)
        --branch            Branch for build
        --repo              Repo for build
        --yassl             build tarball with yassl
        --perconaft_repo    PerconaFT repo
        --perconaft_branch  Branch for PerconaFT
        --tokubackup_repo   TokuBackup repo
        --tokubackup_branch Btanch for TokuBackup
        --rpm_release       RPM version( default = 1)
        --deb_release       DEB version( default = 1)
        --help) usage ;;
Example $0 --builddir=/tmp/PS57 --get_sources=1 --build_src_rpm=1 --build_rpm=1
EOF
        exit 1
}

append_arg_to_args () {
  args="$args "$(shell_quote_string "$1")
}

parse_arguments() {
    pick_args=
    if test "$1" = PICK-ARGS-FROM-ARGV
    then
        pick_args=1
        shift
    fi
  
    for arg do
        val=$(echo "$arg" | sed -e 's;^--[^=]*=;;')
        case "$arg" in
            # these get passed explicitly to mysqld
            --builddir=*) WORKDIR="$val" ;;
            --build_src_rpm=*) SRPM="$val" ;;
            --build_source_deb=*) SDEB="$val" ;;
            --build_rpm=*) RPM="$val" ;;
            --build_deb=*) DEB="$val" ;;
            --get_sources=*) SOURCE="$val" ;;
            --build_tarball=*) TARBALL="$val" ;;
            --branch=*) BRANCH="$val" ;;
            --repo=*) REPO="$val" ;;
            --install_deps=*) INSTALL="$val" ;;
            --yassl=*) YASSL="$val" ;;
            --perconaft_branch=*) PERCONAFT_BRANCH="$val" ;;
            --tokubackup_branch=*) 	TOKUBACKUP_BRANCH="$val" ;;
            --perconaft_repo=*) PERCONAFT_REPO="$val" ;;
            --tokubackup_repo=*) TOKUBACKUP_REPO="$val" ;;
            --rpm_release=*) RPM_RELEASE="$val" ;;
            --deb_release=*) DEB_RELEASE="$val" ;;
            --help) usage ;;      
            *)
              if test -n "$pick_args"
              then
                  append_arg_to_args "$arg"
              fi
              ;;
        esac
    done
}

check_workdir(){
    if [ "x$WORKDIR" = "x$CURDIR" ]
    then
        echo >&2 "Current directory cannot be used for building!"
        exit 1
    else
        if ! test -d "$WORKDIR"
        then
            echo >&2 "$WORKDIR is not a directory."
            exit 1
        fi
    fi
    return
}

add_percona_yum_repo(){
    if [ ! -f /etc/yum.repos.d/percona-dev.repo ]
    then
        curl -o /etc/yum.repos.d/percona-dev.repo https://jenkins.percona.com/yum-repo/percona-dev.repo
	sed -i 's:$basearch:x86_64:g' /etc/yum.repos.d/percona-dev.repo

    fi
    return
}

add_percona_apt_repo(){
  if [ ! -f /etc/apt/sources.list.d/percona-dev.list ]; then
    curl -o /etc/apt/sources.list.d/percona-dev.list https://jenkins.percona.com/apt-repo/percona-dev.list.template
    sed -i "s:@@DIST@@:$OS_NAME:g" /etc/apt/sources.list.d/percona-dev.list
  fi
 
  wget -q -O - http://jenkins.percona.com/apt-repo/8507EFA5.pub | sudo apt-key add -
  wget -q -O - http://jenkins.percona.com/apt-repo/CD2EFD2A.pub | sudo apt-key add -
  return
}

get_sources(){
    cd "${WORKDIR}"
    if [ "${SOURCE}" = 0 ]
    then
        echo "Sources will not be downloaded"
        return 0
    fi

    git clone "$REPO"
    retval=$?
    if [ $retval != 0 ]
    then
        echo "There were some issues during repo cloning from github. Please retry one more time"
        exit 1
    fi

    cd percona-server
    if [ ! -z "$BRANCH" ]
    then
        git reset --hard
        git clean -xdf
        git checkout "$BRANCH"
    fi

    REVISION=$(git rev-parse --short HEAD)
    git reset --hard
    #
    source VERSION
    cat VERSION > ../percona-server-5.7.properties
    echo "REVISION=${REVISION}" >> ../percona-server-5.7.properties
    BRANCH_NAME="${BRANCH}"
    echo "BRANCH_NAME=${BRANCH_NAME}" >> ../percona-server-5.7.properties
    export PRODUCT=Percona-Server-${MYSQL_VERSION_MAJOR}.${MYSQL_VERSION_MINOR}
    echo "PRODUCT=Percona-Server-${MYSQL_VERSION_MAJOR}.${MYSQL_VERSION_MINOR}" >> ../percona-server-5.7.properties
    export PRODUCT_FULL=${PRODUCT}.${MYSQL_VERSION_PATCH}${MYSQL_VERSION_EXTRA}
    echo "PRODUCT_FULL=${PRODUCT}.${MYSQL_VERSION_PATCH}${MYSQL_VERSION_EXTRA}" >> ../percona-server-5.7.properties
    echo "BUILD_NUMBER=${BUILD_NUMBER}" >> ../percona-server-5.7.properties
    echo "BUILD_ID=${BUILD_ID}" >> ../percona-server-5.7.properties
    echo "PERCONAFT_REPO=${PERCONAFT_REPO}" >> ../percona-server-5.7.properties
    echo "PERCONAFT_BRANCH=${PERCONAFT_BRANCH}" >> ../percona-server-5.7.properties
    echo "TOKUBACKUP_REPO=${TOKUBACKUP_REPO}" >> ../percona-server-5.7.properties
    echo "TOKUBACKUP_BRANCH=${TOKUBACKUP_BRANCH}" >> ../percona-server-5.7.properties
    echo "TOKUDB_VERSION=${MYSQL_VERSION_MAJOR}.${MYSQL_VERSION_MINOR}.${MYSQL_VERSION_PATCH}${MYSQL_VERSION_EXTRA}" >> ../percona-server-5.7.properties
    BOOST_PACKAGE_NAME=$(cat cmake/boost.cmake|grep "SET(BOOST_PACKAGE_NAME"|awk -F '"' '{print $2}')
    echo "BOOST_PACKAGE_NAME=${BOOST_PACKAGE_NAME}" >> ../percona-server-5.7.properties
    echo "RPM_RELEASE=${RPM_RELEASE}" >> ../percona-server-5.7.properties
    echo "DEB_RELEASE=${DEB_RELEASE}" >> ../percona-server-5.7.properties

    if [ -z "${DESTINATION:-}" ]; then
        export DESTINATION=experimental
    fi
    TIMESTAMP=$(date "+%Y%m%d-%H%M%S")
    echo "DESTINATION=${DESTINATION}" >> ../percona-server-5.7.properties
    echo "UPLOAD=UPLOAD/${DESTINATION}/BUILDS/${PRODUCT}/${PRODUCT_FULL}/${BRANCH_NAME}/${REVISION}/${TIMESTAMP}" >> ../percona-server-5.7.properties

    rm -rf storage/tokudb/PerconaFT
    rm -rf plugin/tokudb-backup-plugin/Percona-TokuBackup
    git submodule init
    git submodule update
    rm -rf storage/tokudb/PerconaFT
    rm -rf plugin/tokudb-backup-plugin/Percona-TokuBackup
    if [ ${PERCONAFT_REPO} = 0 ]; then
        PERCONAFT_REPO=''
    fi
    if [ ${TOKUBACKUP_REPO} = 0 ]; then
        TOKUBACKUP_REPO=''
    fi

    if [ -z ${PERCONAFT_REPO} -a -z ${TOKUBACKUP_REPO} ]; then
        mkdir plugin/tokudb-backup-plugin/Percona-TokuBackup
        mkdir storage/tokudb/PerconaFT
        git submodule init
        git submodule update
        cd storage/tokudb/PerconaFT
        git fetch origin
        git checkout ${PERCONAFT_BRANCH}
        if [ ${PERCONAFT_BRANCH} = "master" ]; then
            git pull
        fi
        cd ${WORKDIR}/percona-server
        #
        cd plugin/tokudb-backup-plugin/Percona-TokuBackup
        git fetch origin
        git checkout ${TOKUBACKUP_BRANCH}
        if [ ${TOKUBACKUP_BRANCH} = "master" ]; then
            git pull
        fi
        cd ${WORKDIR}/percona-server
    else
        cd storage/tokudb
        git clone ${PERCONAFT_REPO}
        cd PerconaFT
        git checkout ${PERCONAFT_BRANCH}
        cd ${WORKDIR}/percona-server
        #
        cd plugin/tokudb-backup-plugin
        git clone ${TOKUBACKUP_REPO}
        cd Percona-TokuBackup
        git checkout ${TOKUBACKUP_BRANCH}
        cd ${WORKDIR}/percona-server
    fi
    #
    git submodule update
    cmake . -DDOWNLOAD_BOOST=1 -DWITH_BOOST=${WORKDIR}/build-ps/boost
    make dist
    #
    EXPORTED_TAR=$(basename $(find . -type f -name percona-server*.tar.gz | sort | tail -n 1))
    #
    PSDIR=${EXPORTED_TAR%.tar.gz}
    rm -fr ${PSDIR}
    tar xzf ${EXPORTED_TAR}
    rm -f ${EXPORTED_TAR}
    # add git submodules because make dist uses git archive which doesn't include them
    rsync -av storage/tokudb/PerconaFT ${PSDIR}/storage/tokudb --exclude .git
    rsync -av plugin/tokudb-backup-plugin/Percona-TokuBackup ${PSDIR}/plugin/tokudb-backup-plugin --exclude .git
    rsync -av storage/rocksdb/rocksdb/ ${PSDIR}/storage/rocksdb/rocksdb --exclude .git
    rsync -av storage/rocksdb/third_party/lz4/ ${PSDIR}/storage/rocksdb/third_party/lz4 --exclude .git
    rsync -av storage/rocksdb/third_party/zstd/ ${PSDIR}/storage/rocksdb/third_party/zstd --exclude .git
    #
    cd ${PSDIR}
    # set tokudb version - can be seen with show variables like '%version%'
    sed -i "1s/^/SET(TOKUDB_VERSION ${TOKUDB_VERSION})\n/" storage/tokudb/CMakeLists.txt
    #
    sed -i "s:@@PERCONA_VERSION_EXTRA@@:${MYSQL_VERSION_EXTRA#-}:g" build-ps/debian/rules
    sed -i "s:@@REVISION@@:${REVISION}:g" build-ps/debian/rules
    sed -i "s:@@TOKUDB_BACKUP_VERSION@@:${TOKUDB_VERSION}:g" build-ps/debian/rules
    sed -i "s:@@PERCONA_VERSION_EXTRA@@:${MYSQL_VERSION_EXTRA#-}:g" build-ps/debian/rules.notokudb
    sed -i "s:@@REVISION@@:${REVISION}:g" build-ps/debian/rules.notokudb
    #
    sed -i "s:@@PERCONA_VERSION_EXTRA@@:${MYSQL_VERSION_EXTRA#-}:g" build-ps/ubuntu/rules
    sed -i "s:@@REVISION@@:${REVISION}:g" build-ps/ubuntu/rules
    sed -i "s:@@TOKUDB_BACKUP_VERSION@@:${TOKUDB_VERSION}:g" build-ps/ubuntu/rules
    sed -i "s:@@PERCONA_VERSION_EXTRA@@:${MYSQL_VERSION_EXTRA#-}:g" build-ps/ubuntu/rules.notokudb
    sed -i "s:@@REVISION@@:${REVISION}:g" build-ps/ubuntu/rules.notokudb
    #
    sed -i "s:@@MYSQL_VERSION@@:${MYSQL_VERSION_MAJOR}.${MYSQL_VERSION_MINOR}.${MYSQL_VERSION_PATCH}:g" build-ps/percona-server.spec
    sed -i "s:@@PERCONA_VERSION@@:${MYSQL_VERSION_EXTRA#-}:g" build-ps/percona-server.spec
    sed -i "s:@@REVISION@@:${REVISION}:g" build-ps/percona-server.spec
    sed -i "s:@@RPM_RELEASE@@:${RPM_RELEASE}:g" build-ps/percona-server.spec
    sed -i "s:@@BOOST_PACKAGE_NAME@@:${BOOST_PACKAGE_NAME}:g" build-ps/percona-server.spec
    cd ${WORKDIR}/percona-server
    tar --owner=0 --group=0 --exclude=.bzr --exclude=.git -czf ${PSDIR}.tar.gz ${PSDIR}
    
    mkdir $WORKDIR/source_tarball
    mkdir $CURDIR/source_tarball
    cp ${PSDIR}.tar.gz $WORKDIR/source_tarball
    cp ${PSDIR}.tar.gz $CURDIR/source_tarball
    cd $CURDIR
    rm -rf percona-server  
    return
}

get_system(){
    if [ -f /etc/redhat-release ]; then
        RHEL=$(rpm --eval %rhel)
        ARCH=$(echo $(uname -m) | sed -e 's:i686:i386:g')
        OS_NAME="el$RHEL"
        OS="rpm"
    else
        ARCH=$(uname -m)
        OS_NAME="$(lsb_release -sc)"
        OS="deb"
    fi
    return
}

install_deps() {
    if [ $INSTALL = 0 ]
    then
        echo "Dependencies will not be installed"
        return;
    fi
    if [ $( id -u ) -ne 0 ]
    then
        echo "It is not possible to instal dependencies. Please run as root"
        exit 1
    fi
    CURPLACE=$(pwd)
    
    if [ "x$OS" = "xrpm" ]; then
        RHEL=$(rpm --eval %rhel)
        ARCH=$(echo $(uname -m) | sed -e 's:i686:i386:g')
        add_percona_yum_repo
        if [ ${RHEL} -lt 8 ]; then
            yum -y install https://repo.percona.com/yum/percona-release-latest.noarch.rpm || true
            percona-release enable origin release
            yum -y install epel-release
            yum -y install git pkg-config numactl-devel rpm-build gcc-c++ gperf ncurses-devel perl readline-devel openssl-devel jemalloc 
            yum -y install time zlib-devel libaio-devel bison cmake pam-devel libeatmydata jemalloc-devel
            yum -y install perl-Time-HiRes libcurl-devel openldap-devel unzip wget libcurl-devel 
            yum -y install perl-Env perl-Data-Dumper perl-JSON MySQL-python perl-Digest perl-Digest-MD5 perl-Digest-Perl-MD5 || true
            if [ ${RHEL} = 6 ]; then
		if [ $(uname -m) = x86_64 ]; then
                    yum -y install percona-devtoolset-gcc percona-devtoolset-gcc-c++ percona-devtoolset-binutils
		else
                    wget -O /etc/yum.repos.d/slc6-devtoolset.repo http://linuxsoft.cern.ch/cern/devtoolset/slc6-devtoolset.repo
                    wget -O /etc/pki/rpm-gpg/RPM-GPG-KEY-cern https://raw.githubusercontent.com/cms-sw/cms-docker/master/slc6-vanilla/RPM-GPG-KEY-cern
                    yum -y install  devtoolset-2-gcc-c++ devtoolset-2-binutils libevent2-devel
	        fi
	    fi
        else
            yum -y install perl.x86_64
            yum -y install binutils gcc gcc-c++ tar rpm-build rsync bison glibc glibc-devel libstdc++-devel libtirpc-devel make openssl-devel pam-devel perl perl-JSON perl-Memoize 
            yum -y install automake autoconf cmake jemalloc jemalloc-devel
	    yum -y install libcurl-devel openldap-devel selinux-policy-devel
	    yum -y install libaio-devel ncurses-devel numactl-devel readline-devel time
	    yum -y install rpcgen libtirpc-devel
        fi
        if [ "x$RHEL" = "x6" ]; then
            yum -y install Percona-Server-shared-56  
        fi
    else
        apt-get -y install dirmngr || true
        add_percona_apt_repo
      	apt-get update
        apt-get -y install dirmngr || true
        apt-get -y install lsb-release wget
        export DEBIAN_FRONTEND="noninteractive"
        export DIST="$(lsb_release -sc)"
	    until sudo apt-get update; do
    	    sleep 1
            echo "waiting"
        done
        apt-get -y purge eatmydata || true
        echo "deb http://jenkins.percona.com/apt-repo/ ${DIST} main" > percona-dev.list
        mv -f percona-dev.list /etc/apt/sources.list.d/
        wget -q -O - http://jenkins.percona.com/apt-repo/8507EFA5.pub | sudo apt-key add -
        wget -q -O - http://jenkins.percona.com/apt-repo/CD2EFD2A.pub | sudo apt-key add -
        apt-get update
        apt-get -y install psmisc pkg-config
        apt-get -y install libsasl2-dev libsasl2-modules:amd64 libsasl2-modules-ldap || apt-get -y install libsasl2-modules libsasl2-modules-ldap libsasl2-dev
        apt-get -y install dh-systemd || true
        apt-get -y install curl bison cmake perl libssl-dev gcc g++ libaio-dev libldap2-dev libwrap0-dev gdb unzip gawk
        apt-get -y install lsb-release libmecab-dev libncurses5-dev libreadline-dev libpam-dev zlib1g-dev
        apt-get -y install libldap2-dev libnuma-dev libjemalloc-dev libeatmydata libc6-dbg valgrind libjson-perl python-mysqldb libsasl2-dev

        apt-get -y install libmecab2 mecab mecab-ipadic
        apt-get -y install build-essential devscripts libnuma-dev
        apt-get -y install cmake autotools-dev autoconf automake build-essential devscripts debconf debhelper fakeroot 
        apt-get -y install libcurl4-openssl-dev
        if [ "x${DIST}" = "xcosmic" -o "x${DIST}" = "xbionic" -o "x${DIST}" = "xdisco" -o "x${DIST}" = "xbuster"  ]; then
            apt-get -y install libeatmydata1
        fi
    fi
    return;
}

get_tar(){
    TARBALL=$1
    TARFILE=$(basename $(find $WORKDIR/$TARBALL -name 'percona-server*.tar.gz' | sort | tail -n1))
    if [ -z $TARFILE ]
    then
        TARFILE=$(basename $(find $CURDIR/$TARBALL -name 'percona-server*.tar.gz' | sort | tail -n1))
        if [ -z $TARFILE ]
        then
            echo "There is no $TARBALL for build"
            exit 1
        else
            cp $CURDIR/$TARBALL/$TARFILE $WORKDIR/$TARFILE
        fi
    else
        cp $WORKDIR/$TARBALL/$TARFILE $WORKDIR/$TARFILE
    fi
    return
}

get_deb_sources(){
    param=$1
    echo $param
    FILE=$(basename $(find $WORKDIR/source_deb -name "percona-server*.$param" | sort | tail -n1))
    if [ -z $FILE ]
    then
        FILE=$(basename $(find $CURDIR/source_deb -name "percona-server*.$param" | sort | tail -n1))
        if [ -z $FILE ]
        then
            echo "There is no sources for build"
            exit 1
        else
            cp $CURDIR/source_deb/$FILE $WORKDIR/
        fi
    else
        cp $WORKDIR/source_deb/$FILE $WORKDIR/
    fi
    return
}

build_srpm(){
    if [ $SRPM = 0 ]
    then
        echo "SRC RPM will not be created"
        return;
    fi
    if [ "x$OS" = "xdeb" ]
    then
        echo "It is not possible to build src rpm here"
        exit 1
    fi
    cd $WORKDIR
    get_tar "source_tarball"
    rm -fr rpmbuild
    ls | grep -v percona-server*.tar.* | xargs rm -rf
    mkdir -vp rpmbuild/{SOURCES,SPECS,BUILD,SRPMS,RPMS}

    TARFILE=$(basename $(find . -name 'percona-server-*.tar.gz' | sort | tail -n1))
    NAME=$(echo ${TARFILE}| awk -F '-' '{print $1"-"$2}')
    VERSION=$(echo ${TARFILE}| awk -F '-' '{print $3}')
    #
    SHORTVER=$(echo ${VERSION} | awk -F '.' '{print $1"."$2}')
    TMPREL=$(echo ${TARFILE}| awk -F '-' '{print $4}')
    RELEASE=${TMPREL%.tar.gz}
    #
    mkdir -vp rpmbuild/{SOURCES,SPECS,BUILD,SRPMS,RPMS}
    #
    cd ${WORKDIR}/rpmbuild/SPECS
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/*.spec' --strip=2
    #
    cd ${WORKDIR}/rpmbuild/SOURCES
    #wget http://downloads.sourceforge.net/boost/${BOOST_PACKAGE_NAME}.tar.bz2
    wget http://jenkins.percona.com/downloads/boost/${BOOST_PACKAGE_NAME}.tar.gz
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/rpm/*.patch' --strip=3
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/rpm/filter-provides.sh' --strip=3
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/rpm/filter-requires.sh' --strip=3
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/rpm/mysql_config.sh' --strip=3
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/rpm/my_config.h' --strip=3
    #
    cd ${WORKDIR}
    #
    mv -fv ${TARFILE} ${WORKDIR}/rpmbuild/SOURCES
    #
    rpmbuild -bs --define "_topdir ${WORKDIR}/rpmbuild" --define "dist .generic" rpmbuild/SPECS/percona-server.spec
    #

    mkdir -p ${WORKDIR}/srpm
    mkdir -p ${CURDIR}/srpm
    cp rpmbuild/SRPMS/*.src.rpm ${CURDIR}/srpm
    cp rpmbuild/SRPMS/*.src.rpm ${WORKDIR}/srpm
    return
}

build_mecab_lib(){
    MECAB_TARBAL="mecab-0.996.tar.gz"
    MECAB_LINK="http://jenkins.percona.com/downloads/mecab/${MECAB_TARBAL}"
    MECAB_DIR="${WORKDIR}/${MECAB_TARBAL%.tar.gz}"
    MECAB_INSTALL_DIR="${WORKDIR}/mecab-install"
    rm -f ${MECAB_TARBAL}
    rm -rf ${MECAB_DIR}
    rm -rf ${MECAB_INSTALL_DIR}
    mkdir ${MECAB_INSTALL_DIR}
    wget ${MECAB_LINK}
    tar xf ${MECAB_TARBAL}
    cd ${MECAB_DIR}
    ./configure --with-pic --prefix=/usr
    make
    make check
    make DESTDIR=${MECAB_INSTALL_DIR} install
    cd ../${MECAB_INSTALL_DIR}
    if [ -d usr/lib64 ]; then
        mkdir -p usr/lib
        mv usr/lib64/* usr/lib
    fi
    cd ${WORKDIR}
}

build_mecab_dict(){
    MECAB_IPADIC_TARBAL="mecab-ipadic-2.7.0-20070801.tar.gz"
    MECAB_IPADIC_LINK="http://jenkins.percona.com/downloads/mecab/${MECAB_IPADIC_TARBAL}"
    MECAB_IPADIC_DIR="${WORKDIR}/${MECAB_IPADIC_TARBAL%.tar.gz}"
    rm -f ${MECAB_IPADIC_TARBAL}
    rm -rf ${MECAB_IPADIC_DIR}
    wget ${MECAB_IPADIC_LINK}
    tar xf ${MECAB_IPADIC_TARBAL}
    cd ${MECAB_IPADIC_DIR}
    # these two lines should be removed if proper packages are created and used for builds
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${MECAB_INSTALL_DIR}/usr/lib
    sed -i "/MECAB_DICT_INDEX=\"/c\MECAB_DICT_INDEX=\"${MECAB_INSTALL_DIR}\/usr\/libexec\/mecab\/mecab-dict-index\"" configure
    #
    ./configure --with-mecab-config=${MECAB_INSTALL_DIR}/usr/bin/mecab-config
    make
    make DESTDIR=${MECAB_INSTALL_DIR} install
    cd ../
    cd ${MECAB_INSTALL_DIR}
    if [ -d usr/lib64 ]; then
        mv usr/lib64/* usr/lib
    fi
    cd ${WORKDIR}
}

build_rpm(){
    if [ $RPM = 0 ]
    then
        echo "RPM will not be created"
        return;
    fi
    if [ "x$OS" = "xdeb" ]
    then
        echo "It is not possible to build rpm here"
        exit 1
    fi
    SRC_RPM=$(basename $(find $WORKDIR/srpm -name 'Percona-Server-*.src.rpm' | sort | tail -n1))
    if [ -z $SRC_RPM ]
    then
        SRC_RPM=$(basename $(find $CURDIR/srpm -name 'Percona-Server-*.src.rpm' | sort | tail -n1))
        if [ -z $SRC_RPM ]
        then
            echo "There is no src rpm for build"
            echo "You can create it using key --build_src_rpm=1"
            exit 1
        else
            cp $CURDIR/srpm/$SRC_RPM $WORKDIR
        fi
    else
        cp $WORKDIR/srpm/$SRC_RPM $WORKDIR
    fi
    cd $WORKDIR
    rm -fr rpmbuild
    mkdir -vp rpmbuild/{SOURCES,SPECS,BUILD,SRPMS,RPMS}
    cp $SRC_RPM rpmbuild/SRPMS/

    RHEL=$(rpm --eval %rhel)
    ARCH=$(echo $(uname -m) | sed -e 's:i686:i386:g')
    #
    echo "RHEL=${RHEL}" >> percona-server-5.7.properties
    echo "ARCH=${ARCH}" >> percona-server-5.7.properties
    #
    SRCRPM=$(basename $(find . -name '*.src.rpm' | sort | tail -n1))
    mkdir -vp rpmbuild/{SOURCES,SPECS,BUILD,SRPMS,RPMS}
    #
    mv *.src.rpm rpmbuild/SRPMS
    #
    if [ ${RHEL} = 6 ]; then
        if [ ${ARCH} = x86_64 ]; then
            source /opt/percona-devtoolset/enable
	else
	    source /opt/rh/devtoolset-2/enable
        fi
    fi

    build_mecab_lib
    build_mecab_dict

    cd ${WORKDIR}
    #
    if [ ${ARCH} = x86_64 ]; then
        rpmbuild --define "_topdir ${WORKDIR}/rpmbuild" --define "dist .el${RHEL}" --define "with_mecab ${MECAB_INSTALL_DIR}/usr" --rebuild rpmbuild/SRPMS/${SRCRPM}
    else
        rpmbuild --define "_topdir ${WORKDIR}/rpmbuild" --define "dist .el${RHEL}" --define "with_tokudb 0" --define "with_rocksdb 0" --define "with_mecab ${MECAB_INSTALL_DIR}/usr" --rebuild rpmbuild/SRPMS/${SRCRPM}
    fi
    return_code=$?
    if [ $return_code != 0 ]; then
        exit $return_code
    fi
    mkdir -p ${WORKDIR}/rpm
    mkdir -p ${CURDIR}/rpm
    cp rpmbuild/RPMS/*/*.rpm ${WORKDIR}/rpm
    cp rpmbuild/RPMS/*/*.rpm ${CURDIR}/rpm
    
}

build_source_deb(){
    if [ $SDEB = 0 ]
    then
        echo "source deb package will not be created"
        return;
    fi
    if [ "x$OS" = "xrpm" ]
    then
        echo "It is not possible to build source deb here"
        exit 1
    fi
    rm -rf percona-server*
    get_tar "source_tarball"
    rm -f *.dsc *.orig.tar.gz *.debian.tar.gz *.changes
    #

    TARFILE=$(basename $(find . -name 'percona-server-*.tar.gz' | grep -v tokudb | sort | tail -n1))

    NAME=$(echo ${TARFILE}| awk -F '-' '{print $1"-"$2}')
    VERSION=$(echo ${TARFILE}| awk -F '-' '{print $3}')
    SHORTVER=$(echo ${VERSION} | awk -F '.' '{print $1"."$2}')
    TMPREL=$(echo ${TARFILE}| awk -F '-' '{print $4}')
    RELEASE=${TMPREL%.tar.gz}

    NEWTAR=${NAME}-${SHORTVER}_${VERSION}-${RELEASE}.orig.tar.gz
    mv ${TARFILE} ${NEWTAR}

    tar xzf ${NEWTAR}
    cd ${NAME}-${VERSION}-${RELEASE}
    cp -ap build-ps/debian/ .
    dch -D unstable --force-distribution -v "${VERSION}-${RELEASE}-${DEB_RELEASE}" "Update to new upstream release Percona Server ${VERSION}-${RELEASE}-1"
    dpkg-buildpackage -S

    cd ${WORKDIR}

    mkdir -p $WORKDIR/source_deb
    mkdir -p $CURDIR/source_deb
    cp *.debian.tar.* $WORKDIR/source_deb
    cp *_source.changes $WORKDIR/source_deb
    cp *.dsc $WORKDIR/source_deb
    cp *.orig.tar.gz $WORKDIR/source_deb
    cp *.debian.tar.* $CURDIR/source_deb
    cp *_source.changes $CURDIR/source_deb
    cp *.dsc $CURDIR/source_deb
    cp *.orig.tar.gz $CURDIR/source_deb
}

build_deb(){
    if [ $DEB = 0 ]
    then
        echo "source deb package will not be created"
        return;
    fi
    if [ "x$OS" = "xrpm" ]
    then
        echo "It is not possible to build source deb here"
        exit 1
    fi
    for file in 'dsc' 'orig.tar.gz' 'changes' 'debian.tar*'
    do
        get_deb_sources $file
    done
    cd $WORKDIR
    rm -fv *.deb

    export DEBIAN_VERSION="$(lsb_release -sc)"

    DSC=$(basename $(find . -name '*.dsc' | sort | tail -n 1))
    DIRNAME=$(echo ${DSC%-${DEB_RELEASE}.dsc} | sed -e 's:_:-:g')
    VERSION=$(echo ${DSC} | sed -e 's:_:-:g' | awk -F'-' '{print $4}')
    RELEASE=$(echo ${DSC} | sed -e 's:_:-:g' | awk -F'-' '{print $5}')
    ARCH=$(uname -m)
    export EXTRAVER=${MYSQL_VERSION_EXTRA#-}
    #
    echo "ARCH=${ARCH}" >> percona-server-5.7.properties
    echo "DEBIAN_VERSION=${DEBIAN_VERSION}" >> percona-server-5.7.properties
    echo "VERSION=${VERSION}" >> percona-server-5.7.properties
    #

    dpkg-source -x ${DSC}

    cd ${DIRNAME}
    #
    if [ ${DEBIAN_VERSION} = xenial -o ${DEBIAN_VERSION} = artful -o ${DEBIAN_VERSION} = bionic -o ${DEBIAN_VERSION} = trusty -o ${DEBIAN_VERSION} = cosmic -o ${DEBIAN_VERSION} = disco -o ${DEBIAN_VERSION} = buster ]; then
        rm -rf debian
        cp -r build-ps/ubuntu debian
    fi
    dch -b -m -D "$DEBIAN_VERSION" --force-distribution -v "${VERSION}-${RELEASE}-${DEB_RELEASE}.${DEBIAN_VERSION}" 'Update distribution'

    if [ ${ARCH} != "x86_64" ]; then
        rm -f debian/rules
        rm -f debian/control
        mv debian/rules.notokudb debian/rules
        mv debian/control.notokudb debian/control
    else
        if [ ${DEBIAN_VERSION} != trusty -a ${DEBIAN_VERSION} != xenial -a ${DEBIAN_VERSION} != jessie -a ${DEBIAN_VERSION} != stretch -a ${DEBIAN_VERSION} != artful -a ${DEBIAN_VERSION} != bionic -a ${DEBIAN_VERSION} != cosmic -a ${DEBIAN_VERSION} != disco -a ${DEBIAN_VERSION} != buster ]; then
            gcc47=$(which gcc-4.7 2>/dev/null || true)
            if [ -x "${gcc47}" ]; then
                export CC=gcc-4.7
                export USE_THIS_GCC_VERSION="-4.7"
                export CXX=g++-4.7
            else
                export CC=gcc-4.8
                export USE_THIS_GCC_VERSION="-4.8"
                export CXX=g++-4.8
            fi
        fi
    fi

    if [ ${DEBIAN_VERSION} = "xenial" ]; then
        sed -i 's/export CFLAGS=/export CFLAGS=-Wno-error=date-time /' debian/rules 
        sed -i 's/export CXXFLAGS=/export CXXFLAGS=-Wno-error=date-time /' debian/rules
    fi

    if [ ${DEBIAN_VERSION} = "stretch" ]; then
        sed -i 's/export CFLAGS=/export CFLAGS=-Wno-error=deprecated-declarations -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=date-time /' debian/rules 
        sed -i 's/export CXXFLAGS=/export CXXFLAGS=-Wno-error=deprecated-declarations -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=date-time /' debian/rules
    fi

    if [ ${DEBIAN_VERSION} = "artful" -o ${DEBIAN_VERSION} = "bionic" -o ${DEBIAN_VERSION} = "cosmic" -o ${DEBIAN_VERSION} = "disco" -o ${DEBIAN_VERSION} = "buster" ]; then
        sed -i 's/export CFLAGS=/export CFLAGS=-Wno-error=deprecated-declarations -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=date-time /' debian/rules 
        sed -i 's/export CXXFLAGS=/export CXXFLAGS=-Wno-error=deprecated-declarations -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=date-time /' debian/rules
    fi

    dpkg-buildpackage -rfakeroot -uc -us -b

    cd ${WORKDIR}
    mkdir -p $CURDIR/deb
    mkdir -p $WORKDIR/deb
    cp $WORKDIR/*.deb $WORKDIR/deb
    cp $WORKDIR/*.deb $CURDIR/deb
}

build_tarball(){
    if [ $TARBALL = 0 ]
    then
        echo "Binary tarball will not be created"
        return;
    fi
    get_tar "source_tarball"
    cd $WORKDIR
    TARFILE=$(basename $(find . -name 'percona-server-*.tar.gz' | sort | tail -n1))
    if [ -f /etc/debian_version ]; then
      export OS_RELEASE="$(lsb_release -sc)"
    fi
    #
    if [ -f /etc/redhat-release ]; then
      export OS_RELEASE="centos$(lsb_release -sr | awk -F'.' '{print $1}')"
      RHEL=$(rpm --eval %rhel)
    fi
    #

    ARCH=$(uname -m 2>/dev/null||true)
    TARFILE=$(basename $(find . -name 'percona-server-*.tar.gz' | sort | grep -v "tools" | tail -n1))
    NAME=$(echo ${TARFILE}| awk -F '-' '{print $1"-"$2}')
    VERSION=$(echo ${TARFILE}| awk -F '-' '{print $3}')
    #
    SHORTVER=$(echo ${VERSION} | awk -F '.' '{print $1"."$2}')
    TMPREL=$(echo ${TARFILE}| awk -F '-' '{print $4}')
    RELEASE=${TMPREL%.tar.gz}
    #
    export CFLAGS=$(rpm --eval %{optflags} | sed -e "s|march=i386|march=i686|g")
    export CXXFLAGS="${CFLAGS}"
    if [ "${YASSL}" = 0 ]; then
        if [ -f /etc/redhat-release ]; then
            SSL_VER_TMP=$(yum list installed|grep -i openssl|head -n1|awk '{print $2}'|awk -F "-" '{print $1}'|sed 's/\.//g'|sed 's/[a-z]$//')
            export SSL_VER=".ssl${SSL_VER_TMP}"
        else
            SSL_VER_TMP=$(dpkg -l|grep -i libssl|grep -v "libssl\-"|head -n1|awk '{print $2}'|awk -F ":" '{print $1}'|sed 's/libssl/ssl/g'|sed 's/\.//g')
            export SSL_VER=".${SSL_VER_TMP}"
        fi
    fi
    
    build_mecab_lib
    build_mecab_dict
    MECAB_INSTALL_DIR="${WORKDIR}/mecab-install"
    rm -fr TARGET && mkdir TARGET
    rm -rf jemalloc 
    git clone https://github.com/jemalloc/jemalloc
    (
    cd jemalloc
    git checkout 3.6.0
    bash autogen.sh
    )
    #
    rm -fr ${TARFILE%.tar.gz}
    tar xzf ${TARFILE}
    cd ${TARFILE%.tar.gz}
    if [ "${YASSL}" = 1 ]; then
        DIRNAME="tarball_yassl"
        CMAKE_OPTS="-DWITH_ROCKSDB=1" bash -xe ./build-ps/build-binary.sh --with-jemalloc=../jemalloc/ --with-yassl --with-mecab="${MECAB_INSTALL_DIR}/usr" ../TARGET
    else
        CMAKE_OPTS="-DWITH_ROCKSDB=1" bash -xe ./build-ps/build-binary.sh --with-mecab="${MECAB_INSTALL_DIR}/usr" --with-jemalloc=../jemalloc/ ../TARGET
        DIRNAME="tarball"
    fi
    mkdir -p ${WORKDIR}/${DIRNAME}
    mkdir -p ${CURDIR}/${DIRNAME}
    cp ../TARGET/*.tar.gz ${WORKDIR}/${DIRNAME}
    cp ../TARGET/*.tar.gz ${CURDIR}/${DIRNAME}
}

#main

CURDIR=$(pwd)
VERSION_FILE=$CURDIR/percona-server-5.7.properties
args=
WORKDIR=
SRPM=0
SDEB=0
RPM=0
DEB=0
SOURCE=0
TARBALL=0
OS_NAME=
ARCH=
OS=
TOKUBACKUP_REPO=
PERCONAFT_REPO=
INSTALL=0
RPM_RELEASE=1
DEB_RELEASE=1
REVISION=0
BRANCH="5.7"
RPM_RELEASE=1
DEB_RELEASE=1
YASSL=0
MECAB_INSTALL_DIR="${WORKDIR}/mecab-install"
REPO="git://github.com/percona/percona-server.git"
PRODUCT=Percona-Server-5.7
MYSQL_VERSION_MAJOR=5
MYSQL_VERSION_MINOR=7
MYSQL_VERSION_PATCH=22
MYSQL_VERSION_EXTRA=-22
PRODUCT_FULL=Percona-Server-5.7.22-22
BOOST_PACKAGE_NAME=boost_1_59_0
PERCONAFT_BRANCH=Percona-Server-5.7.22-22
TOKUBACKUP_BRANCH=Percona-Server-5.7.22-22
parse_arguments PICK-ARGS-FROM-ARGV "$@"
if [ ${YASSL} = 1 ]; then
  TARBALL=1
fi

check_workdir
get_system
install_deps
get_sources
build_tarball
build_srpm
build_source_deb
build_rpm
build_deb
