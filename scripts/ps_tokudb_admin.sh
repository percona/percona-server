#!/bin/bash
#
# Script for installing TokuDB engine and TokuBackup in Percona Server
#
set -u

# Examine parameters
# default user
USER="root"
# default pass
PASSWORD=""
SOCKET=""
HOST=""
PORT=""
STATUS_THP_SYSTEM=0
STATUS_THP_MYCNF=0
STATUS_PLUGIN=0
ENABLE=0
DISABLE=0
ENABLE_TOKUBACKUP=0
DISABLE_TOKUBACKUP=0
STATUS_HOTBACKUP_MYCNF=0
STATUS_HOTBACKUP_PLUGIN=0
STATUS_JEMALLOC_CONFIG=0
MYCNF_LOCATION=""
DEFAULTS_FILE_OPTION=""
MYSQLD_SAFE_STATUS=0
LIBHOTBACKUP_STATUS=0
FULL_SYSTEMD_MODE=0
JEMALLOC_LOCATION=""
HOTBACKUP_LOCATION=""
FORCE_MYCNF=0
FORCE_ENVFILE=0
DOCKER=0

SCRIPT_PWD=$(cd `dirname $0` && pwd)
MYSQL_CLIENT_BIN="${SCRIPT_PWD}/mysql"
MYSQL_DEFAULTS_BIN="${SCRIPT_PWD}/my_print_defaults"
if [ -f /etc/redhat-release -o -f /etc/system-release ]
then
  SYSTEMD_ENV_FILE="/etc/sysconfig/mysql"
else
  SYSTEMD_ENV_FILE="/etc/default/mysql"
fi

# Check if we have a functional getopt(1)
if ! getopt --test
  then
  go_out="$(getopt --options=u:p::S:h:P:edbrfmD \
  --longoptions=user:,password::,socket:,host:,port:,enable,disable,enable-backup,disable-backup,help,defaults-file:,force-envfile,force-mycnf,docker \
  --name="$(basename "$0")" -- "$@")"
  test $? -eq 0 || exit 1
  eval set -- $go_out
fi

for arg
do
  case "$arg" in
    -- ) shift; break;;
    -u | --user )
    USER="$2"
    shift 2
    ;;
    -p | --password )
    case "$2" in
      "")
      read -s -p "Enter password:" INPUT_PASS
      if [ -z "$INPUT_PASS" ]; then
        PASSWORD=""
	printf "\nContinuing without password...\n";
      else
        PASSWORD="-p$INPUT_PASS"
      fi
      printf "\n\n"
      ;;
      *)
      PASSWORD="-p$2"
      ;;
    esac
    shift 2
    ;;
    -S | --socket )
    SOCKET="-S $2"
    shift 2
    ;;
    -h | --host )
    HOST="-h $2"
    shift 2
    ;;
    -P | --port )
    PORT="-P $2"
    shift 2
    ;;
    --defaults-file )
    MYCNF_LOCATION="$2"
    DEFAULTS_FILE_OPTION="--defaults-file=$MYCNF_LOCATION"
    shift 2
    ;;
    -e | --enable )
    shift
    ENABLE=1
    ;;
    -d | --disable )
    shift
    DISABLE=1
    DISABLE_TOKUBACKUP=1
    ;;
    -b | --enable-backup )
    shift
    ENABLE_TOKUBACKUP=1
    ENABLE=1
    ;;
    -r | --disable-backup )
    shift
    DISABLE_TOKUBACKUP=1
    ;;
    -m | --force-mycnf )
    shift
    FORCE_MYCNF=1
    ;;
    -f | --force-envfile )
    shift
    FORCE_ENVFILE=1
    ;;
    -D | --docker )
    shift
    DOCKER=1
    ;;
    --help )
    printf "WARNING: This script is deprecated and will be removed in 8.0. You can use ps-admin script which has more functionality.\n\n"
    printf "This script is used for installing and uninstalling TokuDB plugin for Percona Server 5.7.\n"
    printf "It can also be used to install or uninstall the Percona TokuBackup plugin (requires mysql server restart).\n"
    printf "If transparent huge pages are enabled on the system it adds thp-setting=never option to my.cnf\n"
    printf "to disable it on runtime.\n\n"
    printf "Valid options are:\n"
    printf "  --user=user_name, -u user_name\t mysql admin username\n"
    printf "  --password[=password], -p[password]\t mysql admin password (on empty will prompt to enter)\n"
    printf "  --socket=path, -S path\t\t the socket file to use for connection\n"
    printf "  --host=host_name, -h host_name\t connect to given host\n"
    printf "  --port=port_num, -P port_num\t\t port number to use for connection\n"
    printf "  --defaults-file=file \t\t\t specify defaults file instead of guessing\n"
    printf "  --enable, -e\t\t\t\t enable TokuDB plugin and disable transparent huge pages in my.cnf\n"
    printf "  --enable-backup, -b\t\t\t enable Percona TokuBackup and add preload-hotbackup option to my.cnf\n"
    printf "\t\t\t\t\t (this option includes --enable option)\n"
    printf "  --disable, d\t\t\t\t disable TokuDB plugin and remove thp-setting=never option in my.cnf\n"
    printf "\t\t\t\t\t (this option includes --disable-backup option)\n"
    printf "  --disable-backup, r\t\t\t disable Percona TokuBackup and remove preload-hotbackup option in my.cnf\n"
    printf "  --force-envfile, f\t\t\t force usage of $SYSTEMD_ENV_FILE instead of my.cnf\n"
    printf "\t\t\t\t\t (use if autodetect doesn't work on distro with systemd and without mysqld_safe)\n"
    printf "  --force-mycnf, m\t\t\t force usage of my.cnf instead of $SYSTEMD_ENV_FILE\n"
    printf "\t\t\t\t\t (use if autodetect doesn't work where mysqld_safe is used for running server)\n"
    printf "  --help\t\t\t\t show this help\n\n"
    printf "For TokuDB requirements and manual steps for installation please visit this webpage:\n"
    printf "http://www.percona.com/doc/percona-server/5.7/tokudb/tokudb_installation.html\n\n"
    exit 0
    ;;
  esac
