#!/bin/bash
#
# Script for doing various administrative tasks in Percona Server
# like installing/uninstalling TokuDB/RocksDB storage engines
# and plugins like Query Response Time, Audit Log, PAM and MySQLX
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
ENABLE_TOKUDB=0
DISABLE_TOKUDB=0
ENABLE_TOKUBACKUP=0
DISABLE_TOKUBACKUP=0
ENABLE_ROCKSDB=0
DISABLE_ROCKSDB=0
ENABLE_QRT=0
DISABLE_QRT=0
ENABLE_AUDIT=0
DISABLE_AUDIT=0
ENABLE_PAM=0
DISABLE_PAM=0
ENABLE_PAM_COMPAT=0
DISABLE_PAM_COMPAT=0
ENABLE_MYSQLX=0
DISABLE_MYSQLX=0
FORCE_MYCNF=0
FORCE_ENVFILE=0
DEFAULTS_FILE=""
DEFAULTS_FILE_OPTION=""
STATUS_THP_SYSTEM=0
STATUS_THP_MYCNF=0
STATUS_TOKUDB_PLUGIN=0
STATUS_ROCKSDB_PLUGIN=0
STATUS_QRT_PLUGIN=0
STATUS_AUDIT_PLUGIN=0
STATUS_PAM_PLUGIN=0
STATUS_PAM_COMPAT_PLUGIN=0
STATUS_MYSQLX_PLUGIN=0
STATUS_HOTBACKUP_MYCNF=0
STATUS_HOTBACKUP_PLUGIN=0
STATUS_JEMALLOC_CONFIG=0
STATUS_MYSQLD_SAFE=0
STATUS_LIBHOTBACKUP=0
FULL_SYSTEMD_MODE=0
JEMALLOC_LOCATION=""
HOTBACKUP_LOCATION=""
HAROCKSDB_LOCATION=""
HATOKUDB_LOCATION=""
DOCKER=0

SCRIPT_PWD=$(cd `dirname $0` && pwd)
MYSQL_CLIENT_BIN="${SCRIPT_PWD}/mysql"
MYSQL_DEFAULTS_BIN="${SCRIPT_PWD}/my_print_defaults"
if [ -f /etc/redhat-release -o -f /etc/system-release ]; then
  SYSTEMD_ENV_FILE="/etc/sysconfig/mysql"
else
  SYSTEMD_ENV_FILE="/etc/default/mysql"
fi

# Check if we have a functional getopt(1)
if ! getopt --test
  then
  go_out="$(getopt --options=c:u:p::S:h:P:edbrfmkotzawinjKxgD \
  --longoptions=config-file:,user:,password::,socket:,host:,port:,enable-tokudb,disable-tokudb,enable-tokubackup,disable-tokubackup,help,defaults-file:,force-envfile,force-mycnf,enable-rocksdb,disable-rocksdb,enable-qrt,disable-qrt,enable-audit,disable-audit,enable-pam,disable-pam,enable-pam-compat,disable-pam-compat,enable-mysqlx,disable-mysqlx,docker \
  --name="$(basename "$0")" -- "$@")"
  test $? -eq 0 || exit 1
  eval set -- $go_out
fi

for arg
do
  case "$arg" in
    -- ) shift; break;;
    -c | --config-file )
    CONFIG_FILE="$2"
    shift 2
    if [ -z "${CONFIG_FILE}" ]; then
      echo "ERROR: The configuration file location (--config-file) was not provided. Terminating."
      exit 1
    fi
    if [ -e "${CONFIG_FILE}" ]; then
      source "${CONFIG_FILE}"
    else
      echo "ERROR: The configuration file ${CONFIG_FILE} specified by --config-file does not exist. Terminating."
      exit 1
    fi
    ;;
    -u | --user )
    USER="$2"
    shift 2
    ;;
    -p | --password )
    case "$2" in
      "")
      read -s -p "Enter password:" PASSWORD
      if [ -z "${PASSWORD}" ]; then
	printf "\nContinuing without password...\n";
      fi
      printf "\n\n"
      ;;
      *)
      PASSWORD="$2"
      ;;
    esac
    shift 2
    ;;
    -S | --socket )
    SOCKET="$2"
    shift 2
    ;;
    -h | --host )
    HOST="$2"
    shift 2
    ;;
    -P | --port )
    PORT="$2"
    shift 2
    ;;
    --defaults-file )
    DEFAULTS_FILE="$2"
    DEFAULTS_FILE_OPTION="--defaults-file=${DEFAULTS_FILE}"
    shift 2
    ;;
    -e | --enable-tokudb )
    shift
    ENABLE_TOKUDB=1
    ;;
    -d | --disable-tokudb )
    shift
    DISABLE_TOKUDB=1
    DISABLE_TOKUBACKUP=1
    ;;
    -b | --enable-tokubackup )
    shift
    ENABLE_TOKUBACKUP=1
    ENABLE_TOKUDB=1
    ;;
    -r | --disable-tokubackup )
    shift
    DISABLE_TOKUBACKUP=1
    ;;
    -k | --enable-rocksdb )
    shift
    ENABLE_ROCKSDB=1
    ;;
    -o | --disable-rocksdb )
    shift
    DISABLE_ROCKSDB=1
    ;;
    -t | --enable-qrt )
    shift
    ENABLE_QRT=1
    ;;
    -z | --disable-qrt )
    shift
    DISABLE_QRT=1
    ;;
    -a | --enable-audit )
    shift
    ENABLE_AUDIT=1
    ;;
    -w | --disable-audit )
    shift
    DISABLE_AUDIT=1
    ;;
    -i | --enable-pam )
    shift
    ENABLE_PAM=1
    ;;
    -n | --disable-pam )
    shift
    DISABLE_PAM=1
    ;;
    -j | --enable-pam-compat )
    shift
    ENABLE_PAM_COMPAT=1
    ;;
    -K | --disable-pam-compat )
    shift
    DISABLE_PAM_COMPAT=1
    ;;
    -x | --enable-mysqlx )
    shift
    ENABLE_MYSQLX=1
    ;;
    -g | --disable-mysqlx )
    shift
    DISABLE_MYSQLX=1
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
    printf "This script can be used to setup plugins, TokuDB and RocksDB storage engines for Percona Server 5.7.\n"
    printf "For TokuDB if transparent huge pages are enabled on the system it adds thp-setting=never option to my.cnf\n"
    printf "to disable it on runtime.\n"
    printf "Valid options are:\n"
    printf "  --config-file=file, -c file\t\t read credentials and options from config file\n"
    printf "  --user=user_name, -u user_name\t mysql admin username\n"
    printf "  --password[=password], -p[password]\t mysql admin password (on empty will prompt to enter)\n"
    printf "  --socket=path, -S path\t\t the socket file to use for connection\n"
    printf "  --host=host_name, -h host_name\t connect to given host\n"
    printf "  --port=port_num, -P port_num\t\t port number to use for connection\n"
    printf "  --defaults-file=file \t\t\t specify defaults file (my.cnf) instead of guessing\n"
    printf "  --enable-tokudb, -e\t\t\t enable TokuDB plugin and disable transparent huge pages in my.cnf\n"
    printf "  --enable-tokubackup, -b\t\t enable Percona TokuBackup and add preload-hotbackup option to my.cnf\n"
    printf "\t\t\t\t\t (this option includes --enable-tokudb option)\n"
    printf "  --disable-tokudb, -d\t\t\t disable TokuDB plugin and remove thp-setting=never option in my.cnf\n"
    printf "\t\t\t\t\t (this option includes --disable-tokubackup option)\n"
    printf "  --disable-tokubackup, -r\t\t disable Percona TokuBackup and remove preload-hotbackup option in my.cnf\n"
    printf "  --enable-rocksdb, -k\t\t\t enable RocksDB storage engine plugin\n"
    printf "  --disable-rocksdb, -o\t\t\t disable RocksDB storage engine plugin\n"
    printf "  --enable-qrt, -t\t\t\t enable Query Response Time plugin\n"
    printf "  --disable-qrt, -z\t\t\t disable Query Response Time plugin\n"
    printf "  --enable-audit, -a\t\t\t enable Audit Log plugin\n"
    printf "  --disable-audit, -w\t\t\t disable Audit Log plugin\n"
    printf "  --enable-pam, -i\t\t\t enable PAM Authentication plugin\n"
    printf "  --disable-pam, -n\t\t\t disable PAM Authentication plugin\n"
    printf "  --enable-pam-compat, -j\t\t enable PAM Compat Authentication plugin\n"
    printf "  --disable-pam-compat, -K\t\t disable PAM Compat Authentication plugin\n"
    printf "  --enable-mysqlx, -x\t\t\t enable MySQL X plugin\n"
    printf "  --disable-mysqlx, -g\t\t\t disable MySQL X plugin\n"
    printf "  --force-envfile, -f\t\t\t force usage of ${SYSTEMD_ENV_FILE} instead of my.cnf (relevant only for TokuDB)\n"
    printf "\t\t\t\t\t (use if autodetect doesn't work on distro with systemd and without mysqld_safe)\n"
    printf "  --force-mycnf, -m\t\t\t force usage of my.cnf instead of ${SYSTEMD_ENV_FILE} (relevant only for TokuDB)\n"
    printf "\t\t\t\t\t (use if autodetect doesn't work where mysqld_safe is used for running server)\n"
    printf "  --help\t\t\t\t show this help\n\n"
    printf "For TokuDB requirements and manual steps for installation please visit this webpage:\n"
    printf "http://www.percona.com/doc/percona-server/5.7/tokudb/tokudb_installation.html\n\n"
    exit 0
    ;;
  esac
