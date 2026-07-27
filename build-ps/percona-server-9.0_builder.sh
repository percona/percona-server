#!/bin/sh

shell_quote_string() {
  echo "$1" | sed -e 's,\([^a-zA-Z0-9/_.=-]\),\\\1,g'
}

usage () {
    cat <<EOF
Usage: $0 [OPTIONS]
    The following options may be given :
        --builddir=DIR              Absolute path to the dir where all actions will be performed
        --get_sources               Source will be downloaded from github
        --build_src_rpm             If it is 1 src rpm will be built
        --build_source_deb          If it is 1 source deb package will be built
        --build_rpm                 If it is 1 rpm will be built
        --build_deb                 If it is 1 deb will be built
        --build_tarball             If it is 1 tarball will be built
        --with_ssl                  If it is 1 tarball will also include ssl libs
        --install_deps              Install build dependencies(root previlages are required)
        --branch                    Branch for build
        --repo                      Repo for build
        --rpm_release               RPM version( default = 1)
        --deb_release               DEB version( default = 1)
        --debug                     Build debug tarball
        --enable_pgo                PGO (Profile-Guided Optimization) build (default = 1, set to 0 to disable)
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
            --with_ssl=*) WITH_SSL="$val" ;;
            --branch=*) BRANCH="$val" ;;
            --repo=*) REPO="$val" ;;
            --install_deps=*) INSTALL="$val" ;;
            --build_tokudb_tokubackup=*) BUILD_TOKUDB_TOKUBACKUP="$val" ;;
            --perconaft_branch=*) PERCONAFT_BRANCH="$val" ;;
            --tokubackup_branch=*)      TOKUBACKUP_BRANCH="$val" ;;
            --perconaft_repo=*) PERCONAFT_REPO="$val" ;;
            --tokubackup_repo=*) TOKUBACKUP_REPO="$val" ;;
            --rpm_release=*) RPM_RELEASE="$val" ;;
            --deb_release=*) DEB_RELEASE="$val" ;;
            --debug=*) DEBUG="$val" ;;
            --enable_pgo=*) ENABLE_PGO="$val" ;;
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

switch_to_vault_repo() {
    sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-*
    sed -i 's|#\s*baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*
    sed -i 's/enabled=0/enabled=1/g' /etc/yum.repos.d/CentOS-Linux-PowerTools.repo
}

