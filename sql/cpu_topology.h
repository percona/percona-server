/******************************************************
CPU topology helpers
*******************************************************/

#ifndef CPU_TOPOLOGY_H
#define CPU_TOPOLOGY_H

#include <vector>

/* Return number of logical CPUs visible to the process.
   Returns >= 1 (never 0). */
unsigned long sql_cpu_get_logical();

/* Return number of physical cores if detectable.
   Returns 0 if cannot be determined reliably. */
unsigned long sql_cpu_get_physical();

/******************************************************
CPU topology description for NUMA‑aware scheduling
*******************************************************/

/** Description of a logical CPU (vCPU). */
struct Sql_cpu_thread {
  /** Logical CPU id (as used by sched_setaffinity/pthread_setaffinity_np). */
  int cpu_id;
};

/** Description of a physical core and all its logical threads. */
struct Sql_cpu_core {
  /** Core id within the physical package (core_id from sysfs). */
  int core_id;
  /** List of logical threads (vCPUs) on this core. */
  std::vector<Sql_cpu_thread> threads;
};

/** Description of a NUMA socket (physical package). */
struct Sql_cpu_socket {
  /** Physical package id (NUMA socket, physical_package_id from sysfs). */
  int socket_id;
  /** List of physical cores on this socket. */
  std::vector<Sql_cpu_core> cores;
};

/** CPU topology snapshot for the current process. */
struct Sql_cpu_topology {
  /** True if this structure has been initialized. */
  bool initialized;
  /** Number of logical CPUs visible to the process. */
  unsigned long logical_cpus;
  /** Number of physical cores (may be 0 if not detectable). */
  unsigned long physical_cores;
  /** Threads per core (0 if unknown or not uniform). */
  unsigned long threads_per_core;
  /** True if HyperThreading (or similar SMT) appears to be enabled. */
  bool hyperthreading_on;
  /** Tree view: sockets -> cores -> threads. */
  std::vector<Sql_cpu_socket> sockets;
  /** Flat view: all logical CPU ids seen in the topology. */
  std::vector<int> logical_ids;
};

/** Global CPU topology snapshot for InnoDB. */
extern Sql_cpu_topology sql_cpu_topology;

/** Initialize CPU topology snapshot.
Fills logical_cpus, physical_cores, sockets[] and logical_ids[] fields.
Safe to call multiple times; subsequent calls will return immediately. */
void sql_cpu_topology_init(Sql_cpu_topology *topology);

/** Return number of physical cores for which we have topology information.
Returns 0 if the topology could not be detected. */
unsigned long sql_cpu_get_core_count_with_topology();

/** Get core description by linear index across all sockets.
Cores are ordered by (socket_id, core_id).
Returns false if index is out of range or topology is unavailable. */
bool sql_cpu_get_core_by_index(unsigned long index, Sql_cpu_core *out);

#endif /* CPU_TOPOLOGY_H */
