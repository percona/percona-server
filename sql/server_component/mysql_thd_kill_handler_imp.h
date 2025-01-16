#ifndef SQL_SERVER_COMPONENT_MYSQL_THD_KILL_HANDLER_IMP_H
#define SQL_SERVER_COMPONENT_MYSQL_THD_KILL_HANDLER_IMP_H

#include "mysql/components/service_implementation.h"
#include "mysql/components/services/mysql_thd_kill_handler.h"

/**
  Implementation of service enabling components to install/remove handler
  for connection/statement being killed.
*/
class Mysql_thd_kill_handler_imp {
 public:
  static DEFINE_BOOL_METHOD(set,
                            (MYSQL_THD thd, kill_handler_fn fn, void *data));

 private:
  /**
    Invoke kill handler and pass THD, kill type and pointer to auxiliary
     data to it as parameters.
  */
  static void call_handler(THD *thd, kill_handler_fn fn, void *data);

  friend class THD;
};

#endif  // !SQL_SERVER_COMPONENT_MYSQL_THD_KILL_HANDLER_IMP_H
