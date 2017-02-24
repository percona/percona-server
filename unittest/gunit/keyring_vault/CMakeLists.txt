IF(NOT GMOCK_FOUND)
    RETURN()
ENDIF()

IF (NOT WITH_CURL_INCLUDE)
    message("Not building keyring_vault, WITH_CURL_INCLUDE not specified")
    RETURN()
ENDIF()

IF (NOT WITH_CURL_LIB)
    message("Not building keyring_vault, WITH_CURL_LIB not specified")
    RETURN()
ENDIF()


FIND_LIBRARY(CURL_LIBRARY
        NAMES curl
        HINTS ${WITH_CURL_LIB})

FIND_LIBRARY(RTMP_LIBRARY
        NAMES rtmp
        HINTS ${WITH_CURL_LIB})

IF (NOT CURL_LIBRARY)
    message("Not building keyring_vault, could not find curl library")
    RETURN()
ENDIF()

CHECK_INCLUDE_FILE_CXX(${WITH_CURL_INCLUDE}/curl.h HAVE_CURL_HEADERS)

IF (NOT HAVE_CURL_HEADERS)
    message("Not building keyring_vault, could not find curl headers")
    RETURN()
ENDIF()

INCLUDE_DIRECTORIES(
        ${GMOCK_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/sql
        ${CMAKE_SOURCE_DIR}/plugin/keyring/common
        ${CMAKE_SOURCE_DIR}/plugin/keyring_vault
        ${CMAKE_SOURCE_DIR}/unittest/gunit
        ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring
        ${OPENSSL_INCLUDE_DIR}
        ${WITH_CURL_INCLUDE}
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
            ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_keys_list.cc
            ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_key.cc
            ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_io.cc
            ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_curl.cc
            ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_parser.cc
            ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_credentials_parser.cc
            ${CMAKE_SOURCE_DIR}/plugin/keyring_vault/vault_keys_container.cc
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
