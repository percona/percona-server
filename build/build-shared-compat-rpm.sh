#!/bin/sh
#
# Execute this tool to setup and build the shared-compat RPM starting
# from a fresh tree
#
# Usage: build-shared-compat-rpm.sh [target dir]
# The default target directory is the current directory. If it is not
# supplied and the current directory is not empty, it will issue an error in
# order to avoid polluting the current directory after a test run.
#
# The program will setup the rpm building environment, download the sources
# and ultimately call rpmbuild with the appropiate parameters.
#

# Bail out on errors, be strict
set -ue

# Examine parameters
TARGET=''
TARGET_ARG=''
TARGET_CFLAGS=''
SIGN='--sign'

# Check if we have a functional getopt(1)
if ! getopt --test
then
    go_out="$(getopt --options="iK" --longoptions=i686,nosign \
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
        TARGET='i686'
        TARGET_ARG="--target i686"
        TARGET_CFLAGS="-m32 -march=i686"
        ;;
    -K | --nosign )
        shift
        SIGN=''
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
PRODUCT="Percona-Server-$MYSQL_VERSION-$PERCONA_SERVER_VERSION"

# Build information
REDHAT_RELEASE="$(grep -o 'release [0-9][0-9]*' /etc/redhat-release | \
    cut -d ' ' -f 2)"
REVISION="$(cd "$SOURCEDIR"; bzr log -r-1 | grep ^revno: | cut -d ' ' -f 2)"

# Compilation flags
export CC=gcc
export CXX=gcc
export CFLAGS="-fPIC -Wall -O3 -g -static-libgcc -fno-omit-frame-pointer $TARGET_CFLAGS"
export CXXFLAGS="-O2 -fno-omit-frame-pointer -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fno-exceptions $TARGET_CFLAGS"
export MAKE_JFLAG=-j4

# Create directories for rpmbuild if these don't exist
(cd "$WORKDIR" && mkdir -p BUILD RPMS SOURCES SPECS SRPMS)

# Prepare sources
(
    cd "$WORKDIR/SOURCES/"
 
    # Download the sources from the community site
    if test "x$TARGET" = "xi686"
    then
        RPMVER=i386
    elif test "x$(uname -m)" = "xx86_64"
    then
        RPMVER=x86_64
    else
        RPMVER=i386
    fi

    wget "http://www.percona.com/downloads/community/shared-compat/MySQL-shared-compat-$MYSQL_VERSION-1.linux2.6.$RPMVER.rpm"

)

# Issue rpmbuild command
(
    cd "$WORKDIR"

    # Issue RPM command
    rpmbuild -ba --clean --with yassl $SIGN \
        "$SOURCEDIR/build/percona-shared-compat.spec" \
        --define "_topdir $WORKDIR_ABS" \
        --define "redhat_version $REDHAT_RELEASE" \
        --define "gotrevision $REVISION" \
        --define "release $PERCONA_SERVER_VERSION" \
        $TARGET_ARG

)

