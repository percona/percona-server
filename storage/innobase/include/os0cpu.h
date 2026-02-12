/******************************************************
CPU topology helpers for InnoDB OS layer
*******************************************************/

#ifndef OS0CPU_H
#define OS0CPU_H

#include "univ.i"
#include "ut0ut.h"

#include <vector>

/* Return number of logical CPUs visible to the process.
   Returns >= 1 (never 0). */
ulint os_cpu_get_logical();

/* Return number of physical cores if detectable.
   Returns 0 if cannot be determined reliably. */
ulint os_cpu_get_physical();

/******************************************************
CPU topology description for NUMA‑aware scheduling
*******************************************************/

/** Description of a logical CPU (vCPU). */
struct os_cpu_thread_t {
  /** Logical CPU id (as used by sched_setaffinity/pthread_setaffinity_np). */
  int cpu_id;
};

/** Description of a physical core and all its logical threads. */
struct os_cpu_core_t {
  /** Core id within the physical package (core_id from sysfs). */
  int core_id;
  /** List of logical threads (vCPUs) on this core. */
  std::vector<os_cpu_thread_t> threads;
};

/** Description of a NUMA socket (physical package). */
struct os_cpu_socket_t {
  /** Physical package id (NUMA socket, physical_package_id from sysfs). */
  int socket_id;
  /** List of physical cores on this socket. */
  std::vector<os_cpu_core_t> cores;
};

/** CPU topology snapshot for the current process. */
struct os_cpu_topology_t {
  /** True if this structure has been initialized. */
  bool initialized;
  /** Number of logical CPUs visible to the process. */
  ulint logical_cpus;
  /** Number of physical cores (may be 0 if not detectable). */
  ulint physical_cores;
  /** Threads per core (0 if unknown or not uniform). */
  ulint threads_per_core;
  /** True if HyperThreading (or similar SMT) appears to be enabled. */
  bool hyperthreading_on;
  /** Tree view: sockets -> cores -> threads. */
  std::vector<os_cpu_socket_t> sockets;
  /** Flat view: all logical CPU ids seen in the topology. */
  std::vector<int> logical_ids;
};

/** Global CPU topology snapshot for InnoDB. */
extern os_cpu_topology_t os_cpu_topology;

/** Initialize CPU topology snapshot.
Fills logical_cpus, physical_cores, sockets[] and logical_ids[] fields.
Safe to call multiple times; subsequent calls will return immediately. */
void os_cpu_topology_init(os_cpu_topology_t *topology);

/** Return number of physical cores for which we have topology information.
Returns 0 if the topology could not be detected. */
ulint os_cpu_get_core_count_with_topology();

/** Get core description by linear index across all sockets.
Cores are ordered by (socket_id, core_id).
Returns false if index is out of range or topology is unavailable. */
bool os_cpu_get_core_by_index(ulint index, os_cpu_core_t *out);

#endif /* OS0CPU_H */
