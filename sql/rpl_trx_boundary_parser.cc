/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/rpl_trx_boundary_parser.h"

#include "my_loglevel.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "sql/log.h"
<<<<<<< HEAD
#include "sql/log_event.h"  // Log_event

#ifndef DBUG_OFF
/* Event parser state names */
static const char *event_parser_state_names[] = {"None", "GTID", "DDL", "DML",
                                                 "Error"};
#endif

/*
  -----------------------------------------
  Transaction_boundary_parser class methods
  -----------------------------------------
*/

/**
   Reset the transaction boundary parser.

   This method initialize the boundary parser state.
*/
void Transaction_boundary_parser::reset() {
  DBUG_TRACE;
  DBUG_PRINT("info", ("transaction boundary parser is changing state "
                      "from '%s' to '%s'",
                      event_parser_state_names[current_parser_state],
                      event_parser_state_names[EVENT_PARSER_NONE]));
  current_parser_state = EVENT_PARSER_NONE;
  last_parser_state = EVENT_PARSER_NONE;
}

/**
   Feed the transaction boundary parser with a Log_event of any type,
   serialized into a char* buffer.

   @param buf            Pointer to the event buffer.
   @param length         The size of the event buffer.
   @param fd_event       The description event of the master which logged
                         the event.
   @param throw_warnings If the function should throw warning messages while
                         updating the boundary parser state.
                         While initializing the Relay_log_info the
                         relay log is scanned backwards and this could
                         generate false warnings. So, in this case, we
                         don't want to throw warnings.

   @return  false if the transaction boundary parser accepted the event.
            true if the transaction boundary parser didn't accepted the event.
*/
bool Transaction_boundary_parser::feed_event(
    const char *buf, size_t length, const Format_description_event *fd_event,
    bool throw_warnings) {
  DBUG_TRACE;
  enum_event_boundary_type event_boundary_type =
      get_event_boundary_type(buf, length, fd_event, throw_warnings);
  return update_state(event_boundary_type, throw_warnings);
}

