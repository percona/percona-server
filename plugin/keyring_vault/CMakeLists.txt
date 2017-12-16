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

MACRO (CHECK_IF_CURL_DEPENDS_ON_RTMP project_name)
  EXECUTE_PROCESS(COMMAND ldd ${CURL_LIBRARY}
                  COMMAND grep rtmp
                  OUTPUT_VARIABLE CURL_DEPENDS_ON_RTMP)
  IF (NOT CURL_DEPENDS_ON_RTMP STREQUAL "")
    message("Not building ${project_name}. The supplied CURL library depends on rtmp library.
Please provide CURL library that does not depend on rtmp library to build keyring_vault unittests.")
    RETURN()
  ENDIF()
ENDMACRO()

CHECK_IF_LIB_FOUND(CURL "keyring_vault")

INCLUDE_DIRECTORIES(${BOOST_PATCHES_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${BOOST_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/plugin/keyring/common)

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
                 LINK_LIBRARIES ${CURL_LIBRARY}
                 MODULE_ONLY
                 MODULE_OUTPUT_NAME "keyring_vault")

IF(WITH_KEYRING_VAULT_TEST)
  ADD_SUBDIRECTORY(keyring_vault-test)
ENDIF()

IF(INSTALL_MYSQLTESTDIR)
  INSTALL(DIRECTORY tests/mtr/ DESTINATION ${INSTALL_MYSQLTESTDIR}/suite/keyring_vault COMPONENT Test)
ENDIF()
