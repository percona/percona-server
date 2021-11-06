/*****************************************************************************

Copyright (c) 1995, 2021, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file os/os0proc.cc
 The interface to the operating system
 process control primitives

 Created 9/30/1995 Heikki Tuuri
 *******************************************************/

#include "my_config.h"

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include "ha_prototypes.h"
#include "os0proc.h"
#include "srv0srv.h"
#include "ut0byte.h"
#include "ut0mem.h"

/* Linux release version */
#if defined(UNIV_LINUX) && defined(_GNU_SOURCE)
#include <string.h>      /* strverscmp() */
#include <sys/utsname.h> /* uname() */
#endif

/* Linux's MAP_POPULATE */
#if defined(MAP_POPULATE)
#define OS_MAP_POPULATE MAP_POPULATE
#else
#define OS_MAP_POPULATE 0
#endif

/** The total amount of memory currently allocated from the operating
system with os_mem_alloc_large(). */
std::atomic<ulint> os_total_large_mem_allocated{0};

/** Whether to use large pages in the buffer pool */
bool os_use_large_pages;

/** Large page size. This may be a boot-time option on some platforms */
ulint os_large_page_size;

/** Retrieve and compare operating system release.
@return	TRUE if the OS release is equal to, or later than release. */
bool os_compare_release(const char *release /*!< in: OS release */
                            MY_ATTRIBUTE((unused))) {
#if defined(UNIV_LINUX) && defined(_GNU_SOURCE)
  struct utsname name;
  return uname(&name) == 0 && strverscmp(name.release, release) >= 0;
#else
  return 0;
#endif
}

