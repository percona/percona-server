#include "./keyring_master_key_manager.h"

#include "mysql/components/my_service.h"
#include "mysql/components/services/registry.h"

#include <mysql/service_plugin_registry.h>
#include <mysql/components/services/keyring_generator.h>
#include <mysql/components/services/keyring_reader_with_status.h>
#include <mysql/components/services/keyring_writer.h>


namespace myrocks {


KeyringMasterKeyManager::KeyringMasterKeyManager()
: oldestMasterKeyId_((uint32_t)-1)
, newestMasterKeyId_(0) {
  InitServices();
}

KeyringMasterKeyManager::~KeyringMasterKeyManager() {

}


void KeyringMasterKeyManager::InitServices()
{
  SERVICE_TYPE(registry) * reg_svc = mysql_plugin_registry_acquire();
  reg_svc = reg_svc;

}


int KeyringMasterKeyManager::GetMostRecentMasterKey(std::string *masterKey, uint32_t *masterKeyId) {
    if (newestMasterKeyId_ == 0) {
        // there are no encrypted files and we are on default MK id
        // generate the new master key
        newestMasterKeyId_++;
    }
    *masterKey = "12345678901234567890123456789012";
    *masterKeyId = newestMasterKeyId_;
    return 0;
}

int KeyringMasterKeyManager::GetServerUuid(std::string *serverUuid) {
    *serverUuid = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
    return 0;
}

int KeyringMasterKeyManager::GetMasterKey(uint32_t masterKeyId, const std::string &suuid, std::string *masterKey) {
    *masterKey = "12345678901234567890123456789012";
    return 0;
}

void KeyringMasterKeyManager::RegisterMasterKeyId(uint32_t masterKeyId) {
    if (masterKeyId > newestMasterKeyId_)
      newestMasterKeyId_ = masterKeyId;
    if(masterKeyId < oldestMasterKeyId_)
      oldestMasterKeyId_ = masterKeyId;
}

}  // namespace