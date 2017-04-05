INCLUDE(CheckFunctionExists)
INCLUDE(curl)

SET(CMAKE_EXTRA_INCLUDE_FILES string.h)
CHECK_FUNCTION_EXISTS(memset_s HAVE_MEMSET_S)
SET(CMAKE_EXTRA_INCLUDE_FILES)

MACRO (CHECK_IF_LIB_FOUND lib_name project_name)
  SET (lib_found_variable_name "${lib_name}_FOUND")
  IF (NOT DEFINED ${lib_found_variable_name} OR NOT ${${lib_found_variable_name}})
    message("Not building ${project_name}, could not find library: ${lib_name}")
    RETURN()
  ENDIF()
ENDMACRO()

FIND_CURL()
CHECK_IF_LIB_FOUND(CURL "keyring_vault")

INCLUDE_DIRECTORIES(${BOOST_PATCHES_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${BOOST_INCLUDE_DIR})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common
  ${CURL_INCLUDE_DIRS})

MYSQL_ADD_PLUGIN(keyring_vault
                 ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keyring_key.cc
                 ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keys_container.cc
                 ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keyring_impl.cc
                 vault_io.cc
                 vault_key.cc
                 vault_base64.cc
                 vault_parser.cc
                 vault_keys_list.cc
                 vault_keys_container.cc
                 vault_curl.cc
                 vault_credentials_parser.cc
                 vault_credentials.cc
                 vault_keyring.cc
                 LINK_LIBRARIES  ${CURL_LIBRARIES}
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
