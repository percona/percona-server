#pragma once

#include <string>

namespace myrocks {

class EncryptionInfoStorage {
public:
    virtual void StoreCurrentMasterKeyId(uint32_t Id) = 0;
    virtual void StoreMasterKeyRotationInProgress(bool flag) = 0;
    virtual bool GetMasterKeyRotationInProgress() = 0;

    // returns 0 if no ID stored
    virtual uint32_t GetCurrentMasterKeyId() = 0;

    virtual std::string GetServerUuid() = 0;
};

} // namespace myrocks