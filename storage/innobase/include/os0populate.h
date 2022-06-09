#ifndef os0populate_h
#define os0populate_h

/* Do not include univ.i because univ.i indirectly includes this. */

#ifndef _WIN32
#include <sys/mman.h>
#endif

/* Linux's MAP_POPULATE */
#if defined(MAP_POPULATE)
#define OS_MAP_POPULATE MAP_POPULATE
#else
#define OS_MAP_POPULATE 0
#endif

void prefault_if_not_map_populate(void *ptr [[maybe_unused]],
                                  size_t n_bytes [[maybe_unused]]);

#endif
