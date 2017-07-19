#!/bin/bash
enable_thp() {
    if [ -f /etc/redhat-release ]
    then
        SYSTEMD_ENV_FILE="/etc/sysconfig/mysql"
    else
        SYSTEMD_ENV_FILE="/etc/default/mysql"
    fi
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

#main
enable_thp
