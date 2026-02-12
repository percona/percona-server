/******************************************************
CPU topology helpers for InnoDB OS layer.

Logical CPUs are obtained from OS APIs (sysconf / GetSystemInfo / sysctl).
Physical cores and topology are detected on Linux via sysfs and exposed
through a shared os_cpu_topology_t snapshot that can be reused by
buffer pool tuning and other components.
*******************************************************/

#include "univ.i"
#include "os0cpu.h"
#include "ut0ut.h"

#ifdef _WIN32
# include <windows.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
# include <sys/types.h>
# include <sys/sysctl.h>
#else
# include <unistd.h>
#endif

#ifdef UNIV_LINUX
# include <dirent.h>
# include <errno.h>
# include <limits.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <set>
# include <map>
#endif /* UNIV_LINUX */

#include <vector>
#include <algorithm>

/** Global CPU topology snapshot for InnoDB. */
os_cpu_topology_t os_cpu_topology;

/* Return number of logical CPUs visible to the process.
   Never returns 0 (minimum is 1). */
ulint
os_cpu_get_logical() {
  ulint n = 1;

#ifdef _WIN32
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  if (si.dwNumberOfProcessors > 0) {
    n = (ulint) si.dwNumberOfProcessors;
  }
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
  int mib[2];
  int cores = 0;
  size_t len = sizeof(cores);

  mib[0] = CTL_HW;
  mib[1] = HW_NCPU;

  if (sysctl(mib, 2, &cores, &len, NULL, 0) == 0 && cores > 0) {
    n = (ulint) cores;
  }
#else
  long v = -1;

# ifdef _SC_NPROCESSORS_ONLN
  v = sysconf(_SC_NPROCESSORS_ONLN);
# elif defined(_SC_NPROC_ONLN)
  v = sysconf(_SC_NPROC_ONLN);
# endif

  if (v > 0) {
    n = (ulint) v;
  }
#endif

  if (n == 0) {
    n = 1;
  }

  return n;
}

