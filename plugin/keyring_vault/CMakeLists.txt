CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_STDCXX11)

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

IF (NOT CURL_LIBRARY)
    message("Not building keyring_vault, could not find curl library")
    RETURN()
ENDIF()

CHECK_INCLUDE_FILE_CXX(${WITH_CURL_INCLUDE}/curl.h HAVE_CURL_HEADERS)

INCLUDE_DIRECTORIES(${BOOST_PATCHES_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${BOOST_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common
  ${WITH_CURL_INCLUDE})

MYSQL_ADD_PLUGIN(keyring_vault
                 ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keyring_key.cc
                 ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keys_container.cc
                 ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keyring_impl.cc
                 vault_io.cc
                 vault_key.cc
                 vault_parser.cc
                 vault_keys_list.cc
                 vault_keys_container.cc
                 vault_curl.cc
                 vault_credentials_parser.cc
                 vault_keyring.cc
                 LINK_LIBRARIES  ${CURL_LIBRARY} ${SSL_LIBRARIES}
                 MODULE_ONLY
                 MODULE_OUTPUT_NAME "keyring_vault")

IF(WITH_KEYRING_VAULT_TEST)
  ADD_SUBDIRECTORY(keyring_vault-test)
ENDIF()

IF(HAVE_DLOPEN)
    SET(DYNLIB_EXTENSION "so")
    SET_TARGET_PROPERTIES(keyring_vault
      PROPERTIES LINK_FLAGS "${CMAKE_SHARED_LIBRARY_C_FLAGS}")
ENDIF()
