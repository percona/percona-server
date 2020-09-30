#!/bin/sh
DIR=$(dirname $(realpath $0))
BRANCH=$(git branch --show-current)
IFS='.' read -r MAJOR MINOR PATCH <<< $(echo $BRANCH | awk -F'-' '{print $2}')
EXTRA=$(echo $BRANCH | awk -F'-' '{print $3}')
source ${DIR}/../VERSION
if [ ${MYSQL_VERSION_MAJOR} != ${MAJOR} ]; then
    sed -i "s:MYSQL_VERSION_MAJOR=${MYSQL_VERSION_MAJOR}:MYSQL_VERSION_MAJOR=${MAJOR}:" ${DIR}/../VERSION
fi
if [ ${MYSQL_VERSION_MINOR} != ${MINOR} ]; then
    sed -i "s:MYSQL_VERSION_MINOR=${MYSQL_VERSION_MINOR}:MYSQL_VERSION_MINOR=${MINOR}:" ${DIR}/../VERSION
fi
if [ ${MYSQL_VERSION_PATCH} != ${PATCH} ]; then
    sed -i "s:MYSQL_VERSION_PATCH=${MYSQL_VERSION_PATCH}:MYSQL_VERSION_PATCH=${PATCH}:" ${DIR}/../VERSION
fi
if [ "${MYSQL_VERSION_EXTRA}" != "-${EXTRA}" ]; then
    sed -i "s:MYSQL_VERSION_EXTRA=${MYSQL_VERSION_EXTRA}:MYSQL_VERSION_EXTRA=-${EXTRA}:" ${DIR}/../VERSION
fi
INNODB_VER=$(grep "define PERCONA_INNODB_VERSION" ${DIR}/../storage/innobase/include/univ.i | awk '{print $3}')
if [ ${INNODB_VER} != ${EXTRA} ]; then
    sed -i "s:define PERCONA_INNODB_VERSION ${INNODB_VER}:define PERCONA_INNODB_VERSION ${EXTRA}:" ${DIR}/../storage/innobase/include/univ.i
fi
FULL_VER=$(echo ${BRANCH} | awk -F'release-' '{print $2}')
sed -i "s:'release-.*', d:'release-${FULL_VER}', d:" ${DIR}/../Jenkinsfile
sed -i "s:'Percona-Server-.*', d:'Percona-Server-${FULL_VER}', d:g" ${DIR}/../Jenkinsfile

