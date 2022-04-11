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

  # get a list of rocksdb library source files
  # run with env -i to avoid passing variables
  EXECUTE_PROCESS(
    COMMAND env -i ${CMAKE_CURRENT_SOURCE_DIR}/get_rocksdb_files.sh
    OUTPUT_VARIABLE SCRIPT_OUTPUT
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  # split the list into lines
  STRING(REGEX MATCHALL "[^\n]+" ROCKSDB_LIB_SOURCES ${SCRIPT_OUTPUT})

  SET(ROCKSDB_PLUGINS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb_plugins)

  LIST(APPEND PLUGINS ${ROCKSDB_PLUGINS})
  FOREACH(PLUGIN IN LISTS PLUGINS)
    SET(PLUGIN_ROOT "${ROCKSDB_PLUGINS_ROOT}/${PLUGIN}/")
    MESSAGE(STATUS "MyRocks: Building RocksDB plugin: ${PLUGIN}")
    FILE(GLOB PLUGINMKFILE "${PLUGIN_ROOT}${PLUGIN}*.mk")
    FILE(GLOB PLUGINCMAKEFILE "${PLUGIN_ROOT}CMakeLists.txt")
    SET(CURRENT_PLUGIN_SOURCES)
    SET(CURRENT_PLUGIN_LD)
    IF (EXISTS ${PLUGINCMAKEFILE})
      IF (PLUGIN STREQUAL "zenfs")
        SET(ZENFS_SKIP_LIBZBD_CHECK 1)
      ENDIF()
      ADD_SUBDIRECTORY("${PLUGIN_ROOT}")
      FOREACH(SOURCE_FILE ${${PLUGIN}_SOURCES})
        LIST(APPEND CURRENT_PLUGIN_SOURCES "${PLUGIN_ROOT}${SOURCE_FILE}")
      ENDFOREACH()
      SET(CURRENT_PLUGIN_LD ${${PLUGIN}_CMAKE_EXE_LINKER_FLAGS})
    ELSEIF (NOT EXISTS ${PLUGINMKFILE})
      MESSAGE(SEND_ERROR "Missing RocksDB plugin CMakeLists.txt/makefile in ${PLUGIN_ROOT}")
    ELSE()
      FILE(READ ${PLUGINMKFILE} PLUGINMK)
      #Remove line continuation/breaking
      STRING(REGEX REPLACE "[ \t\r]*\\\\\n[ \t\r]*" " "  PLUGINMK ${PLUGINMK})
      STRING(REGEX MATCH "SOURCES[ ]*=[ ]*([^\n]*)" FOO ${PLUGINMK})
      SET(MK_SOURCES ${CMAKE_MATCH_1})
      SEPARATE_ARGUMENTS(MK_SOURCES)
      FOREACH(MK_FILE IN LISTS MK_SOURCES)
        LIST(APPEND CURRENT_PLUGIN_SOURCES "${PLUGIN_ROOT}${MK_FILE}")
      ENDFOREACH()
      STRING(REGEX MATCH "LDFLAGS[ ]*=[ ]*([^\n]*)" FOO ${PLUGINMK})
      SET(CURRENT_PLUGIN_LD "${CMAKE_MATCH_1}")
    ENDIF()
    IF (PLUGIN STREQUAL "zenfs")
      SET(ZENFS_PLUGIN_ENABLED 1)
      OPTION(WITH_ZENFS_UTILITY "Build with ZenFS utility" OFF)
      SET(ZENFS_PLUGIN_SOURCES ${CURRENT_PLUGIN_SOURCES})
      STRING(REPLACE "-lzbd" "" CURRENT_PLUGIN_LD ${CURRENT_PLUGIN_LD})
      STRING(STRIP ${CURRENT_PLUGIN_LD} CURRENT_PLUGIN_LD)
      IF (NOT EXISTS ${PLUGINCMAKEFILE})
        EXECUTE_PROCESS(COMMAND "./generate-version.sh" WORKING_DIRECTORY "${PLUGIN_ROOT}" RESULT_VARIABLE GENVER_RESULT)
        IF (NOT GENVER_RESULT EQUAL 0)
          MESSAGE(FATAL_ERROR "Cannot generate zenfs version file")
        ENDIF()
      ENDIF()
      UNSET(ZENFS_SKIP_LIBZBD_CHECK)
    ENDIF()
    LIST(APPEND PLUGIN_SOURCES ${CURRENT_PLUGIN_SOURCES})
    LIST(APPEND PLUGIN_LD ${CURRENT_PLUGIN_LD})
  ENDFOREACH()

  # Statically include all RocksDB source
  SET(ROCKSDB_SOURCES
    ${ROCKSDB_LIB_SOURCES}
    ${PLUGIN_SOURCES}
  )
ENDIF()

IF (ZENFS_PLUGIN_ENABLED)
  INCLUDE(zbd)
  MYSQL_CHECK_ZBD()
  SET(rocksdb_static_libs ${rocksdb_static_libs} ${ZBD_LIBRARY})
ENDIF()

# This is a strong requirement coming from RocksDB. No conditional checks here.
add_definitions(-DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX -DZLIB -DLZ4 -DZSTD)
IF (WITH_UBSAN)
  ADD_DEFINITIONS(-DROCKSDB_UBSAN_RUN)
ENDIF()
IF (WITH_VALGRIND)
  ADD_DEFINITIONS(-DROCKSDB_VALGRIND_RUN)
ENDIF()
IF (WITH_JEMALLOC)
  ADD_DEFINITIONS(-DROCKSDB_JEMALLOC)
ENDIF()

IF (ROCKSDB_CUSTOM_NAMESPACE)
  ADD_DEFINITIONS(-DROCKSDB_NAMESPACE=${ROCKSDB_CUSTOM_NAMESPACE})
  ADD_DEFINITIONS(-DROCKSDB_CUSTOM_NAMESPACE=${ROCKSDB_CUSTOM_NAMESPACE})
ENDIF()

include(compiler_features)

IF (NOT HAVE_THREAD_LOCAL)
  MESSAGE(FATAL_ERROR "No '__thread' support found. Not building MyRocks")
ENDIF()

IF (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  IF (HAVE_SSE42)
    MESSAGE(STATUS "MyRocks: SSE42 support found")
    add_compile_flags(${ROCKSDB_ROOT}/util/crc32c.cc COMPILE_FLAGS "-msse4.2") # use Fast_CRC32
    IF (HAVE_PCLMUL)
      add_compile_flags(${ROCKSDB_ROOT}/util/crc32c.cc COMPILE_FLAGS "-mpclmul") # use crc32c_3way instead of Fast_CRC32
    ENDIF()
  ELSE()
    IF (ALLOW_NO_SSE42)
      MESSAGE(WARNING "No SSE42 support found and ALLOW_NO_SSE42 specified, building MyRocks but without SSE42/FastCRC32 support")
    ELSE()
      MESSAGE(FATAL_ERROR "No SSE42 support found. Not building MyRocks")
    ENDIF()
  ENDIF()
ELSE() # aarch64
  CHECK_C_COMPILER_FLAG("-march=armv8-a+crc+crypto" ARMV8A_CRC_CRYPTO_SUPPORTED)
  IF (ARMV8A_CRC_CRYPTO_SUPPORTED)
    MESSAGE(STATUS "MyRocks: ARMv8-A+crc+crypto support found")
    add_compile_flags(${ROCKSDB_ROOT}/util/crc32c.cc COMPILE_FLAGS "-march=armv8-a+crc+crypto")
    add_compile_flags(${ROCKSDB_ROOT}/util/crc32c_arm64.cc COMPILE_FLAGS "-march=armv8-a+crc+crypto")
  ELSE()
    IF (ALLOW_NO_ARMV8A_CRC_CRYPTO)
      MESSAGE(WARNING "No ARMv8-A+crc+crypto support found and ALLOW_NO_ARMV8A_CRC_CRYPTO specified, building MyRocks but without ARMv8-A+crc+crypto/FastCRC32 support")
    ELSE()
      MESSAGE(FATAL_ERROR "No ARMv8-A+crc+crypto support found. Not building MyRocks")
    ENDIF()
  ENDIF()
ENDIF()

IF (HAVE_AVX2 AND NOT ROCKSDB_DISABLE_AVX2)
  add_compile_flags(${ROCKSDB_ROOT}/table/block_based/filter_policy.cc COMPILE_FLAGS "-mavx2")
ENDIF()

IF (HAVE_URING AND ROCKSDB_USE_IO_URING)
  SET(rocksdb_static_libs ${rocksdb_static_libs} ${URING_LIBRARY})
ENDIF()

IF (HAVE_MEMKIND AND ROCKSDB_USE_MEMKIND)
  SET(rocksdb_static_libs ${rocksdb_static_libs} ${MEMKIND_LIBRARY})
ENDIF()

IF (HAVE_ALIGNED_NEW AND NOT ROCKSDB_DISABLE_ALIGNED_NEW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")
ENDIF()

IF (CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options(-fno-builtin-memcmp)
ENDIF()

IF (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT ROCKSDB_DISABLE_MARCH_NATIVE)
  add_compile_options(-march=native)
ENDIF()

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_definitions(-DOS_LINUX)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  add_definitions(-DOS_MACOSX)
ENDIF()

ROCKSDB_SET_DEFINTIONS()
get_directory_property(COMPILE_DEFINITIONS COMPILE_DEFINITIONS)
list(SORT COMPILE_DEFINITIONS)
message(STATUS "MyRocks compile definitions: ${COMPILE_DEFINITIONS}")


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
)

IF(WITH_FB_TSAN)
  SET(PIC_EXT "_pic")
ELSE()
  SET(PIC_EXT "")
ENDIF()

IF(HAVE_EXTERNAL_ROCKSDB)
  SET(rocksdb_static_libs ${rocksdb_static_libs} "${ROCKSDB_LIB_PATH}/${ROCKSDB_LIB_NAME}")
ENDIF()

SET(rocksdb_static_libs ${rocksdb_static_libs} ${ZLIB_LIBRARY} ${ZSTD_LIBRARY} ${LZ4_LIBRARY} "-ldl" ${PLUGIN_LD})
IF(NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
  SET(rocksdb_static_libs ${rocksdb_static_libs} "-lrt")
ENDIF()

MYSQL_ADD_PLUGIN(rocksdb ${ROCKSDB_SOURCES} STORAGE_ENGINE DEFAULT MODULE_ONLY
  LINK_LIBRARIES ${rocksdb_static_libs}
)

IF (NOT HAVE_EXTERNAL_ROCKSDB)
  # TODO: read this file list from src.mk:TOOL_SOURCES
  SET(ROCKSDB_TOOL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/ldb_tool.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/ldb_cmd.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/sst_dump_tool.cc
    ${ROCKSDB_LIB_SOURCES}
    ${ZSTD_SOURCES}
  )
  MYSQL_ADD_EXECUTABLE(sst_dump ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/sst_dump.cc ${ROCKSDB_TOOL_SOURCES})
  TARGET_LINK_LIBRARIES(sst_dump ${rocksdb_static_libs})

  MYSQL_ADD_EXECUTABLE(ldb ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/tools/ldb.cc ${ROCKSDB_TOOL_SOURCES})
  TARGET_LINK_LIBRARIES(ldb ${rocksdb_static_libs})

  MYSQL_ADD_EXECUTABLE(mysql_ldb ${CMAKE_CURRENT_SOURCE_DIR}/tools/mysql_ldb.cc ${ROCKSDB_TOOL_SOURCES})
  TARGET_LINK_LIBRARIES(mysql_ldb ${rocksdb_static_libs})

  IF (WITH_ZENFS_UTILITY)
    FIND_PATH(GFLAGS_INCLUDE_DIR
      NAMES gflags/gflags.h)
    FIND_LIBRARY(GFLAGS_SYSTEM_LIBRARY
      NAMES gflags)
    IF (NOT GFLAGS_INCLUDE_DIR OR NOT GFLAGS_SYSTEM_LIBRARY)
      MESSAGE(FATAL_ERROR "Cannot find system gflags library.")
    ENDIF()

    MYSQL_ADD_EXECUTABLE(zenfs ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb_plugins/zenfs/util/zenfs.cc ${ROCKSDB_LIB_SOURCES} ${ZENFS_PLUGIN_SOURCES} ${ZSTD_SOURCES})
    TARGET_LINK_LIBRARIES(zenfs ${rocksdb_static_libs} ${GFLAGS_SYSTEM_LIBRARY})

    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/tmp_gen/rocksdb/plugin")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/rocksdb_plugins/zenfs" "${CMAKE_CURRENT_BINARY_DIR}/tmp_gen/rocksdb/plugin/zenfs")
    TARGET_INCLUDE_DIRECTORIES(zenfs SYSTEM PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/tmp_gen")
  ENDIF()
ENDIF()
