INCLUDE(curl)

IF (NOT DEFINED WITH_KEYRING_VAULT)
  SET (WITH_KEYRING_VAULT 1)
ENDIF()
IF(NOT GMOCK_FOUND OR NOT WITH_KEYRING_VAULT)
    MESSAGE (STATUS "Not building keyring_vault unit tests")
    RETURN()
ENDIF()

CHECK_IF_LIB_FOUND(CURL "keyring_vault unit tests" FATAL_ERROR)

message(STATUS "Building keyring_vault unittests")

ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="keyring_vault")

INCLUDE_DIRECTORIES(SYSTEM
        ${GMOCK_INCLUDE_DIRS}
        ${BOOST_PATCHES_DIR}
        ${BOOST_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS})

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
    vault_keyring-api
)

SET(SRC_FILES)
LIST(APPEND SRC_FILES
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keyring_key.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keys_container.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keys_iterator.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_keys_list.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_key.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_io.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_base64.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_curl.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_parser_composer.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_credentials.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_credentials_parser.cc
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_keys_container.cc
  ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring_vault/generate_credential_file.cc
  ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring_vault/vault_environment.cc
  ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring_vault/vault_mount.cc
  ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring_vault/vault_test_base.cc
  ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring_vault/uuid.cc
)

ADD_LIBRARY(keyring_vault_test STATIC ${SRC_FILES})
ADD_DEPENDENCIES(keyring_vault_test GenError)
TARGET_LINK_LIBRARIES(keyring_vault_test extra::rapidjson ext::zlib)

SET(ALL_KEYRING_VAULT_TESTS)
FOREACH(test ${TESTS})
  LIST(APPEND ALL_KEYRING_VAULT_TESTS ${test}-t.cc)
ENDFOREACH()

MYSQL_ADD_EXECUTABLE(merge_keyring_vault_tests-t
  ${ALL_KEYRING_VAULT_TESTS}
  ENABLE_EXPORTS
  ADD_TEST merge_keyring_vault_tests)

TARGET_LINK_LIBRARIES(merge_keyring_vault_tests-t
  keyring_vault_test
  gunit_large
  perfschema
  sql_main
  strings
  mysys
  sql_gis
  sql_dd
  minchassis
  binlog
  rpl
  rpl_source
  rpl_replica
  ext::curl
  ${SSL_LIBRARIES}
  ${ICU_LIBRARIES})

ADD_DEPENDENCIES(merge_keyring_vault_tests-t GenError)

set(LARGE_TESTS "vault_keyring-api" "vault_keys_container")

FOREACH(test ${TESTS})
  MYSQL_ADD_EXECUTABLE(${test}-t ${test}-t.cc
    ENABLE_EXPORTS SKIP_INSTALL EXCLUDE_FROM_ALL)

  TARGET_LINK_LIBRARIES(${test}-t keyring_vault_test)

  IF(${test} IN_LIST LARGE_TESTS)
    TARGET_LINK_LIBRARIES(${test}-t
      gunit_large
      strings
      mysys
      sql_main
      sql_gis
      sql_dd
      binlog
      rpl
      master
      slave
      minchassis)
  ELSE()
    TARGET_LINK_LIBRARIES(${test}-t
      gunit_small
      server_unittest_library)
  ENDIF()

  TARGET_LINK_LIBRARIES(${test}-t
    ${ICU_LIBRARIES}
    ext::curl
    ${SSL_LIBRARIES})
ENDFOREACH()
