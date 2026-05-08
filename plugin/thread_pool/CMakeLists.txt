# Copyright (c) 2011, 2026, Oracle and/or its affiliates.

CHECK_FUNCTION_EXISTS(epoll_wait HAVE_EPOLL_WAIT)

IF(HAVE_EPOLL_WAIT)
  SET(TP_METHOD src/epoll.cc)
ELSE()
  SET(TP_METHOD src/poll.cc)
ENDIF()

MYSQL_ADD_PLUGIN(thread_pool
  src/thread_pool.cc
  src/thread_pool_plugin.cc
  src/thread_pool_tables.cc
  src/table_thread_state.cc
  src/table_thread_group_state.cc
  src/table_thread_group_stats.cc
  src/table_connections.cc
  src/option_usage.cc
  ${TP_METHOD} MODULE_ONLY
  LINK_LIBRARIES extra::rapidjson library_mysys)

ADD_DEFINITIONS(-DMYSQL_SERVER)
ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="thread_pool")