done

# Make sure only root can run this script
if [ $(id -u) -ne 0 -a $DOCKER = 0 ]; then
  echo "ERROR: This script must be run as root!" 1>&2
  exit 1
fi

printf "WARNING: This script is deprecated and will be removed in 8.0. You can use ps-admin script which has more functionality.\n\n"

if [ $ENABLE = 1 -a $DISABLE = 1 ]; then
  printf "ERROR: Only --enable OR --disable can be specified - not both!\n"
  exit 1
elif [ $ENABLE = 0 -a $DISABLE = 0 -a $ENABLE_TOKUBACKUP = 0 -a $DISABLE_TOKUBACKUP = 0 ]; then
  printf "ERROR: You should specify --enable,--disable,--enable-backup or --disable-backup option. Use --help for printing options.\n"
  exit 1
elif [ $ENABLE_TOKUBACKUP = 1 -a $DISABLE_TOKUBACKUP = 1 ]; then
  printf "ERROR: Only --enable-backup OR --disable-backup can be specified - not both!\n\n"
  exit 1
fi

# List plugins
LIST_ENGINE=$($MYSQL_CLIENT_BIN -e "select CONCAT(PLUGIN_NAME,'#') from information_schema.plugins where PLUGIN_NAME like 'TokuDB%';" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null)
if [ $? -ne 0 ]; then
  printf "ERROR: Failed to list mysql plugins! Please check username, password and other options for connecting to server...\n";
  exit 1
fi

# Get PID number for checking preloads
if [ $ENABLE = 1 -o $ENABLE_TOKUBACKUP = 1 ]; then
  PID_LIST=$($MYSQL_CLIENT_BIN -e "show variables like 'pid_file';" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/tmp/ps_tokudb_admin.err)
  if [ $? -ne 0 ]; then
    if [ -f /tmp/ps_tokudb_admin.err ]; then
      cat /tmp/ps_tokudb_admin.err|grep -v "Warning:"
      rm -f /tmp/ps_tokudb_admin.err
    fi
    printf "ERROR: Pid file location unknown!\n";
    exit 1
  fi
  PID_LOCATION=$(echo "${PID_LIST}"|grep pid_file|awk '{print $2}')
  if [ $? -ne 0 ] || [ "${PID_LOCATION}" == "" ]; then
    printf "ERROR: Pid file location unknown!\n";
    exit 1
  fi
  PID_NUM=$(cat ${PID_LOCATION})
