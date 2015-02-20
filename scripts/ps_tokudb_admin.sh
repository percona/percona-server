#!/bin/bash
#
# Script for installing TokuDB plugin in Percona Server
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
MYCNF_LOCATION=
MYSQLD_SAFE_STATUS=0

# Check if we have a functional getopt(1)
if ! getopt --test
  then
  go_out="$(getopt --options=u:p::S:h:P:ed \
  --longoptions=user:,password::,socket:,host:,port:,enable,disable,help \
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
    -e | --enable )
    shift
    ENABLE=1
    ;;
    -d | --disable )
    shift
    DISABLE=1
    ;;
    --help )
    printf "This script is used for installing and uninstalling TokuDB plugin for Percona Server 5.6.\n"
    printf "If transparent huge pages are enabled on the system it adds thp-setting=never option to my.cnf\n"
    printf "to disable it on runtime.\n\n"
    printf "Valid options are:\n"
    printf "  --user=user_name, -u user_name\t mysql admin username\n"
    printf "  --password[=password], -p[password]\t mysql admin password (on empty will prompt to enter)\n"
    printf "  --socket=path, -S path\t\t the socket file to use for connection\n"
    printf "  --host=host_name, -h host_name\t connect to given host\n"
    printf "  --port=port_num, -P port_num\t\t port number to use for connection\n"
    printf "  --enable, -e\t\t\t\t enable TokuDB plugin and disable transparent huge pages in my.cnf\n"
    printf "  --disable, d\t\t\t\t disable TokuDB plugin and remove thp-setting=never option in my.cnf\n"
    printf "  --help\t\t\t\t show this help\n\n"
    printf "For TokuDB requirements and manual steps for installation please visit this webpage:\n"
    printf "http://www.percona.com/doc/percona-server/5.6/tokudb/tokudb_installation.html\n\n"
    exit 0
    ;;
  esac
done

# Make sure only root can run this script
if [ $(id -u) -ne 0 ]; then
  echo "This script must be run as root!" 1>&2
  exit 1
fi

if [ $ENABLE = 1 -a $DISABLE = 1 ]; then
  printf "Only --enable OR --disable can be specified - not both!\n"
  exit 1
elif [ $ENABLE = 0 -a $DISABLE = 0 ]; then
  printf "You should specify --enable or --disable option. Use --help for printing options.\n"
  exit 1
fi

# Check if server is running with jemalloc - if not warn that restart is needed
printf "Checking if Percona server is running with jemalloc enabled...\n"
PID_LOCATION=$(mysql -e "show variables like 'pid_file';" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null|grep pid_file|awk '{print $2}')
if [ $? -ne 0 ] || [ "${PID_LOCATION}" == "" ]; then
  printf ">> Error checking pid file location! Please check username, password and other options...\n";
  exit 1
fi
PID_NUM=$(cat ${PID_LOCATION})
JEMALLOC_STATUS=$(grep -c jemalloc /proc/${PID_NUM}/environ)
if [ $JEMALLOC_STATUS = 0 ]; then
  printf ">> Percona server is not running with jemalloc, please restart server to enable it and then run this script...\n\n";
  exit 1
else
  printf ">> Percona server is running with jemalloc enabled.\n\n";
fi

# Check transparent huge pages status on the system
printf "Checking transparent huge pages status on the system...\n"
if [ -f /sys/kernel/mm/transparent_hugepage/enabled ]; then
  CONTENT_TRANSHP=$(</sys/kernel/mm/transparent_hugepage/enabled)
  STATUS_THP_SYSTEM=$(echo $CONTENT_TRANSHP | grep -cv '\[never\]')
fi
if [ $STATUS_THP_SYSTEM = 0 ]; then
  printf ">> Transparent huge pages are currently disabled on the system.\n\n"
else
  printf ">> Transparent huge pages are enabled (should be disabled).\n\n"
fi

# Check thp-setting=never option in my.cnf
printf "Checking if thp-setting=never option is already set in config file...\n"
STATUS_THP_MYCNF=$(my_print_defaults server mysqld mysqld_safe|grep -c thp-setting=never)
if [ $STATUS_THP_MYCNF = 0 ]; then
  printf ">> Option thp-setting=never is not set in the config file.\n"
  printf ">> (needed only if THP is not disabled permanently on the system)\n\n"
else
  printf ">> Option thp-setting=never is set in the config file.\n\n"
fi

# Check location of my.cnf
if [ -f /etc/my.cnf ]; then
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

