
// vim:sw=2:ai

/*
 * Copyright (C) 2010 DeNA Co.,Ltd.. All rights reserved.
 * See COPYRIGHT.txt for details.
 */

#ifndef DENA_HSTCPSVR_HPP
#define DENA_HSTCPSVR_HPP

#include <memory>
#include <string>
#include <map>

#include "mutex.hpp"
#include "auto_file.hpp"
#include "database.hpp"
#include "config.hpp"
#include "socket.hpp"

namespace dena {

struct hstcpsvr_shared_c {
  config conf;
  long num_threads;
  long nb_conn_per_thread;
  bool for_write_flag;
  bool require_auth;
  std::string plain_secret;
  int readsize;
  socket_args sockargs;
  auto_file listen_fd;
  database_ptr dbptr;
  volatile unsigned int *thread_num_conns; /* 0 .. num_threads-1 */
  hstcpsvr_shared_c() : num_threads(0), nb_conn_per_thread(100),
    for_write_flag(false), require_auth(false), readsize(0),
    thread_num_conns(0) { }
};

struct hstcpsvr_shared_v : private noncopyable {
  int shutdown;
  long threads_started;
  pthread_cond_t threads_started_cond;
  mutex v_mutex;
  hstcpsvr_shared_v() : shutdown(0), threads_started(0)
  {
    pthread_cond_init(&threads_started_cond, NULL);
  }
  ~hstcpsvr_shared_v()
  {
    pthread_cond_destroy(&threads_started_cond);
  }
};

struct hstcpsvr_i;
typedef std::auto_ptr<hstcpsvr_i> hstcpsvr_ptr;

struct hstcpsvr_i {
  virtual ~hstcpsvr_i() { }
  virtual std::string start_listen() = 0;
  static hstcpsvr_ptr create(const config& conf);
};

};

#endif