fi

# Check if we're running in environment without mysqld_safe in which case we want to set
# LD_PRELOAD and THP_SETTING in /etc/sysconfig/mysql
if [ $FORCE_ENVFILE = 1 ]; then
  FULL_SYSTEMD_MODE=1
elif [ $FORCE_MYCNF = 1 ]; then
  FULL_SYSTEMD_MODE=0
else
  ps acx|grep mysqld_safe >/dev/null 2>&1
  FULL_SYSTEMD_MODE=$?
fi

# Check location for libjemalloc.so.1
if [ $FULL_SYSTEMD_MODE = 1 -a $ENABLE = 1 ]; then
  printf "Checking location of jemalloc library ...\n"
  for libjemall in "${SCRIPT_PWD}/lib/mysql" "/usr/lib64" "/usr/lib/x86_64-linux-gnu" "/usr/lib"; do
    if [ -r "$libjemall/libjemalloc.so.1" ]; then
      JEMALLOC_LOCATION="$libjemall/libjemalloc.so.1"
      break
    fi
  done
  if [ -z $JEMALLOC_LOCATION ]; then
    printf "ERROR: Cannot find libjemalloc.so.1 library. Make sure you have libjemalloc1 on debian|ubuntu or jemalloc on centos package installed.\n\n";
    exit 1
  else
    printf "INFO: Using jemalloc library from $JEMALLOC_LOCATION\n\n";
  fi
fi

# Check location for libHotBackup.so
if [ $FULL_SYSTEMD_MODE = 1 -a $ENABLE_TOKUBACKUP = 1 ]; then
  printf "Checking location of TokuBackup library ...\n"
  for libhotbackup in "${SCRIPT_PWD}/lib" "/usr/lib64" "/usr/lib/x86_64-linux-gnu" "/usr/lib" "${SCRIPT_PWD}/lib/mysql" "/usr/lib64/mysql" "/usr/lib/x86_64-linux-gnu/mysql" "/usr/lib/mysql"; do
    if [ -r "$libhotbackup/libHotBackup.so" ]; then
      HOTBACKUP_LOCATION="$libhotbackup/libHotBackup.so"
      break
    fi
  done
  if [ -z $HOTBACKUP_LOCATION ]; then
    printf "ERROR: Cannot find libHotBackup.so library. Make sure you have tokudb package installed.\n\n";
    exit 1
  else
    printf "INFO: Using TokuBackup library from $HOTBACKUP_LOCATION\n\n";
  fi
fi

# Check if server is running with jemalloc - if not warn that restart is needed (only when running with mysqld_safe)
if [ $ENABLE = 1 -a $FULL_SYSTEMD_MODE = 0 -a $DOCKER = 0 ]; then
  printf "Checking if Percona Server is running with jemalloc enabled...\n"
  grep -qc jemalloc /proc/${PID_NUM}/environ || ldd $(which mysqld) | grep -qc jemalloc
  JEMALLOC_STATUS=$?
  if [ $JEMALLOC_STATUS = 1 ]; then
    printf "ERROR: Percona Server is not running with jemalloc, please restart mysql service to enable it and then run this script...\n\n";
    exit 1
  else
    printf "INFO: Percona Server is running with jemalloc enabled.\n\n";
  fi
fi

# Check transparent huge pages status on the system
if [ $ENABLE = 1 -o $DISABLE = 1 ]; then
  printf "Checking transparent huge pages status on the system...\n"
  if [ -f /sys/kernel/mm/transparent_hugepage/enabled ]; then
    CONTENT_TRANSHP=$(</sys/kernel/mm/transparent_hugepage/enabled)
    STATUS_THP_SYSTEM=$(echo $CONTENT_TRANSHP | grep -cv '\[never\]')
  fi
  if [ $STATUS_THP_SYSTEM = 0 ]; then
    printf "INFO: Transparent huge pages are currently disabled on the system.\n\n"
  else
    printf "INFO: Transparent huge pages are enabled (should be disabled).\n\n"
  fi
fi