/**
   Get the boundary type for a given Log_event of any type,
   serialized into a char* buffer, based on event parser logic.

   @param buf               Pointer to the event buffer.
   @param length            The size of the event buffer.
   @param fd_event          The description event of the master which logged
                            the event.
   @param throw_warnings    If the function should throw warnings getting the
                            event boundary type.
                            Please see comments on this at feed_event().

   @return  the transaction boundary type of the event.
*/
Transaction_boundary_parser::enum_event_boundary_type
Transaction_boundary_parser::get_event_boundary_type(
    const char *buf, size_t length, const Format_description_event *fd_event,
    bool throw_warnings) {
  DBUG_TRACE;

  Log_event_type event_type;
  enum_event_boundary_type boundary_type = EVENT_BOUNDARY_TYPE_ERROR;
  uint header_size = fd_event->common_header_len;

  /* Error if the event content is smaller than header size for the format */
  if (length < header_size) goto end;

  event_type =
      (Log_event_type) static_cast<unsigned char>(buf[EVENT_TYPE_OFFSET]);
  DBUG_PRINT("info", ("trx boundary parser was fed with an event of type %s",
                      Log_event::get_type_str(event_type)));

  switch (event_type) {
    case binary_log::GTID_LOG_EVENT:
    case binary_log::ANONYMOUS_GTID_LOG_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_GTID;
      break;

    /*
      There are four types of queries that we have to deal with: BEGIN, COMMIT,
      ROLLBACK and the rest.
    */
    case binary_log::QUERY_EVENT: {
      const char *query = nullptr;
      size_t qlen = 0;
      /* Get the query to let us check for BEGIN/COMMIT/ROLLBACK */
      qlen = Query_log_event::get_query(buf, length, fd_event, &query);
      if (qlen == 0) {
        DBUG_ASSERT(query == nullptr);
        boundary_type = EVENT_BOUNDARY_TYPE_ERROR;
        break;
      }

      /*
        BEGIN is always the begin of a DML transaction.
      */
      if (!strncmp(query, "BEGIN", qlen) ||
          !strncmp(query, STRING_WITH_LEN("XA START")))
        boundary_type = EVENT_BOUNDARY_TYPE_BEGIN_TRX;
      /*
        COMMIT and ROLLBACK are always the end of a transaction.
      */
      else if (!strncmp(query, "COMMIT", qlen) ||
               (!native_strncasecmp(query, STRING_WITH_LEN("ROLLBACK")) &&
                native_strncasecmp(query, STRING_WITH_LEN("ROLLBACK TO "))))
        boundary_type = EVENT_BOUNDARY_TYPE_END_TRX;
      /*
        XA ROLLBACK is always the end of a XA transaction.
      */
      else if (!native_strncasecmp(query, STRING_WITH_LEN("XA ROLLBACK")))
        boundary_type = EVENT_BOUNDARY_TYPE_END_XA_TRX;
      /*
        If the query is not (BEGIN | XA START | COMMIT | [XA] ROLLBACK), it can
        be considered an ordinary statement.
      */
      else
        boundary_type = EVENT_BOUNDARY_TYPE_STATEMENT;
||||||| 91a17cedb1e
#include "sql/log_event.h"  // Log_event

#ifndef DBUG_OFF
/* Event parser state names */
static const char *event_parser_state_names[] = {"None", "GTID", "DDL", "DML",
                                                 "Error"};
#endif

/*
  -----------------------------------------
  Transaction_boundary_parser class methods
  -----------------------------------------
*/

/**
   Reset the transaction boundary parser.

   This method initialize the boundary parser state.
*/
void Transaction_boundary_parser::reset() {
  DBUG_TRACE;
  DBUG_PRINT("info", ("transaction boundary parser is changing state "
                      "from '%s' to '%s'",
                      event_parser_state_names[current_parser_state],
                      event_parser_state_names[EVENT_PARSER_NONE]));
  current_parser_state = EVENT_PARSER_NONE;
  last_parser_state = EVENT_PARSER_NONE;
}

/**
   Feed the transaction boundary parser with a Log_event of any type,
   serialized into a char* buffer.

   @param buf            Pointer to the event buffer.
   @param length         The size of the event buffer.
   @param fd_event       The description event of the master which logged
                         the event.
   @param throw_warnings If the function should throw warning messages while
                         updating the boundary parser state.
                         While initializing the Relay_log_info the
                         relay log is scanned backwards and this could
                         generate false warnings. So, in this case, we
                         don't want to throw warnings.

   @return  false if the transaction boundary parser accepted the event.
            true if the transaction boundary parser didn't accepted the event.
*/
bool Transaction_boundary_parser::feed_event(
    const char *buf, size_t length, const Format_description_event *fd_event,
    bool throw_warnings) {
  DBUG_TRACE;
  enum_event_boundary_type event_boundary_type =
      get_event_boundary_type(buf, length, fd_event, throw_warnings);
  return update_state(event_boundary_type, throw_warnings);
}

/**
   Get the boundary type for a given Log_event of any type,
   serialized into a char* buffer, based on event parser logic.

   @param buf               Pointer to the event buffer.
   @param length            The size of the event buffer.
   @param fd_event          The description event of the master which logged
                            the event.
   @param throw_warnings    If the function should throw warnings getting the
                            event boundary type.
                            Please see comments on this at feed_event().

   @return  the transaction boundary type of the event.
*/
Transaction_boundary_parser::enum_event_boundary_type
Transaction_boundary_parser::get_event_boundary_type(
    const char *buf, size_t length, const Format_description_event *fd_event,
    bool throw_warnings) {
  DBUG_TRACE;

  Log_event_type event_type;
  enum_event_boundary_type boundary_type = EVENT_BOUNDARY_TYPE_ERROR;
  uint header_size = fd_event->common_header_len;

  /* Error if the event content is smaller than header size for the format */
  if (length < header_size) goto end;

  event_type = (Log_event_type)buf[EVENT_TYPE_OFFSET];
  DBUG_PRINT("info", ("trx boundary parser was fed with an event of type %s",
                      Log_event::get_type_str(event_type)));

  switch (event_type) {
    case binary_log::GTID_LOG_EVENT:
    case binary_log::ANONYMOUS_GTID_LOG_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_GTID;
      break;

    /*
      There are four types of queries that we have to deal with: BEGIN, COMMIT,
      ROLLBACK and the rest.
    */
    case binary_log::QUERY_EVENT: {
      const char *query = nullptr;
      size_t qlen = 0;
      /* Get the query to let us check for BEGIN/COMMIT/ROLLBACK */
      qlen = Query_log_event::get_query(buf, length, fd_event, &query);
      if (qlen == 0) {
        DBUG_ASSERT(query == nullptr);
        boundary_type = EVENT_BOUNDARY_TYPE_ERROR;
        break;
      }

      /*
        BEGIN is always the begin of a DML transaction.
      */
      if (!strncmp(query, "BEGIN", qlen) ||
          !strncmp(query, STRING_WITH_LEN("XA START")))
        boundary_type = EVENT_BOUNDARY_TYPE_BEGIN_TRX;
      /*
        COMMIT and ROLLBACK are always the end of a transaction.
      */
      else if (!strncmp(query, "COMMIT", qlen) ||
               (!native_strncasecmp(query, STRING_WITH_LEN("ROLLBACK")) &&
                native_strncasecmp(query, STRING_WITH_LEN("ROLLBACK TO "))))
        boundary_type = EVENT_BOUNDARY_TYPE_END_TRX;
      /*
        XA ROLLBACK is always the end of a XA transaction.
      */
      else if (!native_strncasecmp(query, STRING_WITH_LEN("XA ROLLBACK")))
        boundary_type = EVENT_BOUNDARY_TYPE_END_XA_TRX;
      /*
        If the query is not (BEGIN | XA START | COMMIT | [XA] ROLLBACK), it can
        be considered an ordinary statement.
      */
      else
        boundary_type = EVENT_BOUNDARY_TYPE_STATEMENT;
=======
>>>>>>> mysql-8.0.19

void Replication_transaction_boundary_parser::log_server_warning(
    int error, const char *message) {
  int server_log_error = 0;
  switch (error) {
    case ER_TRX_BOUND_UNSUPPORTED_UNIGNORABLE_EVENT_IN_STREAM: {
      server_log_error = ER_RPL_UNSUPPORTED_UNIGNORABLE_EVENT_IN_STREAM;
      break;
    }
    case ER_TRX_BOUND_GTID_LOG_EVENT_IN_STREAM: {
      server_log_error = ER_RPL_GTID_LOG_EVENT_IN_STREAM;
      break;
    }
    case ER_TRX_BOUND_UNEXPECTED_BEGIN_IN_STREAM: {
      server_log_error = ER_RPL_UNEXPECTED_BEGIN_IN_STREAM;
      break;
    }
    case ER_TRX_BOUND_UNEXPECTED_COMMIT_ROLLBACK_OR_XID_LOG_EVENT_IN_STREAM: {
      server_log_error =
          ER_RPL_UNEXPECTED_COMMIT_ROLLBACK_OR_XID_LOG_EVENT_IN_STREAM;
      break;
<<<<<<< HEAD

    /*
      Rotate, Format_description and Heartbeat should be ignored.
      Also, any other kind of event not listed in the "cases" above
      will be ignored.
    */
    case binary_log::ROTATE_EVENT:
    case binary_log::FORMAT_DESCRIPTION_EVENT:
    case binary_log::HEARTBEAT_LOG_EVENT:
    case binary_log::PREVIOUS_GTIDS_LOG_EVENT:
    case binary_log::STOP_EVENT:
    case binary_log::SLAVE_EVENT:
    case binary_log::DELETE_FILE_EVENT:
    case binary_log::TRANSACTION_CONTEXT_EVENT:
    case binary_log::START_5_7_ENCRYPTION_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_IGNORE;
||||||| 91a17cedb1e

    /*
      Rotate, Format_description and Heartbeat should be ignored.
      Also, any other kind of event not listed in the "cases" above
      will be ignored.
    */
    case binary_log::ROTATE_EVENT:
    case binary_log::FORMAT_DESCRIPTION_EVENT:
    case binary_log::HEARTBEAT_LOG_EVENT:
    case binary_log::PREVIOUS_GTIDS_LOG_EVENT:
    case binary_log::STOP_EVENT:
    case binary_log::SLAVE_EVENT:
    case binary_log::DELETE_FILE_EVENT:
    case binary_log::TRANSACTION_CONTEXT_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_IGNORE;
=======
    }
    case ER_TRX_BOUND_UNEXPECTED_XA_ROLLBACK_IN_STREAM: {
      server_log_error = ER_RPL_UNEXPECTED_XA_ROLLBACK_IN_STREAM;
>>>>>>> mysql-8.0.19
      break;
    }
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
      return;
  }

  if (message != nullptr)
    LogErr(WARNING_LEVEL, server_log_error, message);
  else
    LogErr(WARNING_LEVEL, server_log_error);
}
