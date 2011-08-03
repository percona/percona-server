#!/bin/sh
#
# Execute this tool to setup and build RPMs for Percona-Server starting
# from a fresh tree
#
# Usage: build-rpm.sh [--i686] [--nosign] [--test] [target dir]
# The default target directory is the current directory. If it is not
# supplied and the current directory is not empty, it will issue an error in
# order to avoid polluting the current directory after a test run.
#
# The program will setup the rpm building environment and ultimately call
# rpmbuild with the appropiate parameters.
#

# Bail out on errors, be strict
set -ue

# Examine parameters
TARGET=''
TARGET_CFLAGS=''
TARGET_ARG=''
SIGN='--sign'  # We sign by default
TEST='no'    # We don't test by default
QUIET=''

# Check if we have got a functional getopt(1)
if ! getopt --test
then
    go_out="$(getopt --options="iKtq" --longoptions=i686,nosign,test,quiet \
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
        TARGET="--target i686"
        TARGET_CFLAGS="-m32 -march=i686"
        TARGET_ARG='i686'
        ;;
    -K | --nosign )
        shift
        SIGN=''
        ;;
    -t | --test )
        shift
        TEST='yes'
        ;;
    -q | --quiet )
        shift
        QUIET='--quiet'
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
PRODUCT="Percona-Server-$MYSQL_VERSION"

# Build information
REDHAT_RELEASE="$(grep -o 'release [0-9][0-9]*' /etc/redhat-release | \
    cut -d ' ' -f 2)"
PATCHSET="$(grep ^PATCHSET= "$SOURCEDIR/Makefile" | cut -d = -f 2)"
REVISION="$(cd "$SOURCEDIR"; bzr log -r-1 | grep ^revno: | cut -d ' ' -f 2)"

# Compilation flags
export CC=${CC:-gcc}
export CXX=${CXX:-gcc}
export HS_CXX=${HS_CXX:-g++}
export UDF_CXX=${UDF_CXX:-g++}
export CFLAGS="-fPIC -Wall -O3 -g -static-libgcc -fno-omit-frame-pointer $TARGET_CFLAGS"
export CXXFLAGS="-O2 -fno-omit-frame-pointer -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fno-exceptions $TARGET_CFLAGS"
export MAKE_JFLAG=-j4

export MYSQL_RPMBUILD_TEST="$TEST"

# Fix problems in rpmbuild for rhel4: _libdir and _arch are not correctly set.
if test "x$REDHAT_RELEASE" == "x4" && test "x$TARGET_ARG" == "xi686"
then
    TARGET_LIBDIR='--define=_libdir\ /usr/lib'
    TARGET_ARCH='--define=_arch\ i386'
else
    TARGET_LIBDIR=''
    TARGET_ARCH=''
fi

# Create directories for rpmbuild if these don't exist
(cd "$WORKDIR" && mkdir -p BUILD RPMS SOURCES SPECS SRPMS) || false

(
    cd "$SOURCEDIR"

    # Download mysql, prepare patches
    make clean all

    # Create tarball for build
    tar czf "$WORKDIR_ABS/SOURCES/$PRODUCT.tar.gz" "$PRODUCT/"*

) || false

(
    cd "$WORKDIR"

    # Issue RPM command
    eval rpmbuild -ba --clean --with yassl $TARGET $TARGET_LIBDIR $TARGET_ARCH \
        $QUIET $SIGN "$SOURCEDIR/build/percona-server.spec" \
        --define "_topdir\ $WORKDIR_ABS" \
        --define "gotrevision\ $REVISION"

)