done

# Make sure only root can run this script
if [ ${ENABLE_TOKUDB} = 1 -o ${DISABLE_TOKUDB} = 1 -o ${ENABLE_TOKUBACKUP} = 1 -o ${DISABLE_TOKUBACKUP} = 1 ]; then
  if [ $(id -u) -ne 0 -a $DOCKER = 0 ]; then
    echo "ERROR: For TokuDB install/uninstall this script must be run as root!" 1>&2
    exit 1
  fi
fi

# Assign options for mysql client
PASSWORD=${PASSWORD:+"-p${PASSWORD}"}
SOCKET=${SOCKET:+"-S ${SOCKET}"}
HOST=${HOST:+"-h ${HOST}"}
PORT=${PORT:+"-P ${PORT}"}

if [ ${ENABLE_TOKUDB} = 1 -a ${DISABLE_TOKUDB} = 1 ]; then
  printf "ERROR: Only --enable-tokudb OR --disable-tokudb can be specified - not both!\n"
  exit 1
elif [ ${ENABLE_TOKUDB} = 0 -a ${DISABLE_TOKUDB} = 0 -a ${ENABLE_TOKUBACKUP} = 0 -a ${DISABLE_TOKUBACKUP} = 0 -a ${ENABLE_ROCKSDB} = 0 -a ${DISABLE_ROCKSDB} = 0 -a ${ENABLE_QRT} = 0 -a ${DISABLE_QRT} = 0 -a ${ENABLE_AUDIT} = 0 -a ${DISABLE_AUDIT} = 0 -a ${ENABLE_PAM} = 0 -a ${DISABLE_PAM} = 0 -a ${ENABLE_PAM_COMPAT} = 0 -a ${DISABLE_PAM_COMPAT} = 0 -a ${ENABLE_MYSQLX} = 0 -a ${DISABLE_MYSQLX} = 0 ]; then
  printf "ERROR: You should specify one of the --enable or --disable options.\n"
  printf "Use --help for printing options.\n"
  exit 1
elif [ ${ENABLE_TOKUBACKUP} = 1 -a ${DISABLE_TOKUBACKUP} = 1 ]; then
  printf "ERROR: Only --enable-tokubackup OR --disable-tokubackup can be specified - not both!\n\n"
  exit 1
elif [ ${ENABLE_ROCKSDB} = 1 -a ${DISABLE_ROCKSDB} = 1 ]; then
  printf "ERROR: Only --enable-rocksdb OR --disable-rocksdb can be specified - not both!\n\n"
  exit 1
elif [ ${ENABLE_QRT} = 1 -a ${DISABLE_QRT} = 1 ]; then
  printf "ERROR: Only --enable-qrt OR --disable-qrt can be specified - not both!\n\n"
  exit 1
elif [ ${ENABLE_AUDIT} = 1 -a ${DISABLE_AUDIT} = 1 ]; then
  printf "ERROR: Only --enable-audit OR --disable-audit can be specified - not both!\n\n"
  exit 1
elif [ ${ENABLE_PAM} = 1 -a ${DISABLE_PAM} = 1 ]; then
  printf "ERROR: Only --enable-pam OR --disable-pam can be specified - not both!\n\n"
  exit 1
elif [ ${ENABLE_PAM_COMPAT} = 1 -a ${DISABLE_PAM_COMPAT} = 1 ]; then
  printf "ERROR: Only --enable-pam-compat OR --disable-pam-compat can be specified - not both!\n\n"
  exit 1
elif [ ${ENABLE_MYSQLX} = 1 -a ${DISABLE_MYSQLX} = 1 ]; then
  printf "ERROR: Only --enable-mysqlx OR --disable-mysqlx can be specified - not both!\n\n"
  exit 1
