#include "os0populate.h"
#include "univ.i"

#if defined(UNIV_LINUX) && defined(_GNU_SOURCE)
#include <string.h>      /* strverscmp() */
#include <sys/utsname.h> /* uname() */
#endif

#if OS_MAP_POPULATE

/** Retrieve and compare operating system release.
@return	TRUE if the OS release is equal to, or later than release. */
static bool os_compare_release(const char *release [[maybe_unused]]) {
#if defined(UNIV_LINUX) && defined(_GNU_SOURCE)
  struct utsname name;
  return uname(&name) == 0 && strverscmp(name.release, release) >= 0;
#else
  return 0;
#endif
}

#endif

void prefault_if_not_map_populate(void *ptr [[maybe_unused]],
                                  size_t n_bytes [[maybe_unused]]) {
#if OS_MAP_POPULATE
  /* MAP_POPULATE is only supported for private mappings
     since Linux 2.6.23. */
  if (os_compare_release("2.6.23")) return;

  ib::warn() << "mmap(MAP_POPULATE) is not supported for private mappings. "
                "Forcing preallocation by faulting in pages.";
#endif

  /* Initialize the entire buffer to force the allocation of physical memory
   * page frames. */
  memset(ptr, '\0', n_bytes);
}
