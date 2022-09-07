#!/bin/bash
#
# Script for doing various administrative tasks in Percona Server
# like installing/uninstalling RocksDB storage engine
# and plugins like Audit Log and PAM
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
ENABLE_ROCKSDB=0
DISABLE_ROCKSDB=0
ENABLE_AUDIT=0
DISABLE_AUDIT=0
ENABLE_PAM=0
DISABLE_PAM=0
ENABLE_PAM_COMPAT=0
DISABLE_PAM_COMPAT=0
ENABLE_MYSQLX=0
DISABLE_MYSQLX=0
STATUS_ROCKSDB_PLUGIN=0
STATUS_AUDIT_PLUGIN=0
STATUS_PAM_PLUGIN=0
STATUS_PAM_COMPAT_PLUGIN=0
STATUS_MYSQLX_PLUGIN=0
STATUS_MYSQLD_SAFE=0
HAROCKSDB_LOCATION=""

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
  --longoptions=config-file:,user:,password::,socket:,host:,port:,enable-tokudb,disable-tokudb,enable-tokubackup,disable-tokubackup,help,defaults-file:,force-envfile,force-mycnf,enable-rocksdb,disable-rocksdb,enable-audit,disable-audit,enable-pam,disable-pam,enable-pam-compat,disable-pam-compat,enable-mysqlx,disable-mysqlx,docker \
  --name="$(basename "$0")" -- "$@")"
  test $? -eq 0 || exit 1
  eval set -- $go_out
fi

function print_tokudb_removal()
{
  printf "ERROR: As of Percona Server 8.0.28-19, the TokuDB storage engine and backup plugins have been completely removed and are no longer supported or distributed with Percona Server. Please see this blog post for more information https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0\n"
}

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
    print_tokudb_removal
    exit 1
    ;;
    -e | --enable-tokudb )
    print_tokudb_removal
    exit 1
    ;;
    -d | --disable-tokudb )
    print_tokudb_removal
    exit 1
    ;;
    -b | --enable-tokubackup )
    print_tokudb_removal
    exit 1
    ;;
    -r | --disable-tokubackup )
    print_tokudb_removal
    exit 1
    ;;
    -k | --enable-rocksdb )
    shift
    ENABLE_ROCKSDB=1
    ;;
    -o | --disable-rocksdb )
    shift
    DISABLE_ROCKSDB=1
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
    print_tokudb_removal
    exit 1
    ;;
    -f | --force-envfile )
    print_tokudb_removal
    exit 1
    ;;
    --help )
    printf "This script can be used to setup plugins and the RocksDB storage engine for Percona Server 8.0.\n"
    printf "Valid options are:\n"
    printf "  --config-file=file, -c file\t\t read credentials and options from config file\n"
    printf "  --user=user_name, -u user_name\t mysql admin username\n"
    printf "  --password[=password], -p[password]\t mysql admin password (on empty will prompt to enter)\n"
    printf "  --socket=path, -S path\t\t the socket file to use for connection\n"
    printf "  --host=host_name, -h host_name\t connect to given host\n"
    printf "  --port=port_num, -P port_num\t\t port number to use for connection\n"
    printf "  --enable-rocksdb, -k\t\t\t enable RocksDB storage engine plugin\n"
    printf "  --disable-rocksdb, -o\t\t\t disable RocksDB storage engine plugin\n"
    printf "  --enable-audit, -a\t\t\t enable Audit Log plugin\n"
    printf "  --disable-audit, -w\t\t\t disable Audit Log plugin\n"
    printf "  --enable-pam, -i\t\t\t enable PAM Authentication plugin\n"
    printf "  --disable-pam, -n\t\t\t disable PAM Authentication plugin\n"
    printf "  --enable-pam-compat, -j\t\t enable PAM Compat Authentication plugin\n"
    printf "  --disable-pam-compat, -K\t\t disable PAM Compat Authentication plugin\n"
    printf "  --enable-mysqlx, -x\t\t\t enable MySQL X plugin\n"
    printf "  --disable-mysqlx, -g\t\t\t disable MySQL X plugin\n"
    printf "  --help\t\t\t\t show this help\n\n"
    exit 0
    ;;
  esac
done

# Assign options for mysql client
PASSWORD=${PASSWORD:+"-p${PASSWORD}"}
SOCKET=${SOCKET:+"-S ${SOCKET}"}
HOST=${HOST:+"-h ${HOST}"}
PORT=${PORT:+"-P ${PORT}"}

if [ ${ENABLE_ROCKSDB} = 0 -a ${DISABLE_ROCKSDB} = 0 -a ${ENABLE_AUDIT} = 0 -a ${DISABLE_AUDIT} = 0 -a ${ENABLE_PAM} = 0 -a ${DISABLE_PAM} = 0 -a ${ENABLE_PAM_COMPAT} = 0 -a ${DISABLE_PAM_COMPAT} = 0 -a ${ENABLE_MYSQLX} = 0 -a ${DISABLE_MYSQLX} = 0 ]; then
  printf "ERROR: You should specify one of the --enable or --disable options.\n"
  printf "Use --help for printing options.\n"
  exit 1
elif [ ${ENABLE_ROCKSDB} = 1 -a ${DISABLE_ROCKSDB} = 1 ]; then
  printf "ERROR: Only --enable-rocksdb OR --disable-rocksdb can be specified - not both!\n\n"
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

# Check if RocksDB plugin available on the system
if [ ${ENABLE_ROCKSDB} = 1 ]; then
  printf "Checking if RocksDB plugin is available for installation ...\n"
  for ha_rocksdb_loc in "${SCRIPT_PWD%/*}/lib/plugin" "/usr/lib64/mysql/plugin" "/usr/lib/mysql/plugin"; do
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
INSTALL PLUGIN ROCKSDB_COMPACTION_HISTORY SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_COMPACTION_STATS SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_ACTIVE_COMPACTION_STATS SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_DDL SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_INDEX_FILE_MAP SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_LOCKS SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_TRX SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_DEADLOCK SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_SST_PROPS SONAME 'ha_rocksdb.so';
INSTALL PLUGIN ROCKSDB_LIVE_FILES_METADATA SONAME 'ha_rocksdb.so';
EOFROCKSDBENABLE
  if [ $? -eq 0 ]; then
    printf "INFO: Successfully installed RocksDB engine plugin.\n\n"
  else
    printf "ERROR: Failed to install RocksDB engine plugin. Please check error log.\n\n"
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

# Uninstall RocksDB engine plugin
if [ ${DISABLE_ROCKSDB} = 1 -a ${STATUS_ROCKSDB_PLUGIN} -gt 0 ]; then
  printf "Uninstalling RocksDB engine plugin...\n"
  for plugin in ROCKSDB ROCKSDB_CFSTATS ROCKSDB_DBSTATS ROCKSDB_PERF_CONTEXT_GLOBAL ROCKSDB_PERF_CONTEXT ROCKSDB_CF_OPTIONS ROCKSDB_GLOBAL_INFO ROCKSDB_COMPACTION_HISTORY ROCKSDB_COMPACTION_STATS ROCKSDB_ACTIVE_COMPACTION_STATS ROCKSDB_DDL ROCKSDB_INDEX_FILE_MAP ROCKSDB_LOCKS ROCKSDB_TRX ROCKSDB_DEADLOCK ROCKSDB_SST_PROPS ROCKSDB_LIVE_FILES_METADATA; do
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
