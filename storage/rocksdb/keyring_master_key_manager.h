#pragma once

#include "rocksdb/env/master_key_manager.h"


namespace myrocks {

class KeyringMasterKeyManager : public rocksdb::MasterKeyManager {
    public:
        KeyringMasterKeyManager();
        ~KeyringMasterKeyManager() override;

        int GetMostRecentMasterKey(std::string *masterKey, uint32_t *masterKeyId) override;
        int GetMasterKey(uint32_t masterKeyId, const std::string &suuid, std::string *masterKey) override;
        int GetServerUuid(std::string *serverUuid) override;

        void RegisterMasterKeyId(uint32_t masterKeyId) override;
    private:
        void InitServices();
        
        uint32_t oldestMasterKeyId_;
        uint32_t newestMasterKeyId_;
};



}  // namespace