fi

# List plugins
LIST_PLUGINS=$(${MYSQL_CLIENT_BIN} -e "select CONCAT(PLUGIN_NAME,'#') from information_schema.plugins where plugin_status = 'ACTIVE';" -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} 2>/tmp/ps-admin.err)
if [ $? -ne 0 ]; then
  if [ -f /tmp/ps-admin.err ]; then
    grep -v "Warning:" /tmp/ps-admin.err
    rm -f /tmp/ps-admin.err
  fi
  printf "ERROR: Failed to list mysql plugins! Please check username, password and other options for connecting to server...\n";
  exit 1
fi

# Get PID number for checking preloads
if [ ${ENABLE_TOKUDB} = 1 -o ${ENABLE_TOKUBACKUP} = 1 ]; then
  PID_LIST=$(${MYSQL_CLIENT_BIN} -e "show variables like 'pid_file';" -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} 2>/tmp/ps-admin.err)
  if [ $? -ne 0 ]; then
    if [ -f /tmp/ps-admin.err ]; then
      grep -v "Warning:" /tmp/ps-admin.err
      rm -f /tmp/ps-admin.err
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
if [ ${FORCE_ENVFILE} = 1 ]; then
  FULL_SYSTEMD_MODE=1
elif [ ${FORCE_MYCNF} = 1 ]; then
  FULL_SYSTEMD_MODE=0
else
  ps acx|grep mysqld_safe >/dev/null 2>&1
  FULL_SYSTEMD_MODE=$?
fi

# Check if TokuDB plugin available on the system
if [ ${ENABLE_TOKUDB} = 1 ]; then
  printf "Checking if TokuDB plugin is available for installation ...\n"
  for ha_tokudb_loc in "${SCRIPT_PWD%/*}/lib/mysql/plugin" "/usr/lib64/mysql/plugin" "/usr/lib/mysql/plugin"; do
    if [ -r "${ha_tokudb_loc}/ha_tokudb.so" ]; then
      HATOKUDB_LOCATION="${ha_tokudb_loc}/ha_tokudb.so"
      break
    fi
  done
  if [ -z ${HATOKUDB_LOCATION} ]; then
    printf "ERROR: Cannot find ha_tokudb.so library for TokuDB installation.\n";
    printf "Make sure you have TokuDB package installed or if running from binary tarball that this library is available.\n\n";
    exit 1
  else
    printf "INFO: ha_tokudb.so library for TokuDB found at ${HATOKUDB_LOCATION}.\n\n";
  fi
fi

# Check if RocksDB plugin available on the system
if [ ${ENABLE_ROCKSDB} = 1 ]; then
  printf "Checking if RocksDB plugin is available for installation ...\n"
  for ha_rocksdb_loc in "${SCRIPT_PWD%/*}/lib/mysql/plugin" "/usr/lib64/mysql/plugin" "/usr/lib/mysql/plugin"; do
    if [ -r "${ha_rocksdb_loc}/ha_rocksdb.so" ]; then
      HAROCKSDB_LOCATION="${ha_rocksdb_loc}/ha_rocksdb.so"
      break
    fi
  done
  if [ -z ${HAROCKSDB_LOCATION} ]; then
    printf "ERROR: Cannot find ha_rocksdb.so library for RocksDB installation.\n";
    printf "Make sure you have RocksDB package installed or if running from binary tarball that this library is available.\n\n";
    exit 1
  else
    printf "INFO: ha_rocksdb.so library for RocksDB found at ${HAROCKSDB_LOCATION}.\n\n";
  fi
fi

# Check location for libjemalloc.so.1
if [ ${ENABLE_TOKUDB} = 1 ]; then
  printf "Checking location of jemalloc library ...\n"
  for libjemall in "${SCRIPT_PWD%/*}/lib/mysql" "/usr/lib64" "/usr/lib/x86_64-linux-gnu" "/usr/lib"; do
    if [ -r "${libjemall}/libjemalloc.so.1" ]; then
      JEMALLOC_LOCATION="${libjemall}/libjemalloc.so.1"
      break
    fi
  done
  if [ -z ${JEMALLOC_LOCATION} ]; then
    printf "ERROR: Cannot find libjemalloc.so.1 library. Make sure you have libjemalloc1 on debian|ubuntu or jemalloc on centos package installed.\n\n";
    exit 1
  else
    printf "INFO: jemalloc library needed for TokuDB found at ${JEMALLOC_LOCATION}\n\n";
  fi
fi

# Check location for libHotBackup.so
if [ ${ENABLE_TOKUBACKUP} = 1 ]; then
  printf "Checking location of TokuBackup library ...\n"
  for libhotbackup in "${SCRIPT_PWD%/*}/lib" "/usr/lib64" "/usr/lib/x86_64-linux-gnu" "/usr/lib" "${SCRIPT_PWD%/*}/lib/mysql" "/usr/lib64/mysql" "/usr/lib/x86_64-linux-gnu/mysql" "/usr/lib/mysql"; do
    if [ -r "${libhotbackup}/libHotBackup.so" ]; then
      HOTBACKUP_LOCATION="${libhotbackup}/libHotBackup.so"
      break
    fi
  done
  if [ -z ${HOTBACKUP_LOCATION} ]; then
    printf "ERROR: Cannot find libHotBackup.so library. Make sure you have TokuDB package installed.\n\n";
    exit 1
  else
    printf "INFO: TokuBackup library found at ${HOTBACKUP_LOCATION}\n\n";
  fi
fi

# Check if server is running with jemalloc - if not warn that restart is needed (only when running with mysqld_safe)
if [ ${ENABLE_TOKUDB} = 1 -a ${FULL_SYSTEMD_MODE} = 0 -a ${DOCKER} = 0 ]; then
  printf "Checking if Percona Server is running with jemalloc enabled...\n"
  grep -qc jemalloc /proc/${PID_NUM}/environ || ldd $(which mysqld) | grep -qc jemalloc
  JEMALLOC_STATUS=$?
  if [ ${JEMALLOC_STATUS} = 1 ]; then
    printf "ERROR: Percona Server is not running with jemalloc, please restart mysql service to enable it and then run this script...\n\n";
    exit 1
  else
    printf "INFO: Percona Server is running with jemalloc enabled.\n\n";
  fi
fi

# Check transparent huge pages status on the system
if [ ${ENABLE_TOKUDB} = 1 -o ${DISABLE_TOKUDB} = 1 ]; then
  printf "Checking transparent huge pages status on the system...\n"
  if [ -f /sys/kernel/mm/transparent_hugepage/enabled ]; then
    CONTENT_TRANSHP=$(</sys/kernel/mm/transparent_hugepage/enabled)
    STATUS_THP_SYSTEM=$(echo ${CONTENT_TRANSHP} | grep -cv '\[never\]')
  fi
  if [ ${STATUS_THP_SYSTEM} = 0 ]; then
    printf "INFO: Transparent huge pages are currently disabled on the system.\n\n"
  else
    printf "INFO: Transparent huge pages are enabled (should be disabled).\n\n"
  fi
