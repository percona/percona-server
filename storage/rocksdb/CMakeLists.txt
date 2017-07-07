# TODO: Copyrights

# by default MyRocks is not built
IF (NOT DEFINED WITH_ROCKSDB)
  IF (WITHOUT_ROCKSDB OR WITHOUT_ROCKSDB_STORAGE_ENGINE)
    SET (WITH_ROCKSDB 0)
  ELSE ()
    SET (WITH_ROCKSDB 1)
  ENDIF ()
ENDIF ()

IF (NOT WITH_ROCKSDB)
  MESSAGE (STATUS "Not building with MyRocks storage engine, you must specify -DWITH_ROCKSDB=1|ON|FORCE to cmake in order to build with MyRocks.")
  RETURN ()
ELSE ()
  MESSAGE (STATUS "Building with MyRocks storage engine = " ${WITH_ROCKSDB})
  IF (WITH_ROCKSDB STREQUAL "FORCE")
    SET (MYROCKS_STATUS_MODE FATAL_ERROR)
  ELSE ()
    SET (MYROCKS_STATUS_MODE WARNING)
  ENDIF ()
ENDIF ()

# check platform support, no 32 bit
IF (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  MESSAGE (${MYROCKS_STATUS_MODE} "x86_64 is only platform supported. ${CMAKE_SYSTEM_PROCESSOR} found. Not building MyRocks")
  RETURN ()
ENDIF ()

# check compiler version, 4.8.0 is minimal accepted
IF ((CMAKE_CXX_COMPILER_ID STREQUAL GNU) AND
    (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.0"))
  MESSAGE (${MYROCKS_STATUS_MODE} "GCC >= 4.8.0 required. ${CMAKE_CXX_COMPILER_VERSION} found. Not building MyRocks")
  RETURN ()
ENDIF ()

# check that compiler supports cxx11 and set options
INCLUDE (check_stdcxx11)
IF (!HAVE_STDCXX11)
  MESSAGE (${MYROCKS_STATUS_MODE} "${CMAKE_CXX_COMPILER} doesn't support -std=c++11, you need one that does. Not building MyRocks")
  RETURN ()
ENDIF ()

IF (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/Makefile")
  MESSAGE(SEND_ERROR "Missing Makefile in rocksdb directory. Try \"git submodule update\".")
ENDIF ()

CHECK_FUNCTION_EXISTS(sched_getcpu  HAVE_SCHED_GETCPU)
IF(HAVE_SCHED_GETCPU)
  ADD_DEFINITIONS(-DHAVE_SCHED_GETCPU=1 -DROCKSDB_SCHED_GETCPU_PRESENT)
ENDIF()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
SET(CMAKE_REQUIRED_FLAGS ${CMAKE_CXX_FLAGS})
INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
#include <cstdint>
#include <nmmintrin.h>
int main() {
  volatile uint32_t x = _mm_crc32_u32(0, 0);
}
" HAVE_SSE42)
IF (HAVE_SSE42)
  ADD_DEFINITIONS(-DHAVE_SSE42)
ELSE ()
  MESSAGE(${MYROCKS_STATUS_MODE} "No SSE42 support found. Not building MyRocks")
ENDIF ()

IF(UNIX)
  IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    ADD_DEFINITIONS(-DOS_LINUX)

    CHECK_INCLUDE_FILES(linux/falloc.h HAVE_LINUX_FALLOC_H)
    CHECK_FUNCTION_EXISTS(fallocate HAVE_FALLOCATE)

    IF(HAVE_FALLOCATE AND HAVE_LINUX_FALLOC_H)
      ADD_DEFINITIONS(-DROCKSDB_FALLOCATE_PRESENT)
    ENDIF()
  ENDIF()
ENDIF()

# get a list of rocksdb library source files
# run with env -i to avoid passing variables
EXECUTE_PROCESS(
  COMMAND env -i ${CMAKE_CURRENT_SOURCE_DIR}/get_rocksdb_files.sh
  OUTPUT_VARIABLE SCRIPT_OUTPUT
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
# split the list into lines
STRING(REGEX MATCHALL "[^\n]+" ROCKSDB_LIB_SOURCES ${SCRIPT_OUTPUT})

# add bundled compression code
SET(ROCKSDB_LIB_SOURCES
  ${ROCKSDB_LIB_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lz4/lib/lz4.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lz4/lib/lz4hc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lz4/lib/lz4frame.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/common/entropy_common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/common/error_private.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/common/fse_decompress.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/common/pool.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/common/threading.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/common/xxhash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/common/zstd_common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/compress/fse_compress.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/compress/huf_compress.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/compress/zstd_compress.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/compress/zstdmt_compress.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/decompress/huf_decompress.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/decompress/zstd_decompress.c
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/include
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/include/rocksdb
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/third-party/gtest-1.7.0/fused-src
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lz4/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/common
)

ADD_DEFINITIONS(-DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX -DZLIB -DLZ4
  -DZSTD -DROCKSDB_SUPPORT_THREAD_LOCAL)

IF(WITH_FB_TSAN)
  SET(PIC_EXT "_pic")
ELSE()
  SET(PIC_EXT "")
ENDIF()

SET(ROCKSDB_SOURCES
  ha_rocksdb.cc ha_rocksdb.h ha_rocksdb_proto.h
  logger.h
  rdb_comparator.h
  rdb_datadic.cc rdb_datadic.h
  rdb_cf_options.cc rdb_cf_options.h
  rdb_cf_manager.cc rdb_cf_manager.h
  properties_collector.cc properties_collector.h
  event_listener.cc event_listener.h
  rdb_i_s.cc rdb_i_s.h
  rdb_index_merge.cc rdb_index_merge.h
  rdb_perf_context.cc rdb_perf_context.h
  rdb_mutex_wrapper.cc rdb_mutex_wrapper.h
  rdb_psi.h rdb_psi.cc
  rdb_sst_info.cc rdb_sst_info.h
  rdb_utils.cc rdb_utils.h rdb_buff.h
  rdb_threads.cc rdb_threads.h
  ${ROCKSDB_LIB_SOURCES}
)

SET(rocksdb_static_libs ${rocksdb_static_libs} ${ZLIB_LIBRARY} "-lrt")

MYSQL_ADD_PLUGIN(rocksdb ${ROCKSDB_SOURCES} STORAGE_ENGINE DEFAULT MODULE_ONLY
  LINK_LIBRARIES ${rocksdb_static_libs}
)

IF(WITH_EMBEDDED_SERVER)
  ADD_SUBDIRECTORY(unittest)
ENDIF()

# TODO: read this file list from src.mk:TOOL_SOURCES
SET(ROCKSDB_TOOL_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/ldb_tool.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/ldb_cmd.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/sst_dump_tool.cc
  ${ROCKSDB_LIB_SOURCES}
)
MYSQL_ADD_EXECUTABLE(sst_dump ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/sst_dump.cc ${ROCKSDB_TOOL_SOURCES})
TARGET_LINK_LIBRARIES(sst_dump ${rocksdb_static_libs})

MYSQL_ADD_EXECUTABLE(ldb ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/ldb.cc ${ROCKSDB_TOOL_SOURCES})
TARGET_LINK_LIBRARIES(ldb ${rocksdb_static_libs})

MYSQL_ADD_EXECUTABLE(mysql_ldb ${CMAKE_CURRENT_SOURCE_DIR}/tools/mysql_ldb.cc ${ROCKSDB_TOOL_SOURCES})
TARGET_LINK_LIBRARIES(mysql_ldb ${rocksdb_static_libs})
