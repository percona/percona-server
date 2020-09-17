#!/bin/bash

function check_libs {
    local elf_path=$1

    for elf in $(find $elf_path -maxdepth 1 -exec file {} \; | grep 'ELF ' | cut -d':' -f1); do
        echo "$elf"
        ldd "$elf"
    done
    return
}

function prepare {
    CURDIR=$(pwd)
    TMP_DIR="$CURDIR/temp"

    mkdir -p "$CURDIR"/temp
    mkdir -p "$TMP_DIR"/dbdeployer/tarball "$TMP_DIR"/dbdeployer/deployment

    TARBALLS=""
    for tarball in $(find . -name "*.tar.gz"); do
        TARBALLS+=" $(basename $tarball)"
    done

    DIRLIST="bin lib lib/private lib/plugin lib/mysqlrouter/plugin lib/mysqlrouter/private"
}

function install_deps {
    if [ -f /etc/redhat-release ]; then
        yum install -y epel-release
        yum install -y wget perl-Time-HiRes perl numactl numactl-libs libaio libidn libcurl-devel || true
    else
        apt install -y wget perl numactl libaio-dev libidn11 libcurl4-openssl-dev || true
    fi
    wget -c https://github.com/datacharmer/dbdeployer/releases/download/v1.52.0/dbdeployer-1.52.0.linux.tar.gz -O - | tar -xz
    mv dbdeployer*.linux /usr/local/bin/dbdeployer
    export PATH=$PATH:/usr/local/bin
}

main () {
    prepare
    for tarfile in $TARBALLS; do
        echo "Unpacking tarball: $tarfile"
        cd "$TMP_DIR"
        tar xf "$CURDIR/$tarfile"
        cd "${tarfile%.tar.gz}"

        echo "Building ELFs ldd output list"
        for DIR in $DIRLIST; do
            if ! check_libs "$DIR" >> "$TMP_DIR"/libs_err.log; then
                echo "There is an error with libs linkage"
                echo "Displaying log: "
                cat "$TMP_DIR"/libs_err.log
                exit 1
            fi
        done

         echo "Checking for missing libraries"
         if [[ ! -z $(grep "not found" $TMP_DIR/libs_err.log) ]]; then
            echo "ERROR: There are missing libraries: "
            grep "not found" "$TMP_DIR"/libs_err.log
            echo "Log: "
            cat "$TMP_DIR"/libs_err.log
            exit 1
        fi

        echo "Invoking dbdeployer to make a test run"
        dbdeployer unpack --sandbox-binary="$TMP_DIR"/dbdeployer/tarball --prefix=ps "$CURDIR/$tarfile"
        dbdeployer deploy single --sandbox-home="$TMP_DIR"/dbdeployer/deployment --sandbox-binary="$TMP_DIR"/dbdeployer/tarball "$(ls $TMP_DIR/dbdeployer/tarball)"
        if [[ $? -eq 0 ]]; then
            SANDBOX="$(dbdeployer sandboxes --sandbox-home=$TMP_DIR/dbdeployer/deployment | awk '{print $1}')"
            if ! "$TMP_DIR"/dbdeployer/deployment/"$SANDBOX"/test_sb; then
                exit 1
            else
                dbdeployer delete --sandbox-home="$TMP_DIR"/dbdeployer/deployment --sandbox-binary="$TMP_DIR"/dbdeployer/tarball "$SANDBOX"
                dbdeployer delete-binaries --skip-confirm --sandbox-home="$TMP_DIR"/dbdeployer/deployment --sandbox-binary="$TMP_DIR"/dbdeployer/tarball "$(ls $TMP_DIR/dbdeployer/tarball)"
            fi
        fi
        rm -rf "${TMP_DIR:?}/*"
    done
}

case "$1" in
    --install_deps) install_deps ;;
    --test) main ;;
    --help|*)
    cat <<EOF
Usage: $0 [OPTIONS]
    The following options may be given :
        --install_deps
        --test
        --help) usage ;;
Example $0 --install_deps 
EOF
    ;;
esac