fi

# Check location of my.cnf
if [ ${FULL_SYSTEMD_MODE} = 0  ]; then
  if [ -z ${DEFAULTS_FILE} ]; then
    if [ -f /etc/mysql/percona-server.conf.d/mysqld_safe.cnf -a -h /etc/mysql/my.cnf ]; then
      DEFAULTS_FILE=/etc/mysql/percona-server.conf.d/mysqld_safe.cnf
    elif [ -f /etc/percona-server.conf.d/mysqld_safe.cnf -a -h /etc/my.cnf ]; then
      DEFAULTS_FILE=/etc/percona-server.conf.d/mysqld_safe.cnf
    elif [ -f /etc/my.cnf ]; then
      DEFAULTS_FILE=/etc/my.cnf
    elif [ -f /etc/mysql/my.cnf ]; then
      DEFAULTS_FILE=/etc/mysql/my.cnf
    elif [ -f /usr/etc/my.cnf ]; then
      DEFAULTS_FILE=/usr/etc/my.cnf
    else
      if [ -d /etc/mysql ]; then
        DEFAULTS_FILE=/etc/mysql/my.cnf
      else
        DEFAULTS_FILE=/etc/my.cnf
      fi
      echo -n "" >> ${DEFAULTS_FILE}
    fi
  else
    if [ ! -f ${DEFAULTS_FILE} ]; then
      printf "ERROR: Specified defaults file cannot be found!\n\n"
      exit 1
    fi
  fi
fi

# Check thp-setting=never option in my.cnf or THP_SETTING variable in /etc/sysconfig/mysql
if [ ${ENABLE_TOKUDB} = 1 -o ${DISABLE_TOKUDB} = 1 ]; then
  if [ ${FULL_SYSTEMD_MODE} = 0  ]; then
    printf "Checking if thp-setting=never option is already set in config file...\n"
    STATUS_THP_MYCNF=$(${MYSQL_DEFAULTS_BIN} mysqld_safe ${DEFAULTS_FILE_OPTION}|grep -c thp-setting=never)
    if [ ${STATUS_THP_MYCNF} = 0 ]; then
      printf "INFO: Option thp-setting=never is not set in the config file.\n"
      printf "      (needed only if THP is not disabled permanently on the system)\n\n"
    else
      printf "INFO: Option thp-setting=never is set in the config file.\n\n"
    fi
  else
    printf "Checking if THP_SETTING variable is set to never or madvise in ${SYSTEMD_ENV_FILE}...\n"
    if [ -f ${SYSTEMD_ENV_FILE} ]; then
      STATUS_THP_MYCNF=$(grep -c -e "THP_SETTING=never\|THP_SETTING=madvise" ${SYSTEMD_ENV_FILE})
    else
      STATUS_THP_MYCNF=0
    fi
    if [ ${STATUS_THP_MYCNF} = 0 ]; then
      printf "INFO: Variable THP_SETTING is not set to never or madvise in ${SYSTEMD_ENV_FILE}.\n\n"
    else
      printf "INFO: Variable THP_SETTING is set in ${SYSTEMD_ENV_FILE}.\n\n"
    fi
  fi
fi

# Check if we have variable for preloading jemalloc in /etc/sysconfig/mysql
if [ ${ENABLE_TOKUDB} = 1 -o ${DISABLE_TOKUDB} = 1 ]; then
  if [ ${FULL_SYSTEMD_MODE} = 1 ]; then
    printf "Checking if LD_PRELOAD variable is set for libjemalloc.so.1 in ${SYSTEMD_ENV_FILE}...\n"
    if [ -f ${SYSTEMD_ENV_FILE} ]; then
      STATUS_JEMALLOC_CONFIG=$(grep -c -e "LD_PRELOAD=.*libjemalloc.so.1" ${SYSTEMD_ENV_FILE})
    else
      STATUS_JEMALLOC_CONFIG=0
    fi
    if [ ${STATUS_JEMALLOC_CONFIG} = 0 ]; then
      printf "INFO: Variable LD_PRELOAD for libjemalloc.so.1 is not set in ${SYSTEMD_ENV_FILE}.\n\n"
    else
      printf "INFO: Variable LD_PRELOAD for libjemalloc.so.1 is set in ${SYSTEMD_ENV_FILE}.\n\n"
    fi
  fi
fi

# Check if we have options for preloading libHotBackup.so
if [ ${ENABLE_TOKUBACKUP} = 1 -o ${DISABLE_TOKUBACKUP} = 1 ]; then
  if [ ${FULL_SYSTEMD_MODE} = 0  ]; then
    printf "Checking if preload-hotbackup option is already set in config file...\n"
    STATUS_HOTBACKUP_MYCNF=$(${MYSQL_DEFAULTS_BIN} mysqld_safe ${DEFAULTS_FILE_OPTION}|grep -c preload-hotbackup)
    if [ ${STATUS_HOTBACKUP_MYCNF} = 0 ]; then
      printf "INFO: Option preload-hotbackup is not set in the config file.\n\n"
    else
      printf "INFO: Option preload-hotbackup is set in the config file.\n\n"
    fi
  else
    printf "Checking if LD_PRELOAD variable is set for libHotBackup.so in ${SYSTEMD_ENV_FILE}...\n"
    if [ -f ${SYSTEMD_ENV_FILE} ]; then
      STATUS_HOTBACKUP_MYCNF=$(grep -c -e "LD_PRELOAD=.*libHotBackup.so" ${SYSTEMD_ENV_FILE})
    else
      STATUS_HOTBACKUP_MYCNF=0
    fi
    if [ ${STATUS_HOTBACKUP_MYCNF} = 0 ]; then
      printf "INFO: Variable LD_PRELOAD for libHotBackup.so is not set in ${SYSTEMD_ENV_FILE}.\n\n"
    else
      printf "INFO: Variable LD_PRELOAD for libHotBackup.so is set in ${SYSTEMD_ENV_FILE}.\n\n"
    fi
  fi
fi

