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
WITH_ZENFS='OFF'
ZENFS_EXTRA=''
OPENSSL_INCLUDE=''
OPENSSL_LIBRARY=''
CRYPTO_LIBRARY=''
TAG=''
#
CMAKE_BUILD_TYPE=''
COMMON_FLAGS=''
#
TOKUDB_BACKUP_VERSION=''
# enable asan
ENABLE_ASAN=0
#
# Some programs that may be overriden
TAR=${TAR:-tar}

# Check if we have a functional getopt(1)
if ! getopt --test
then
    go_out="$(getopt --options=iqdvj:m:t: \
        --longoptions=i686,quiet,debug,valgrind,with-jemalloc:,with-zenfs,with-mecab:,with-ssl:,tag: \
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
        TARBALL_SUFFIX="-debug"
        DEBUG_EXTRA="-DDEBUG_EXTNAME=OFF -DWITH_DEBUG=ON"
        ;;
    -a | --asan )
        shift
        ENABLE_ASAN=1
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
    --with-zenfs )
        shift
        TARBALL_SUFFIX="-zenfs"
        WITH_ZENFS="ON"
        ZENFS_EXTRA="-DROCKSDB_PLUGINS=zenfs -DWITH_ZENFS_UTILITY=ON"
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
test -e "$SOURCEDIR/MYSQL_VERSION" || exit 2

# The number of processors is a good default for -j
if test -e "/proc/cpuinfo"
then
    PROCESSORS="$(grep -c ^processor /proc/cpuinfo)"
else
    PROCESSORS=4
fi

# Extract version from the VERSION file
source "$SOURCEDIR/MYSQL_VERSION"
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
PRODUCT_FULL="$PRODUCT_FULL-$TAG$(uname -s)${DIST_NAME:-}.$TARGET${GLIBC_VER:-}${TARBALL_SUFFIX:-}"
COMMENT="Percona Server (GPL), Release ${MYSQL_VERSION_EXTRA#-}"
COMMENT="$COMMENT, Revision $REVISION${BUILD_COMMENT:-}"

# Compilation flags
export CC=${CC:-gcc}
export CXX=${CXX:-g++}

