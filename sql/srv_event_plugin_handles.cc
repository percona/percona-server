/* Copyright (c) 2024, 2025, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "srv_event_plugin_handles.h"
#include <list>
#include <string>
#include "mysql/components/my_registry_query.h"
#include "mysql/components/my_service.h"
#include "mysql/components/services/defs/event_tracking_authentication_defs.h"
#include "mysql/components/services/event_tracking_authentication_service.h"
#include "mysql/components/services/event_tracking_command_service.h"
#include "mysql/components/services/event_tracking_connection_service.h"
#include "mysql/components/services/event_tracking_general_service.h"
#include "mysql/components/services/event_tracking_global_variable_service.h"
#include "mysql/components/services/event_tracking_lifecycle_service.h"
#include "mysql/components/services/event_tracking_message_service.h"
#include "mysql/components/services/event_tracking_parse_service.h"
#include "mysql/components/services/event_tracking_query_service.h"
#include "mysql/components/services/event_tracking_stored_program_service.h"
#include "mysql/components/services/event_tracking_table_access_service.h"
#include "mysql/components/services/registry.h"
#include "sql/mysqld.h"  // srv_registry
#include "sql/sql_audit.h"

static struct s_state {
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_authentication)>
      authentication;
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_command)> command;
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_connection)>
      connection;
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_general)> general;
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_global_variable)>
      global_var;
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_message)> message;
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_parse)> parse;
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_query)> query;
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_lifecycle)>
      lifecycle;
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_stored_program)>
      stored_program;
  My_registry_query_and_acquire<SERVICE_TYPE(event_tracking_table_access)>
      table_access;

  s_state()
      : authentication("event_tracking_authentication", srv_registry,
                       srv_registry_query),
        command("event_tracking_command", srv_registry, srv_registry_query),
        connection("event_tracking_connection", srv_registry,
                   srv_registry_query),
        general("event_tracking_general", srv_registry, srv_registry_query),
        global_var("event_tracking_global_variable", srv_registry,
                   srv_registry_query),
        message("event_tracking_message", srv_registry, srv_registry_query),
        parse("event_tracking_parse", srv_registry, srv_registry_query),
        query("event_tracking_query", srv_registry, srv_registry_query),
        lifecycle("event_tracking_lifecycle", srv_registry, srv_registry_query),
        stored_program("event_tracking_stored_program", srv_registry,
                       srv_registry_query),
        table_access("event_tracking_table_access", srv_registry,
                     srv_registry_query) {}
  bool has_any() {
    /*
      We compare >1 for each because the component to plugin bridge is supposed
      must be present for every class.
    */
    return (authentication.size() > 1 || command.size() > 1 ||
            connection.size() > 1 || general.size() > 1 ||
            global_var.size() > 1 || message.size() > 1 || parse.size() > 1 ||
            query.size() > 1 || lifecycle.size() > 1 ||
            stored_program.size() > 1 || table_access.size() > 1);
  }
  bool init() {
    bool const retval = authentication.init() || command.init() ||
                        connection.init() || general.init() ||
                        global_var.init() || message.init() || parse.init() ||
                        query.init() || lifecycle.init() ||
                        stored_program.init() || table_access.init();
    /* now assert that we have the component->plugin bridge for each class */
    assert(authentication.size() > 0);
    assert(command.size() > 0);
    assert(connection.size() > 0);
    assert(general.size() > 0);
    assert(global_var.size() > 0);
    assert(message.size() > 0);
    assert(parse.size() > 0);
    assert(query.size() > 0);
    assert(lifecycle.size() > 0);
    assert(stored_program.size() > 0);
    assert(table_access.size() > 0);
    return retval;
  }
} *state{nullptr};

bool srv_event_acquire_plugin_handles() {
  state = new s_state;
  return state->init();
}

void srv_event_release_plugin_handles() {
  if (state) {
    delete state;
    state = nullptr;
  }
}

bool srv_event_call_plugin_handles(struct st_mysql_event_generic *event_data) {
  if (!state) return false;

  bool retval = false;

  switch (event_data->event_class) {
    case Event_tracking_class::AUTHENTICATION: {
      for (const auto *handle : state->authentication) {
        retval |=
            (handle->notify(reinterpret_cast<
                            const mysql_event_tracking_authentication_data *>(
                event_data->event))) != 0;
      }
      break;
    }
    case Event_tracking_class::COMMAND: {
      for (const auto *handle : state->command) {
        retval |=
            (handle->notify(
                reinterpret_cast<const mysql_event_tracking_command_data *>(
                    event_data->event))) != 0;
      }
      break;
    }

    case Event_tracking_class::CONNECTION: {
      for (const auto *handle : state->connection) {
        retval |=
            (handle->notify(
                 reinterpret_cast<const mysql_event_tracking_connection_data *>(
                     event_data->event)) != 0);
      }
      break;
    }
    case Event_tracking_class::GENERAL: {
      for (const auto *handle : state->general) {
        retval |=
            (handle->notify(
                 reinterpret_cast<const mysql_event_tracking_general_data *>(
                     event_data->event)) != 0);
      }
      break;
    }
    case Event_tracking_class::GLOBAL_VARIABLE: {
      for (const auto *handle : state->global_var) {
        retval |=
            (handle->notify(reinterpret_cast<
                            const mysql_event_tracking_global_variable_data *>(
                event_data->event))) != 0;
      }
      break;
    }
    case Event_tracking_class::MESSAGE: {
      for (const auto *handle : state->message) {
        retval |=
            (handle->notify(
                reinterpret_cast<const mysql_event_tracking_message_data *>(
                    event_data->event))) != 0;
      }
      break;
    }
    case Event_tracking_class::PARSE: {
      for (const auto *handle : state->parse) {
        retval |=
            (handle->notify(reinterpret_cast<mysql_event_tracking_parse_data *>(
                const_cast<void *>(event_data->event)))) != 0;
      }
      break;
    }
    case Event_tracking_class::QUERY: {
      for (const auto *handle : state->query) {
        retval |= (handle->notify(
                      reinterpret_cast<const mysql_event_tracking_query_data *>(
                          event_data->event))) != 0;
      }
      break;
    }
    case Event_tracking_class::SHUTDOWN: {
      for (const auto *handle : state->lifecycle) {
        retval |=
            (handle->notify_shutdown(
                reinterpret_cast<const mysql_event_tracking_shutdown_data *>(
                    event_data->event))) != 0;
      }
      break;
    }
    case Event_tracking_class::STARTUP: {
      for (const auto *handle : state->lifecycle) {
        retval |=
            (handle->notify_startup(
                reinterpret_cast<const mysql_event_tracking_startup_data *>(
                    event_data->event))) != 0;
      }
      break;
    }
    case Event_tracking_class::STORED_PROGRAM: {
      for (const auto *handle : state->stored_program) {
        retval |=
            (handle->notify(reinterpret_cast<
                            const mysql_event_tracking_stored_program_data *>(
                event_data->event))) != 0;
      }
      break;
    }
    case Event_tracking_class::TABLE_ACCESS: {
      for (const auto *handle : state->table_access) {
        retval |=
            (handle->notify(reinterpret_cast<
                            const mysql_event_tracking_table_access_data *>(
                event_data->event))) != 0;
      }
      break;
    }
    default: {
      assert(false);
      retval = false;
      break;
    }
  };
  return retval;
}

bool srv_event_have_plugin_handles() { return state && state->has_any(); }
