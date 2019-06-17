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

INCLUDE_DIRECTORIES(SYSTEM
  ${GMOCK_INCLUDE_DIRS}
  ${BOOST_PATCHES_DIR}
  ${BOOST_INCLUDE_DIR}
  ${CURL_INCLUDE_DIRS}
)
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

FOREACH(test ${TESTS})
    SET(SRC_FILES ${test}-t.cc
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
            )
    IF(WIN32)
        LIST(APPEND SRC_FILES ../../../sql/nt_servc.cc)
    ENDIF()
    ADD_EXECUTABLE(${test}-t ${SRC_FILES})

    TARGET_LINK_LIBRARIES(${test}-t gunit_large strings dbug regex mysys sql)
    TARGET_LINK_LIBRARIES(${test}-t sql binlog rpl master slave sql ${CURL_LIBRARY} ${SSL_LIBRARIES})

    IF(WITH_PERFSCHEMA_STORAGE_ENGINE)
        TARGET_LINK_LIBRARIES(${test}-t perfschema)
        TARGET_LINK_LIBRARIES(${test}-t perfschema pfs_server_stubs)
    ENDIF()

    ADD_TEST(${test} ${test}-t)
ENDFOREACH()