# Check TokuDB engine plugin status
if [ ${ENABLE_TOKUDB} = 1 -o ${DISABLE_TOKUDB} = 1 ]; then
  printf "Checking TokuDB engine plugin status...\n"
  STATUS_TOKUDB_PLUGIN=$(echo "${LIST_PLUGINS}" | grep -c "TokuDB")
  if [ ${STATUS_TOKUDB_PLUGIN} = 0 ]; then
    printf "INFO: TokuDB engine plugin is not installed.\n\n"
  elif [ ${STATUS_TOKUDB_PLUGIN} -gt 6 ]; then
    printf "INFO: TokuDB engine plugin is installed.\n\n"
  else
    printf "ERROR: TokuDB engine plugin is partially installed. Please cleanup manually.\n\n"
    printf "For TokuDB requirements and manual steps for installation please visit this webpage:\n"
    printf "http://www.percona.com/doc/percona-server/5.7/tokudb/tokudb_installation.html\n\n"
    exit 1
  fi
fi

# Check RocksDB engine plugin status
if [ ${ENABLE_ROCKSDB} = 1 -o ${DISABLE_ROCKSDB} = 1 ]; then
  printf "Checking RocksDB engine plugin status...\n"
  STATUS_ROCKSDB_PLUGIN=$(echo "${LIST_PLUGINS}" | grep -c "ROCKSDB")
  if [ ${STATUS_ROCKSDB_PLUGIN} = 0 ]; then
    printf "INFO: RocksDB engine plugin is not installed.\n\n"
  elif [ ${STATUS_ROCKSDB_PLUGIN} -gt 11 ]; then
    printf "INFO: RocksDB engine plugin is installed.\n\n"
  else
    printf "ERROR: RocksDB engine plugin is partially installed. Please cleanup manually.\n\n"
    exit 1
  fi
fi

# Check TokuDB backup plugin status
if [ ${ENABLE_TOKUBACKUP} = 1 -o ${DISABLE_TOKUBACKUP} = 1 ]; then
  printf "Checking TokuBackup plugin status...\n"
  STATUS_HOTBACKUP_PLUGIN=$(echo "${LIST_PLUGINS}" | grep -c "tokudb_backup")
  if [ ${STATUS_HOTBACKUP_PLUGIN} = 0 ]; then
    printf "INFO: TokuBackup plugin is not installed.\n\n"
  else
    printf "INFO: TokuBackup plugin is installed.\n\n"
  fi
fi

# Check Query Response Time plugin status
if [ ${ENABLE_QRT} = 1 -o ${DISABLE_QRT} = 1 ]; then
  printf "Checking Query Response Time plugin status...\n"
  STATUS_QRT_PLUGIN=$(echo "${LIST_PLUGINS}" | grep -c "QUERY_RESPONSE_TIME")
  if [ ${STATUS_QRT_PLUGIN} = 0 ]; then
    printf "INFO: Query Response Time plugin is not installed.\n\n"
  elif [ ${STATUS_QRT_PLUGIN} -gt 3 ]; then
    printf "INFO: Query Response Time plugin is installed.\n\n"
  else
    printf "ERROR: Query Response Time plugin is partially installed.\n"
    printf "Check this page for manual install/uninstall steps:\n"
    printf "https://www.percona.com/doc/percona-server/5.7/diagnostics/response_time_distribution.html\n\n"
    exit 1
  fi
fi

# Check Audit Log plugin status
if [ ${ENABLE_AUDIT} = 1 -o ${DISABLE_AUDIT} = 1 ]; then
  printf "Checking Audit Log plugin status...\n"
  STATUS_AUDIT_PLUGIN=$(echo "${LIST_PLUGINS}" | grep -c "audit_log")
  if [ ${STATUS_AUDIT_PLUGIN} = 0 ]; then
    printf "INFO: Audit Log plugin is not installed.\n\n"
  else
    printf "INFO: Audit Log plugin is installed.\n\n"
  fi
fi

# Check PAM plugin status
if [ ${ENABLE_PAM} = 1 -o ${DISABLE_PAM} = 1 ]; then
  printf "Checking PAM plugin status...\n"
  STATUS_PAM_PLUGIN=$(echo "${LIST_PLUGINS}" | grep -c "auth_pam#")
  if [ ${STATUS_PAM_PLUGIN} = 0 ]; then
    printf "INFO: PAM Authentication plugin is not installed.\n\n"
  else
    printf "INFO: PAM Authentication plugin is installed.\n\n"
  fi
fi

# Check PAM compat plugin status
if [ ${ENABLE_PAM_COMPAT} = 1 -o ${DISABLE_PAM_COMPAT} = 1 ]; then
  printf "Checking PAM compat plugin status...\n"
  STATUS_PAM_COMPAT_PLUGIN=$(echo "${LIST_PLUGINS}" | grep -c "auth_pam_compat#")
  if [ ${STATUS_PAM_COMPAT_PLUGIN} = 0 ]; then
    printf "INFO: PAM Compat Authentication plugin is not installed.\n\n"
  else
    printf "INFO: PAM Compat Authentication plugin is installed.\n\n"
  fi
fi

# Check MySQL X plugin status
if [ ${ENABLE_MYSQLX} = 1 -o ${DISABLE_MYSQLX} = 1 ]; then
  printf "Checking MySQL X plugin status...\n"
  STATUS_MYSQLX_PLUGIN=$(echo "${LIST_PLUGINS}" | grep -c "mysqlx")
  if [ ${STATUS_MYSQLX_PLUGIN} = 0 ]; then
    printf "INFO: MySQL X plugin is not installed.\n\n"
  else
    printf "INFO: MySQL X plugin is installed.\n\n"
  fi
fi

# Add option to preload libHotBackup.so into my.cnf or LD_PRELOAD
# for jemalloc and libHotBackup.so into /etc/sysconfig/mysql
if [ ${FULL_SYSTEMD_MODE} = 0  ]; then
  if [ ${ENABLE_TOKUBACKUP} = 1 -a ${STATUS_HOTBACKUP_MYCNF} = 0 ]; then
    printf "Adding preload-hotbackup option into ${DEFAULTS_FILE}\n"
    for MYCNF_SECTION in mysqld_safe MYSQLD_SAFE
    do
      STATUS_MYSQLD_SAFE=$(grep -c "^\[${MYCNF_SECTION}\]$" ${DEFAULTS_FILE})
      if [ ${STATUS_MYSQLD_SAFE} != 0 ]; then
        MYSQLD_SAFE_SECTION=${MYCNF_SECTION}
        break
      fi
    done
    if [ ${STATUS_MYSQLD_SAFE} = 0 ]; then
      echo -e "\n[mysqld_safe]\npreload-hotbackup" >> ${DEFAULTS_FILE}
    else
      sed -i "/^\[${MYSQLD_SAFE_SECTION}\]$/a preload-hotbackup" ${DEFAULTS_FILE}
    fi
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully added preload-hotbackup option into ${DEFAULTS_FILE}\n";
      printf "PLEASE RESTART MYSQL SERVICE AND RUN THIS SCRIPT AGAIN TO FINISH INSTALLATION!\n\n";
      exit 0
    else
      printf "ERROR: Failed to add preload-hotbackup option into ${DEFAULTS_FILE}\n\n";
      exit 1
    fi
  fi
