#pragma once

#include "./enc_master_key_manager.h"
#include "mysql/components/my_service.h"
#include "mysql/components/services/registry.h"

#include <mysql/service_plugin_registry.h>
#include <mysql/components/services/keyring_generator.h>
#include <mysql/components/services/keyring_reader_with_status.h>
#include <mysql/components/services/keyring_writer.h>


namespace myrocks {

class KeyringMasterKeyManager : public MasterKeyManager {
    public:
        KeyringMasterKeyManager(const std::string& uuid);
        ~KeyringMasterKeyManager() override;

        int GetMostRecentMasterKey(std::string *masterKey, uint32_t *masterKeyId) override;
        int GetMasterKey(uint32_t masterKeyId, const std::string &suuid, std::string *masterKey) override;
        int GetServerUuid(std::string *serverUuid) override;
        virtual int GenerateNewMasterKey() override;

        void RegisterMasterKeyId(uint32_t masterKeyId, const std::string& serverUuid) override;

    private:
        void InitKeyringServices();
        void DeinitKeyringServices();
        int ReadSecret(const std::string& keyName, std::string* secret);

        SERVICE_TYPE(keyring_reader_with_status) *keyring_reader_service_{nullptr};
        SERVICE_TYPE(keyring_writer) *keyring_writer_service_{nullptr};
        SERVICE_TYPE(keyring_generator) *keyring_generator_service_{nullptr};

        uint32_t oldestMasterKeyId_;
        uint32_t newestMasterKeyId_;
        std::string serverUuid_;
        std::string seedUuid_;;
};

}  // namespace