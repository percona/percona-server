#!/bin/bash

AUX_DIR_BASE=/tmp/zenfs_aux

VARNAME=FS_URI$1
VARVALUE=${!VARNAME}
ZBD=${VARVALUE#zenfs://dev:}

echo reinitializing ${ZBD} for worker $1
AUX_DIR=${AUX_DIR_BASE}/${ZBD}

rm -rf ${AUX_DIR}
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

${ZENFS_UTIL} mkfs --zbd=${ZBD} --aux_path=${AUX_DIR} --force
