#include "./enc_keyring_master_key_manager.h"
#include "./enc_info_storage.h"
#include <scope_guard.h>
#include <memory>
#include <mutex>
#include <string>
#include "./rdb_mutex_wrapper.h"
#include "./rdb_psi.h"
#include "rocksdb/env.h"
#include "rocksdb/logging/logging.h"
#include "sql/mysqld.h"

// #define USE_DEVEL_KEY

namespace myrocks {
// The implementation of KeyringMasterManager is based on the implementation
// in InnoDB (os0enc.cc). That logic works fine, so no point in reinventing
// the wheel.

using rocksdb::InfoLogLevel;

static constexpr uint32_t DEFAULT_MASTER_KEY_ID = 0;
constexpr char rocksdb_key_type[] = "AES";
static constexpr size_t KEY_LEN = 32;

static std::once_flag master_key_mutex_init_once_flag;
static Rds_mysql_mutex master_key_id_mutex_;

KeyringMasterKeyManager::KeyringMasterKeyManager(
    std::shared_ptr<EncryptionInfoStorage> encInfoStorage,
    std::shared_ptr<rocksdb::Logger> logger)
    : oldestMasterKeyId_((uint32_t)-1),
      encInfoStorage_(encInfoStorage),
      logger_(logger) {
  std::call_once(master_key_mutex_init_once_flag, []() {
    master_key_id_mutex_.init(rdb_master_key_mutex_key, MY_MUTEX_INIT_FAST);
  });

#ifdef USE_DEVEL_KEY
  ROCKS_LOG_WARN(
      logger_, "KeyringMasterKeyManger uses hardcoded development master key.");
#endif

  InitKeyringServices();
  serverUuid_ = encInfoStorage_->GetServerUuid();
  newestMasterKeyId_ = encInfoStorage_->GetCurrentMasterKeyId();
  Recover();
}

KeyringMasterKeyManager::~KeyringMasterKeyManager() { DeinitKeyringServices(); }

void KeyringMasterKeyManager::InitKeyringServices() {
  SERVICE_TYPE(registry) *reg_svc = mysql_plugin_registry_acquire();

  if (reg_svc == nullptr) {
    ROCKS_LOG_ERROR(logger_, "Failed to get plugin registry");
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

    ROCKS_LOG_ERROR(logger_, "Failed initializing keyring services");
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
  SERVICE_TYPE(registry) *reg_svc = mysql_plugin_registry_acquire();

  if (reg_svc == nullptr) {
    ROCKS_LOG_ERROR(logger_, "Failed to get plugin registry");
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

/* Why we need this cache?
   This is because of MySql components deinitialization sequence, which is
   executed on SHUTDOWN.
   Keyring component is deinitialized before SEs. When RocksDB is going down
   it creates several files, which are to be encrypted as well, so we need to
   cache MK.
   This is not ideal situation as MK will be visible in core dump, ideally we
   would need to keep it in memory as short as possible.
*/
int KeyringMasterKeyManager::GetSecretFromCache(const std::string &keyName,
                                                std::string *secret) {
  std::unique_lock<std::mutex> lock(keysCacheMtx_);
  auto it = keysCache_.find(keyName);
  if (it == keysCache_.end()) {
    return -1;
  }
  secret->assign(it->second);
  return 0;
}

void KeyringMasterKeyManager::StoreSecretInCache(const std::string &keyName,
                                                 const std::string &secret) {
  std::unique_lock<std::mutex> lock(keysCacheMtx_);
  auto it = keysCache_.find(keyName);
  if (it != keysCache_.end()) {
    // It may happen if it is already added if two thread simultaneously
    // accessed it for the 1st time.
    return;
  }

  /* todo: Cache only the newest secret. It should be good,
     as new files are always created with the newest MK, and files already open
     do not need access to MK. (The problem would be if RocksDB opened already
     existing files on its shutdown)
  */
  // keysCache_.clear();

  keysCache_[keyName] = secret;
}

int KeyringMasterKeyManager::ReadSecret(const std::string &keyName,
                                        std::string *secret) {
  if (0 == GetSecretFromCache(keyName, secret)) {
    return 0;
  }

#ifdef USE_DEVEL_KEY
  *secret = "12345678901234567890123456789012";
#else
  size_t secret_length = 0;
  size_t secret_type_length = 0;
  my_h_keyring_reader_object reader_object = nullptr;
  bool retval =
      keyring_reader_service_->init(keyName.c_str(), nullptr, &reader_object);

  /* Keyring error */
  if (retval == true) {
    ROCKS_LOG_ERROR(logger_, "Failed to initialize keyring reader service");
    return -1;
  }
  /* Key absent */
  if (reader_object == nullptr) return -1;

  auto cleanup_guard = create_scope_guard([&] {
    if (reader_object != nullptr)
      (void)keyring_reader_service_->deinit(reader_object);
    reader_object = nullptr;
  });

  /* Fetch length */
  if (keyring_reader_service_->fetch_length(reader_object, &secret_length,
                                            &secret_type_length) == true) {
    ROCKS_LOG_ERROR(
        logger_, "Failed to fetch secret length from keyring reader service");
    return -1;
  }

  if (secret_length == 0 || secret_type_length == 0) return -1;

  /* Allocate requried memory for key and secret_type */
  std::vector<unsigned char> secret_v;
  std::vector<char> secret_type_v;
  secret_v.reserve(secret_length);
  secret_type_v.reserve(secret_type_length + 1);
  memset(secret_v.data(), 0, secret_length);
  memset(secret_type_v.data(), 0, secret_type_length + 1);

  if (keyring_reader_service_->fetch(reader_object, secret_v.data(),
                                     secret_length, &secret_length,
                                     secret_type_v.data(), secret_type_length,
                                     &secret_type_length) == true) {
    ROCKS_LOG_ERROR(logger_,
                    "Failed to fetch secret from keyring reader service");
    return -1;
  }
  secret->assign((char *)(secret_v.data()), secret_length);

#endif  /* USE_DEVEL_KEY */

  StoreSecretInCache(keyName, *secret);

  return 0;
}

int KeyringMasterKeyManager::GetMostRecentMasterKey(std::string *masterKey,
                                                    uint32_t *masterKeyId) {
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
    /* If newestMasterKeyId_ is DEFAULT_MASTER_KEY_ID, it means there's
    no encrypted file yet. Generate the first master key now and store
    it to keyring. */

    // This is the key with id '1'
    auto keyName = CreateKeyName(1);
    encInfoStorage_->StoreMasterKeyRotationInProgress(true);

#ifdef USE_DEVEL_KEY
    // Nothing here. "key" generation is handled by ReadSecret()
#else
    // We call keyring API to generate master key here.
    if (keyring_generator_service_->generate(
            keyName.c_str(), nullptr, rocksdb_key_type, KEY_LEN) == true) {
      if (key_id_locked) {
        RDB_MUTEX_UNLOCK_CHECK(master_key_id_mutex_);
      }
      ROCKS_LOG_ERROR(logger_,
                      "MasterKey generation FAILED. Check if Keyring component "
                      "is installed.");
      return -2;
    }
#endif /* USE_DEVEL_KEY */

    // We call keyring API to get master key here. It will cache the new key.
    retval = ReadSecret(keyName, masterKey);

    if (retval > -1 && masterKey->length() > 0) {
      ++newestMasterKeyId_;
      *masterKeyId = newestMasterKeyId_;
      encInfoStorage_->StoreCurrentMasterKeyId(newestMasterKeyId_);
      encInfoStorage_->StoreMasterKeyRotationInProgress(false);
    }
  } else {
    *masterKeyId = newestMasterKeyId_;

    // We call keyring API to get master key here.
    retval = ReadSecret(CreateKeyName(newestMasterKeyId_), masterKey);
  }

  if (retval == -1) {
    masterKey->clear();
    ROCKS_LOG_ERROR(logger_,
                    "Encryption can't find master key, please check that the "
                    "keyring is installed.");
  }

  if (key_id_locked) {
    RDB_MUTEX_UNLOCK_CHECK(master_key_id_mutex_);
  }

  return retval;
}

void KeyringMasterKeyManager::GetServerUuid(std::string *serverUuid) {
  *serverUuid = serverUuid_;
}

void KeyringMasterKeyManager::GetMasterKeyInfo(uint32_t *oldestMasterKeyId,
    uint32_t *newestMasterKeyId, std::string *serverUuid) {
  *oldestMasterKeyId = oldestMasterKeyId_;
  *newestMasterKeyId = newestMasterKeyId_;
  *serverUuid = serverUuid_;
}

int KeyringMasterKeyManager::GetMasterKey(uint32_t masterKeyId,
                                          const std::string &suuid,
                                          std::string *masterKey) {
  if (masterKeyId < oldestMasterKeyId_) {
    oldestMasterKeyId_ = masterKeyId;
  }

  return ReadSecret(CreateKeyName(masterKeyId), masterKey);
}

int KeyringMasterKeyManager::GenerateNewMasterKey() {
  RDB_MUTEX_LOCK_CHECK(master_key_id_mutex_);
  encInfoStorage_->StoreMasterKeyRotationInProgress(true);

  uint32_t newMasterKeyId = newestMasterKeyId_ + 1;

  // We call keyring API to generate master key here.
  if (keyring_generator_service_->generate(
          CreateKeyName(newMasterKeyId).c_str(), nullptr, rocksdb_key_type,
          KEY_LEN) == true) {
    RDB_MUTEX_UNLOCK_CHECK(master_key_id_mutex_);
    ROCKS_LOG_ERROR(logger_,
                    "MasterKey generation FAILED. Check if Keyring component "
                    "is installed.");
    return -1;
  }

  // We call keyring API to get master key here. It will store the key in the cache.
  std::string masterKey;
  auto retval = ReadSecret(CreateKeyName(newMasterKeyId).c_str(), &masterKey);

  if (retval < 0 || masterKey.length() == 0) {
    RDB_MUTEX_UNLOCK_CHECK(master_key_id_mutex_);
    return -1;
  }

  encInfoStorage_->StoreCurrentMasterKeyId(newMasterKeyId);
  newestMasterKeyId_ = newMasterKeyId;

  encInfoStorage_->StoreMasterKeyRotationInProgress(false);
  RDB_MUTEX_UNLOCK_CHECK(master_key_id_mutex_);
  return 0;
}

const std::string KeyringMasterKeyManager::CreateKeyName(uint32_t keyId) const {
  static const std::string MASTER_KEY_PREFIX = "ROCKSDBKey-";
  static const std::string MASTER_KEY_SEPARATOR = "-";

  return MASTER_KEY_PREFIX + serverUuid_ + MASTER_KEY_SEPARATOR +
         std::to_string(keyId);
}

void KeyringMasterKeyManager::Recover() {
    RDB_MUTEX_LOCK_CHECK(master_key_id_mutex_);
    if (encInfoStorage_->GetMasterKeyRotationInProgress()) {
        auto masterKeyId = encInfoStorage_->GetCurrentMasterKeyId();
        std::string masterKey;

        // check if we failed after rotation.
        // This means that new key was generated, but its Id was not stored
        auto retval = ReadSecret(CreateKeyName(masterKeyId+1).c_str(), &masterKey);
        if (!retval) {
            // new key was generated, but info was not stored
            newestMasterKeyId_ = masterKeyId+1;
            encInfoStorage_->StoreCurrentMasterKeyId(newestMasterKeyId_);
        }
        encInfoStorage_->StoreMasterKeyRotationInProgress(false);
    }
    RDB_MUTEX_UNLOCK_CHECK(master_key_id_mutex_);
}

}  // namespace myrocks