# Check location of my.cnf
if [ $FULL_SYSTEMD_MODE = 0  ]; then
  if [ -z $MYCNF_LOCATION ]; then
    if [ -f /etc/mysql/percona-server.conf.d/mysqld_safe.cnf -a -h /etc/mysql/my.cnf ]; then
      MYCNF_LOCATION=/etc/mysql/percona-server.conf.d/mysqld_safe.cnf
    elif [ -f /etc/percona-server.conf.d/mysqld_safe.cnf -a -h /etc/my.cnf ]; then
      MYCNF_LOCATION=/etc/percona-server.conf.d/mysqld_safe.cnf
    elif [ -f /etc/my.cnf ]; then
      MYCNF_LOCATION=/etc/my.cnf
    elif [ -f /etc/mysql/my.cnf ]; then
      MYCNF_LOCATION=/etc/mysql/my.cnf
    elif [ -f /usr/etc/my.cnf ]; then
      MYCNF_LOCATION=/usr/etc/my.cnf
    else
      if [ -d /etc/mysql ]; then
        MYCNF_LOCATION=/etc/mysql/my.cnf
      else
        MYCNF_LOCATION=/etc/my.cnf
      fi
      echo -n "" >> ${MYCNF_LOCATION}
    fi
  else
    if [ ! -f $MYCNF_LOCATION ]; then
      printf "ERROR: Specified defaults file cannot be found!\n\n"
      exit 1
    fi
  fi
fi

# Check thp-setting=never option in my.cnf or THP_SETTING variable in /etc/sysconfig/mysql
if [ $ENABLE = 1 -o $DISABLE = 1 ]; then
  if [ $FULL_SYSTEMD_MODE = 0  ]; then
    printf "Checking if thp-setting=never option is already set in config file...\n"
    STATUS_THP_MYCNF=$($MYSQL_DEFAULTS_BIN mysqld_safe $DEFAULTS_FILE_OPTION|grep -c thp-setting=never)
    if [ $STATUS_THP_MYCNF = 0 ]; then
      printf "INFO: Option thp-setting=never is not set in the config file.\n"
      printf "      (needed only if THP is not disabled permanently on the system)\n\n"
    else
      printf "INFO: Option thp-setting=never is set in the config file.\n\n"
    fi
  else
    printf "Checking if THP_SETTING variable is set to never or madvise in $SYSTEMD_ENV_FILE...\n"
    if [ -f $SYSTEMD_ENV_FILE ]; then
      STATUS_THP_MYCNF=$(cat $SYSTEMD_ENV_FILE|grep -c -e "THP_SETTING=never\|THP_SETTING=madvise")
    else
      STATUS_THP_MYCNF=0
    fi
    if [ $STATUS_THP_MYCNF = 0 ]; then
      printf "INFO: Variable THP_SETTING is not set to never or madvise in $SYSTEMD_ENV_FILE.\n\n"
    else
      printf "INFO: Variable THP_SETTING is set in $SYSTEMD_ENV_FILE.\n\n"
    fi
  fi
fi

# Check if we have variable for preloading jemalloc in /etc/sysconfig/mysql
if [ $FULL_SYSTEMD_MODE = 1 ]; then
  printf "Checking if LD_PRELOAD variable is set for libjemalloc.so.1 in $SYSTEMD_ENV_FILE...\n"
  if [ -f $SYSTEMD_ENV_FILE ]; then
    STATUS_JEMALLOC_CONFIG=$(cat $SYSTEMD_ENV_FILE|grep -c -e "LD_PRELOAD=.*libjemalloc.so.1")
  else
    STATUS_JEMALLOC_CONFIG=0
  fi
  if [ $STATUS_JEMALLOC_CONFIG = 0 ]; then
    printf "INFO: Variable LD_PRELOAD for libjemalloc.so.1 is not set in $SYSTEMD_ENV_FILE.\n\n"
  else
    printf "INFO: Variable LD_PRELOAD for libjemalloc.so.1 is set in $SYSTEMD_ENV_FILE.\n\n"
  fi
fi

