/*
  Copyright (c) 2024, Oracle and/or its affiliates.
*/
#ifndef CONNECTION_CONTROL_FAILED_ATTEMPTS_LIST_IMP_H
#define CONNECTION_CONTROL_FAILED_ATTEMPTS_LIST_IMP_H

#include <map>
#include <shared_mutex>
#include "connection_control.h"
#include "connection_control_pfs_table.h"

namespace connection_control {
class Failed_attempts_list_imp : public Connection_control_alloc {
 public:
  void failed_attempts_define(const char *userhost);
  bool failed_attempts_undefine(const char *userhost);

  /**
    Fetch a copy of the queue data to return to a PFS table
    @retval the data to put in the PFS table
  */
  Connection_control_pfs_table_data *copy_pfs_table_data();
  unsigned long long get_failed_attempts_list_count();
  unsigned long long get_failed_attempts_count(const char *userhost);
  void reset();

 private:
  //* A case insensitive comparator using the C library */
  struct ciLessLibC {
    bool operator()(const std::string &lhs, const std::string &rhs) const {
#if defined _WIN32
      return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
#else
      return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
#endif
    }
  };
  std::map<std::string, PSI_ulong, ciLessLibC> failed_attempts_map;
  std::mutex LOCK_failed_attempts_list;
  std::shared_mutex LOCK_shared_failed_attempts_list;
};
}  // namespace connection_control
extern connection_control::Failed_attempts_list_imp g_failed_attempts_list;

#endif /* CONNECTION_CONTROL_FAILED_ATTEMPTS_LIST_IMP_H */
