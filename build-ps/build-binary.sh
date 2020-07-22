#!/bin/bash
#
# Execute this tool to setup the environment and build binary releases
# for Percona-Server starting from a fresh tree.
#
# Usage: build-binary.sh [target dir]
# The default target directory is the current directory. If it is not
# supplied and the current directory is not empty, it will issue an error in
# order to avoid polluting the current directory after a test run.
#

# Bail out on errors, be strict
set -ue

# Examine parameters
TARGET="$(uname -m)"
TARGET_CFLAGS=''
QUIET='VERBOSE=1'
WITH_JEMALLOC=''
WITH_MECAB_OPTION=''
DEBUG_EXTRA=''
WITH_SSL='/usr'
OPENSSL_INCLUDE=''
OPENSSL_LIBRARY=''
CRYPTO_LIBRARY=''
TAG=''
#
CMAKE_BUILD_TYPE=''
COMMON_FLAGS=''
#
TOKUDB_BACKUP_VERSION=''
#
# Some programs that may be overriden
TAR=${TAR:-tar}

# Check if we have a functional getopt(1)
if ! getopt --test
then
    go_out="$(getopt --options=iqdvj:m:t: \
        --longoptions=i686,quiet,debug,valgrind,with-jemalloc:,with-mecab:,with-ssl:,tag: \
        --name="$(basename "$0")" -- "$@")"
    test $? -eq 0 || exit 1
    eval set -- $go_out
fi

for arg
do
    case "$arg" in
    -- ) shift; break;;
    -i | --i686 )
        shift
        TARGET="i686"
        TARGET_CFLAGS="-m32 -march=i686"
        ;;
    -d | --debug )
        shift
        CMAKE_BUILD_TYPE='Debug'
        BUILD_COMMENT="${BUILD_COMMENT:-}-debug"
        DEBUG_EXTRA="-DDEBUG_EXTNAME=OFF"
        ;;
    -v | --valgrind )
        shift
        CMAKE_OPTS="${CMAKE_OPTS:-} -DWITH_VALGRIND=ON"
        BUILD_COMMENT="${BUILD_COMMENT:-}-valgrind"
        ;;
    -q | --quiet )
        shift
        QUIET=''
        ;;
    -j | --with-jemalloc )
        shift
        WITH_JEMALLOC="$1"
        shift
        ;;
    -m | --with-mecab )
        shift
        WITH_MECAB_OPTION="-DWITH_MECAB=$1"
        shift
        ;;
    --with-ssl )
        shift
        WITH_SSL="$1"
        shift
        # Set openssl and crypto library path
        if test -e "$WITH_SSL/lib/libssl.a"
        then
            OPENSSL_INCLUDE="-DOPENSSL_INCLUDE_DIR=$WITH_SSL/include"
            OPENSSL_LIBRARY="-DOPENSSL_LIBRARY=$WITH_SSL/lib/libssl.a"
            CRYPTO_LIBRARY="-DCRYPTO_LIBRARY=$WITH_SSL/lib/libcrypto.a"
        elif test -e "$WITH_SSL/lib64/libssl.a"
        then
            OPENSSL_INCLUDE="-DOPENSSL_INCLUDE_DIR=$WITH_SSL/include"
            OPENSSL_LIBRARY="-DOPENSSL_LIBRARY=$WITH_SSL/lib64/libssl.a"
            CRYPTO_LIBRARY="-DCRYPTO_LIBRARY=$WITH_SSL/lib64/libcrypto.a"
        else
            echo >&2 "Cannot find libssl.a in $WITH_SSL"
            exit 3
        fi
        ;;
    -t | --tag )
        shift
        TAG="$1"
        shift
        ;;
    esac
done

# Working directory
if test "$#" -eq 0
then
    WORKDIR="$(pwd)"

    # Check that the current directory is not empty
    if test "x$(echo *)" != "x*"
    then
        echo >&2 \
            "Current directory is not empty. Use $0 . to force build in ."
        exit 1
    fi
elif test "$#" -eq 1
then
    WORKDIR="$1"

    # Check that the provided directory exists and is a directory
    if ! test -d "$WORKDIR"
    then
        echo >&2 "$WORKDIR is not a directory"
        exit 1
    fi
else
    echo >&2 "Usage: $0 [target dir]"
    exit 1
fi

WORKDIR_ABS="$(cd "$WORKDIR"; pwd)"

SOURCEDIR="$(cd $(dirname "$0"); cd ..; pwd)"
test -e "$SOURCEDIR/VERSION" || exit 2

# The number of processors is a good default for -j
if test -e "/proc/cpuinfo"
then
    PROCESSORS="$(grep -c ^processor /proc/cpuinfo)"
else
    PROCESSORS=4
fi