/** Initialize CPU topology snapshot.
Fills logical_cpus, physical_cores, sockets[] and logical_ids[] fields.
Safe to call multiple times; subsequent calls will return immediately. */
void
os_cpu_topology_init(os_cpu_topology_t *topology) {
  if (topology == nullptr) {
    return;
  }

  if (topology->initialized) {
    return;
  }

  topology->initialized = false;
  topology->logical_cpus = 0;
  topology->physical_cores = 0;
  topology->threads_per_core = 0;
  topology->hyperthreading_on = false;
  topology->sockets.clear();
  topology->logical_ids.clear();

  /* Always populate logical CPU count, even if topology detection fails. */
  topology->logical_cpus = os_cpu_get_logical();

#ifdef UNIV_LINUX
  const char *cpu_sysfs = "/sys/devices/system/cpu";
  DIR *dir = opendir(cpu_sysfs);

  if (dir != nullptr) {
    /* Map socket_id -> (core_id -> core) */
    std::map<int, std::map<int, os_cpu_core_t> > socket_cores;
    struct dirent *entry;

    while ((entry = readdir(dir)) != nullptr) {
      if (strncmp(entry->d_name, "cpu", 3) != 0) {
        continue;
      }

      char *endptr = nullptr;
      long cpu_id_long = strtol(entry->d_name + 3, &endptr, 10);
      if (*endptr != '\0' || cpu_id_long < 0) {
        continue;
      }

      int cpu_id = static_cast<int>(cpu_id_long);

      char path_pkg[PATH_MAX];
      char path_core[PATH_MAX];

      snprintf(path_pkg, sizeof(path_pkg),
               "%s/cpu%ld/topology/physical_package_id",
               cpu_sysfs, cpu_id_long);
      snprintf(path_core, sizeof(path_core),
               "%s/cpu%ld/topology/core_id",
               cpu_sysfs, cpu_id_long);

      FILE *f_pkg = fopen(path_pkg, "r");
      FILE *f_core = fopen(path_core, "r");
      if (f_pkg == nullptr || f_core == nullptr) {
        if (f_pkg != nullptr) {
          fclose(f_pkg);
        }
        if (f_core != nullptr) {
          fclose(f_core);
        }
        continue;
      }

      int socket_id = -1;
      int core_id = -1;

      if (fscanf(f_pkg, "%d", &socket_id) != 1 ||
          fscanf(f_core, "%d", &core_id) != 1) {
        fclose(f_pkg);
        fclose(f_core);
        continue;
      }

      fclose(f_pkg);
      fclose(f_core);

      if (socket_id < 0 || core_id < 0) {
        continue;
      }

      /* Track all logical CPUs we see. */
      topology->logical_ids.push_back(cpu_id);

      /* Group by socket and core. */
      auto &cores_map = socket_cores[socket_id];
      os_cpu_core_t &core_ref = cores_map[core_id];

      if (core_ref.threads.empty()) {
        core_ref.core_id = core_id;
      }

      os_cpu_thread_t thread;
      thread.cpu_id = cpu_id;
      core_ref.threads.push_back(thread);
    }

    closedir(dir);

    /* Build sockets[] vector from socket_cores map. */
    for (auto &s_pair : socket_cores) {
      os_cpu_socket_t socket;
      socket.socket_id = s_pair.first;

      for (auto &c_pair : s_pair.second) {
        os_cpu_core_t core = c_pair.second;

        /* Sort and deduplicate threads by cpu_id. */
        std::sort(core.threads.begin(), core.threads.end(),
                  [](const os_cpu_thread_t &a,
                     const os_cpu_thread_t &b) {
                    return a.cpu_id < b.cpu_id;
                  });
        core.threads.erase(
          std::unique(core.threads.begin(), core.threads.end(),
                      [](const os_cpu_thread_t &a,
                         const os_cpu_thread_t &b) {
                        return a.cpu_id == b.cpu_id;
                      }),
          core.threads.end());

        socket.cores.push_back(core);
      }

      /* Cores are already ordered by core_id due to std::map,
      but sorting here makes it explicit. */
      std::sort(socket.cores.begin(), socket.cores.end(),
                [](const os_cpu_core_t &a,
                   const os_cpu_core_t &b) {
                  return a.core_id < b.core_id;
                });

      topology->sockets.push_back(socket);
    }

    /* Sockets are already ordered by socket_id due to std::map,
    but sorting here makes it explicit. */
    std::sort(topology->sockets.begin(), topology->sockets.end(),
              [](const os_cpu_socket_t &a,
                 const os_cpu_socket_t &b) {
                return a.socket_id < b.socket_id;
              });

    /* Sort and deduplicate flat logical_ids as well. */
    std::sort(topology->logical_ids.begin(), topology->logical_ids.end());
    topology->logical_ids.erase(
      std::unique(topology->logical_ids.begin(),
                  topology->logical_ids.end()),
      topology->logical_ids.end());

    /* Compute logical CPU count from topology if possible. */
    if (!topology->logical_ids.empty()) {
      topology->logical_cpus =
        static_cast<ulint>(topology->logical_ids.size());
    }

    /* Compute physical cores as total number of cores across all sockets. */
    topology->physical_cores = 0;
    for (const auto &socket : topology->sockets) {
      topology->physical_cores +=
        static_cast<ulint>(socket.cores.size());
    }

#ifdef UNIV_DEBUG
    if (topology->physical_cores > 0) {
      ib::info(ER_IB_MSG_CPU_CORES_INFO)
        << "os_cpu_topology_init(): Linux sysfs topology reports "
        << (unsigned long) topology->physical_cores
        << " physical cores on "
        << (unsigned long) topology->sockets.size()
        << " sockets.";
    }
#endif
  }
#endif /* UNIV_LINUX */

  /* Derive threads_per_core and HyperThreading flag when possible. */
  if (topology->physical_cores > 0 &&
      topology->logical_cpus >= topology->physical_cores &&
      topology->logical_cpus % topology->physical_cores == 0) {
    topology->threads_per_core =
      topology->logical_cpus / topology->physical_cores;
    topology->hyperthreading_on = (topology->threads_per_core > 1);
  }

  /* If we still do not know physical_cores (e.g. non-Linux or sysfs
  not available), fall back to heuristic based on logical_cpus. */
  if (topology->physical_cores == 0) {
    ulint logical = topology->logical_cpus;

    if (logical == 0) {
      ib::warn(ER_IB_MSG_CPU_CORES_INFO)
        << "os_cpu_topology_init(): logical CPU count is 0; "
        << "cannot determine physical cores.";
    } else {
      ulint physical = logical;
      if (logical % 2 == 0) {
        physical = logical / 2;
      }
      topology->physical_cores = physical;

      ib::warn(ER_IB_MSG_CPU_CORES_INFO)
        << "os_cpu_topology_init(): using heuristic estimate: logical CPUs="
        << (unsigned long) logical
        << " physical cores (estimated)="
        << (unsigned long) topology->physical_cores;
    }
  }

  topology->initialized = true;
}

/* Public API: return number of physical CPU cores if detectable.
Uses topology snapshot; falls back to heuristic if needed. */
ulint
os_cpu_get_physical() {
  os_cpu_topology_init(&os_cpu_topology);
  return os_cpu_topology.physical_cores;
}

/** Return number of physical cores for which we have topology information.
Returns 0 if the topology could not be detected. */
ulint
os_cpu_get_core_count_with_topology() {
  os_cpu_topology_init(&os_cpu_topology);
  return os_cpu_topology.physical_cores;
}

/** Get core description by linear index across all sockets.
Cores are ordered by (socket_id, core_id).
Returns false if index is out of range or topology is unavailable. */
bool
os_cpu_get_core_by_index(ulint index, os_cpu_core_t *out) {
  os_cpu_topology_init(&os_cpu_topology);

  if (out == nullptr) {
    return false;
  }

  ulint count = 0;
  for (const auto &socket : os_cpu_topology.sockets) {
    for (const auto &core : socket.cores) {
      if (count == index) {
        *out = core;
        return true;
      }
      ++count;
    }
  }

  return false;
}
