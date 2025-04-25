/*
  Copyright (c) 2024, Oracle and/or its affiliates.
*/

#ifndef CONNECTION_CONTROL_PFS_TABLE_H
#define CONNECTION_CONTROL_PFS_TABLE_H

#include <mysql/components/services/pfs_plugin_table_service.h>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include "connection_control_memory.h"

namespace connection_control {
bool register_pfs_table();
bool unregister_pfs_table();

template <typename T>
class CustomAllocator : public Connection_control_alloc {
 public:
  using value_type = T;

  CustomAllocator() = default;

  template <typename U>
  explicit CustomAllocator(const CustomAllocator<U> &) {}

  // Allocate memory
  T *allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
      throw std::bad_alloc();
    }
    // Use ::operator new with proper alignment for T
    T *temp = static_cast<T *>(operator new(n * sizeof(T)));
    if (temp == nullptr) throw std::bad_alloc();
    return temp;
  }

  // Deallocate memory
  void deallocate(T *ptr, std::size_t) {
    // Use ::operator delete with alignment
    operator delete(ptr);
  }
};

// Stores row data for
// performance_schema.connection_control_failed_login_attempts table
class Connection_control_pfs_table_data_row {
 public:
  // Constructor taking parameters
  Connection_control_pfs_table_data_row(const std::string &userhost,
                                        const PSI_ulong &failed_attempts);
  std::string m_userhost;
  PSI_ulong m_failed_attempts;
};

typedef std::vector<Connection_control_pfs_table_data_row,
                    CustomAllocator<Connection_control_pfs_table_data_row>>
    Connection_control_pfs_table_data;

}  // namespace connection_control

#endif /* CONNECTION_CONTROL_PFS_TABLE_H */