# Extract version from the VERSION file
source "$SOURCEDIR/VERSION"
MYSQL_VERSION="$MYSQL_VERSION_MAJOR.$MYSQL_VERSION_MINOR.$MYSQL_VERSION_PATCH"
PERCONA_SERVER_VERSION="$(echo $MYSQL_VERSION_EXTRA | sed 's/^-//')"
PRODUCT="Percona-Server-$MYSQL_VERSION-$PERCONA_SERVER_VERSION"
TOKUDB_BACKUP_VERSION="${MYSQL_VERSION}${MYSQL_VERSION_EXTRA}"

# Build information
if test -e "$SOURCEDIR/Docs/INFO_SRC"
then
    REVISION="$(cd "$SOURCEDIR"; grep '^short: ' Docs/INFO_SRC |sed -e 's/short: //')"
elif [ -n "$(which git)" -a -d "$SOURCEDIR/.git" ];
then
    REVISION="$(git rev-parse --short HEAD)"
else
    REVISION=""
fi
PRODUCT_FULL="Percona-Server-$MYSQL_VERSION-$PERCONA_SERVER_VERSION"
PRODUCT_FULL="$PRODUCT_FULL${BUILD_COMMENT:-}-$TAG$(uname -s)${DIST_NAME:-}.$TARGET${GLIBC_VER:-}"
COMMENT="Percona Server (GPL), Release ${MYSQL_VERSION_EXTRA#-}"
COMMENT="$COMMENT, Revision $REVISION${BUILD_COMMENT:-}"

# Compilation flags
export CC=${CC:-gcc}
export CXX=${CXX:-g++}

# If gcc >= 4.8 we can use ASAN in debug build but not if valgrind build also
if [[ "$CMAKE_BUILD_TYPE" == "Debug" ]] && [[ "${CMAKE_OPTS:-}" != *WITH_VALGRIND=ON* ]]; then
  GCC_VERSION=$(${CC} -dumpversion)
  GT_VERSION=$(echo -e "4.8.0\n${GCC_VERSION}" | sort -t. -k1,1nr -k2,2nr -k3,3nr | head -1)
  if [ "${GT_VERSION}" = "${GCC_VERSION}" ]; then
    DEBUG_EXTRA="${DEBUG_EXTRA} -DWITH_ASAN=ON"
  fi
fi

# TokuDB cmake flags
if test -d "$SOURCEDIR/storage/tokudb"
then
    CMAKE_OPTS="${CMAKE_OPTS:-} -DBUILD_TESTING=OFF -DUSE_GTAGS=OFF -DUSE_CTAGS=OFF -DUSE_ETAGS=OFF -DUSE_CSCOPE=OFF -DTOKUDB_BACKUP_PLUGIN_VERSION=${TOKUDB_BACKUP_VERSION}"

    if test "x$CMAKE_BUILD_TYPE" != "xDebug"
    then
        CMAKE_OPTS="${CMAKE_OPTS:-} -DTOKU_DEBUG_PARANOID=OFF"
    else
        CMAKE_OPTS="${CMAKE_OPTS:-} -DTOKU_DEBUG_PARANOID=ON"
    fi

    if [[ $CMAKE_OPTS == *WITH_VALGRIND=ON* ]]
    then
        CMAKE_OPTS="${CMAKE_OPTS:-} -DUSE_VALGRIND=ON"
    fi
fi

#
# Attempt to remove any optimisation flags from the debug build
# BLD-238 - bug1408232
if [ -n "$(which rpm)" ]; then
  export COMMON_FLAGS=$(rpm --eval %optflags | sed -e "s|march=i386|march=i686|g")
  if test "x$CMAKE_BUILD_TYPE" = "xDebug"
  then
    COMMON_FLAGS=`echo " ${COMMON_FLAGS} " | \
              sed -e 's/ -O[0-9]* / /' \
                  -e 's/-Wp,-D_FORTIFY_SOURCE=2/ /' \
                  -e 's/ -unroll2 / /' \
                  -e 's/ -ip / /' \
                  -e 's/^ //' \
                  -e 's/ $//'`
  fi
fi
#
export COMMON_FLAGS="$COMMON_FLAGS -DPERCONA_INNODB_VERSION=$PERCONA_SERVER_VERSION"
export CFLAGS="$COMMON_FLAGS ${CFLAGS:-}"
export CXXFLAGS="$COMMON_FLAGS ${CXXFLAGS:-}"
#
export MAKE_JFLAG="${MAKE_JFLAG:--j$PROCESSORS}"
#
# Create a temporary working directory
INSTALLDIR="$(cd "$WORKDIR" && TMPDIR="$WORKDIR_ABS" mktemp -d percona-build.XXXXXX)"
INSTALLDIR="$WORKDIR_ABS/$INSTALLDIR"   # Make it absolute