# Check if we have options for preloading libHotBackup.so
if [ $ENABLE_TOKUBACKUP = 1 -o $DISABLE_TOKUBACKUP = 1 ]; then
  if [ $FULL_SYSTEMD_MODE = 0  ]; then
    printf "Checking if preload-hotbackup option is already set in config file...\n"
    STATUS_HOTBACKUP_MYCNF=$($MYSQL_DEFAULTS_BIN mysqld_safe $DEFAULTS_FILE_OPTION|grep -c preload-hotbackup)
    if [ $STATUS_HOTBACKUP_MYCNF = 0 ]; then
      printf "INFO: Option preload-hotbackup is not set in the config file.\n\n"
    else
      printf "INFO: Option preload-hotbackup is set in the config file.\n\n"
    fi
  else
    printf "Checking if LD_PRELOAD variable is set for libHotBackup.so in $SYSTEMD_ENV_FILE...\n"
    if [ -f $SYSTEMD_ENV_FILE ]; then
      STATUS_HOTBACKUP_MYCNF=$(cat $SYSTEMD_ENV_FILE|grep -c -e "LD_PRELOAD=.*libHotBackup.so")
    else
      STATUS_HOTBACKUP_MYCNF=0
    fi
    if [ $STATUS_HOTBACKUP_MYCNF = 0 ]; then
      printf "INFO: Variable LD_PRELOAD for libHotBackup.so is not set in $SYSTEMD_ENV_FILE.\n\n"
    else
      printf "INFO: Variable LD_PRELOAD for libHotBackup.so is set in $SYSTEMD_ENV_FILE.\n\n"
    fi
  fi
fi

# Check TokuDB engine plugin status
if [ $ENABLE = 1 -o $DISABLE = 1 ]; then
  printf "Checking TokuDB engine plugin status...\n"
  STATUS_PLUGIN=$(echo "$LIST_ENGINE" | grep -c "TokuDB")
  if [ $STATUS_PLUGIN = 0 ]; then
    printf "INFO: TokuDB engine plugin is not installed.\n\n"
  elif [ $STATUS_PLUGIN -gt 6 ]; then
    printf "INFO: TokuDB engine plugin is installed.\n\n"
  else
    printf "ERROR: TokuDB engine plugin is partially installed. Please cleanup manually.\n\n"
    printf "For TokuDB requirements and manual steps for installation please visit this webpage:\n"
    printf "http://www.percona.com/doc/percona-server/5.7/tokudb/tokudb_installation.html\n\n"
    exit 1
  fi
fi

# Check TokuDB backup plugin status
if [ $ENABLE_TOKUBACKUP = 1 -o $DISABLE_TOKUBACKUP = 1 ]; then
  printf "Checking TokuBackup plugin status...\n"
  STATUS_HOTBACKUP_PLUGIN=$(echo "$LIST_ENGINE" | grep -c "tokudb_backup")
  if [ $STATUS_HOTBACKUP_PLUGIN = 0 ]; then
    printf "INFO: TokuBackup plugin is not installed.\n\n"
  else
    printf "INFO: TokuBackup plugin is installed.\n\n"
  fi
fi

# Add option to preload libHotBackup.so into my.cnf or LD_PRELOAD
# for jemalloc and libHotBackup.so into /etc/sysconfig/mysql
if [ $FULL_SYSTEMD_MODE = 0  ]; then
  if [ $ENABLE_TOKUBACKUP = 1 -a $STATUS_HOTBACKUP_MYCNF = 0 ]; then
    printf "Adding preload-hotbackup option into $MYCNF_LOCATION\n"
    for MYCNF_SECTION in mysqld_safe MYSQLD_SAFE
    do
      MYSQLD_SAFE_STATUS=$(grep -c "^\[${MYCNF_SECTION}\]$" $MYCNF_LOCATION)
      if [ $MYSQLD_SAFE_STATUS != 0 ]; then
        MYSQLD_SAFE_SECTION=${MYCNF_SECTION}
        break
      fi
    done
    if [ $MYSQLD_SAFE_STATUS = 0 ]; then
      echo -e "\n[mysqld_safe]\npreload-hotbackup" >> $MYCNF_LOCATION
    else
      sed -i "/^\[${MYSQLD_SAFE_SECTION}\]$/a preload-hotbackup" $MYCNF_LOCATION
    fi
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully added preload-hotbackup option into $MYCNF_LOCATION\n";
      printf "PLEASE RESTART MYSQL SERVICE AND RUN THIS SCRIPT AGAIN TO FINISH INSTALLATION!\n\n";
      exit 0
    else
      printf "ERROR: Failed to add preload-hotbackup option into $MYCNF_LOCATION\n\n";
      exit 1
    fi
  fi
