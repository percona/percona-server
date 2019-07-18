INCLUDE(curl)

IF (NOT DEFINED WITH_KEYRING_VAULT)
  SET (WITH_KEYRING_VAULT 1)
ENDIF()
IF(NOT GMOCK_FOUND OR NOT WITH_KEYRING_VAULT)
    MESSAGE (STATUS "Not building keyring_vault unit tests")
    RETURN()
ENDIF()

CHECK_IF_LIB_FOUND(CURL "keyring_vault unit tests" FATAL_ERROR)
CHECK_IF_CURL_DEPENDS_ON_RTMP("keyring_vault unit tests")

message(STATUS "Building keyring_vault unittests")

ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="keyring_vault")

INCLUDE_DIRECTORIES(SYSTEM
        ${GMOCK_INCLUDE_DIRS}
        ${BOOST_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
        ${BOOST_PATCHES_DIR})

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/sql
        ${CMAKE_SOURCE_DIR}/plugin/keyring/common
        ${CMAKE_SOURCE_DIR}/plugin/keyring
        ${CMAKE_SOURCE_DIR}/plugin/keyring_vault
        ${CMAKE_SOURCE_DIR}/unittest/gunit
        ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring
)

SET(TESTS
    vault_io
    vault_parser
    vault_credentials_parser
    vault_keys_container
    vault_keyring-api
)

SET(SRC_FILES)
LIST(APPEND SRC_FILES
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keyring_key.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keys_container.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keys_iterator.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common/system_keys_container.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common/system_key_adapter.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_keys_list.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_key.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_io.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_base64.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_curl.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_parser.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_credentials.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_credentials_parser.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_keys_container.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring/file_io.cc
  ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring_vault/vault_mount.cc
  ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring_vault/uuid.cc
)

IF (NOT MERGE_UNITTESTS)
  LIST(APPEND SRC_FILES
    ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring/psi_memory_key.cc
  )
ENDIF(NOT MERGE_UNITTESTS)

ADD_LIBRARY(keyring_vault_test STATIC ${SRC_FILES})
ADD_DEPENDENCIES(keyring_vault_test GenError)

SET(ALL_KEYRING_VAULT_TESTS)
FOREACH(test ${TESTS})
  LIST(APPEND ALL_KEYRING_VAULT_TESTS ${test}-t.cc)
ENDFOREACH()

IF (MERGE_UNITTESTS)

  ADD_DEFINITIONS(-DMERGE_UNITTESTS)

  MYSQL_ADD_EXECUTABLE(merge_keyring_vault_tests-t
    ${ALL_KEYRING_VAULT_TESTS} ADD_TEST merge_keyring_vault_tests)
  TARGET_LINK_LIBRARIES(merge_keyring_vault_tests-t keyring_vault_test)

  TARGET_LINK_LIBRARIES(merge_keyring_vault_tests-t
    gunit_large strings dbug mysys sql_main sql_gis sql_dd)
  TARGET_LINK_LIBRARIES(merge_keyring_vault_tests-t
    sql_main binlog rpl master slave sql_gis sql_dd sql_main)
  TARGET_LINK_LIBRARIES(merge_keyring_vault_tests-t ${CURL_LIBRARY} ${SSL_LIBRARIES})
  TARGET_LINK_LIBRARIES(merge_keyring_vault_tests-t ${ICU_LIBRARIES})

  IF(WITH_PERFSCHEMA_STORAGE_ENGINE)
    TARGET_LINK_LIBRARIES(merge_keyring_vault_tests-t perfschema)
  ENDIF()
  ADD_DEPENDENCIES(merge_keyring_vault_tests-t GenError)

ELSE(MERGE_UNITTESTS)

  FOREACH(test ${TESTS})
    MYSQL_ADD_EXECUTABLE(${test}-t ${test}-t.cc SKIP_INSTALL)

  TARGET_LINK_LIBRARIES(${test}-t keyring_vault_test)
  TARGET_LINK_LIBRARIES(${test}-t
    gunit_large strings dbug mysys sql_main sql_gis sql_dd)
  TARGET_LINK_LIBRARIES(${test}-t
    sql_main binlog rpl master slave sql_main sql_gis sql_dd)
  TARGET_LINK_LIBRARIES(${test}-t ${CURL_LIBRARY} ${SSL_LIBRARIES})
  TARGET_LINK_LIBRARIES(${test}-t ${ICU_LIBRARIES})


    IF(WITH_PERFSCHEMA_STORAGE_ENGINE)
        TARGET_LINK_LIBRARIES(${test}-t perfschema)
    ENDIF()

    ADD_TEST(${test} ${CMAKE_BINARY_DIR}/runtime_output_directory/${test}-t)
  ENDFOREACH()

ENDIF(MERGE_UNITTESTS)
