#include "./master_key_manager.h"

namespace myrocks {


MasterKeyManager::MasterKeyManager() {
}

MasterKeyManager::~MasterKeyManager() {
}

int MasterKeyManager::GetMostRecentMasterKey(std::string *masterKey, uint32_t *masterKeyId) {
    *masterKey = "12345678901234567890123456789012";
    *masterKeyId = 1;
    return 0;
}

int MasterKeyManager::GetServerUuid(std::string *serverUuid) {
    *serverUuid = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
    return 0;
}

int MasterKeyManager::GetMasterKey(uint32_t masterKeyId, const std::string &suuid, std::string *masterKey) {
    *masterKey = "12345678901234567890123456789012";
    return 0;
}

int MasterKeyManager::GenerateNewMasterKey() {
    return 0;
}

void MasterKeyManager::RegisterMasterKeyId(uint32_t masterKeyId, const std::string& serverUuid) {
}

}  // namespace