#ifndef CPU_BINDING_H
#define CPU_BINDING_H

#include <pthread.h>
#include <string>

/** Thread role for CPU binding configuration. */
enum class ThreadRole
  {
  MAIN_THREAD,
  REPLICA_IO,
  REPLICA_APPLIER,
  REPLICA_WORKER,
  CLIENT_THREAD,
  BUFPOOL_LRU_T,
  };

enum class BindingMode
  {
  ANY,
  SPARSE,
  FIXED
  };

/** Parsed CPU binding for a single thread role. */
struct CpuBindingEntry
  {
  BindingMode mode;
  int socket;                                     // valid if socket_mode == FIXED

  CpuBindingEntry() : mode(BindingMode::ANY), socket(-1) {}
  };

bool parse_cpu_binding_string(const std::string &raw,
CpuBindingEntry &out);

// Register raw option value for given role.
void cpu_binding_register_option(ThreadRole role, const char *raw_value);

/// Apply previously registered binding for given role to a thread.
/// planned_threads is the total number of threads of this role configured
/// in the server (for example replica workers, client threads, etc.).
void cpu_binding_apply_for_role(ThreadRole role,
pthread_t thread,
unsigned long planned_threads);

int cpu_binding_get_role_socket(ThreadRole role);
int cpu_binding_get_socket_cores(int socket_id);
#endif                                            /* CPU_BINDING_H */
