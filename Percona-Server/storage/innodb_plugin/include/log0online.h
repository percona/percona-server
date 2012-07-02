/*****************************************************************************

Copyright (c) 2011-2012, Percona Inc. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

*****************************************************************************/

/**************************************************//**
@file include/log0online.h
Online database log parsing for changed page tracking
*******************************************************/

#ifndef log0online_h
#define log0online_h

#include "univ.i"

/*********************************************************************//**
Initializes the online log following subsytem. */
UNIV_INTERN
void
log_online_read_init();
/*===================*/

/*********************************************************************//**
Shuts down the online log following subsystem. */
UNIV_INTERN
void
log_online_read_shutdown();
/*=======================*/

/*********************************************************************//**
Reads and parses the redo log up to last checkpoint LSN to build the changed
page bitmap which is then written to disk.  */
UNIV_INTERN
void
log_online_follow_redo_log();
/*=========================*/

#endif