elif [ $ENABLE = 1 -a $STATUS_JEMALLOC_CONFIG = 0 ] || [ $ENABLE_TOKUBACKUP = 1 -a $STATUS_HOTBACKUP_MYCNF = 0 ]; then
  printf "Adding LD_PRELOAD variable into $SYSTEMD_ENV_FILE\n"
  NEW_LD_PRELOAD=""
  FILE_ADD=""
  if [ $ENABLE = 1 -a $STATUS_JEMALLOC_CONFIG = 0 ] && [ $ENABLE_TOKUBACKUP = 1 -a $STATUS_HOTBACKUP_MYCNF = 0 ]; then
    NEW_LD_PRELOAD="${JEMALLOC_LOCATION} ${HOTBACKUP_LOCATION}"
    FILE_ADD="libjemalloc.so.1 and libHotBackup.so"
  elif [ $ENABLE = 1 -a $STATUS_JEMALLOC_CONFIG = 0 ]; then
    NEW_LD_PRELOAD="${JEMALLOC_LOCATION}"
    FILE_ADD="libjemalloc.so.1"
  elif [ $ENABLE_TOKUBACKUP = 1 -a $STATUS_HOTBACKUP_MYCNF = 0 ]; then
    NEW_LD_PRELOAD="${HOTBACKUP_LOCATION}"
    FILE_ADD="libHotBackup.so"
  fi
  # Add desired LD_PRELOAD into config file
  if [ ! -f $SYSTEMD_ENV_FILE ]; then
    echo "LD_PRELOAD=${NEW_LD_PRELOAD}" > $SYSTEMD_ENV_FILE
  elif [ $(grep -c LD_PRELOAD $SYSTEMD_ENV_FILE) = 0 ]; then
    echo "LD_PRELOAD=${NEW_LD_PRELOAD}" >> $SYSTEMD_ENV_FILE
  else
    sed -i '/^LD_PRELOAD=/ s:$: '"${NEW_LD_PRELOAD}"':' $SYSTEMD_ENV_FILE
  fi
  # Print status
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully added LD_PRELOAD variable for $FILE_ADD into $SYSTEMD_ENV_FILE\n\n";
    printf "PLEASE RESTART MYSQL SERVICE AND RUN THIS SCRIPT AGAIN TO FINISH INSTALLATION!\n\n";
    exit 0
  else
    printf "ERROR: Failed to add LD_PRELOAD variable for $FILE_ADD into $SYSTEMD_ENV_FILE\n\n";
    exit 1
  fi
fi

# Check if server is running with libHotBackup.so preloaded - if not warn that restart is needed
if [ $ENABLE_TOKUBACKUP = 1 ]; then
  printf "Checking if Percona Server is running with libHotBackup.so preloaded...\n"
  LIBHOTBACKUP_STATUS=$(grep -c libHotBackup.so /proc/${PID_NUM}/environ)
  if [ $LIBHOTBACKUP_STATUS = 0 ]; then
    printf "ERROR: Percona Server is not running with libHotBackup.so preloaded, please restart mysql service to enable it and then run this script again...\n\n";
    exit 1
  else
    printf "INFO: Percona Server is running with libHotBackup.so preloaded.\n\n";
  fi
fi

# Disable transparent huge pages in the current session so
# that the plugin can be installed without restarting PS
if [ $ENABLE = 1 -a $STATUS_THP_SYSTEM = 1 ]; then
  printf "Disabling transparent huge pages for the current session...\n"
  if test -f /sys/kernel/mm/transparent_hugepage/defrag; then
    echo never > /sys/kernel/mm/transparent_hugepage/defrag
  fi
  if test -f /sys/kernel/mm/transparent_hugepage/enabled; then
    echo never > /sys/kernel/mm/transparent_hugepage/enabled
  fi
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully disabled transparent huge pages for this session.\n\n"
  else
    printf "ERROR: Failed to disable transparent huge pages for this session.\n\n"
    exit 1
  fi
fi

