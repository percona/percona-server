
// vim:sw=2:ai

/*
 * Copyright (C) 2010 DeNA Co.,Ltd.. All rights reserved.
 * See COPYRIGHT.txt for details.
 */

#ifndef DENA_DATABASE_HPP
#define DENA_DATABASE_HPP

#include <string>
#include <memory>
#include <vector>
#include <stdint.h>

#include "string_buffer.hpp"
#include "string_ref.hpp"
#include "config.hpp"

namespace dena {

struct database_i;
typedef std::auto_ptr<volatile database_i> database_ptr;

struct dbcontext_i;
typedef std::auto_ptr<dbcontext_i> dbcontext_ptr;

struct database_i {
  virtual ~database_i() { }
  virtual dbcontext_ptr create_context(bool for_write) volatile = 0;
  virtual void stop() volatile = 0;
  virtual const config& get_conf() const volatile = 0;
  static database_ptr create(const config& conf);
};

struct prep_stmt {
  typedef std::vector<uint32_t> retfields_type;
 private:
  dbcontext_i *dbctx; /* must be valid while *this is alive */
  size_t table_id;
  size_t idxnum;
  retfields_type retfields;
 public:
  prep_stmt();
  prep_stmt(dbcontext_i *c, size_t tbl, size_t idx, const retfields_type& rf);
  ~prep_stmt();
  prep_stmt(const prep_stmt& x);
  prep_stmt& operator =(const prep_stmt& x);
 public:
  size_t get_table_id() const { return table_id; }
  size_t get_idxnum() const { return idxnum; }
  const retfields_type& get_retfields() const { return retfields; }
};

struct dbcallback_i {
  virtual ~dbcallback_i () { }
  virtual void dbcb_set_prep_stmt(size_t pst_id, const prep_stmt& v) = 0;
  virtual const prep_stmt *dbcb_get_prep_stmt(size_t pst_id) const = 0;
  virtual void dbcb_resp_short(uint32_t code, const char *msg) = 0;
  virtual void dbcb_resp_short_num(uint32_t code, uint32_t value) = 0;
  virtual void dbcb_resp_begin(size_t num_flds) = 0;
  virtual void dbcb_resp_entry(const char *fld, size_t fldlen) = 0;
  virtual void dbcb_resp_end() = 0;
  virtual void dbcb_resp_cancel() = 0;
};

struct cmd_exec_args {
  const prep_stmt *pst;
  string_ref op;
  const string_ref *kvals;
  size_t kvalslen;
  uint32_t limit;
  uint32_t skip;
  string_ref mod_op;
  const string_ref *uvals; /* size must be pst->retfieelds.size() */
  cmd_exec_args() : pst(0), kvals(0), kvalslen(0), limit(0), skip(0) { }
};

struct dbcontext_i {
  virtual ~dbcontext_i() { }
  virtual void init_thread(const void *stack_bottom,
    volatile int& shutdown_flag) = 0;
  virtual void term_thread() = 0;
  virtual bool check_alive() = 0;
  virtual void lock_tables_if() = 0;
  virtual void unlock_tables_if() = 0;
  virtual bool get_commit_error() = 0;
  virtual void clear_error() = 0;
  virtual void close_tables_if() = 0;
  virtual void table_addref(size_t tbl_id) = 0; /* TODO: hide */
  virtual void table_release(size_t tbl_id) = 0; /* TODO: hide */
  virtual void cmd_open_index(dbcallback_i& cb, size_t pst_id, const char *dbn,
    const char *tbl, const char *idx, const char *retflds) = 0;
  virtual void cmd_exec_on_index(dbcallback_i& cb, const cmd_exec_args& args)
    = 0;
  virtual void set_statistics(size_t num_conns, size_t num_active) = 0;
};

};

#endif