get_sources(){
    cd "${WORKDIR}"
    if [ "${SOURCE}" = 0 ]
    then
        echo "Sources will not be downloaded"
        return 0
    fi

    git clone --depth 1 --branch $BRANCH "$REPO"
    retval=$?
    if [ $retval != 0 ]
    then
        echo "There were some issues during repo cloning from github. Please retry one more time"
        #exit 1
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
    cd ${WORKDIR}/percona-server
    #
    ls
    if [ -f VERSION ]; then
        source VERSION
        cat VERSION > ../percona-server-9.0.properties
    elif [ -f MYSQL_VERSION ]; then
        source MYSQL_VERSION
        cat MYSQL_VERSION > ../percona-server-9.0.properties
    else
        echo "VERSION file does not exist"
	exit 1
    fi
    IS_RELEASE_BRANCH=$(echo ${BRANCH} | grep -c release);
    if [ ${IS_RELEASE_BRANCH} != 0 ]; then
        IFS='.' read -r MAJOR MINOR PATCH <<< $(echo $BRANCH | awk -F'-' '{print $2}')
        EXTRA=$(echo $BRANCH | awk -F'-' '{print $3}')
        if [ ${MYSQL_VERSION_MAJOR} != ${MAJOR} ]; then
            echo "Major version differs from defined in version file"
            exit 1
        fi
        if [ ${MYSQL_VERSION_MINOR} != ${MINOR} ]; then
            echo "Minor version differs from defined in version file"
            exit 1
        fi
        if [ ${MYSQL_VERSION_PATCH} != ${PATCH} ]; then
            echo "Patch version differs from defined in version file"
            exit 1
        fi
        # Extra/InnoDB version checks intentionally skipped — branch name may
        # legitimately diverge from MYSQL_VERSION_EXTRA during release preparation
        # (e.g., release-9.6.0-2 branch can be cut before VERSION file is bumped).
        # if [ "${MYSQL_VERSION_EXTRA}" != "-${EXTRA}" ]; then
        #     echo "Extra version differs from defined in version file"
        #     exit 1
        # fi
        # INNODB_VER=$(grep "define PERCONA_INNODB_VERSION" ./storage/innobase/include/univ.i | awk '{print $3}')
        # if [ ${INNODB_VER} != ${EXTRA} ]; then
        #     echo "InnoDB version differs from defined in version file"
        #     exit 1
        # fi
        if [ ${BUILD_TOKUDB_TOKUBACKUP} = 1 ]; then
            FT_TAG=$(git ls-remote --tags https://github.com/percona/PerconaFT.git | grep -c ${PERCONAFT_BRANCH})
            if [ ${FT_TAG} = 0 ]; then
                echo "There is no TAG for PerconaFT. Please set it and re-run build!"
                exit 1
            fi
            TOKUBACKUP_TAG=$(git ls-remote --tags https://github.com/percona/Percona-TokuBackup.git | grep -c ${TOKUBACKUP_BRANCH})
            if [ ${TOKUBACKUP_TAG} = 0 ]; then
                echo "There is no TAG for Percona-TokuBackup. Please set it and re-run build!"
                exit 1
            fi
        fi
    fi
    echo >> ../percona-server-9.0.properties
    echo "REVISION=${REVISION}" >> ../percona-server-9.0.properties
    BRANCH_NAME="${BRANCH}"
    echo "BRANCH_NAME=${BRANCH_NAME}" >> ../percona-server-9.0.properties
    export PRODUCT=Percona-Server-${MYSQL_VERSION_MAJOR}.${MYSQL_VERSION_MINOR}
    echo "PRODUCT=Percona-Server-${MYSQL_VERSION_MAJOR}.${MYSQL_VERSION_MINOR}" >> ../percona-server-9.0.properties
    export PRODUCT_FULL=${PRODUCT}.${MYSQL_VERSION_PATCH}${MYSQL_VERSION_EXTRA}
    echo "PRODUCT_FULL=${PRODUCT}.${MYSQL_VERSION_PATCH}${MYSQL_VERSION_EXTRA}" >> ../percona-server-9.0.properties
    echo "BUILD_NUMBER=${BUILD_NUMBER}" >> ../percona-server-9.0.properties
    echo "BUILD_ID=${BUILD_ID}" >> ../percona-server-9.0.properties
    echo "BUILD_TOKUDB_TOKUBACKUP=${BUILD_TOKUDB_TOKUBACKUP}" >> ../percona-server-9.0.properties
    echo "PERCONAFT_REPO=${PERCONAFT_REPO}" >> ../percona-server-9.0.properties
    echo "PERCONAFT_BRANCH=${PERCONAFT_BRANCH}" >> ../percona-server-9.0.properties
    echo "TOKUBACKUP_REPO=${TOKUBACKUP_REPO}" >> ../percona-server-9.0.properties
    echo "TOKUBACKUP_BRANCH=${TOKUBACKUP_BRANCH}" >> ../percona-server-9.0.properties
    export TOKUDB_VERSION=${MYSQL_VERSION_MAJOR}.${MYSQL_VERSION_MINOR}.${MYSQL_VERSION_PATCH}${MYSQL_VERSION_EXTRA}
    echo "TOKUDB_VERSION=${MYSQL_VERSION_MAJOR}.${MYSQL_VERSION_MINOR}.${MYSQL_VERSION_PATCH}${MYSQL_VERSION_EXTRA}" >> ../percona-server-9.0.properties
    echo "RPM_RELEASE=${RPM_RELEASE}" >> ../percona-server-9.0.properties
    echo "DEB_RELEASE=${DEB_RELEASE}" >> ../percona-server-9.0.properties

    if [ -z "${DESTINATION:-}" ]; then
        export DESTINATION=experimental
    fi
    TIMESTAMP=$(date "+%Y%m%d-%H%M%S")
    echo "DESTINATION=${DESTINATION}" >> ../percona-server-9.0.properties
    echo "UPLOAD=UPLOAD/${DESTINATION}/BUILDS/${PRODUCT}/${PRODUCT_FULL}/${BRANCH_NAME}/${REVISION}/${TIMESTAMP}" >> ../percona-server-9.0.properties

    rm -rf storage/tokudb/PerconaFT
    rm -rf plugin/tokudb-backup-plugin/Percona-TokuBackup
    git submodule init
    git submodule update
    rm -rf storage/tokudb/PerconaFT
    rm -rf plugin/tokudb-backup-plugin/Percona-TokuBackup
    # Quote both sides + provide :- default so an empty / unset variable
    # does not collapse to literal `[ = 0 ]` and trip "unary operator expected".
    if [ "${PERCONAFT_REPO:-}" = "0" ]; then
        PERCONAFT_REPO=''
    fi
    if [ "${TOKUBACKUP_REPO:-}" = "0" ]; then
        TOKUBACKUP_REPO=''
    fi

    if [ ${BUILD_TOKUDB_TOKUBACKUP} = 1 ]; then
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
    fi
    #
    git submodule update
    #
    # `make dist` is implemented as a cmake-driven custom target in
    # sql/CMakeLists.txt → cmake/make_dist.cmake.in, which not only
    # git-archives HEAD but also copies bison-generated parser files
    # (sql_yacc.{h,cc}, sql_hints.yy.{h,cc}) into the tarball and
    # autoreconfs the UDF subtree. cmake configure must succeed first
    # for this target to exist, and PS 9.x requires cmake >= 3.17.5.
    #
    # On hosts where the default `cmake` is older than 3.17.5 (some
    # build agents still ship cmake 3.16.3), fall back to `cmake3`
    # which is the EPEL/backports alternative providing a newer
    # version. Same pattern PXC uses in build-ps/pxc_builder.sh.
    CMAKE_BIN="cmake"
    if ! cmake --version 2>/dev/null | head -1 | awk '{print $3}' | grep -qE "^([4-9]|3\.(1[7-9]|[2-9][0-9]))"; then
        if command -v cmake3 >/dev/null 2>&1; then
            CMAKE_BIN="cmake3"
        fi
    fi
    "${CMAKE_BIN}" .  -DWITH_SSL=system -DFORCE_INSOURCE_BUILD=1 -DWITH_ZLIB=bundled -DWITH_CURL=bundled
    make dist
    #
    EXPORTED_TAR=$(basename $(find . -type f -name percona-server*.tar.gz | sort | tail -n 1))
    #
    PSDIR=${EXPORTED_TAR%.tar.gz}
    rm -fr ${PSDIR}
    tar xzf ${EXPORTED_TAR}
    rm -f ${EXPORTED_TAR}

    # PS-7429 Remove TokuDB and TokuBackup from Percona Server 8.0.28 packages
    if [ ${BUILD_TOKUDB_TOKUBACKUP} != 1 ]; then
        git submodule deinit -f storage/tokudb/PerconaFT/
        rm -rf .git/modules/PerconaFT/
        git rm -f storage/tokudb/PerconaFT/
        git submodule deinit -f plugin/tokudb-backup-plugin/Percona-TokuBackup
        rm -rf .git/modules/Percona-TokuBackup/
        git rm -f plugin/tokudb-backup-plugin/Percona-TokuBackup
    fi

    # add git submodules because make dist uses git archive which doesn't include them
    if [ ${BUILD_TOKUDB_TOKUBACKUP} = 1 ]; then
        rsync -av storage/tokudb/PerconaFT ${PSDIR}/storage/tokudb --exclude .git
        rsync -av plugin/tokudb-backup-plugin/Percona-TokuBackup ${PSDIR}/plugin/tokudb-backup-plugin --exclude .git
    fi
    rsync -av storage/rocksdb/rocksdb/ ${PSDIR}/storage/rocksdb/rocksdb --exclude .git
    rsync -av storage/rocksdb/third_party/lz4/ ${PSDIR}/storage/rocksdb/third_party/lz4 --exclude .git
    rsync -av storage/rocksdb/third_party/zstd/ ${PSDIR}/storage/rocksdb/third_party/zstd --exclude .git
    rsync -av extra/coredumper/ ${PSDIR}/extra/coredumper --exclude .git
    rsync -av extra/libkmip/ ${PSDIR}/extra/libkmip/ --exclude .git
    rsync -av extra/jwt-cpp/ ${PSDIR}/extra/jwt-cpp --exclude .git
    #
    cd ${PSDIR}

    # PS-7429 Remove TokuDB and TokuBackup from Percona Server 8.0.28 packages
    if [ ${BUILD_TOKUDB_TOKUBACKUP} != 1 ]; then
        rm -rf storage/tokudb
        rm -rf plugin/tokudb-backup-plugin
        rm -rf mysql-test/suite/tokudb*
    else
        # set tokudb version - can be seen with show variables like '%version%'
        sed -i "1s/^/SET(TOKUDB_VERSION ${TOKUDB_VERSION})\n/" storage/tokudb/CMakeLists.txt
    fi

    sed -i "s:V8PWD=:V8PWD=${WORKDIR}/v8:g" build-ps/debian/rules

    # Generate debian/rules from template
    sed -i "s:@@PERCONA_VERSION_EXTRA@@:${MYSQL_VERSION_EXTRA#-}:g" build-ps/debian/rules
    sed -i "s:@@REVISION@@:${REVISION}:g" build-ps/debian/rules
    sed -i "s:@@TOKUDB_BACKUP_VERSION@@:${TOKUDB_VERSION}:g" build-ps/debian/rules
    #
    # Generate percona-server.spec from .spec.in template
    PERCONA_VERSION="${MYSQL_VERSION_EXTRA#-}"
    MYSQL_VERSION="${MYSQL_VERSION_MAJOR}.${MYSQL_VERSION_MINOR}.${MYSQL_VERSION_PATCH}"
    COPYRIGHT_YEAR=$(date +"%Y")
    cp build-ps/percona-server.spec.in build-ps/percona-server.spec
    sed -i "s:@MYSQL_NO_DASH_VERSION@:${MYSQL_VERSION}:g" build-ps/percona-server.spec
    sed -i "s:@PERCONA_SERVER_VERSION@:${PERCONA_VERSION}:g" build-ps/percona-server.spec
    sed -i "s:@PERCONA_REVISION@:${REVISION}:g" build-ps/percona-server.spec
    sed -i "s:@RPM_RELEASE@:${RPM_RELEASE}:g" build-ps/percona-server.spec
    sed -i "s:@MYSQL_COPYRIGHT_YEAR@:${COPYRIGHT_YEAR}:g" build-ps/percona-server.spec
    cd ${WORKDIR}/percona-server
#
    mv "${WORKDIR}"/v8/LICENSE "${WORKDIR}"/v8/LICENSE.v8.libraries
    mkdir ${PSDIR}/js
    cp -v "${WORKDIR}"/v8/LICENSE* ${PSDIR}/js
#
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
	GLIBC_VER_TMP="$(rpm glibc -qa --qf %{VERSION})"
        RHEL=$(rpm --eval %rhel)
        ARCH=$(echo $(uname -m) | sed -e 's:i686:i386:g')
        OS_NAME="el$RHEL"
        OS="rpm"
    elif [ -f /etc/amazon-linux-release ]; then
        GLIBC_VER_TMP="$(rpm glibc -qa --qf %{VERSION})"
        RHEL=$(rpm --eval %amzn)
        ARCH=$(echo $(uname -m) | sed -e 's:i686:i386:g')
        OS_NAME="amzn$RHEL"
        OS="rpm"
    else
	GLIBC_VER_TMP="$(dpkg-query -W -f='${Version}' libc6 | awk -F'-' '{print $1}')"
        ARCH=$(uname -m)
        OS_NAME="$(lsb_release -sc)"
        OS="deb"
    fi
    export GLIBC_VER=".glibc${GLIBC_VER_TMP}"
    return
}

apply_workaround_bug_304121(){
    cat > /tmp/bugzilla_bug_304121.patch <<- EOF
--- /usr/lib/rpm/find-debuginfo.sh	2022-07-29 11:43:38.582289.03 +0000
+++ /usr/lib/rpm/find-debuginfo.sh	2022-07-29 11:43:17.089255640 +0000
@@ -309,7 +309,7 @@

   echo "extracting debug info from \$f"
   id=\$(/usr/lib/rpm/debugedit -b "\$RPM_BUILD_DIR" -d /usr/src/debug \\
-			      -i -l "\$SOURCEFILE" "\$f") || exit
+			      -i -l "\$SOURCEFILE" "\$f") || true
   if [ \$nlinks -gt 1 ]; then
     eval linkedid_\$inum=\\\$id
   fi
EOF
    patch -ruN -d /usr/lib/rpm < /tmp/bugzilla_bug_304121.patch
    return
}

get_v8(){
    cd ${WORKDIR}
    if [ x"$ARCH" = "xx86_64" ]; then
        wget -q --no-check-certificate https://downloads.percona.com/downloads/packaging/v8_12.9.202.22-glibc2.31.tar.gz
        tar -xzf v8_12.9.202.22-glibc2.31.tar.gz
        rm -rf v8_12.9.202.22-glibc2.31.tar.gz
    else
        wget -q --no-check-certificate https://downloads.percona.com/downloads/packaging/v8_12.9.202.22-arm64.tar.gz
        tar -xzf v8_12.9.202.22-arm64.tar.gz
        rm -rf v8_12.9.202.22-arm64.tar.gz
    fi
}

install_deps() {
    if [ $INSTALL = 0 ]
    then
        echo "Dependencies will not be installed"
        return;
    fi
    if [ ! $( id -u ) -eq 0 ]
    then
        echo "It is not possible to instal dependencies. Please run as root"
        exit 1
    fi
    CURPLACE=$(pwd)

    if [ "x$OS" = "xrpm" ]; then
        ARCH=$(echo $(uname -m) | sed -e 's:i686:i386:g')
        if [ "x${RHEL}" = "x7" ]; then
            switch_to_vault_repo
        fi
        if [ x"$ARCH" = "xx86_64" ]; then
            if [ "${RHEL}" -lt 9 ]; then
                yum install -y https://repo.percona.com/yum/percona-release-latest.noarch.rpm
                percona-release enable tools testing
            fi
        fi
        yum -y install yum-utils
        if [ "x${RHEL}" != "x2023" ]; then
            yum-config-manager --enable ol"${RHEL}"_codeready_builder
        else
            yum -y install yum-utils
        fi
        yum -y update
        if [ "x${RHEL}" != "x2023" ]; then
            if [ "x${RHEL}" = "x10" ]; then
                dnf install https://dl.fedoraproject.org/pub/epel/epel-release-latest-10.noarch.rpm
            else
                yum -y install epel-release
            fi
            yum -y install libeatmydata
        fi
        yum -y install git numactl-devel rpm-build gcc-c++ gperf ncurses-devel perl readline-devel openssl-devel jemalloc zstd
        yum -y install time zlib-devel libaio-devel bison cmake3 cmake pam-devel jemalloc-devel pkg-config
        yum -y install perl-Time-HiRes libcurl-devel openldap-devel unzip wget libcurl-devel patchelf systemd-devel
        yum -y install perl-Env perl-Data-Dumper perl-JSON perl-Digest perl-Digest-MD5 perl-Digest-Perl-MD5 || true
        if [ "${RHEL}" -lt 8 ]; then
            until yum -y install centos-release-scl; do
                echo "waiting"
                sleep 1
            done
            switch_to_vault_repo
            yum -y install gcc-c++ devtoolset-8-gcc-c++ devtoolset-8-binutils devtoolset-8-gcc devtoolset-8-gcc-c++
            yum -y install ccache devtoolset-8-libasan-devel devtoolset-8-libubsan-devel devtoolset-8-valgrind devtoolset-8-valgrind-devel
            yum -y install libasan libicu-devel libtool libzstd-devel lz4-devel make pkg-config
            yum -y install re2-devel redhat-lsb-core lz4-static
            source /opt/rh/devtoolset-8/enable
            yum -y install cyrus-sasl-devel cyrus-sasl-scram krb5-devel
        else
	    yum -y install perl.x86_64
            yum -y install binutils gcc gcc-c++ tar rpm-build rsync bison glibc glibc-devel libstdc++-devel make openssl-devel pam-devel perl perl-JSON perl-Memoize pkg-config
            yum -y install automake autoconf cmake cmake3 jemalloc jemalloc-devel
	    yum -y install libaio-devel ncurses-devel numactl-devel readline-devel time
	    yum -y install rpcgen
	    yum -y install libzstd libzstd-devel
	    yum -y install cyrus-sasl-devel cyrus-sasl-scram krb5-devel
        fi
        if [ "x${RHEL}" = "x7" ]; then
            apply_workaround_bug_304121
            yum -y install devtoolset-11
            yum -y install devtoolset-11-annobin-plugin-gcc
            yum -y install cyrus-sasl-gssapi cyrus-sasl-gs2 cyrus-sasl-md5 cyrus-sasl-plain
            source /opt/rh/devtoolset-11/enable
        fi
	if [ "x${RHEL}" = "x6" ]; then
            source /opt/rh/devtoolset-8/enable
        fi
        if [ "x$RHEL" = "x6" ]; then
            rm -f /usr/bin/cmake
            cp -p /usr/bin/cmake3 /usr/bin/cmake
            yum -y install Percona-Server-shared-56
	    yum -y install libevent2-devel
	else
            yum -y install libevent-devel
        fi
        if [ "x$RHEL" = "x7" ]; then
            yum -y --enablerepo=centos-sclo-rh-testing install devtoolset-11-gcc-c++ devtoolset-11-binutils devtoolset-11-valgrind devtoolset-11-valgrind-devel devtoolset-11-libatomic-devel
            yum -y --enablerepo=centos-sclo-rh-testing install devtoolset-11-libasan-devel devtoolset-11-libubsan-devel
            rm -f /usr/bin/cmake
	    cp -p /usr/bin/cmake3 /usr/bin/cmake
        fi
        yum -y install libtirpc-devel
        if [ "x$RHEL" = "x8" -o "x$RHEL" = "x9" ]; then
            yum -y install gcc-toolset-14-gcc gcc-toolset-14-gcc-c++ gcc-toolset-14-binutils gcc-toolset-14-annobin-annocheck gcc-toolset-14-annobin-plugin-gcc gcc-toolset-14-libatomic-devel
            if [ x"$ARCH" = "xx86_64" ]; then
                pushd /opt/rh/gcc-toolset-14/root/usr/lib/gcc/x86_64-redhat-linux/14/plugin/
                ln -s annobin.so gcc-annobin.so
                popd
            else
                pushd /opt/rh/gcc-toolset-14/root/usr/lib/gcc/aarch64-redhat-linux/14/plugin/
                ln -s annobin.so gcc-annobin.so
                popd
            fi
        fi
        if [ "x$RHEL" = "x10" ]; then
            yum -y install gcc gcc-c++
            yum -y install libatomic
        fi
        if [ "x$RHEL" = "x2023" ]; then
           yum -y install libtirpc-devel libatomic annobin-annocheck annobin-plugin-gcc
           yum -y install pip mariadb105-devel python3-devel
           pip install mysqlclient
        fi
    else
        apt-get update
        apt-get -y install lsb_release || true
        apt-get -y install dirmngr || true
        apt-get -y install gnupg2 lsb-release wget git curl
        wget https://repo.percona.com/apt/percona-release_latest.$(lsb_release -sc)_all.deb && dpkg -i percona-release_latest.$(lsb_release -sc)_all.deb
        percona-release enable tools testing
        export DEBIAN_FRONTEND="noninteractive"
        export DIST="$(lsb_release -sc)"
            until apt-get update; do
            sleep 1
            echo "waiting"
        done
        apt-get -y purge eatmydata || true

        apt-get update
        apt-get -y install psmisc pkg-config
        apt-get -y install libsasl2-modules:amd64 || apt-get -y install libsasl2-modules
        apt-get -y install dh-systemd || true
        apt-get -y install copyright-update
        apt-get -y install curl bison cmake perl libssl-dev libaio-dev libldap2-dev libwrap0-dev gdb unzip gawk
        apt-get -y install lsb-release libmecab-dev libncurses5-dev libpam-dev zlib1g-dev libcurl4-openssl-dev libsystemd-dev
        apt-get -y install libldap2-dev libnuma-dev libjemalloc-dev libc6-dbg valgrind libjson-perl libsasl2-dev patchelf
        if [ x"${DIST}" = xfocal -o x"${DIST}" = xbullseye -o x"${DIST}" = xjammy -o x"${DIST}" = xbookworm -o x"${DIST}" = xnoble -o x"${DIST}" = xtrixie ]; then
            apt-get -y install python3-mysqldb
        else
            apt-get -y install python-mysqldb
        fi
        if [ x"${DIST}" = xbionic ]; then
            apt-get -y install gcc-8 g++-8
            update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 700 --slave /usr/bin/g++ g++ /usr/bin/g++-7
            update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8
        elif [ x"${DIST}" = xbookworm ]; then
            apt-get -y install gcc-12 g++-12
            update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100 --slave /usr/bin/g++ g++ /usr/bin/g++-12
            update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-12 100
        elif [ x"${DIST}" = xnoble ]; then
            apt-get -y install gcc-13 g++-13
            update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 --slave /usr/bin/g++ g++ /usr/bin/g++-13
            update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-13 100
        elif [ x"${DIST}" = xtrixie ]; then
            apt-get -y install gcc-14 g++-14
            update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 --slave /usr/bin/g++ g++ /usr/bin/g++-14
            update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-14 100
        elif [ x"${DIST}" = xresolute ]; then
            # Ubuntu 26.04 LTS ships gcc-15 as the default toolchain. The
            # legacy `else` fallback below installs gcc-11 and points the
            # gcc/g++/cc alternatives at it, but NOT /usr/bin/c++ — which on
            # resolute is still g++-15. The result is a mixed-version build
            # where libkmip's C sources compile with gcc-11 (writing LTO
            # bytecode 11.3 into libkmip.a) but the C++ link is g++-15 (lto1
            # 15.1), and link-time LTO aborts with:
            #   lto1: fatal error: bytecode stream in file libkmip/src/libkmip.a
            #   generated with LTO version 11.3 instead of the expected 15.1
            # Pin everything (including the C++ default) at gcc-15 to keep
            # the whole build on a single toolchain.
            apt-get -y install gcc-15 g++-15
            update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-15 100 --slave /usr/bin/g++ g++ /usr/bin/g++-15
            update-alternatives --install /usr/bin/cc  cc  /usr/bin/gcc-15 100
            update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-15 100
        else
            apt-get -y install gcc-11 g++-11 cpp-11
            update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 --slave /usr/bin/g++ g++ /usr/bin/g++-11 --slave /usr/bin/gcov gcov /usr/bin/gcov-11
            update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-11 100
        fi
        apt-get -y install libeatmydata
        apt-get -y install dh-apparmor
        apt-get -y install libmecab2 mecab mecab-ipadic
        apt-get -y install libudev-dev
        apt-get -y install build-essential devscripts doxygen doxygen-gui graphviz rsync
        apt-get -y install cmake autotools-dev autoconf automake build-essential devscripts debconf debhelper fakeroot libaio-dev
        apt-get -y install ccache libevent-dev libre2-dev libtool po-debconf liblz4-dev
        apt-get -y install libgsasl7 || apt-get -y install gsasl-common || true
        # libtirpc-dev is required on every modern Debian/Ubuntu: glibc >= 2.26
        # removed the built-in Sun RPC implementation, and cmake/rpc.cmake
        # FATAL_ERRORs if system rpc/rpc.h is absent (no bundled-tirpc fallback
        # despite extra/tirpc/ existing in the tree). Previously only installed
        # for noble/trixie, so every other distro/arch agent failed
        # override_dh_auto_configure with "Could not find rpc/rpc.h". Install
        # it unconditionally. (Ported from release-8.4.9-9-1 commit 3c2a6bab4e1.)
        apt-get -y install libtirpc-dev
        if [ x"${DIST}" = xfocal -o x"${DIST}" = xbionic -o x"${DIST}" = xdisco -o x"${DIST}" = xbuster -o x"${DIST}" = xbullseye -o x"${DIST}" = xjammy -o x"${DIST}" = xbookworm -o x"${DIST}" = xnoble -o x"${DIST}" = xtrixie ]; then
            apt-get -y install libeatmydata1
        fi
        if [ x"${DIST}" = xfocal -o x"${DIST}" = xbionic -o x"${DIST}" = xstretch -o x"${DIST}" = xdisco -o x"${DIST}" = xbuster -o x"${DIST}" = xbullseye -o x"${DIST}" = xjammy -o x"${DIST}" = xbookworm -o x"${DIST}" = xnoble -o x"${DIST}" = xtrixie ]; then
            apt-get -y install libzstd-dev
        else
            apt-get -y install libzstd1-dev
        fi
        apt-get install -y libsasl2-dev libsasl2-modules-gssapi-mit libkrb5-dev
        if [ x${DIST} = xnoble -o x"${DIST}" = xtrixie ]; then
            apt-get -y install libtirpc-dev gsasl-common
        fi
    fi
    if [ ! -d /usr/local/percona-subunit2junitxml ]; then
        cd /usr/local
        git clone https://github.com/percona/percona-subunit2junitxml.git
        rm -rf /usr/bin/subunit2junitxml
        ln -s /usr/local/percona-subunit2junitxml/subunit2junitxml /usr/bin/subunit2junitxml
        cd ${CURPLACE}
    fi
    get_v8
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
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/percona-server.spec' --strip=2
    #
    sed -i "/^%changelog/a - Release ${VERSION}-${RELEASE}" percona-server.spec
    sed -i "/^%changelog/a * $(date "+%a") $(date "+%b") $(date "+%d") $(date "+%Y") Percona Development Team <info@percona.com> - ${VERSION}-${RELEASE}" percona-server.spec
    #
    cd ${WORKDIR}/rpmbuild/SOURCES
    CALLHOME_SHA="0e3a2ed40336c70727f9aad8402a8a820ebc8db0"
    CALLHOME_SHA256="3497f6631e71799bed9dedb1d72350bf1f0565d93578955234ac30cf2fb6eba4"
    wget -q "https://raw.githubusercontent.com/percona/telemetry-agent/${CALLHOME_SHA}/call-home.sh"
    echo "${CALLHOME_SHA256}  call-home.sh" | sha256sum -c - || { echo "ERROR: call-home.sh checksum mismatch"; exit 1; }
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/rpm/*.patch' --strip=3
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/rpm/mysql_config.sh' --strip=3
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/rpm/percona-telemetry-setup.sh' --strip=3
    tar vxzf ${WORKDIR}/${TARFILE} --wildcards '*/build-ps/rpm/percona-telemetry-cleanup.sh' --strip=3
    #
    cd ${WORKDIR}/rpmbuild/SPECS
    ls -la
    cat percona-server.spec
    grep -n SOURCE999 percona-server.spec
    line_number=$(grep -n SOURCE999 percona-server.spec | awk -F ':' '{print $1}')
    cp ../SOURCES/call-home.sh ./
    awk -v n=$line_number 'NR <= n {print > "part1.txt"} NR > n {print > "part2.txt"}' percona-server.spec
    head -n -1 part1.txt > temp && mv temp part1.txt
    echo "cat <<'CALLHOME' > \$tfn" >> part1.txt
    cat call-home.sh >> part1.txt
    echo "CALLHOME" >> part1.txt
    cat part2.txt >> part1.txt
    rm -f call-home.sh part2.txt
    mv part1.txt percona-server.spec
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
    ARCH=$(echo $(uname -m) | sed -e 's:i686:i386:g')
    MECAB_TARBAL="mecab-0.996.tar.gz"
    MECAB_LINK="https://downloads.percona.com/downloads/packaging/mecab/${MECAB_TARBAL}"
    MECAB_DIR="${WORKDIR}/${MECAB_TARBAL%.tar.gz}"
    MECAB_INSTALL_DIR="${WORKDIR}/mecab-install"
    rm -f ${MECAB_TARBAL}
    rm -rf ${MECAB_DIR}
    rm -rf ${MECAB_INSTALL_DIR}
    mkdir ${MECAB_INSTALL_DIR}
    wget --no-check-certificate ${MECAB_LINK}
    tar xf ${MECAB_TARBAL}
    if [ x"$ARCH" = "xaarch64" ]; then
        # mecab-0.996 ships an old config.guess/config.sub that doesn't
        # recognize aarch64. Pull updated copies. Use HTTPS — the git://
        # protocol on port 9418 is deprecated and commonly blocked.
        rm -rf config
        git clone --depth=1 https://git.savannah.gnu.org/git/config.git
        unalias cp 2>/dev/null || true
        cp config/config.guess ${MECAB_DIR}
        cp config/config.sub ${MECAB_DIR}
    fi
    cd ${MECAB_DIR}
    ./configure --with-pic --prefix=/usr
    make
    make check
    make DESTDIR=${MECAB_INSTALL_DIR} install
    cd ${MECAB_INSTALL_DIR}
    if [ -d usr/lib64 ]; then
        mkdir -p usr/lib
        mv usr/lib64/* usr/lib
    fi
    cd ${WORKDIR}
}

build_mecab_dict(){
    ARCH=$(echo $(uname -m) | sed -e 's:i686:i386:g')
    MECAB_IPADIC_TARBAL="mecab-ipadic-2.7.0-20070801.tar.gz"
    MECAB_IPADIC_LINK="https://downloads.percona.com/downloads/packaging/mecab/${MECAB_IPADIC_TARBAL}"
    MECAB_IPADIC_DIR="${WORKDIR}/${MECAB_IPADIC_TARBAL%.tar.gz}"
    rm -f ${MECAB_IPADIC_TARBAL}
    rm -rf ${MECAB_IPADIC_DIR}
    wget --no-check-certificate ${MECAB_IPADIC_LINK}
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
    SRC_RPM=$(basename $(find $WORKDIR/srpm -name 'percona-server-*.src.rpm' | sort | tail -n1))
    if [ -z $SRC_RPM ]
    then
        SRC_RPM=$(basename $(find $CURDIR/srpm -name 'percona-server-*.src.rpm' | sort | tail -n1))
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
    echo "RHEL=${RHEL}" >> percona-server-9.0.properties
    echo "ARCH=${ARCH}" >> percona-server-9.0.properties
    #
    SRCRPM=$(basename $(find . -name '*.src.rpm' | sort | tail -n1))
    mkdir -vp rpmbuild/{SOURCES,SPECS,BUILD,SRPMS,RPMS}
    #
    mv *.src.rpm rpmbuild/SRPMS
    if [ "x${RHEL}" = "x7" ]; then
        source /opt/rh/devtoolset-11/enable
    fi
    if [ "x${RHEL}" = "x8" ]; then
        source /opt/rh/gcc-toolset-13/enable
    fi
    build_mecab_lib
    build_mecab_dict

    cd ${WORKDIR}
    if [ "x${RHEL}" = "x7" ]; then
        source /opt/rh/devtoolset-11/enable
    fi
    EXTRA_DEFINES=()
    if [ ${ARCH} != x86_64 ]; then
        EXTRA_DEFINES+=(--define "with_tokudb 0")
    fi
    if [ "${ENABLE_PGO}" = "1" ]; then
        EXTRA_DEFINES+=(--define "with_pgo 1")
    fi

    rpmbuild \
        --define "_topdir ${WORKDIR}/rpmbuild" \
        --define "dist .${OS_NAME}" \
        --define "with_mecab ${MECAB_INSTALL_DIR}/usr" \
        --define "with_js_lang ${WORKDIR}/v8" \
        "${EXTRA_DEFINES[@]}" \
        --rebuild rpmbuild/SRPMS/${SRCRPM}

    if [ $RHEL = 6 ]; then
        sudo rm -f /usr/bin/strip
        sudo mv /usr/bin/strip_back /usr/bin/strip
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

    NEWTAR=percona-server_${VERSION}-${RELEASE}.orig.tar.gz
    mv ${TARFILE} ${NEWTAR}

    tar xzf ${NEWTAR}
    ls -la
    cd percona-server-${VERSION}-${RELEASE}
    cp -ap build-ps/debian/ .
    dch -D unstable --force-distribution -v "${VERSION}-${RELEASE}-${DEB_RELEASE}" "Update to new upstream release Percona Server ${VERSION}-${RELEASE}-1"
    copyright-update -d debian/copyright
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
    VERSION=$(echo ${DSC} | sed -e 's:_:-:g' | awk -F'-' '{print $3}')
    RELEASE=$(echo ${DSC} | sed -e 's:_:-:g' | awk -F'-' '{print $4}')
    ARCH=$(uname -m)
    export EXTRAVER=${MYSQL_VERSION_EXTRA#-}
    #
    echo "ARCH=${ARCH}" >> percona-server-9.0.properties
    echo "DEBIAN_VERSION=${DEBIAN_VERSION}" >> percona-server-9.0.properties
    echo "VERSION=${VERSION}" >> percona-server-9.0.properties
    #

    dpkg-source -x ${DSC}

    cd ${DIRNAME}
    dch -b -m -D "$DEBIAN_VERSION" --force-distribution -v "${VERSION}-${RELEASE}-${DEB_RELEASE}.${DEBIAN_VERSION}" 'Update distribution'

    cd debian/
    CALLHOME_SHA="0e3a2ed40336c70727f9aad8402a8a820ebc8db0"
    CALLHOME_SHA256="3497f6631e71799bed9dedb1d72350bf1f0565d93578955234ac30cf2fb6eba4"
    wget -q "https://raw.githubusercontent.com/percona/telemetry-agent/${CALLHOME_SHA}/call-home.sh"
    echo "${CALLHOME_SHA256}  call-home.sh" | sha256sum -c - || { echo "ERROR: call-home.sh checksum mismatch"; exit 1; }
    sed -i 's:exit 0::' percona-server-server.postinst
    echo "tfn=\$(/usr/bin/mktemp -p \$(/usr/bin/mktemp -d /tmp/XXXXXXXX) call-home.XXXXXX.sh)" >> percona-server-server.postinst
    echo "cat <<'CALLHOME' > \$tfn" >> percona-server-server.postinst
    cat call-home.sh >> percona-server-server.postinst
    echo "CALLHOME" >> percona-server-server.postinst
    echo "bash +x \$tfn -f \"PRODUCT_FAMILY_PS\" -v \"${VERSION}-${RELEASE}-${DEB_RELEASE}\" -d \"PACKAGE\" &>/dev/null || :" >> percona-server-server.postinst
    echo "chgrp percona-telemetry /usr/local/percona/telemetry_uuid &>/dev/null || :" >> percona-server-server.postinst
    echo "chmod 664 /usr/local/percona/telemetry_uuid &>/dev/null || :" >> percona-server-server.postinst
    echo "rm -rf \$tfn" >> percona-server-server.postinst
    echo "exit 0" >> percona-server-server.postinst
    rm -f call-home.sh
    cd ../

    # NOTE: a legacy block here used to force CC=gcc-4.7 / gcc-4.8 for any
    # DEBIAN_VERSION not present in a hardcoded allowlist (trusty, xenial,
    # ..., trixie). It was a MySQL 5.6/5.7-era relic and is actively
    # harmful for the 8.4/9.x line:
    #   * MySQL 9.x requires a C++20 toolchain (gcc >= 10); gcc-4.x cannot
    #     build it even if present.
    #   * Modern build agents don't ship gcc-4.7/4.8, so cmake aborts with
    #       Could not find compiler set in environment variable CC: gcc-4.8.
    #   * Every distro IN the allowlist already builds with the system
    #     default compiler (the block was skipped for them), so they prove
    #     the system-default path is correct.
    #   * USE_THIS_GCC_VERSION it exported was never consumed anywhere.
    # The block has been removed; let dpkg-buildpackage / cmake use the
    # agent's default toolchain like the allowlisted distros already do.
    # Same fix applied to percona-server-8.0_builder.sh on the
    # release-8.4.9-9-1 line (commit 1a2cb624ee4).

#    if [ ${DEBIAN_VERSION} = "stretch" -o ${DEBIAN_VERSION} = "bionic" -o ${DEBIAN_VERSION} = "focal" -o ${DEBIAN_VERSION} = "buster" -o ${DEBIAN_VERSION} = "disco"  -o ${DEBIAN_VERSION} = "bullseye" -o ${DEBIAN_VERSION} = "bookworm" -o ${DEBIAN_VERSION} = "noble" -o ${DEBIAN_VERSION} = "trixie" ]; then
        sed -i 's/export CFLAGS=/export CFLAGS=-Wno-error=deprecated-declarations -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=date-time -Wno-error=maybe-uninitialized /' debian/rules
        sed -i 's/export CXXFLAGS=/export CXXFLAGS=-Wno-error=deprecated-declarations -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=date-time -Wno-error=maybe-uninitialized /' debian/rules
#    fi

    if [ "${ENABLE_PGO}" = "1" ]; then
        export DEB_PGO=1
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
      if [ "x${RHEL}" = "x7" ]; then
          source /opt/rh/devtoolset-11/enable
      fi
      # Source the newest gcc-toolset available. Percona Server 9.x requires
      # GCC 11+; the system gcc on EL8 is 8.5 and on EL9 is 11.x. We prefer
      # the newest toolset for performance (especially with PGO+LTO).
      if [ "x${RHEL}" = "x8" ] || [ "x${RHEL}" = "x9" ]; then
          for ts in 14 13 12 11; do
              if [ -f /opt/rh/gcc-toolset-${ts}/enable ]; then
                  source /opt/rh/gcc-toolset-${ts}/enable
                  echo "build_tarball: using gcc-toolset-${ts} ($(gcc --version | head -1))"
                  break
              fi
          done
      fi
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
    mkdir -p ${WORKDIR}/ssl/lib
    if [ "x$OS" = "xdeb" ]; then
        cp -av /usr/lib/x86_64-linux-gnu/libssl* ${WORKDIR}/ssl/lib
	cp -av /usr/lib/x86_64-linux-gnu/libcrypto* ${WORKDIR}/ssl/lib
        cp -av /usr/include/openssl ${WORKDIR}/ssl/include/
    else
        cp -av /usr/lib*/libssl.so* ${WORKDIR}/ssl/lib
	cp -av /usr/lib*/libcrypto* ${WORKDIR}/ssl/lib
        cp -av /usr/include/openssl ${WORKDIR}/ssl/include/
    fi

    cd ${TARFILE%.tar.gz}
    # Pass ENABLE_PGO through to build-binary.sh as WITH_PGO so tarball builds
    # match the PGO behavior of RPM and DEB artifacts.
    export WITH_PGO="${ENABLE_PGO}"
    if [ "x$WITH_SSL" = "x1" ]; then
        CMAKE_OPTS="-DMINIMAL_RELWITHDEBINFO=OFF -DWITH_ROCKSDB=1 -DINSTALL_LAYOUT=STANDALONE -DWITH_SSL=$PWD/../ssl/ " bash -xe ./build-ps/build-binary.sh --with-mecab="${MECAB_INSTALL_DIR}/usr" --with-v8="${WORKDIR}/v8" --with-jemalloc=../jemalloc/ ../TARGET
        DIRNAME="yassl"
    else
        if [[ "${DEBUG}" == 1 ]]; then
            CMAKE_OPTS="-DWITH_ROCKSDB=1" bash -xe ./build-ps/build-binary.sh --debug --with-mecab="${MECAB_INSTALL_DIR}/usr" --with-v8="${WORKDIR}/v8" --with-jemalloc=../jemalloc/ ../TARGET
            DIRNAME="tarball"
        else
            CMAKE_OPTS="-DMINIMAL_RELWITHDEBINFO=OFF -DWITH_ROCKSDB=1" bash -xe ./build-ps/build-binary.sh --with-mecab="${MECAB_INSTALL_DIR}/usr" --with-v8="${WORKDIR}/v8" --with-jemalloc=../jemalloc/ ../TARGET
            DIRNAME="tarball"
        fi
    fi
    mkdir -p ${WORKDIR}/${DIRNAME}
    mkdir -p ${CURDIR}/${DIRNAME}
    cp ../TARGET/*.tar.gz ${WORKDIR}/${DIRNAME}
    cp ../TARGET/*.tar.gz ${CURDIR}/${DIRNAME}
}

#main

CURDIR=$(pwd)
VERSION_FILE=$CURDIR/percona-server-9.0.properties
args=
WORKDIR=
SRPM=0
SDEB=0
RPM=0
DEB=0
SOURCE=0
TARBALL=0
WITH_SSL=0
OS_NAME=
ARCH=
OS=
TOKUBACKUP_REPO=
PERCONAFT_REPO=
INSTALL=0
RPM_RELEASE=1
DEB_RELEASE=1
DEBUG=0
ENABLE_PGO=1
REVISION=0
BRANCH="release-9.0.1-1"
RPM_RELEASE=1
DEB_RELEASE=1
MECAB_INSTALL_DIR="${WORKDIR}/mecab-install"
REPO="https://github.com/percona/percona-server.git"
PRODUCT=Percona-Server-9.0
MYSQL_VERSION_MAJOR=9
MYSQL_VERSION_MINOR=0
MYSQL_VERSION_PATCH=1
MYSQL_VERSION_EXTRA=-1
PRODUCT_FULL=Percona-Server-9.0.1
BUILD_TOKUDB_TOKUBACKUP=0
parse_arguments PICK-ARGS-FROM-ARGV "$@"

check_workdir
get_system
install_deps
get_sources
build_tarball
build_srpm
build_source_deb
build_rpm
build_deb