# Add option to disable transparent huge pages into my.cnf
if [ $ENABLE = 1 -a $STATUS_THP_MYCNF = 0 ]; then
  if [ $FULL_SYSTEMD_MODE = 0 ]; then
    printf "Adding thp-setting=never option into $MYCNF_LOCATION\n"
    for MYCNF_SECTION in mysqld_safe MYSQLD_SAFE
    do
      MYSQLD_SAFE_STATUS=$(grep -c "^\[${MYCNF_SECTION}\]$" $MYCNF_LOCATION)
      if [ $MYSQLD_SAFE_STATUS != 0 ]; then
        MYSQLD_SAFE_SECTION=${MYCNF_SECTION}
        break
      fi
    done
    if [ $MYSQLD_SAFE_STATUS = 0 ]; then
      echo -e "\n[mysqld_safe]\nthp-setting=never" >> $MYCNF_LOCATION
    else
      sed -i "/^\[${MYSQLD_SAFE_SECTION}\]$/a thp-setting=never" $MYCNF_LOCATION
    fi
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully added thp-setting=never option into $MYCNF_LOCATION\n\n";
    else
      printf "ERROR: Failed to add thp-setting=never option into $MYCNF_LOCATION\n\n";
      exit 1
    fi
  else
    printf "Adding THP_SETTING=never variable into $SYSTEMD_ENV_FILE\n"
    echo -e "THP_SETTING=never" >> $SYSTEMD_ENV_FILE
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully added THP_SETTING=never option into $SYSTEMD_ENV_FILE\n\n";
    else
      printf "ERROR: Failed to add THP_SETTING=never option into $SYSTEMD_ENV_FILE\n\n";
      exit 1
    fi
  fi
fi

# Remove option for disabling transparent huge pages from config files
if [ $DISABLE = 1 -a $STATUS_THP_MYCNF = 1 ]; then
  if [ $FULL_SYSTEMD_MODE = 0  ]; then
    printf "Removing thp-setting=never option from $MYCNF_LOCATION\n"
    sed -i '/^thp-setting=never$/d' $MYCNF_LOCATION
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully removed thp-setting=never option from $MYCNF_LOCATION\n\n";
    else
      printf "ERROR: Failed to remove thp-setting=never option from $MYCNF_LOCATION\n\n";
      exit 1
    fi
  else
    printf "Removing THP_SETTING variable from $SYSTEMD_ENV_FILE\n"
    sed -i '/^THP_SETTING=/d' $SYSTEMD_ENV_FILE
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully removed THP_SETTING variable from $SYSTEMD_ENV_FILE\n\n";
    else
      printf "ERROR: Failed to remove THP_SETTING variable from $SYSTEMD_ENV_FILE\n\n";
      exit 1
    fi
  fi
fi

# Remove option for preloading libHotBackup.so and jemalloc from config files
if [ $FULL_SYSTEMD_MODE = 0  ]; then
  if [ $DISABLE_TOKUBACKUP = 1 -a $STATUS_HOTBACKUP_MYCNF = 1 ]; then
    printf "Removing preload-hotbackup option from $MYCNF_LOCATION\n"
    sed -i '/^preload-hotbackup$/d' $MYCNF_LOCATION
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully removed preload-hotbackup option from $MYCNF_LOCATION\n\n";
    else
      printf "ERROR: Failed to remove preload-hotbackup option from $MYCNF_LOCATION\n\n";
      exit 1
    fi
  fi
elif [ $DISABLE = 1 -o $DISABLE_TOKUBACKUP = 1 ]; then
  printf "Removing LD_PRELOAD option from $SYSTEMD_ENV_FILE\n"
  if [ $DISABLE = 1 -a $DISABLE_TOKUBACKUP = 1 ]; then
    sed -i '/^LD_PRELOAD/d' $SYSTEMD_ENV_FILE
    FILE_REMOVE="libjemalloc.so.1 and libHotBackup.so"
  else
    NEW_LD_PRELOAD=""
    if [ $DISABLE = 1 ]; then
      FILE_REMOVE="libjemalloc.so.1"
    else
      FILE_REMOVE="libHotBackup.so"
    fi
    for file in $(cat $SYSTEMD_ENV_FILE|awk -v var="$FILE_REMOVE" -F ' ' '/LD_PRELOAD/{for(i=1;i<=NF;++i)if($i !~ var)print $i}'|sed 's/LD_PRELOAD=//')
    do
      NEW_LD_PRELOAD="${file} "
    done
    NEW_LD_PRELOAD=$(echo "LD_PRELOAD=${NEW_LD_PRELOAD}"|sed 's/[ ]*$//')
    sed -i 's:^LD_PRELOAD=.*:'"${NEW_LD_PRELOAD}"':' $SYSTEMD_ENV_FILE
  fi
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully removed LD_PRELOAD option for $FILE_REMOVE from $SYSTEMD_ENV_FILE\n\n";
    else
      printf "ERROR: Failed to remove LD_PRELOAD option for $FILE_REMOVE from $SYSTEMD_ENV_FILE\n\n";
      exit 1
    fi