elif [ ${ENABLE_TOKUDB} = 1 -a ${STATUS_JEMALLOC_CONFIG} = 0 ] || [ ${ENABLE_TOKUBACKUP} = 1 -a ${STATUS_HOTBACKUP_MYCNF} = 0 ]; then
  printf "Adding LD_PRELOAD variable into ${SYSTEMD_ENV_FILE}\n"
  NEW_LD_PRELOAD=""
  FILE_ADD=""
  if [ ${ENABLE_TOKUDB} = 1 -a ${STATUS_JEMALLOC_CONFIG} = 0 ] && [ ${ENABLE_TOKUBACKUP} = 1 -a ${STATUS_HOTBACKUP_MYCNF} = 0 ]; then
    NEW_LD_PRELOAD="${JEMALLOC_LOCATION} ${HOTBACKUP_LOCATION}"
    FILE_ADD="libjemalloc.so.1 and libHotBackup.so"
  elif [ ${ENABLE_TOKUDB} = 1 -a ${STATUS_JEMALLOC_CONFIG} = 0 ]; then
    NEW_LD_PRELOAD="${JEMALLOC_LOCATION}"
    FILE_ADD="libjemalloc.so.1"
  elif [ ${ENABLE_TOKUBACKUP} = 1 -a ${STATUS_HOTBACKUP_MYCNF} = 0 ]; then
    NEW_LD_PRELOAD="${HOTBACKUP_LOCATION}"
    FILE_ADD="libHotBackup.so"
  fi
  # Add desired LD_PRELOAD into config file
  if [ ! -f ${SYSTEMD_ENV_FILE} ]; then
    echo "LD_PRELOAD=${NEW_LD_PRELOAD}" > ${SYSTEMD_ENV_FILE}
  elif [ $(grep -c LD_PRELOAD ${SYSTEMD_ENV_FILE}) = 0 ]; then
    echo "LD_PRELOAD=${NEW_LD_PRELOAD}" >> ${SYSTEMD_ENV_FILE}
  else
    sed -i '/^LD_PRELOAD=/ s:$: '"${NEW_LD_PRELOAD}"':' ${SYSTEMD_ENV_FILE}
  fi
  # Print status
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully added LD_PRELOAD variable for ${FILE_ADD} into ${SYSTEMD_ENV_FILE}\n\n";
    printf "PLEASE RESTART MYSQL SERVICE AND RUN THIS SCRIPT AGAIN TO FINISH INSTALLATION!\n\n";
    exit 0
  else
    printf "ERROR: Failed to add LD_PRELOAD variable for ${FILE_ADD} into ${SYSTEMD_ENV_FILE}\n\n";
    exit 1
  fi
fi

# Check if server is running with libHotBackup.so preloaded - if not warn that restart is needed
if [ ${ENABLE_TOKUBACKUP} = 1 ]; then
  printf "Checking if Percona Server is running with libHotBackup.so preloaded...\n"
  STATUS_LIBHOTBACKUP=$(grep -c libHotBackup.so /proc/${PID_NUM}/environ)
  if [ $STATUS_LIBHOTBACKUP = 0 ]; then
    printf "ERROR: Percona Server is not running with libHotBackup.so preloaded, please restart mysql service to enable it and then run this script again...\n\n";
    exit 1
  else
    printf "INFO: Percona Server is running with libHotBackup.so preloaded.\n\n";
  fi
fi

# Disable transparent huge pages in the current session so
# that the plugin can be installed without restarting PS
if [ ${ENABLE_TOKUDB} = 1 -a ${STATUS_THP_SYSTEM} = 1 ]; then
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
if [ ${ENABLE_TOKUDB} = 1 -a ${STATUS_THP_MYCNF} = 0 ]; then
  if [ ${FULL_SYSTEMD_MODE} = 0 ]; then
    printf "Adding thp-setting=never option into ${DEFAULTS_FILE}\n"
    for MYCNF_SECTION in mysqld_safe MYSQLD_SAFE
    do
      STATUS_MYSQLD_SAFE=$(grep -c "^\[${MYCNF_SECTION}\]$" ${DEFAULTS_FILE})
      if [ ${STATUS_MYSQLD_SAFE} != 0 ]; then
        MYSQLD_SAFE_SECTION=${MYCNF_SECTION}
        break
      fi
    done
    if [ ${STATUS_MYSQLD_SAFE} = 0 ]; then
      echo -e "\n[mysqld_safe]\nthp-setting=never" >> ${DEFAULTS_FILE}
    else
      sed -i "/^\[${MYSQLD_SAFE_SECTION}\]$/a thp-setting=never" ${DEFAULTS_FILE}
    fi
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully added thp-setting=never option into ${DEFAULTS_FILE}\n\n";
    else
      printf "ERROR: Failed to add thp-setting=never option into ${DEFAULTS_FILE}\n\n";
      exit 1
    fi
  else
    printf "Adding THP_SETTING=never variable into ${SYSTEMD_ENV_FILE}\n"
    echo -e "THP_SETTING=never" >> ${SYSTEMD_ENV_FILE}
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully added THP_SETTING=never option into ${SYSTEMD_ENV_FILE}\n\n";
    else
      printf "ERROR: Failed to add THP_SETTING=never option into ${SYSTEMD_ENV_FILE}\n\n";
      exit 1
    fi
  fi
fi

# Remove option for disabling transparent huge pages from config files
if [ ${DISABLE_TOKUDB} = 1 -a ${STATUS_THP_MYCNF} = 1 ]; then
  if [ ${FULL_SYSTEMD_MODE} = 0  ]; then
    printf "Removing thp-setting=never option from ${DEFAULTS_FILE}\n"
    sed -i '/^thp-setting=never$/d' ${DEFAULTS_FILE}
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully removed thp-setting=never option from ${DEFAULTS_FILE}\n\n";
    else
      printf "ERROR: Failed to remove thp-setting=never option from ${DEFAULTS_FILE}\n\n";
      exit 1
    fi
  else
    printf "Removing THP_SETTING variable from ${SYSTEMD_ENV_FILE}\n"
    sed -i '/^THP_SETTING=/d' ${SYSTEMD_ENV_FILE}
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully removed THP_SETTING variable from ${SYSTEMD_ENV_FILE}\n\n";
    else
      printf "ERROR: Failed to remove THP_SETTING variable from ${SYSTEMD_ENV_FILE}\n\n";
      exit 1
    fi
  fi
