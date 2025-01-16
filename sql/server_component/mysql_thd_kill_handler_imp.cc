#include "sql/server_component/mysql_thd_kill_handler_imp.h"

#include <mysql/components/minimal_chassis.h>
#include <mysql/components/service_implementation.h>

#include "sql/sql_class.h"

DEFINE_BOOL_METHOD(Mysql_thd_kill_handler_imp::set,
                   (MYSQL_THD thd_arg, kill_handler_fn fn, void *data)) {
  THD *thd = static_cast<THD *>(thd_arg);
  if (thd == nullptr) thd = current_thd;
  return thd->set_kill_handler(fn, data);
}

void Mysql_thd_kill_handler_imp::call_handler(THD *thd, kill_handler_fn fn,
                                              void *data) {
  uint16_t int_state = [](auto state) {
    switch (state) {
      case THD::killed_state::NOT_KILLED:
        return STATUS_SESSION_OK;
      case THD::killed_state::KILL_CONNECTION:
        return STATUS_SESSION_KILLED;
      case THD::killed_state::KILL_QUERY:
        return STATUS_QUERY_KILLED;
      case THD::killed_state::KILL_TIMEOUT:
        return STATUS_QUERY_TIMEOUT;
      case THD::killed_state::KILLED_NO_VALUE:
        return STATUS_SESSION_OK;
      default:
        return STATUS_SESSION_OK;
    }
  }(thd->is_killed());

  fn(thd, int_state, data);
}
