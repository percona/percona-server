#!/bin/bash

AUX_DIR_BASE=/tmp/zenfs_aux

SOURCE_VARNAME=SOURCE_FS_URI$1
REPLICA_VARNAME=REPLICA_FS_URI$1
SOURCE_FS_URI=${!SOURCE_VARNAME}
REPLICA_FS_URI=${!REPLICA_VARNAME}
SOURCE_ZBD=${SOURCE_FS_URI#zenfs://dev:}
REPLICA_ZBD=${REPLICA_FS_URI#zenfs://dev:}

echo reinitializing ${SOURCE_ZBD} \(source\) and ${REPLICA_ZBD} \(replica\) for worker $1

SOURCE_AUX_DIR=${AUX_DIR_BASE}/${SOURCE_ZBD}
REPLICA_AUX_DIR=${AUX_DIR_BASE}/${REPLICA_ZBD}

rm -rf ${SOURCE_AUX_DIR}
rm -rf ${REPLICA_AUX_DIR}
mkdir -p ${AUX_DIR_BASE}

if [ -z "$MTR_BINDIR" ]
then
  # in tarballs (where MTR_BINDIR is not defined) use relative path based on the
  # location of this script
  ZENFS_UTIL=$(dirname $0)/../..
else
  # in out-of-source builds (where MTR_BINDIR is defined) use this variable
  ZENFS_UTIL=${MTR_BINDIR}
fi
ZENFS_UTIL+=/bin/zenfs

${ZENFS_UTIL} mkfs --zbd=${SOURCE_ZBD} --aux_path=${SOURCE_AUX_DIR} --force
${ZENFS_UTIL} mkfs --zbd=${REPLICA_ZBD} --aux_path=${REPLICA_AUX_DIR} --force