fi

# Remove option for preloading libHotBackup.so and jemalloc from config files
if [ ${FULL_SYSTEMD_MODE} = 0  ]; then
  if [ ${DISABLE_TOKUBACKUP} = 1 -a ${STATUS_HOTBACKUP_MYCNF} = 1 ]; then
    printf "Removing preload-hotbackup option from ${DEFAULTS_FILE}\n"
    sed -i '/^preload-hotbackup$/d' ${DEFAULTS_FILE}
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully removed preload-hotbackup option from ${DEFAULTS_FILE}\n\n";
    else
      printf "ERROR: Failed to remove preload-hotbackup option from ${DEFAULTS_FILE}\n\n";
      exit 1
    fi
  fi
elif [ ${DISABLE_TOKUDB} = 1 -o ${DISABLE_TOKUBACKUP} = 1 ]; then
  printf "Removing LD_PRELOAD option from ${SYSTEMD_ENV_FILE}\n"
  if [ ${DISABLE_TOKUDB} = 1 -a ${DISABLE_TOKUBACKUP} = 1 ]; then
    sed -i '/^LD_PRELOAD/d' ${SYSTEMD_ENV_FILE}
    FILE_REMOVE="libjemalloc.so.1 and libHotBackup.so"
  else
    NEW_LD_PRELOAD=""
    if [ ${DISABLE_TOKUDB} = 1 ]; then
      FILE_REMOVE="libjemalloc.so.1"
    else
      FILE_REMOVE="libHotBackup.so"
    fi
    for file in $(cat ${SYSTEMD_ENV_FILE}|awk -v var="${FILE_REMOVE}" -F ' ' '/LD_PRELOAD/{for(i=1;i<=NF;++i)if($i !~ var)print $i}'|sed 's/LD_PRELOAD=//')
    do
      NEW_LD_PRELOAD="${file} "
    done
    NEW_LD_PRELOAD=$(echo "LD_PRELOAD=${NEW_LD_PRELOAD}"|sed 's/[ ]*$//')
    sed -i 's:^LD_PRELOAD=.*:'"${NEW_LD_PRELOAD}"':' ${SYSTEMD_ENV_FILE}
  fi
    if [ $? -eq 0 ]; then
      printf "INFO: Successfully removed LD_PRELOAD option for ${FILE_REMOVE} from ${SYSTEMD_ENV_FILE}\n\n";
    else
      printf "ERROR: Failed to remove LD_PRELOAD option for ${FILE_REMOVE} from ${SYSTEMD_ENV_FILE}\n\n";
      exit 1
    fi
fi

# Install TokuDB engine plugin
if [ ${ENABLE_TOKUDB} = 1 -a ${STATUS_TOKUDB_PLUGIN} = 0 ]; then
  printf "Installing TokuDB engine...\n"
${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} 2>/dev/null<<EOFTOKUDBENABLE
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

# Install RocksDB engine plugin
if [ ${ENABLE_ROCKSDB} = 1 -a ${STATUS_ROCKSDB_PLUGIN} = 0 ]; then
  printf "Installing RocksDB engine...\n"
${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} 2>/dev/null<<EOFROCKSDBENABLE
INSTALL PLUGIN ROCKSDB SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_CFSTATS SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_DBSTATS SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_PERF_CONTEXT SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_PERF_CONTEXT_GLOBAL SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_CF_OPTIONS SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_GLOBAL_INFO SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_COMPACTION_STATS SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_DDL SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_INDEX_FILE_MAP SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_LOCKS SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_TRX SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_DEADLOCK SONAME 'ha_rocksdb.so';
EOFROCKSDBENABLE
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully installed RocksDB engine plugin.\n\n"
  else
    printf "ERROR: Failed to install RocksDB engine plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Install Query Response Time plugin
if [ ${ENABLE_QRT} = 1 -a ${STATUS_QRT_PLUGIN} = 0 ]; then
  printf "Installing Query Response Time plugin...\n"
${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} 2>/dev/null<<EOFQRTENABLE
INSTALL PLUGIN QUERY_RESPONSE_TIME_AUDIT SONAME 'query_response_time.so';
INSTALL PLUGIN QUERY_RESPONSE_TIME SONAME 'query_response_time.so';
INSTALL PLUGIN QUERY_RESPONSE_TIME_READ SONAME 'query_response_time.so';
INSTALL PLUGIN QUERY_RESPONSE_TIME_WRITE SONAME 'query_response_time.so';
EOFQRTENABLE
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully installed Query Response Time plugin.\n\n"
  else
    printf "ERROR: Failed to install Query Response Time plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Install Audit Log plugin
if [ ${ENABLE_AUDIT} = 1 -a ${STATUS_AUDIT_PLUGIN} = 0 ]; then
  printf "Installing Audit Log plugin...\n"
  ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "INSTALL PLUGIN audit_log SONAME 'audit_log.so';" 2>/dev/null
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully installed Audit Log plugin.\n\n"
  else
    printf "ERROR: Failed to install Audit Log plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Install PAM plugin
if [ ${ENABLE_PAM} = 1 -a ${STATUS_PAM_PLUGIN} = 0 ]; then
  printf "Installing PAM Authentication plugin...\n"
  ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "INSTALL PLUGIN auth_pam SONAME 'auth_pam.so';" 2>/dev/null
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully installed PAM Authentication plugin.\n\n"
  else
    printf "ERROR: Failed to install PAM Authentication plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Install PAM compat plugin
if [ ${ENABLE_PAM_COMPAT} = 1 -a ${STATUS_PAM_COMPAT_PLUGIN} = 0 ]; then
  printf "Installing PAM Compat Authentication plugin...\n"
  ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "INSTALL PLUGIN auth_pam_compat SONAME 'auth_pam_compat.so';" 2>/dev/null
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully installed PAM Compat Authentication plugin.\n\n"
  else
    printf "ERROR: Failed to install PAM Compat Authentication plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Install MySQL X plugin
if [ ${ENABLE_MYSQLX} = 1 -a ${STATUS_MYSQLX_PLUGIN} = 0 ]; then
  printf "Installing MySQL X plugin...\n"
  ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "INSTALL PLUGIN mysqlx SONAME 'mysqlx.so';" 2>/dev/null
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully installed MySQL X plugin.\n\n"
  else
    printf "ERROR: Failed to install MySQL X plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Install TokuDB backup plugin
if [ ${ENABLE_TOKUBACKUP} = 1 -a ${STATUS_HOTBACKUP_PLUGIN} = 0 ]; then
  printf "Installing TokuBackup plugin...\n"