# Test jemalloc directory
if test "x$WITH_JEMALLOC" != "x"
then
    if ! test -d "$WITH_JEMALLOC"
    then
        echo >&2 "Jemalloc dir $WITH_JEMALLOC does not exist"
        exit 1
    fi

    JEMALLOCDIR="$(cd "$WITH_JEMALLOC"; pwd)"
fi

# Build
(
    rm -rf "$WORKDIR_ABS/bld"
    mkdir "$WORKDIR_ABS/bld"
    cd "$WORKDIR_ABS/bld"

    cmake $SOURCEDIR ${CMAKE_OPTS:-} -DBUILD_CONFIG=mysql_release \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:-RelWithDebInfo} \
        $DEBUG_EXTRA \
        -DFEATURE_SET=community \
        -DCMAKE_INSTALL_PREFIX="/usr/local/$PRODUCT_FULL" \
        -DMYSQL_DATADIR="/usr/local/$PRODUCT_FULL/data" \
        -DROUTER_INSTALL_LIBDIR="/usr/local/$PRODUCT_FULL/lib/mysqlrouter/private" \
        -DROUTER_INSTALL_PLUGINDIR="/usr/local/$PRODUCT_FULL/lib/mysqlrouter/plugin" \
        -DCOMPILATION_COMMENT="$COMMENT" \
        -DWITH_PAM=ON \
        -DWITH_ROCKSDB=ON \
        -DWITH_INNODB_MEMCACHED=ON \
        -DWITH_ZLIB=system \
        -DWITH_NUMA=ON \
        -DWITH_LDAP=system \
        -DFORCE_INSOURCE_BUILD=1 \
        -DWITH_LIBEVENT=bundled \
        -DWITH_ZSTD=bundled \
        $WITH_MECAB_OPTION $OPENSSL_INCLUDE $OPENSSL_LIBRARY $CRYPTO_LIBRARY

    make $MAKE_JFLAG $QUIET
    make DESTDIR="$INSTALLDIR" install

    # Build jemalloc
    if test "x$WITH_JEMALLOC" != x
    then
    (
        cd "$JEMALLOCDIR"

        unset CFLAGS
        unset CXXFLAGS

        ./autogen.sh
        ./configure --prefix="/usr/local/$PRODUCT_FULL/" \
                --libdir="/usr/local/$PRODUCT_FULL/lib/mysql/"
        make $MAKE_JFLAG
        make DESTDIR="$INSTALLDIR" install_lib_shared

        # Copy COPYING file
        cp COPYING "$INSTALLDIR/usr/local/$PRODUCT_FULL/COPYING-jemalloc"
    )
    fi
)