fi

# Installing TokuDB engine plugin
if [ $ENABLE = 1 -a $STATUS_PLUGIN = 0 ]; then
  printf "Installing TokuDB engine...\n"
$MYSQL_CLIENT_BIN -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null<<EOFTOKUDBENABLE
INSTALL PLUGIN TokuDB SONAME 'ha_tokudb.so';
INSTALL PLUGIN TokuDB_file_map SONAME 'ha_tokudb.so';
INSTALL PLUGIN TokuDB_fractal_tree_info SONAME 'ha_tokudb.so';
INSTALL PLUGIN TokuDB_fractal_tree_block_map SONAME 'ha_tokudb.so';
INSTALL PLUGIN TokuDB_trx SONAME 'ha_tokudb.so';
INSTALL PLUGIN TokuDB_locks SONAME 'ha_tokudb.so';
INSTALL PLUGIN TokuDB_lock_waits SONAME 'ha_tokudb.so';
INSTALL PLUGIN TokuDB_background_job_status SONAME 'ha_tokudb.so';
EOFTOKUDBENABLE
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully installed TokuDB engine plugin.\n\n"
  else
    printf "ERROR: Failed to install TokuDB engine plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Installing TokuDB backup plugin
if [ $ENABLE_TOKUBACKUP = 1 -a $STATUS_HOTBACKUP_PLUGIN = 0 ]; then
  printf "Installing TokuBackup plugin...\n"
$MYSQL_CLIENT_BIN -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null<<EOFTOKUBACKUPENABLE
INSTALL PLUGIN tokudb_backup SONAME 'tokudb_backup.so';
EOFTOKUBACKUPENABLE
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully installed TokuBackup plugin.\n\n"
  else
    printf "ERROR: Failed to install TokuBackup plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Uninstalling TokuDB backup plugin
if [ $DISABLE_TOKUBACKUP = 1 -a $STATUS_HOTBACKUP_PLUGIN = 1 ]; then
  printf "Uninstalling TokuBackup plugin...\n"
$MYSQL_CLIENT_BIN -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null<<EOFTOKUBACKUPDISABLE
UNINSTALL PLUGIN tokudb_backup;
EOFTOKUBACKUPDISABLE
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully uninstalled TokuBackup plugin.\n\n"
  else
    printf "ERROR: Failed to uninstall TokuBackup plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Uninstalling TokuDB engine plugin
if [ $DISABLE = 1 -a $STATUS_PLUGIN -gt 0 ]; then
  printf "Uninstalling TokuDB engine plugin...\n"
  for plugin in TokuDB TokuDB_file_map TokuDB_fractal_tree_info TokuDB_fractal_tree_block_map TokuDB_trx TokuDB_locks TokuDB_lock_waits TokuDB_background_job_status; do
    SPECIFIC_PLUGIN_STATUS=$(echo "$LIST_ENGINE" | grep -c "$plugin#")
    if [ $SPECIFIC_PLUGIN_STATUS -gt 0 ]; then
      $MYSQL_CLIENT_BIN -u $USER $PASSWORD $SOCKET $HOST $PORT -e "UNINSTALL PLUGIN $plugin" 2>/dev/null
      if [ $? -ne 0 ]; then
        printf "ERROR: Failed to uninstall TokuDB engine plugin. Please check error log.\n\n"
        exit 1
      fi
    fi
  done
  printf "INFO: Successfully uninstalled TokuDB engine plugin.\n\n"
fi
