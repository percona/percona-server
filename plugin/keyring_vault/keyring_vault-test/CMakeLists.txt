INCLUDE_DIRECTORIES(SYSTEM
  ${BOOST_PATCHES_DIR}
  ${BOOST_INCLUDE_DIR}
  ${CURL_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/libbinlogevents/include)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/sql
  ${CMAKE_SOURCE_DIR}/plugin/keyring
  ${CMAKE_SOURCE_DIR}/plugin/keyring/common
  ${CMAKE_SOURCE_DIR}/plugin/keyring_vault
  ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring_vault)

ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="keyring_vault" -DEXTRA_CODE_FOR_UNIT_TESTING)

MYSQL_ADD_EXECUTABLE(keyring_vault-test
               ./keyring_vault-test.cc
               ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keyring_key.cc
               ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keys_container.cc
               ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keys_iterator.cc
               ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keyring_impl.cc
               ../vault_io.cc
               ../vault_key.cc
               ../vault_base64.cc
               ../vault_parser_composer.cc
               ../vault_keys_list.cc
               ../vault_keys_container.cc
               ../vault_curl.cc
               ../vault_credentials_parser.cc
               ../vault_credentials.cc
               ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring_vault/generate_credential_file.cc
               ${CMAKE_SOURCE_DIR}/unittest/gunit/keyring_vault/vault_mount.cc
               ADD_TEST keyring_vault-test)

TARGET_LINK_LIBRARIES(keyring_vault-test strings mysys sql_main minchassis)
TARGET_LINK_LIBRARIES(keyring_vault-test binlog rpl rpl_source rpl_replica)
TARGET_LINK_LIBRARIES(keyring_vault-test ext::curl ${SSL_LIBRARIES})
