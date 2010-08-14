
// vim:sw=2:ai

/*
 * Copyright (C) 2010 DeNA Co.,Ltd.. All rights reserved.
 * See COPYRIGHT.txt for details.
 */

#ifndef DENA_MYSQL_INCL_HPP
#define DENA_MYSQL_INCL_HPP

#define HAVE_CONFIG_H
#define MYSQL_DYNAMIC_PLUGIN
#define MYSQL_SERVER 1

#include <mysql/my_config.h>
#ifdef DBUG_ON
#define SAFE_MUTEX
#endif

#include "mysql_priv.h"
#include <mysql/plugin.h>
#if MYSQL_VERSION_ID >= 60000
#include <transaction.h>
#endif

#undef min
#undef max

#endif

