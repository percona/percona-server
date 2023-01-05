SET (ROCKSDB_DYNAMIC_PLUGIN 1)

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
IF (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|AARCH64")
  MESSAGE (${MYROCKS_STATUS_MODE} "x86_64/aarch64 are the only platforms supported. ${CMAKE_SYSTEM_PROCESSOR} found. Not building MyRocks")
  RETURN ()
ENDIF ()


IF(HAVE_EXTERNAL_ROCKSDB)
  MESSAGE(STATUS "MyRocks: Using external RocksDB")
  SET(ROCKSDB_ROOT ${ROCKSDB_SRC_PATH})
ELSE()
  MESSAGE(STATUS "MyRocks: Using local RocksDB")

  SET(ROCKSDB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb)

  FIND_PACKAGE(Git)

  IF (GIT_FOUND AND EXISTS "${ROCKSDB_ROOT}/.git")
    EXECUTE_PROCESS(WORKING_DIRECTORY "${ROCKSDB_ROOT}" OUTPUT_VARIABLE GIT_SHA COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD)
    EXECUTE_PROCESS(WORKING_DIRECTORY "${ROCKSDB_ROOT}" RESULT_VARIABLE GIT_MOD COMMAND "${GIT_EXECUTABLE}" diff-index HEAD --quiet)
    EXECUTE_PROCESS(WORKING_DIRECTORY "${ROCKSDB_ROOT}" OUTPUT_VARIABLE GIT_DATE COMMAND "${GIT_EXECUTABLE}" log -1 --date=format:"%Y-%m-%d %T" --format="%ad")
    EXECUTE_PROCESS(WORKING_DIRECTORY "${ROCKSDB_ROOT}" OUTPUT_VARIABLE GIT_TAG RESULT_VARIABLE rv COMMAND "${GIT_EXECUTABLE}" symbolic-ref -q --short HEAD OUTPUT_STRIP_TRAILING_WHITESPACE)
    IF (rv AND NOT rv EQUAL 0)
      EXECUTE_PROCESS(
        WORKING_DIRECTORY "${ROCKSDB_ROOT}"
        OUTPUT_VARIABLE GIT_TAG
        ERROR_VARIABLE GIT_ERROR
        RESULT_VARIABLE rv
        COMMAND "${GIT_EXECUTABLE}" describe --tags --exact-match
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      IF (rv AND NOT rv EQUAL 0)
        EXECUTE_PROCESS(
          WORKING_DIRECTORY "${ROCKSDB_ROOT}"
          OUTPUT_VARIABLE GIT_TAG
          COMMAND "${GIT_EXECUTABLE}" for-each-ref --contains HEAD --format=%\(refname:lstrip=-1\) --sort=version:refname --count=1 refs/remotes/*/*.fb
          OUTPUT_STRIP_TRAILING_WHITESPACE
        )
      ENDIF()
    ENDIF()
  ELSE()
    SET(GIT_SHA 0)
    SET(GIT_MOD 1)
  ENDIF()
  STRING(REGEX REPLACE "[^0-9a-fA-F]+" "" GIT_SHA "${GIT_SHA}")
  STRING(REGEX REPLACE "[^0-9: /-]+" "" GIT_DATE "${GIT_DATE}")

  SET(BUILD_VERSION_CC ${ROCKSDB_ROOT}/util/build_version.cc)
  CONFIGURE_FILE(${ROCKSDB_ROOT}/util/build_version.cc.in ${BUILD_VERSION_CC} @ONLY)

  IF (NOT EXISTS "${ROCKSDB_ROOT}/Makefile")
    MESSAGE(SEND_ERROR "Missing Makefile in rocksdb directory. Try \"git submodule update\".")
  ENDIF()

  # We assume that make checkout_folly has already been run before attempting
  # to build with folly.
  IF(ROCKSDB_FOLLY)
    IF (NOT EXISTS "${CMAKE_SOURCE_DIR}/rocksdb/third-party/folly")
      MESSAGE(SEND_ERROR "Missing folly in rocksdb directory. Try \"make checkout_folly\".")
    ENDIF()
    ADD_DEFINITIONS(-DUSE_FOLLY -DFOLLY_NO_CONFIG)
    INCLUDE_DIRECTORIES(
      ${ROCKSDB_ROOT}/third-party/folly
    )
    IF (WITH_GLOG)
      FIND_LIBRARY(GLOG_LIBRARY
        NAMES libglog${PIC_EXT}.a glog
        HINTS ${WITH_GLOG}/lib)
      SET(rocksdb_static_libs ${rocksdb_static_libs}
        ${GLOG_LIBRARY})
    ENDIF()
    IF (WITH_GFLAGS)
      FIND_LIBRARY(GFLAGS_LIBRARY
        NAMES libgflags${PIC_EXT}.a gflags
        HINTS ${WITH_GFLAGS}/lib)
      SET(rocksdb_static_libs ${rocksdb_static_libs}
        ${GFLAGS_LIBRARY})
    ENDIF()
  ENDIF()
  # get a list of rocksdb library source files
  # run with env -i to avoid passing variables
  EXECUTE_PROCESS(
    COMMAND env -i ${CMAKE_CURRENT_SOURCE_DIR}/get_rocksdb_files.sh ${ROCKSDB_FOLLY}
    OUTPUT_VARIABLE SCRIPT_OUTPUT
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  # split the list into lines
  STRING(REGEX MATCHALL "[^\n]+" ROCKSDB_LIB_SOURCES ${SCRIPT_OUTPUT})

  # Statically include all RocksDB source
  SET(ROCKSDB_SOURCES
    ${ROCKSDB_LIB_SOURCES}
  )
ENDIF()

# This is a strong requirement coming from RocksDB. No conditional checks here.
ADD_DEFINITIONS(-DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX -DZLIB -DLZ4 -DZSTD)

IF (ROCKSDB_CUSTOM_NAMESPACE)
  ADD_DEFINITIONS(-DROCKSDB_NAMESPACE=${ROCKSDB_CUSTOM_NAMESPACE})
  ADD_DEFINITIONS(-DROCKSDB_CUSTOM_NAMESPACE=${ROCKSDB_CUSTOM_NAMESPACE})
ENDIF()

IF (WITH_UBSAN)
  ADD_DEFINITIONS(-DROCKSDB_UBSAN_RUN)
ENDIF()
IF (WITH_VALGRIND)
  ADD_DEFINITIONS(-DROCKSDB_VALGRIND_RUN)
ENDIF()
IF (WITH_JEMALLOC)
  ADD_DEFINITIONS(-DROCKSDB_JEMALLOC)
ENDIF()
IF (WITH_NUMA)
  ADD_DEFINITIONS(-DNUMA)
ENDIF()

IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  ADD_DEFINITIONS(-DOS_LINUX)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  ADD_DEFINITIONS(-DOS_MACOSX)
ENDIF()

IF (CMAKE_COMPILER_IS_GNUCXX)
  ADD_COMPILE_OPTIONS(-fno-builtin-memcmp)
ENDIF()


INCLUDE(compiler_features)

IF (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
  IF (NOT ROCKSDB_BUILD_ARCH)
    IF (ROCKSDB_DISABLE_MARCH_NATIVE)
      # Intel Westmere is oldest CPU that supports SSE 4.2 and PCLMUL instruction sets
      # the compiled binary will also work on "core2" and "nehalem" because "sse4.2" and "pclmul" are checked on runtime
      SET(ROCKSDB_BUILD_ARCH "westmere")
    ELSE()
      SET(ROCKSDB_BUILD_ARCH "native")
    ENDIF()
    MESSAGE(STATUS "MyRocks x86_64 build architecture: -march=${ROCKSDB_BUILD_ARCH}")
  ELSE()
    MESSAGE(STATUS "MyRocks x86_64 build architecture: -march=${ROCKSDB_BUILD_ARCH}. If the enforced processor architecture is newer than the processor architecture of this build machine, the created binary may crash if executed on this machine.")
  ENDIF()

  # CMAKE_CXX_FLAGS are used by CHECK_CXX_SOURCE_COMPILES called from ROCKSDB_SET_X86_DEFINTIONS()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=${ROCKSDB_BUILD_ARCH}")
  ROCKSDB_SET_X86_DEFINTIONS()
ELSE() # aarch64
  IF (NOT ROCKSDB_BUILD_ARCH)
    IF (ROCKSDB_DISABLE_MARCH_NATIVE)
      # the compiled binary will also work on "armv8-a" because "+crc" and "+crypto" are checked on runtime
      SET(ROCKSDB_BUILD_ARCH "armv8-a+crc+crypto")
    ELSE()
      SET(ROCKSDB_BUILD_ARCH "native")
    ENDIF()
    MESSAGE(STATUS "MyRocks arm64 build architecture: -march=${ROCKSDB_BUILD_ARCH}")
  ELSE()
    MESSAGE(STATUS "MyRocks arm64 build architecture: -march=${ROCKSDB_BUILD_ARCH}. If the enforced processor architecture is newer than the processor architecture of this build machine, the created binary may crash if executed on this machine.")
  ENDIF()

  # CMAKE_CXX_FLAGS are used by CHECK_CXX_SOURCE_COMPILES called from ROCKSDB_SET_ARM64_DEFINTIONS()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=${ROCKSDB_BUILD_ARCH}")
  ROCKSDB_SET_ARM64_DEFINTIONS()
ENDIF()

ROCKSDB_SET_DEFINTIONS()  # sets HAVE_URING, HAVE_MEMKIND, HAVE_ALIGNED_NEW

IF (HAVE_URING AND ROCKSDB_USE_IO_URING)
  SET(rocksdb_static_libs ${rocksdb_static_libs} ${URING_LIBRARY})
ENDIF()

IF (HAVE_MEMKIND AND ROCKSDB_USE_MEMKIND)
  SET(rocksdb_static_libs ${rocksdb_static_libs} ${MEMKIND_LIBRARY})
ENDIF()

IF (HAVE_ALIGNED_NEW AND NOT ROCKSDB_DISABLE_ALIGNED_NEW)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")
ENDIF()

GET_DIRECTORY_PROPERTY(COMPILE_DEFINITIONS COMPILE_DEFINITIONS)
LIST(SORT COMPILE_DEFINITIONS)
MESSAGE(STATUS "MyRocks compile definitions: ${COMPILE_DEFINITIONS}")


# Suppress warnings for all compilers
append_cflags_if_supported(-Wno-unused-parameter -Wno-suggest-override)
# "cmake/maintainer.cmake" sets "-Wcast-qual -Wundef -Wextra-semi" which cause warnings with RocksDB
remove_compile_flags(-Wcast-qual -Wundef -Wextra-semi)
append_cflags_if_supported(-Wno-invalid-offsetof)

# Suppress warnings for all clang versions
IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  remove_compile_flags(-Wdeprecated -Winconsistent-missing-destructor-override -Wnon-virtual-dtor)
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-shadow-field FILES ha_rocksdb.cc rocksdb/memtable/hash_skiplist_rep.cc)
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-conditional-uninitialized FILES ha_rocksdb.cc rdb_datadic.cc rocksdb/db/db_impl/db_impl.cc rocksdb/db/write_thread.cc rocksdb/env/env_posix.cc rocksdb/env/fs_posix.cc)
ENDIF()

# Suppress warnings for clang-13 or newer
IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0)
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-unused-but-set-variable FILES rocksdb/utilities/env_mirror.cc)
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-deprecated-copy FILES rocksdb/options/options_helper.cc rocksdb/options/db_options.cc)
ENDIF()

# Suppress warnings for clang-10 or newer
IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-range-loop-construct FILES rocksdb/db/db_impl/db_impl_compaction_flush.cc rocksdb/options/options_parser.cc rocksdb/options/options_helper.cc)
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-deprecated-copy FILES rocksdb/db/db_impl/db_impl.cc)
ENDIF()

# Suppress warnings for all gcc versions
IF(CMAKE_COMPILER_IS_GNUCXX)
  # "cmake/maintainer.cmake" sets "-Wmissing-format-attribute" which cause warnings with RocksDB
  remove_compile_flags(-Wmissing-format-attribute)
ENDIF()

# Suppress warnings for gcc-6 or newer
IF(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6)
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-logical-op FILES rocksdb/file/filename.cc)
ENDIF()

# Suppress warnings for gcc-9 or newer
IF(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-deprecated-copy FILES rocksdb/db/db_impl/db_impl.cc)
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-maybe-uninitialized FILES rocksdb/env/env.cc rocksdb/table/block_based/block_based_table_builder.cc)
ENDIF()

# Suppress warnings for gcc-12 or newer
IF(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12)
  IF(HAVE_AVX512F)
    MESSAGE(STATUS "AVX-512: add -Wno-uninitialized for xxhash.cc (check https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105593)")
    ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-uninitialized -Wno-maybe-uninitialized FILES rocksdb/util/xxhash.cc ${BUNDLED_LZ4_PATH}/xxhash.c)
  ENDIF()
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-maybe-uninitialized FILES rocksdb/db/blob/blob_file_reader.cc)
ENDIF()

# Suppress warnings for gcc ASan build
IF(CMAKE_COMPILER_IS_GNUCXX AND WITH_ASAN)
  ADD_CXX_COMPILE_FLAGS_TO_FILES(-Wno-error=maybe-uninitialized FILES rocksdb/util/bloom.cc)
ENDIF()


IF(FB_WITH_WSENV)
  ADD_DEFINITIONS(-DFB_HAVE_WSENV=1)
  INCLUDE_DIRECTORIES(
    ${FB_WITH_WSENV}
  )
ENDIF()

INCLUDE_DIRECTORIES(
  ${ROCKSDB_ROOT}
  ${ROCKSDB_ROOT}/include
  ${ROCKSDB_ROOT}/include/rocksdb
  ${ROCKSDB_ROOT}/third-party/gtest-1.8.1/fused-src
)

IF(BUILD_BUNDLED_ZSTD)
  INCLUDE_DIRECTORIES(BEFORE SYSTEM
    ${ZSTD_INCLUDE_DIR}/dictBuilder)
ENDIF()

# add bundled compression code
SET(ZSTD_SOURCES ${BUNDLED_LZ4_PATH}/xxhash.c)

SET(ROCKSDB_SOURCES
  ${ROCKSDB_SOURCES}
  ${ZSTD_SOURCES}
  ha_rocksdb.cc ha_rocksdb.h ha_rocksdb_proto.h
  logger.h
  rdb_datadic.cc rdb_datadic.h
  rdb_iterator.cc rdb_iterator.h
  rdb_cf_options.cc rdb_cf_options.h
  rdb_cf_manager.cc rdb_cf_manager.h
  rdb_converter.cc rdb_converter.h
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
  sql_dd.cc sql_dd.h
)

IF(WITH_FB_TSAN OR ROCKSDB_DYNAMIC_PLUGIN)
  SET(PIC_EXT "_pic")
ELSE()
  SET(PIC_EXT "")
ENDIF()

IF(HAVE_EXTERNAL_ROCKSDB)
  SET(rocksdb_static_libs ${rocksdb_static_libs} "${ROCKSDB_LIB_PATH}/${ROCKSDB_LIB_NAME}")
ENDIF()

SET(rocksdb_static_libs ${rocksdb_static_libs} ${ZLIB_LIBRARY} ${ZSTD_LIBRARY} ${LZ4_LIBRARY} "-ldl")
IF(NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
  SET(rocksdb_static_libs ${rocksdb_static_libs} "-lrt")
ENDIF()

IF (ROCKSDB_DYNAMIC_PLUGIN)
  MYSQL_ADD_PLUGIN(rocksdb ${ROCKSDB_SOURCES} STORAGE_ENGINE DEFAULT
    MODULE_ONLY LINK_LIBRARIES ${rocksdb_static_libs})
ELSE()
  MYSQL_ADD_PLUGIN(rocksdb_se ${ROCKSDB_SOURCES} STORAGE_ENGINE DEFAULT
    STATIC_ONLY LINK_LIBRARIES ${rocksdb_static_libs})
ENDIF()

IF (NOT HAVE_EXTERNAL_ROCKSDB)
  # TODO: read this file list from src.mk:TOOL_SOURCES
  SET(ROCKSDB_TOOL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/ldb_tool.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/ldb_cmd.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/sst_dump_tool.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/utilities/blob_db/blob_dump_tool.cc
    ${ROCKSDB_LIB_SOURCES}
    ${ZSTD_SOURCES}
  )
  MYSQL_ADD_EXECUTABLE(sst_dump ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/sst_dump.cc ${ROCKSDB_TOOL_SOURCES})
  TARGET_LINK_LIBRARIES(sst_dump ${rocksdb_static_libs})

  MYSQL_ADD_EXECUTABLE(ldb ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/ldb.cc ${ROCKSDB_TOOL_SOURCES})
  TARGET_LINK_LIBRARIES(ldb ${rocksdb_static_libs})
ENDIF()