(
    LIBLIST="libcrypto.so libssl.so libreadline.so libtinfo.so libsasl2.so libcurl.so libldap liblber libssh libbrotlidec.so libbrotlicommon.so libgssapi_krb5.so libkrb5.so libkrb5support.so libk5crypto.so librtmp.so libgssapi.so libfreebl3.so libssl3.so libsmime3.so libnss3.so libnssutil3.so libplds4.so libplc4.so libnspr4.so libssl3.so libplds4.so"
    DIRLIST="bin lib lib/private lib/plugin lib/mysqlrouter/plugin lib/mysqlrouter/private"

    LIBPATH=""
    OVERRIDE=false

    function gather_libs {
        local elf_path=$1
        for lib in $LIBLIST; do
            for elf in $(find $elf_path -maxdepth 1 -exec file {} \; | grep 'ELF ' | cut -d':' -f1); do
                IFS=$'\n'
                for libfromelf in $(ldd $elf | grep $lib | awk '{print $3}'); do
                    if [ ! -f lib/private/$(basename $(readlink -f $libfromelf)) ] && [ ! -L lib/$(basename $(readlink -f $libfromelf)) ]; then
                        echo "Copying lib $(basename $(readlink -f $libfromelf))"
                        cp $(readlink -f $libfromelf) lib/private

                        echo "Symlinking lib $(basename $(readlink -f $libfromelf))"
                        cd lib
                        ln -s private/$(basename $(readlink -f $libfromelf)) $(basename $(readlink -f $libfromelf))
                        cd -
                        
                        LIBPATH+=" $(echo $libfromelf | grep -v $(pwd))"
                    fi
                done
                unset IFS
            done
        done
    }

    function set_runpath {
        # Set proper runpath for bins but check before doing anything
        local elf_path=$1
        local r_path=$2
        for elf in $(find $elf_path -maxdepth 1 -exec file {} \; | grep 'ELF ' | cut -d':' -f1); do
            echo "Checking LD_RUNPATH for $elf"
            if [[ -z $(patchelf --print-rpath $elf) ]]; then
                echo "Changing RUNPATH for $elf"
                patchelf --set-rpath $r_path $elf
            fi
            if [[ ! -z $override ]] && [[ $override == "true" ]]; then
                echo "Overriding RUNPATH for $elf"
                patchelf --set-rpath $r_path $elf
            fi
        done
    }

    function replace_libs {
        local elf_path=$1
        for libpath_sorted in $LIBPATH; do
            for elf in $(find $elf_path -maxdepth 1 -exec file {} \; | grep 'ELF ' | cut -d':' -f1); do
                LDD=$(ldd $elf | grep $libpath_sorted|head -n1|awk '{print $1}')
                if [[ ! -z $LDD  ]]; then
                    echo "Replacing lib $(basename $(readlink -f $libpath_sorted)) for $elf"
                    patchelf --replace-needed $LDD $(basename $(readlink -f $libpath_sorted)) $elf
                fi
            done
        done
    }

    function check_libs {
        local elf_path=$1
        for elf in $(find $elf_path -maxdepth 1 -exec file {} \; | grep 'ELF ' | cut -d':' -f1); do
            if ! ldd $elf; then
                exit 1
            fi
        done
    }

    function link {
        if [ ! -d lib/private ]; then
            mkdir -p lib/private
        fi
        # Gather libs
        for DIR in $DIRLIST; do
            gather_libs $DIR
        done
        # Set proper runpath
        export override=false
        set_runpath bin '$ORIGIN/../lib/private/'
        set_runpath lib '$ORIGIN/private/'
        set_runpath lib/plugin '$ORIGIN/../private/'
        set_runpath lib/private '$ORIGIN'
        # LIBS MYSQLROUTER
        unset override && export override=true && set_runpath lib/mysqlrouter/plugin '$ORIGIN/:$ORIGIN/../private/:$ORIGIN/../../private/'
        unset override && export override=true && set_runpath lib/mysqlrouter/private '$ORIGIN/:$ORIGIN/../plugin/:$ORIGIN/../../private/'
        #  BINS MYSQLROUTER
        unset override && export override=true && set_runpath bin/mysqlrouter_passwd '$ORIGIN/../lib/mysqlrouter/private/:$ORIGIN/../lib/mysqlrouter/plugin/:$ORIGIN/../lib/private/'
        unset override && export override=true && set_runpath bin/mysqlrouter_plugin_info '$ORIGIN/../lib/mysqlrouter/private/:$ORIGIN/../lib/mysqlrouter/plugin/:$ORIGIN/../lib/private/'
        unset override && export override=true && set_runpath bin/mysqlrouter '$ORIGIN/../lib/mysqlrouter/private/:$ORIGIN/../lib/mysqlrouter/plugin/:$ORIGIN/../lib/private/'
        unset override && export override=true && set_runpath bin/mysqlrouter_keyring '$ORIGIN/../lib/mysqlrouter/private/:$ORIGIN/../lib/mysqlrouter/plugin/:$ORIGIN/../lib/private/'
        # Replace libs
        for DIR in $DIRLIST; do
            replace_libs $DIR
        done
        # Make final check in order to determine any error after linkage
        for DIR in $DIRLIST; do
            check_libs $DIR
        done
    }

    mkdir $INSTALLDIR/usr/local/minimal
    cp -r "$INSTALLDIR/usr/local/$PRODUCT_FULL" "$INSTALLDIR/usr/local/minimal/$PRODUCT_FULL"

    # NORMAL TARBALL
    cd "$INSTALLDIR/usr/local/$PRODUCT_FULL"
    link

    # MIN TARBALL
    cd "$INSTALLDIR/usr/local/minimal/$PRODUCT_FULL"
    rm -rf mysql-test 2> /dev/null
    find . -type f -exec file '{}' \; | grep ': ELF ' | cut -d':' -f1 | xargs strip --strip-unneeded
    link
)

# Package the archive
(
    cd "$INSTALLDIR/usr/local/"
    #PS-4854 Percona Server for MySQL tarball without AGPLv3 dependency/license
    find $PRODUCT_FULL -type f -name 'COPYING.AGPLv3' -delete
    $TAR --owner=0 --group=0 -czf "$WORKDIR_ABS/$PRODUCT_FULL.tar.gz" $PRODUCT_FULL

    cd "$INSTALLDIR/usr/local/minimal/"
    find $PRODUCT_FULL -type f -name 'COPYING.AGPLv3' -delete
    $TAR --owner=0 --group=0 -czf "$WORKDIR_ABS/$PRODUCT_FULL-minimal.tar.gz" $PRODUCT_FULL
)

# Clean up
rm -rf "$INSTALLDIR"
rm -rf "$WORKDIR_ABS/bld"

