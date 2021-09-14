#include "./enc_keyring_master_key_manager.h"
#include "./rdb_mutex_wrapper.h"
#include "./rdb_psi.h"
#include <string>
#include <memory>
#include <mutex>
#include "sql/mysqld.h"
#include <scope_guard.h>

namespace myrocks {
// The implementation of KeyringMasterManager is based on the implementation
// in InnoDB (os0enc.cc). That logic works fine, so no point in reinventing
// the wheel.

static std::once_flag master_key_mutex_init_once_flag;
static Rds_mysql_mutex master_key_id_mutex_;


KeyringMasterKeyManager::KeyringMasterKeyManager(const std::string& uuid)
: oldestMasterKeyId_((uint32_t)-1)
, newestMasterKeyId_(0)
, seedUuid_(uuid) {
  std::call_once(master_key_mutex_init_once_flag, [](){
    master_key_id_mutex_.init(rdb_master_key_mutex_key, MY_MUTEX_INIT_FAST);
  });

  InitKeyringServices();
}

KeyringMasterKeyManager::~KeyringMasterKeyManager() {
  DeinitKeyringServices();
}


void KeyringMasterKeyManager::InitKeyringServices()
{
  SERVICE_TYPE(registry) * reg_svc = mysql_plugin_registry_acquire();

  if (reg_svc == nullptr) {
    return;
  }

  my_h_service h_keyring_reader_service = nullptr;
  my_h_service h_keyring_writer_service = nullptr;
  my_h_service h_keyring_generator_service = nullptr;

  auto cleanup = [&]() {
    if (h_keyring_reader_service) {
      reg_svc->release(h_keyring_reader_service);
    }
    if (h_keyring_writer_service) {
      reg_svc->release(h_keyring_writer_service);
    }
    if (h_keyring_generator_service) {
      reg_svc->release(h_keyring_generator_service);
    }
    mysql_plugin_registry_release(reg_svc);

    keyring_reader_service_ = nullptr;
    keyring_writer_service_ = nullptr;
    keyring_generator_service_ = nullptr;
  };

  if (reg_svc->acquire("keyring_reader_with_status",
                       &h_keyring_reader_service) ||
      reg_svc->acquire_related("keyring_writer", h_keyring_reader_service,
                               &h_keyring_writer_service) ||
      reg_svc->acquire_related("keyring_generator", h_keyring_reader_service,
                               &h_keyring_generator_service)) {
    cleanup();
    return;
  }

  keyring_reader_service_ =
      reinterpret_cast<SERVICE_TYPE(keyring_reader_with_status) *>(
          h_keyring_reader_service);
  keyring_writer_service_ = reinterpret_cast<SERVICE_TYPE(keyring_writer) *>(
      h_keyring_writer_service);
  keyring_generator_service_ =
      reinterpret_cast<SERVICE_TYPE(keyring_generator) *>(
          h_keyring_generator_service);

  mysql_plugin_registry_release(reg_svc);
}

void KeyringMasterKeyManager::DeinitKeyringServices() {

  SERVICE_TYPE(registry) * reg_svc = mysql_plugin_registry_acquire();

  if (reg_svc == nullptr) {
    return;
  }

  using keyring_reader_t = SERVICE_TYPE_NO_CONST(keyring_reader_with_status);
  using keyring_writer_t = SERVICE_TYPE_NO_CONST(keyring_writer);
  using keyring_generator_t = SERVICE_TYPE_NO_CONST(keyring_generator);

  if (keyring_reader_service_) {
    reg_svc->release(reinterpret_cast<my_h_service>(
        const_cast<keyring_reader_t *>(keyring_reader_service_)));
  }
  if (keyring_writer_service_) {
    reg_svc->release(reinterpret_cast<my_h_service>(
        const_cast<keyring_writer_t *>(keyring_writer_service_)));
  }
  if (keyring_generator_service_) {
    reg_svc->release(reinterpret_cast<my_h_service>(
        const_cast<keyring_generator_t *>(keyring_generator_service_)));
  }

  keyring_reader_service_ = nullptr;
  keyring_writer_service_ = nullptr;
  keyring_generator_service_ = nullptr;

  mysql_plugin_registry_release(reg_svc);
}

static constexpr uint32_t DEFAULT_MASTER_KEY_ID = 0;
static constexpr char MASTER_KEY_PREFIX[] = "ROCKSDBKey";
constexpr char rocksdb_key_type[] = "AES";
static constexpr size_t KEY_LEN = 32;

int KeyringMasterKeyManager::ReadSecret(const std::string& keyName, std::string* secret) {
  size_t secret_length = 0;
  size_t secret_type_length = 0;
  my_h_keyring_reader_object reader_object = nullptr;
  bool retval = keyring_reader_service_->init(keyName.c_str(), nullptr, &reader_object);

  /* Keyring error */
  if (retval == true) return -1;

  /* Key absent */
  if (reader_object == nullptr) return -1;

  auto cleanup_guard = create_scope_guard([&] {
    if (reader_object != nullptr) (void)keyring_reader_service_->deinit(reader_object);
    reader_object = nullptr;
  });

  /* Fetch length */
  if (keyring_reader_service_->fetch_length(reader_object, &secret_length,
                                   &secret_type_length) == true)
    return -1;

  if (secret_length == 0 || secret_type_length == 0) return -1;

  /* Allocate requried memory for key and secret_type */
  std::vector<unsigned char> secret_v;
  std::vector<char> secret_type_v;
  secret_v.reserve(secret_length);
  secret_type_v.reserve(secret_type_length + 1);
  memset(secret_v.data(), 0, secret_length);
  memset(secret_type_v.data(), 0, secret_type_length + 1);

  if (keyring_reader_service_->fetch(reader_object, secret_v.data(), secret_length,
                            &secret_length, secret_type_v.data(), secret_type_length,
                            &secret_type_length) == true) {
    return -1;
  }
  secret->assign((char*)(secret_v.data()), secret_length);

  return 0;
}

int KeyringMasterKeyManager::GetMostRecentMasterKey(std::string *masterKey, uint32_t *masterKeyId) {
  std::string keyName;
  int retval;
  bool key_id_locked = false;

  if (newestMasterKeyId_ == DEFAULT_MASTER_KEY_ID) {
    /* Take mutex as master_key_id is going to change. */
    RDB_MUTEX_LOCK_CHECK(master_key_id_mutex_);
    key_id_locked = true;
  }

  /* Check for s_master_key_id again, as a parallel rotation might have caused
  it to change. */
  if (newestMasterKeyId_ == DEFAULT_MASTER_KEY_ID) {

    /* If m_master_key is DEFAULT_MASTER_KEY_ID, it means there's
    no encrypted tablespace yet. Generate the first master key now and store
    it to keyring. */
    serverUuid_ = seedUuid_;

    /* Prepare the server s_uuid. */
    keyName = MASTER_KEY_PREFIX + std::string("-") + serverUuid_ + std::string("-1");

    /* We call keyring API to generate master key here. */
    if (keyring_generator_service_->generate(keyName.c_str(), nullptr, rocksdb_key_type,
                                          KEY_LEN) == true) {
      return -1;
    }

    /* We call keyring API to get master key here. */
    retval = ReadSecret(keyName, masterKey);

    if (retval > -1 && masterKey->length() > 0) {
      ++newestMasterKeyId_;
      *masterKeyId = newestMasterKeyId_;
    }
  } else {
    *masterKeyId = newestMasterKeyId_;

    /* We call keyring API to get master key here. */
    retval = GetMasterKey(*masterKeyId, serverUuid_, masterKey);
  }

  if (retval == -1) {
    masterKey->clear();
    // error
    //ib::error(ER_IB_MSG_836) << "Encryption can't find master key, please check"
    //                         << " the keyring is loaded.";
  }

  if (key_id_locked) {
    RDB_MUTEX_UNLOCK_CHECK(master_key_id_mutex_);
  }


#if 0
    if (newestMasterKeyId_ == 0) {
        // there are no encrypted files and we are on default MK id
        // generate the new master key
        newestMasterKeyId_++;
    }
    *masterKey = "12345678901234567890123456789012";
    *masterKeyId = newestMasterKeyId_;
#endif
    return retval;
}

int KeyringMasterKeyManager::GetServerUuid(std::string *serverUuid) {
    *serverUuid = serverUuid_;
    return 0;
}

int KeyringMasterKeyManager::GetMasterKey(uint32_t masterKeyId, const std::string &suuid, std::string *masterKey) {
    std::string keyName = MASTER_KEY_PREFIX + std::string("-") + suuid + std::string("-") + std::to_string(masterKeyId);

    return ReadSecret(keyName, masterKey);
}

int KeyringMasterKeyManager::GenerateNewMasterKey() {
    RDB_MUTEX_LOCK_CHECK(master_key_id_mutex_);
    uint32_t newMasterKeyId = newestMasterKeyId_+1;
    std::string keyName = MASTER_KEY_PREFIX + std::string("-") + serverUuid_ + std::string("-") + std::to_string(newMasterKeyId);

    /* We call keyring API to generate master key here. */
    if (keyring_generator_service_->generate(keyName.c_str(), nullptr, rocksdb_key_type,
                                          KEY_LEN) == true) {
      RDB_MUTEX_UNLOCK_CHECK(master_key_id_mutex_);
      return -1;
    }
    newestMasterKeyId_ = newMasterKeyId;

    RDB_MUTEX_UNLOCK_CHECK(master_key_id_mutex_);
    return 0;
}

void KeyringMasterKeyManager::RegisterMasterKeyId(uint32_t masterKeyId, const std::string& serverUuid) {
    if (masterKeyId > newestMasterKeyId_) {
      newestMasterKeyId_ = masterKeyId;
      serverUuid_ = serverUuid;
    } else if(masterKeyId < oldestMasterKeyId_) {
      oldestMasterKeyId_ = masterKeyId;
    }
}

}  // namespace