/** Converts the current process id to a number.
@return process id as a number */
ulint os_proc_get_number(void) {
#ifdef _WIN32
  return (static_cast<ulint>(GetCurrentProcessId()));
#else
  return (static_cast<ulint>(getpid()));
#endif
}
// TODOLUIS: implement the populate in the new config
//<<<<<<< ours
//
///** Allocates large pages memory.
//@param[in,out]	n	Number of bytes to allocate
//@return allocated memory */
//void *os_mem_alloc_large(ulint *n, bool populate) {
//  void *ptr;
//  ulint size;
//#if defined HAVE_LINUX_LARGE_PAGES && defined UNIV_LINUX
//  int shmid;
//  struct shmid_ds buf;
//
//  if (!os_use_large_pages || !os_large_page_size) {
//    goto skip;
//  }
//
//  /* Align block size to os_large_page_size */
//  ut_ad(ut_is_2pow(os_large_page_size));
//  size = ut_2pow_round(*n + (os_large_page_size - 1), os_large_page_size);
//
//  shmid = shmget(IPC_PRIVATE, (size_t)size, SHM_HUGETLB | SHM_R | SHM_W);
//  if (shmid < 0) {
//    ib::warn(ER_IB_MSG_852)
//        << "Failed to allocate " << size << " bytes. errno " << errno;
//    ptr = nullptr;
//  } else {
//    ptr = shmat(shmid, nullptr, 0);
//    if (ptr == (void *)-1) {
//      ib::warn(ER_IB_MSG_853) << "Failed to attach shared memory segment,"
//                                 " errno "
//                              << errno;
//      ptr = nullptr;
//    }
//
//    /* Remove the shared memory segment so that it will be
//    automatically freed after memory is detached or
//    process exits */
//    shmctl(shmid, IPC_RMID, &buf);
//  }
//
//  if (ptr) {
//    *n = size;
//    os_total_large_mem_allocated.fetch_add(size);
//
//    UNIV_MEM_ALLOC(ptr, size);
//    return (ptr);
//  }
//
//  ib::warn(ER_IB_MSG_854) << "Using conventional memory pool";
//skip:
//#endif /* HAVE_LINUX_LARGE_PAGES && UNIV_LINUX */
//
//#ifdef _WIN32
//  SYSTEM_INFO system_info;
//  GetSystemInfo(&system_info);
//
//  /* Align block size to system page size */
//  ut_ad(ut_is_2pow(system_info.dwPageSize));
//  /* system_info.dwPageSize is only 32-bit. Casting to ulint is required
//  on 64-bit Windows. */
//  size = *n = ut_2pow_round(*n + (system_info.dwPageSize - 1),
//                            (ulint)system_info.dwPageSize);
//  ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
//  if (!ptr) {
//    ib::info(ER_IB_MSG_855) << "VirtualAlloc(" << size
//                            << " bytes) failed;"
//                               " Windows error "
//                            << GetLastError();
//  } else {
//    os_total_large_mem_allocated.fetch_add(size);
//    UNIV_MEM_ALLOC(ptr, size);
//  }
//#else
//  size = getpagesize();
//  /* Align block size to system page size */
//  ut_ad(ut_is_2pow(size));
//  size = *n = ut_2pow_round(*n + (size - 1), size);
//  ptr =
//      mmap(nullptr, size, PROT_READ | PROT_WRITE,
//           MAP_PRIVATE | OS_MAP_ANON | (populate ? OS_MAP_POPULATE : 0), -1, 0);
//  if (UNIV_UNLIKELY(ptr == (void *)-1)) {
//    ib::error(ER_IB_MSG_856) << "mmap(" << size
//                             << " bytes) failed;"
//                                " errno "
//                             << errno;
//    return nullptr;
//  } else {
//    os_total_large_mem_allocated.fetch_add(size);
//    UNIV_MEM_ALLOC(ptr, size);
//  }
//#endif
//
//#if OS_MAP_ANON && OS_MAP_POPULATE
//  /* MAP_POPULATE is only supported for private mappings
//  since Linux 2.6.23. */
//  populate = populate && !os_compare_release("2.6.23");
//
//  if (populate) {
//    ib::warn() << "mmap(MAP_POPULATE) is not supported for private mappings. "
//                  "Forcing preallocation by faulting in pages.";
//  }
//#endif
//
//  /* Initialize the entire buffer to force the allocation
//  of physical memory page frames. */
//  if (populate) {
//    memset(ptr, '\0', size);
//  }
//
//  return (ptr);
//}
//||||||| base
//
///** Allocates large pages memory.
//@param[in,out]	n	Number of bytes to allocate
//@return allocated memory */
//void *os_mem_alloc_large(ulint *n) {
//  void *ptr;
//  ulint size;
//#if defined HAVE_LINUX_LARGE_PAGES && defined UNIV_LINUX
//  int shmid;
//  struct shmid_ds buf;
//
//  if (!os_use_large_pages || !os_large_page_size) {
//    goto skip;
//  }
//
//  /* Align block size to os_large_page_size */
//  ut_ad(ut_is_2pow(os_large_page_size));
//  size = ut_2pow_round(*n + (os_large_page_size - 1), os_large_page_size);
//
//  shmid = shmget(IPC_PRIVATE, (size_t)size, SHM_HUGETLB | SHM_R | SHM_W);
//  if (shmid < 0) {
//    ib::warn(ER_IB_MSG_852)
//        << "Failed to allocate " << size << " bytes. errno " << errno;
//    ptr = nullptr;
//  } else {
//    ptr = shmat(shmid, nullptr, 0);
//    if (ptr == (void *)-1) {
//      ib::warn(ER_IB_MSG_853) << "Failed to attach shared memory segment,"
//                                 " errno "
//                              << errno;
//      ptr = nullptr;
//    }
//
//    /* Remove the shared memory segment so that it will be
//    automatically freed after memory is detached or
//    process exits */
//    shmctl(shmid, IPC_RMID, &buf);
//  }
//
//  if (ptr) {
//    *n = size;
//    os_total_large_mem_allocated.fetch_add(size);
//
//    UNIV_MEM_ALLOC(ptr, size);
//    return (ptr);
//  }
//
//  ib::warn(ER_IB_MSG_854) << "Using conventional memory pool";
//skip:
//#endif /* HAVE_LINUX_LARGE_PAGES && UNIV_LINUX */
//
//#ifdef _WIN32
//  SYSTEM_INFO system_info;
//  GetSystemInfo(&system_info);
//
//  /* Align block size to system page size */
//  ut_ad(ut_is_2pow(system_info.dwPageSize));
//  /* system_info.dwPageSize is only 32-bit. Casting to ulint is required
//  on 64-bit Windows. */
//  size = *n = ut_2pow_round(*n + (system_info.dwPageSize - 1),
//                            (ulint)system_info.dwPageSize);
//  ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
//  if (!ptr) {
//    ib::info(ER_IB_MSG_855) << "VirtualAlloc(" << size
//                            << " bytes) failed;"
//                               " Windows error "
//                            << GetLastError();
//  } else {
//    os_total_large_mem_allocated.fetch_add(size);
//    UNIV_MEM_ALLOC(ptr, size);
//  }
//#else
//  size = getpagesize();
//  /* Align block size to system page size */
//  ut_ad(ut_is_2pow(size));
//  size = *n = ut_2pow_round(*n + (size - 1), size);
//  ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | OS_MAP_ANON,
//             -1, 0);
//  if (UNIV_UNLIKELY(ptr == (void *)-1)) {
//    ib::error(ER_IB_MSG_856) << "mmap(" << size
//                             << " bytes) failed;"
//                                " errno "
//                             << errno;
//    ptr = nullptr;
//  } else {
//    os_total_large_mem_allocated.fetch_add(size);
//    UNIV_MEM_ALLOC(ptr, size);
//  }
//#endif
//  return (ptr);
//}
//=======
//>>>>>>> theirs
