#!/bin/sh
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

# Some programs that may be overriden
TAR=${TAR:-tar}

# Check if we have a functional getopt(1)
if ! getopt --test
then
    go_out="$(getopt --options="i" --longoptions=i686 \
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

    WORKDIR_ABS="$(cd "$WORKDIR"; pwd)"

elif test "$#" -eq 1
then
    WORKDIR="$1"

    # Check that the provided directory exists and is a directory
    if ! test -d "$WORKDIR"
    then
        echo >&2 "$WORKDIR is not a directory"
        exit 1
    fi

    WORKDIR_ABS="$(cd "$WORKDIR"; pwd)"

else
    echo >&2 "Usage: $0 [target dir]"
    exit 1

fi

SOURCEDIR="$(cd $(dirname "$0"); cd ..; pwd)"
test -e "$SOURCEDIR/Makefile" || exit 2

# Extract version from the Makefile
MYSQL_VERSION="$(grep ^MYSQL_VERSION= "$SOURCEDIR/Makefile" \
    | cut -d = -f 2)"
PERCONA_SERVER_VERSION="$(grep ^PERCONA_SERVER_VERSION= \
    "$SOURCEDIR/Makefile" | cut -d = -f 2)"
PRODUCT="Percona-Server-$MYSQL_VERSION"

# Build information
REVISION="$(cd "$SOURCEDIR"; bzr log -r-1 | grep ^revno: | cut -d ' ' -f 2)"
PRODUCT_FULL="Percona-Server-$MYSQL_VERSION-$PERCONA_SERVER_VERSION"
PRODUCT_FULL="$PRODUCT_FULL-$REVISION.$(uname -s).$TARGET"
COMMENT="Percona Server with XtraDB (GPL), Release $PERCONA_SERVER_VERSION"
COMMENT="$COMMENT, Revision $REVISION"

# Compilation flags
export CC=${CC:-gcc}
export CXX=${CXX:-gcc}
export CFLAGS="-fPIC -Wall -O3 -g -static-libgcc -fno-omit-frame-pointer $TARGET_CFLAGS ${CFLAGS:-}"
export CXXFLAGS="-O2 -fno-omit-frame-pointer -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fno-exceptions $TARGET_CFLAGS ${CXXFLAGS:-}"
export MAKE_JFLAG=-j4

# Create a temporary working directory
INSTALLDIR="$(cd "$WORKDIR" && TMPDIR="$WORKDIR_ABS" mktemp -d percona-build.XXXXXX)"
INSTALLDIR="$WORKDIR_ABS/$INSTALLDIR"   # Make it absolute

# Build
(
    cd "$SOURCEDIR"
 
    # Execute clean and download mysql, apply patches
    make clean all

    cd "$PRODUCT"
    ./configure \
        --prefix="/usr/local/$PRODUCT_FULL" \
        --localstatedir="/usr/local/$PRODUCT_FULL/data" \
        --with-server-suffix="$PERCONA_SERVER_VERSION" \
        --with-plugins=partition,archive,blackhole,csv,example,federated,innodb_plugin \
        --without-embedded-server \
        --with-comment="$COMMENT" \
        --enable-assembler \
        --enable-local-infile \
        --with-mysqld-user=mysql \
        --with-unix-socket-path=/var/lib/mysql/mysql.sock \
        --with-pic \
        --with-extra-charsets=complex \
        --with-ssl \
        --enable-thread-safe-client \
        --enable-profiling \
        --with-readline 

    make $MAKE_JFLAG VERBOSE=1
    make DESTDIR="$INSTALLDIR" install

    # Build HandlerSocket
    (
        cd "storage/HandlerSocket-Plugin-for-MySQL"
        ./autogen.sh
        CXX=${HS_CXX:-g++} ./configure --with-mysql-source="$SOURCEDIR/$PRODUCT" \
            --with-mysql-bindir="$SOURCEDIR/$PRODUCT/scripts" \
            --with-mysql-plugindir="/usr/local/$PRODUCT_FULL/lib/mysql/plugin" \
            --libdir="/usr/local/$PRODUCT_FULL/lib/mysql/plugin" \
            --prefix="/usr/local/$PRODUCT_FULL"
        make
        make DESTDIR="$INSTALLDIR" install

    )

    # Build UDF
    (
        cd "UDF"
        CXX=${UDF_CXX:-g++} ./configure --includedir="$SOURCEDIR/$PRODUCT/include" \
            --libdir="/usr/local/$PRODUCT_FULL/mysql/plugin"
        make
        make DESTDIR="$INSTALLDIR" install

    )

)

# Package the archive
(
    cd "$INSTALLDIR/usr/local/"

    $TAR czf "$WORKDIR_ABS/$PRODUCT_FULL.tar.gz" \
        --owner=0 --group=0 "$PRODUCT_FULL/"
    
)

# Clean up
rm -rf "$INSTALLDIR"

