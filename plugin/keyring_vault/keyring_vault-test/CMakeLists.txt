ADD_EXECUTABLE(keyring_vault-test
               ./keyring_vault-test.cc
               ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keyring_key.cc
               ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keys_container.cc
               ${CMAKE_SOURCE_DIR}/plugin/keyring/common/keyring_impl.cc
               ../vault_io.cc
               ../vault_key.cc
               ../vault_parser.cc
               ../vault_keys_list.cc
               ../vault_keys_container.cc
               ../vault_curl.cc
               ../vault_credentials_parser.cc)

TARGET_LINK_LIBRARIES(keyring_vault-test strings dbug regex mysys sql)
TARGET_LINK_LIBRARIES(keyring_vault-test binlog rpl master slave)
TARGET_LINK_LIBRARIES(keyring_vault-test ${CURL_LIBRARY} ${SSL_LIBRARIES})
