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

# Suppress warnings for all compilers
remove_compile_flags(-Wundef)
append_cflags_if_supported(-Wno-unused-parameter)

MY_CHECK_CXX_COMPILER_FLAG("-Wno-unused-variable" HAVE_NO_UNUSED_VARIABLE)
IF(HAVE_NO_UNUSED_VARIABLE)
  ADD_COMPILE_FLAGS(ha_rocksdb.cc COMPILE_FLAGS "-Wno-unused-variable")
ENDIF()

MY_CHECK_CXX_COMPILER_FLAG("-Wcast-qual" HAVE_CAST_EQUAL)
IF(HAVE_CAST_EQUAL)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-cast-qual")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-cast-qual")
ENDIF()

MY_CHECK_CXX_COMPILER_FLAG("-Wno-shadow-field" HAVE_NO_SHADOW_FIELD)
IF(HAVE_NO_SHADOW_FIELD)
  ADD_COMPILE_FLAGS(ha_rocksdb.cc rocksdb/utilities/geodb/geodb_impl.cc rocksdb/memtable/hash_skiplist_rep.cc rocksdb/memtable/hash_cuckoo_rep.cc COMPILE_FLAGS "-Wno-shadow-field")
ENDIF()


# Suppress warnings for all clang versions
IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  remove_compile_flags(-Winconsistent-missing-destructor-override -Wdeprecated -Wextra-semi -Wnon-virtual-dtor)

  MY_CHECK_CXX_COMPILER_FLAG("-Wno-conditional-uninitialized" HAVE_NO_CONDITIONAL_UNINITIALIZED)
  IF(HAVE_NO_CONDITIONAL_UNINITIALIZED)
    ADD_COMPILE_FLAGS(ha_rocksdb.cc rocksdb/env/env_posix.cc rocksdb/db/write_thread.cc COMPILE_FLAGS "-Wno-conditional-uninitialized")
  ENDIF()
ENDIF()


# Suppress warnings for clang-8 or newer
IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
  append_cflags_if_supported(-Wno-defaulted-function-deleted)
ENDIF()


# Suppress warnings for gcc-5 or newer
IF(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
  append_cflags_if_supported(-Wno-suggest-attribute=format)
ENDIF()


# Suppress warnings for gcc-7 or newer (it's in zstd)
IF(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
  append_cflags_if_supported(-Wimplicit-fallthrough=0)
ENDIF()


# Suppress warnings for gcc-9 or newer
IF(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
  append_cflags_if_supported(-Wno-deprecated-copy -Wno-pessimizing-move)

  MY_CHECK_CXX_COMPILER_FLAG("-Wno-redundant-move" HAVE_NO_REDUNDANT_MOVE)
  IF(HAVE_NO_REDUNDANT_MOVE)
    ADD_COMPILE_FLAGS(rocksdb/db/range_del_aggregator.cc COMPILE_FLAGS "-Wno-redundant-move")
  ENDIF()

  MY_CHECK_CXX_COMPILER_FLAG("-Wno-sign-compare" HAVE_NO_SIGN_COMPARE)
  IF(HAVE_NO_SIGN_COMPARE)
    ADD_COMPILE_FLAGS(rocksdb/env/io_posix.cc rocksdb/port/port_posix.cc COMPILE_FLAGS "-Wno-sign-compare")
  ENDIF()
ENDIF()



IF (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/Makefile")
  MESSAGE(SEND_ERROR "Missing Makefile in rocksdb directory. Try \"git submodule update\".")
ENDIF ()

CHECK_FUNCTION_EXISTS(sched_getcpu  HAVE_SCHED_GETCPU)
IF(HAVE_SCHED_GETCPU)
  ADD_DEFINITIONS(-DHAVE_SCHED_GETCPU=1 -DROCKSDB_SCHED_GETCPU_PRESENT)
ENDIF()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -mpclmul")
SET(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
#include <cstdint>
#include <nmmintrin.h>
#include <wmmintrin.h>
int main() {
  volatile uint32_t x __attribute__((unused)) = _mm_crc32_u32(0, 0);
  const auto a = _mm_set_epi64x(0, 0);
  const auto b = _mm_set_epi64x(0, 0);
  const auto c = _mm_clmulepi64_si128(a, b, 0x00);
  auto d __attribute__((unused)) = _mm_cvtsi128_si64(c);
}
" HAVE_SSE42)
IF (HAVE_SSE42)
  ADD_DEFINITIONS(-DHAVE_SSE42)
ELSE ()
  IF (ALLOW_NO_SSE42)
    MESSAGE(WARNING "No SSE42 support found and ALLOW_NO_SSE42 specified, building MyRocks but without SSE42/FastCRC32 support")
  ELSE ()
    MESSAGE(FATAL_ERROR "No SSE42 support found. Not building MyRocks")
  ENDIF ()
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
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/dictBuilder/divsufsort.c
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/dictBuilder/zdict.c
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/include
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/include/rocksdb
  ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/third-party/gtest-1.7.0/fused-src
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lz4/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/common
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd/lib/dictBuilder
)

ADD_DEFINITIONS(-DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX -DZLIB -DLZ4
  -DZSTD -DROCKSDB_SUPPORT_THREAD_LOCAL)

IF(WITH_FB_TSAN)
  SET(PIC_EXT "_pic")
ELSE()
  SET(PIC_EXT "")
ENDIF()

IF(DEFINED WITH_UBSAN)
  add_definitions(-DROCKSDB_UBSAN_RUN)
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
  ha_rockspart.cc ha_rockspart.h
  ${ROCKSDB_LIB_SOURCES}
)

SET(rocksdb_static_libs ${rocksdb_static_libs} ${ZLIB_LIBRARY} "-lrt")

MYSQL_ADD_PLUGIN(rocksdb ${ROCKSDB_SOURCES} STORAGE_ENGINE DEFAULT MODULE_ONLY
  LINK_LIBRARIES ${rocksdb_static_libs}
)

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
