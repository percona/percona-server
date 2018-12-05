// Copyright (c) 2014, Google Inc.
// Copyright (c) 2017, MariaDB Corporation.

/**************************************************/ /**
 @file btr/btr0scrub.cc
 Scrubbing of btree pages

 *******************************************************/

#include "btr0scrub.h"
#include "btr0btr.h"
#include "btr0cur.h"
#include "dict0dict.h"
#include "fsp0fsp.h"
#include "ibuf0ibuf.h"
#include "mtr0mtr.h"

/**
 * scrub data at delete time (e.g purge thread)
 */
bool srv_immediate_scrub_data_uncompressed = false;