# If gcc >= 4.8 we can use ASAN in debug build but not if valgrind build also
if [[ $ENABLE_ASAN -eq 1 ]]; then
    if [[ "$CMAKE_BUILD_TYPE" == "Debug" ]] && [[ "${CMAKE_OPTS:-}" != *WITH_VALGRIND=ON* ]]; then
        GCC_VERSION=$(${CC} -dumpversion)
        GT_VERSION=$(echo -e "4.8.0\n${GCC_VERSION}" | sort -t. -k1,1nr -k2,2nr -k3,3nr | head -1)
        if [ "${GT_VERSION}" = "${GCC_VERSION}" ]; then
            DEBUG_EXTRA="${DEBUG_EXTRA} -DWITH_ASAN=ON"
        fi
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
        $ZENFS_EXTRA \
        -DFEATURE_SET=community \
        -DCMAKE_INSTALL_PREFIX="/usr/local/$PRODUCT_FULL" \
        -DMYSQL_DATADIR="/usr/local/$PRODUCT_FULL/data" \
        -DROUTER_INSTALL_LIBDIR="/usr/local/$PRODUCT_FULL/lib/mysqlrouter/private" \
        -DROUTER_INSTALL_PLUGINDIR="/usr/local/$PRODUCT_FULL/lib/mysqlrouter/plugin" \
        -DCOMPILATION_COMMENT="$COMMENT" \
        -DWITH_PAM=ON \
        -DWITH_ROCKSDB=ON \
        -DROCKSDB_DISABLE_AVX2=1 \
        -DROCKSDB_DISABLE_MARCH_NATIVE=1 \
        -DWITH_INNODB_MEMCACHED=ON \
        -DWITH_ZLIB=bundled \
        -DWITH_NUMA=ON \
        -DWITH_LDAP=system \
        -DDOWNLOAD_BOOST=1 \
        -DWITH_PACKAGE_FLAGS=OFF \
        -DFORCE_INSOURCE_BUILD=1 \
        -DWITH_LIBEVENT=bundled \
        -DWITH_ZSTD=bundled \
        -DWITH_BOOST="$WORKDIR_ABS/libboost" \
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
    LIBLIST="libgssapi.so libldap_r-2.4.so.2 libldap.so.2 liblber-2.4.so.2 liblber.so.2 libcrypto.so libssl.so libreadline.so libtinfo.so libsasl2.so libbrotlidec.so libbrotlicommon.so librtmp.so libgssapi_krb5.so libkrb5.so libk5crypto.so libssl3.so libsmime3.so libnss3.so libnssutil3.so libplc4.so libnspr4.so libssl3.so libplds4.so libncurses.so.5 libtinfo.so.5 component_encryption_udf.so component_keyring_kms.so"
    DIRLIST="bin lib lib/private lib/plugin lib/mysqlrouter/plugin lib/mysqlrouter/private"

    LIBPATH=""

    function gather_libs {
        local elf_path=$1
        for lib in ${LIBLIST}; do
            for elf in $(find ${elf_path} -maxdepth 1 -exec file {} \; | grep 'ELF ' | cut -d':' -f1); do
                IFS=$'\n'
                for libfromelf in $(ldd ${elf} | grep ${lib} | awk '{print $3}'); do
                    lib_realpath="$(readlink -f ${libfromelf})"
                    lib_realpath_basename="$(basename $(readlink -f ${libfromelf}))"
                    lib_without_version_suffix=$(echo ${lib_realpath_basename} | awk -F"." 'BEGIN { OFS = "." }{ print $1, $2}')

                    if [ ! -f "lib/private/${lib_realpath_basename}" ] && [ ! -L "lib/private/${lib_without_version_suffix}" ]; then
                    
                        echo "Copying lib ${lib_realpath_basename}"
                        cp ${lib_realpath} lib/private

                        echo "Symlinking lib from ${lib_realpath_basename} to ${lib_without_version_suffix}"
                        cd lib/
                        ln -s private/${lib_realpath_basename} ${lib_without_version_suffix}
                        cd -
                        if [ ${lib_realpath_basename} != ${lib_without_version_suffix} ]; then
                            cd lib/private
                            ln -s ${lib_realpath_basename} ${lib_without_version_suffix}
                            cd -
                        fi

                        patchelf --set-soname ${lib_without_version_suffix} lib/private/${lib_realpath_basename}

                        LIBPATH+=" $(echo ${libfromelf} | grep -v $(pwd))"
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
        local mode=${3:-}
        for elf in $(find ${elf_path} -maxdepth 1 -exec file {} \; | grep 'ELF ' | cut -d':' -f1); do
            echo "Checking LD_RUNPATH for ${elf}"
            if [[ -z $(patchelf --print-rpath ${elf}) ]]; then
                echo "Changing RUNPATH for ${elf}"
                patchelf --set-rpath ${r_path} ${elf}
            elif [[ ! -z ${mode} ]] && [[ ${mode} == "override" ]]; then
                echo "Overriding RUNPATH for ${elf}"
                patchelf --set-rpath ${r_path} ${elf}
            fi
        done
    }

    function replace_libs {
        local elf_path=$1
        for libpath_sorted in ${LIBPATH}; do
            for elf in $(find ${elf_path} -maxdepth 1 -exec file {} \; | grep 'ELF ' | cut -d':' -f1); do
                LDD=$(ldd ${elf} | grep ${libpath_sorted}|head -n1|awk '{print $1}')
                lib_realpath_basename="$(basename $(readlink -f ${libpath_sorted}))"
                lib_without_version_suffix="$(echo ${lib_realpath_basename} | awk -F"." 'BEGIN { OFS = "." }{ print $1, $2}')"
                if [[ ! -z $LDD  ]]; then
                    echo "Replacing lib ${lib_realpath_basename} to ${lib_without_version_suffix} for ${elf}"
                    patchelf --replace-needed ${LDD} ${lib_without_version_suffix} ${elf}
                fi
            done
        done
    }

    function check_libs {
        local elf_path=$1
        for elf in $(find ${elf_path} -maxdepth 1 -exec file {} \; | grep 'ELF ' | cut -d':' -f1); do
            if ! ldd ${elf}; then
                exit 1
            fi
        done
    }

    function link {
        if [ ! -d lib/private ]; then
            mkdir -p lib/private
        fi
        # Gather libs
        for DIR in ${DIRLIST}; do
            gather_libs ${DIR}
        done
        # Set proper runpath
        set_runpath bin '$ORIGIN/../lib/private/'
        if [[ ${CMAKE_BUILD_TYPE} == "Debug" ]]; then
            set_runpath lib '$ORIGIN/private/' override
            set_runpath lib/plugin '$ORIGIN/../private/' override
        else
            set_runpath lib '$ORIGIN/private/'
            set_runpath lib/plugin '$ORIGIN/../private/'
        fi
        set_runpath lib/private '$ORIGIN'
        # LIBS MYSQLROUTER
        set_runpath lib/mysqlrouter/plugin '$ORIGIN/:$ORIGIN/../private/:$ORIGIN/../../private/' override
        set_runpath lib/mysqlrouter/private '$ORIGIN/:$ORIGIN/../plugin/:$ORIGIN/../../private/' override
        #  BINS MYSQLROUTER
        set_runpath bin/mysqlrouter_passwd '$ORIGIN/../lib/mysqlrouter/private/:$ORIGIN/../lib/mysqlrouter/plugin/:$ORIGIN/../lib/private/' override
        set_runpath bin/mysqlrouter_plugin_info '$ORIGIN/../lib/mysqlrouter/private/:$ORIGIN/../lib/mysqlrouter/plugin/:$ORIGIN/../lib/private/' override
        set_runpath bin/mysqlrouter '$ORIGIN/../lib/mysqlrouter/private/:$ORIGIN/../lib/mysqlrouter/plugin/:$ORIGIN/../lib/private/' override
        set_runpath bin/mysqlrouter_keyring '$ORIGIN/../lib/mysqlrouter/private/:$ORIGIN/../lib/mysqlrouter/plugin/:$ORIGIN/../lib/private/' override
        # Replace libs
        for DIR in ${DIRLIST}; do
            replace_libs ${DIR}
        done
        # Make final check in order to determine any error after linkage
        for DIR in ${DIRLIST}; do
            check_libs ${DIR}
        done
    }

    if [[ $CMAKE_BUILD_TYPE != "Debug" ]]; then
        mkdir $INSTALLDIR/usr/local/minimal
        cp -r "$INSTALLDIR/usr/local/$PRODUCT_FULL" "$INSTALLDIR/usr/local/minimal/$PRODUCT_FULL-minimal"
    fi

    # NORMAL TARBALL
    if [[ ${WITH_ZENFS} != "ON" ]]; then
        cd "$INSTALLDIR/usr/local/$PRODUCT_FULL"
        link
    fi

    # MIN TARBALL
    if [[ $CMAKE_BUILD_TYPE != "Debug" ]]; then
        cd "$INSTALLDIR/usr/local/minimal/$PRODUCT_FULL-minimal"
        rm -rf mysql-test 2> /dev/null
        find . -type f -exec file '{}' \; | grep ': ELF ' | cut -d':' -f1 | xargs strip --strip-unneeded
        if [[ ${WITH_ZENFS} != "ON" ]]; then
            link
        fi
    fi
)

# Package the archive
(
    cd "$INSTALLDIR/usr/local/"
    #PS-4854 Percona Server for MySQL tarball without AGPLv3 dependency/license
    find $PRODUCT_FULL -type f -name 'COPYING.AGPLv3' -delete
    $TAR --owner=0 --group=0 -czf "$WORKDIR_ABS/$PRODUCT_FULL.tar.gz" $PRODUCT_FULL

    if [[ $CMAKE_BUILD_TYPE != "Debug" ]]; then
        cd "$INSTALLDIR/usr/local/minimal/"
        find $PRODUCT_FULL-minimal -type f -name 'COPYING.AGPLv3' -delete
        $TAR --owner=0 --group=0 -czf "$WORKDIR_ABS/$PRODUCT_FULL-minimal.tar.gz" $PRODUCT_FULL-minimal
    fi
)

# Clean up
rm -rf "$INSTALLDIR"
rm -rf "$WORKDIR_ABS/libboost"
rm -rf "$WORKDIR_ABS/bld"
