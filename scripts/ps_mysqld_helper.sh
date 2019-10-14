#!/bin/bash
if [ -f /etc/redhat-release -o -f /etc/system-release ]
then
    SYSTEMD_ENV_FILE="/etc/sysconfig/mysql"
else
    SYSTEMD_ENV_FILE="/etc/default/mysql"
fi
enable_thp() {
    STATUS_THP_SYSTEM=0
    if [ -f /sys/kernel/mm/transparent_hugepage/enabled ]; then
        CONTENT_TRANSHP=$(</sys/kernel/mm/transparent_hugepage/enabled)
        STATUS_THP_SYSTEM=$(echo $CONTENT_TRANSHP | grep -cv '\[never\]')
    fi
    if [ $STATUS_THP_SYSTEM = 1 ]
    then
        THP_CONF=$(grep -c "THP_SETTING=never" $SYSTEMD_ENV_FILE)
        if [ $THP_CONF = 1 ]
        then
            if test -f /sys/kernel/mm/transparent_hugepage/defrag; then
                echo never > /sys/kernel/mm/transparent_hugepage/defrag
            fi
            if test -f /sys/kernel/mm/transparent_hugepage/enabled; then
                echo never > /sys/kernel/mm/transparent_hugepage/enabled
            fi
            if [ $? -ne 0 ]; then
                exit 1
            fi
        fi
    fi
}

enable_jemalloc(){
    JEMALLOC_LOCATION=0
    for libjemall in "/usr/lib64" "/usr/lib/x86_64-linux-gnu" "/usr/lib"; do
        if [ -r "${libjemall}/libjemalloc.so.1" ]; then
            JEMALLOC_LOCATION="${libjemall}/libjemalloc.so.1"
            break
        fi
    done
    if [ "x${JEMALLOC_LOCATION}" != "x0" ]; then
        STATUS_JEMALLOC_CONFIG=$(grep -c -e "LD_PRELOAD=.*libjemalloc.so.1" ${SYSTEMD_ENV_FILE})
        if [ $STATUS_JEMALLOC_CONFIG = 0 ]; then
            echo "LD_PRELOAD=${JEMALLOC_LOCATION}" >> ${SYSTEMD_ENV_FILE}
        fi
    fi
}

#main
enable_thp
enable_jemalloc