# Check TokuDB plugin status
printf "Checking TokuDB plugin status...\n"
LIST_ENGINE=$(mysql -e "show plugins;" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null)
if [ $? -ne 0 ]; then
  printf ">> Error checking TokuDB plugin status! Please check username, password and other options...\n";
  exit 1
fi
STATUS_PLUGIN=$(echo "$LIST_ENGINE" | grep -c "TokuDB")
if [ $STATUS_PLUGIN = 0 ]; then
  printf ">> TokuDB plugin is not installed.\n\n"
elif [ $STATUS_PLUGIN = 7 ]; then
  printf ">> TokuDB plugin is installed.\n\n"
else
  printf ">> TokuDB plugin is partially installed. Please cleanup manually.\n\n"
  exit 1
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
    printf ">> Successfuly disabled transparent huge pages for this session.\n\n"
  else
    printf ">> Error disabling transparent huge pages for this session.\n\n"
    exit 1
  fi
fi

# Add option to disable transparent huge pages into my.cnf
if [ $ENABLE = 1 -a $STATUS_THP_MYCNF = 0 ]; then
  printf "Adding thp-setting=never option into $MYCNF_LOCATION\n"
  MYSQLD_SAFE_STATUS=$(grep -c "^\[mysqld_safe\]$" $MYCNF_LOCATION)
  if [ $MYSQLD_SAFE_STATUS = 0 ]; then
    echo -e "\n[mysqld_safe]\nthp-setting=never" >> $MYCNF_LOCATION
  else
    sed -i '/^\[mysqld_safe\]$/a thp-setting=never' $MYCNF_LOCATION
  fi
  if [ $? -eq 0 ]; then
    printf ">> Successfuly added thp-setting=never option into $MYCNF_LOCATION\n\n";
  else
    printf ">> Error adding thp-setting=never option into $MYCNF_LOCATION\n\n";
    exit 1
  fi
fi

# Remove option for disabling transparent huge pages from my.cnf
if [ $DISABLE = 1 -a $STATUS_THP_MYCNF = 1 ]; then
  printf "Removing thp-setting=never option from $MYCNF_LOCATION\n"
  sed -i '/^thp-setting=never$/d' $MYCNF_LOCATION
  if [ $? -eq 0 ]; then
    printf ">> Successfuly removed thp-setting=never option from $MYCNF_LOCATION\n\n";
  else
    printf ">> Error removing thp-setting=never option from $MYCNF_LOCATION\n\n";
    exit 1
  fi
fi

# Installing TokuDB plugin
if [ $ENABLE = 1 -a $STATUS_PLUGIN = 0 ]; then
  printf "Installing TokuDB engine...\n"
  mysql -e "INSTALL PLUGIN tokudb SONAME 'ha_tokudb.so';" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "INSTALL PLUGIN tokudb_file_map SONAME 'ha_tokudb.so';" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "INSTALL PLUGIN tokudb_fractal_tree_info SONAME 'ha_tokudb.so';" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "INSTALL PLUGIN tokudb_fractal_tree_block_map SONAME 'ha_tokudb.so';" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "INSTALL PLUGIN tokudb_trx SONAME 'ha_tokudb.so';" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "INSTALL PLUGIN tokudb_locks SONAME 'ha_tokudb.so';" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "INSTALL PLUGIN tokudb_lock_waits SONAME 'ha_tokudb.so';" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null
  if [ $? -eq 0 ]; then
    printf ">> Successfuly installed TokuDB plugin.\n\n"
  else
    printf ">> Error installing TokuDB plugin. Please check error log.\n\n"
    exit 1
  fi
fi

# Uninstalling TokuDB plugin
if [ $DISABLE = 1 -a $STATUS_PLUGIN = 7 ]; then
  printf "Uninstalling TokuDB plugin...\n"
  mysql -e "UNINSTALL PLUGIN tokudb;" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "UNINSTALL PLUGIN tokudb_file_map;" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "UNINSTALL PLUGIN tokudb_fractal_tree_info;" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "UNINSTALL PLUGIN tokudb_fractal_tree_block_map;" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "UNINSTALL PLUGIN tokudb_trx;" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "UNINSTALL PLUGIN tokudb_locks;" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null &&
  mysql -e "UNINSTALL PLUGIN tokudb_lock_waits;" -u $USER $PASSWORD $SOCKET $HOST $PORT 2>/dev/null
  if [ $? -eq 0 ]; then
    printf ">> Successfuly uninstalled TokuDB plugin.\n\n"
  else
    printf ">> Error uninstalling TokuDB plugin. Please check error log.\n\n"
    exit 1
  fi
fi