${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} 2>/dev/null<<EOFTOKUBACKUPENABLE
INSTALL PLUGIN tokudb_backup SONAME 'tokudb_backup.so';
EOFTOKUBACKUPENABLE
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully installed TokuBackup plugin.\n\n"
  else
    printf "ERROR: Failed to install TokuBackup plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Uninstall TokuDB backup plugin
if [ ${DISABLE_TOKUBACKUP} = 1 -a ${STATUS_HOTBACKUP_PLUGIN} = 1 ]; then
  printf "Uninstalling TokuBackup plugin...\n"
${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} 2>/dev/null<<EOFTOKUBACKUPDISABLE
UNINSTALL PLUGIN tokudb_backup;
EOFTOKUBACKUPDISABLE
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully uninstalled TokuBackup plugin.\n\n"
  else
    printf "ERROR: Failed to uninstall TokuBackup plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Uninstall TokuDB engine plugin
if [ ${DISABLE_TOKUDB} = 1 -a ${STATUS_TOKUDB_PLUGIN} -gt 0 ]; then
  printf "Uninstalling TokuDB engine plugin...\n"
  for plugin in TokuDB TokuDB_file_map TokuDB_fractal_tree_info TokuDB_fractal_tree_block_map TokuDB_trx TokuDB_locks TokuDB_lock_waits TokuDB_background_job_status; do
    SPECIFIC_PLUGIN_STATUS=$(echo "${LIST_PLUGINS}" | grep -c "${plugin}#")
    if [ ${SPECIFIC_PLUGIN_STATUS} -gt 0 ]; then
      ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "UNINSTALL PLUGIN ${plugin};" 2>/dev/null
      if [ $? -ne 0 ]; then
        printf "ERROR: Failed to uninstall TokuDB engine plugin. Please check error log.\n\n"
        exit 1
      fi
    fi
  done
  printf "INFO: Successfully uninstalled TokuDB engine plugin.\n\n"
fi

# Uninstall RocksDB engine plugin
if [ ${DISABLE_ROCKSDB} = 1 -a ${STATUS_ROCKSDB_PLUGIN} -gt 0 ]; then
  printf "Uninstalling RocksDB engine plugin...\n"
  for plugin in ROCKSDB ROCKSDB_CFSTATS ROCKSDB_DBSTATS ROCKSDB_PERF_CONTEXT_GLOBAL ROCKSDB_PERF_CONTEXT ROCKSDB_CF_OPTIONS ROCKSDB_GLOBAL_INFO ROCKSDB_COMPACTION_STATS ROCKSDB_DDL ROCKSDB_INDEX_FILE_MAP ROCKSDB_LOCKS ROCKSDB_TRX ROCKSDB_DEADLOCK; do
    SPECIFIC_PLUGIN_STATUS=$(echo "${LIST_PLUGINS}" | grep -c "${plugin}#")
    if [ ${SPECIFIC_PLUGIN_STATUS} -gt 0 ]; then
      ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "UNINSTALL PLUGIN ${plugin};" 2>/dev/null
      if [ $? -ne 0 ]; then
        printf "ERROR: Failed to uninstall RocksDB engine plugin. Please check error log.\n\n"
        exit 1
      fi
    fi
  done
  printf "INFO: Successfully uninstalled RocksDB engine plugin.\n\n"
fi

# Uninstall Query Response Time plugin
if [ ${DISABLE_QRT} = 1 -a ${STATUS_QRT_PLUGIN} -gt 0 ]; then
  printf "Uninstalling Query Response Time plugin...\n"
  for plugin in QUERY_RESPONSE_TIME QUERY_RESPONSE_TIME_AUDIT QUERY_RESPONSE_TIME_READ QUERY_RESPONSE_TIME_WRITE; do
    SPECIFIC_PLUGIN_STATUS=$(echo "${LIST_PLUGINS}" | grep -c "${plugin}#")
    if [ ${SPECIFIC_PLUGIN_STATUS} -gt 0 ]; then
      ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "UNINSTALL PLUGIN ${plugin};" 2>/dev/null
      if [ $? -ne 0 ]; then
        printf "ERROR: Failed to uninstall Query Response Time plugin. Please check error log.\n\n"
        exit 1
      fi
    fi
  done
  printf "INFO: Successfully uninstalled Query Response Time plugin.\n\n"
fi

# Uninstall Audit Log plugin
if [ ${DISABLE_AUDIT} = 1 -a ${STATUS_AUDIT_PLUGIN} -gt 0 ]; then
  printf "Uninstalling Audit Log plugin...\n"
  ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "UNINSTALL PLUGIN audit_log;" 2>/dev/null
  if [ $? -ne 0 ]; then
    printf "ERROR: Failed to uninstall Audit Log plugin. Please check error log.\n\n"
    exit 1
  else
    printf "INFO: Successfully uninstalled Audit Log plugin.\n\n"
  fi
fi

# Uninstall PAM plugin
if [ ${DISABLE_PAM} = 1 -a ${STATUS_PAM_PLUGIN} -gt 0 ]; then
  printf "Uninstalling PAM Authentication plugin...\n"
  ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "UNINSTALL PLUGIN auth_pam;" 2>/dev/null
  if [ $? -ne 0 ]; then
    printf "ERROR: Failed to uninstall PAM Authentication plugin. Please check error log.\n\n"
    exit 1
  else
    printf "INFO: Successfully uninstalled PAM Authentication plugin.\n\n"
  fi
fi

# Uninstall PAM compat plugin
if [ ${DISABLE_PAM_COMPAT} = 1 -a ${STATUS_PAM_COMPAT_PLUGIN} -gt 0 ]; then
  printf "Uninstalling PAM Compat Authentication plugin...\n"
  ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "UNINSTALL PLUGIN auth_pam_compat;" 2>/dev/null
  if [ $? -ne 0 ]; then
    printf "ERROR: Failed to uninstall PAM Compat Authentication plugin. Please check error log.\n\n"
    exit 1
  else
    printf "INFO: Successfully uninstalled PAM Compat Authentication plugin.\n\n"
  fi
fi

# Uninstall MySQL X plugin
if [ ${DISABLE_MYSQLX} = 1 -a ${STATUS_MYSQLX_PLUGIN} -gt 0 ]; then
  printf "Uninstalling MySQL X plugin...\n"
  ${MYSQL_CLIENT_BIN} -u ${USER} ${PASSWORD} ${SOCKET} ${HOST} ${PORT} -e "UNINSTALL PLUGIN mysqlx;" 2>/dev/null
  if [ $? -ne 0 ]; then
    printf "ERROR: Failed to uninstall MySQL X plugin. Please check error log.\n\n"
    exit 1
  else
    printf "INFO: Successfully uninstalled MySQL X plugin.\n\n"
  fi
fi
