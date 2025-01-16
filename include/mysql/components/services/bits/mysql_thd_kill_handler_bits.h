#ifndef COMPONENTS_SERVICES_MYSQL_THD_KILL_HANDLER_BITS_H
#define COMPONENTS_SERVICES_MYSQL_THD_KILL_HANDLER_BITS_H

#include <cstdint>  // int16_t

#include "mysql/components/services/bits/mysql_thd_attributes_bits.h"  // STATUS_SESSION_KILLED,...
#include "mysql/components/services/bits/thd.h"  // MYSQL_THD

typedef void (*kill_handler_fn)(MYSQL_THD thd, uint16_t state, void *data);

#endif /* COMPONENTS_SERVICES_MYSQL_THD_KILL_HANDLER_BITS_H */
