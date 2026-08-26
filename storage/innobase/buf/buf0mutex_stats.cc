/** @file buf/buf0mutex_stats.cc
 Per-call-site instrumentation of buf_block_t/buf_page_t mutex acquisitions.
 See buf0mutex_stats.h for the rationale (PS-11120). */

#include "buf0mutex_stats.h"

#ifdef UNIV_BUF_MUTEX_STATS

#include <algorithm>
#include <bit>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <thread>

#include "fil0fil.h"
#include "my_io.h"
#include "my_rdtsc.h"
#include "os0thread-create.h"
#include "srv0srv.h"
#include "srv0start.h"
#include "univ.i"
#include "ut0log.h"

Buf_mutex_stats_registry buf_mutex_stats;
os_event_t srv_buf_mutex_stats_event;

size_t Buf_mutex_stats_registry::register_site(const char *label) {
  const size_t idx = m_next_idx.fetch_add(1, std::memory_order_relaxed);
  ut_a(idx < MAX_SITES);
  m_sites[idx].label.store(label, std::memory_order_release);
  return idx;
}

void buf_mutex_stats_record(size_t site_idx, uint64_t cycles) {
  Buf_mutex_site_stats &s = buf_mutex_stats.site(site_idx);

  s.count.fetch_add(1, std::memory_order_relaxed);
  s.total_cycles.fetch_add(cycles, std::memory_order_relaxed);

  uint64_t cur_max = s.max_cycles.load(std::memory_order_relaxed);
  while (cycles > cur_max && !s.max_cycles.compare_exchange_weak(
                                 cur_max, cycles, std::memory_order_relaxed)) {
  }

  const size_t bucket =
      cycles == 0
          ? 0
          : std::min<size_t>(static_cast<size_t>(std::bit_width(cycles) - 1),
                             Buf_mutex_site_stats::N_BUCKETS - 1);
  s.buckets[bucket].fetch_add(1, std::memory_order_relaxed);
}

namespace {

/** How often the registry is snapshotted to the CSV file. Deliberately
short: the hot-page repro from PS-11120 completes in 1.5-3s wall time. */
constexpr std::chrono::milliseconds DUMP_INTERVAL{150};

/** @return true if InnoDB has started shutting down. */
inline bool SHUTTING_DOWN() {
  return srv_shutdown_state.load(std::memory_order_relaxed) >=
         SRV_SHUTDOWN_CLEANUP;
}

/** @return path to the CSV file, under the same directory buf0dump uses. */
void generate_csv_path(char *path, size_t path_size) {
  const char *dir = strcmp(srv_data_home, "") == 0
                        ? static_cast<const char *>(MySQL_datadir_path)
                        : srv_data_home;
  snprintf(path, path_size, "%s%c%s", dir, OS_PATH_SEPARATOR,
           "buf_mutex_contention_stats.csv");
}

/** Self-calibrates TSC cycles against wall-clock time over a short window.
my_timer_info().cycles.frequency is NOT usable for this: MySQL hardcodes it
to 1e9 regardless of the CPU's actual TSC rate (see my_timer_init() in
mysys/my_rdtsc.cc), so it would silently misreport cycle counts as
nanoseconds. Cycles (not std::chrono) are still used at each call site
because reading the TSC is ~10 cycles versus ~15-25ns for a
steady_clock::now() call, and we don't want the instrumentation itself to
dominate the hot path we're trying to measure.
@return nanoseconds per TSC cycle on this host */
double calibrate_ns_per_cycle() {
  constexpr auto CALIBRATION_WINDOW = std::chrono::milliseconds{100};

  const uint64_t c0 = my_timer_cycles();
  const auto t0 = std::chrono::steady_clock::now();

  std::this_thread::sleep_for(CALIBRATION_WINDOW);

  const uint64_t c1 = my_timer_cycles();
  const auto t1 = std::chrono::steady_clock::now();

  const uint64_t cycles = c1 - c0;
  const double elapsed_ns = static_cast<double>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());

  return cycles > 0 ? elapsed_ns / static_cast<double>(cycles) : 0.0;
}

/** Appends one CSV row per registered site to `f`, then flushes.
@param[in]  f             file to append to
@param[in]  ns_per_cycle  conversion factor from calibrate_ns_per_cycle() */
void dump_snapshot(FILE *f, double ns_per_cycle) {
  const auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();

  for (size_t i = 0; i < Buf_mutex_stats_registry::MAX_SITES; ++i) {
    Buf_mutex_site_stats &s = buf_mutex_stats.site(i);

    const char *label = s.label.load(std::memory_order_acquire);
    if (label == nullptr) {
      /* Slot not registered yet (index reserved but label not published,
      or simply unused so far); skip, don't stop, since publication order
      across concurrently-registering threads is not guaranteed. */
      continue;
    }

    const uint64_t count = s.count.load(std::memory_order_relaxed);
    const uint64_t total_cycles =
        s.total_cycles.load(std::memory_order_relaxed);
    const uint64_t max_cycles = s.max_cycles.load(std::memory_order_relaxed);

    fprintf(f, "%lld,%s,%" PRIu64 ",%.0f,%.0f", (long long)now_us, label, count,
            static_cast<double>(total_cycles) * ns_per_cycle,
            static_cast<double>(max_cycles) * ns_per_cycle);

    for (size_t b = 0; b < Buf_mutex_site_stats::N_BUCKETS; ++b) {
      fprintf(f, ",%" PRIu64, s.buckets[b].load(std::memory_order_relaxed));
    }

    fprintf(f, "\n");
  }

  fflush(f);
}

}  // namespace

void buf_mutex_stats_thread() {
  char path[FN_REFLEN];
  generate_csv_path(path, sizeof(path));

  FILE *f = fopen(path, "a");
  if (f == nullptr) {
    ib::error() << "buf_mutex_stats: failed to open '" << path
                << "' for writing, errno=" << errno;
    return;
  }

  fprintf(f, "ts_us,site,count,total_wait_ns,max_wait_ns");
  for (size_t b = 0; b < Buf_mutex_site_stats::N_BUCKETS; ++b) {
    fprintf(f, ",bucket_ge_2p%zu_cycles", b);
  }
  fprintf(f, "\n");
  fflush(f);

  const double ns_per_cycle = calibrate_ns_per_cycle();

  while (!SHUTTING_DOWN()) {
    os_event_wait_time(srv_buf_mutex_stats_event, DUMP_INTERVAL);
    os_event_reset(srv_buf_mutex_stats_event);

    dump_snapshot(f, ns_per_cycle);
  }

  /* Final snapshot so the last window before shutdown is not lost. */
  dump_snapshot(f, ns_per_cycle);

  fclose(f);
}

#endif /* UNIV_BUF_MUTEX_STATS */
