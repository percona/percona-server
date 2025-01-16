#ifndef COMPONENTS_SERVICES_MYSQL_THD_KILL_HANDLER_H
#define COMPONENTS_SERVICES_MYSQL_THD_KILL_HANDLER_H

#include "mysql/components/services/bits/mysql_thd_kill_handler_bits.h"  // MYSQL_THD, kill_handler_fn

/**
  Service enabling components to install/remove handler for connection/
  statement being killed.
*/

BEGIN_SERVICE_DEFINITION(mysql_thd_kill_handler)

/**
  Install/remove a handler to be invoked when connection or statement being
  executed in it are killed or reach MAX_EXECUTION_TIME limit.
  The idea is that component might want to abort some long running or
  blocking operation it executes in this case.

  @param thd    MYSQL_THD representing the connection.
  @param fn     Handler function to be invoked when connection or statement
                in it is killed. This function will be passed MYSQL_THD for
                the connection affected, type of KILL operation and value
                of 'data' parameter.
                NULL indicates that already installed handler should be
                removed.
  @param data   Pointer to auxiliary data to be passed to the handler
                function.

  @retval False - success.
  @retval True -  failure (callback already installed).

  @note The handler is invoked from a thread other than one handling
        connection/statement being killed. Hence one needs to ensure
        handler's thread-safety.
*/

DECLARE_BOOL_METHOD(set, (MYSQL_THD thd, kill_handler_fn fn, void *data));

END_SERVICE_DEFINITION(mysql_thd_kill_handler)

#endif  // !COMPONENTS_SERVICES_MYSQL_THD_KILL_HANDLER_H
