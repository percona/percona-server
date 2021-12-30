#pragma once

#include <string>

namespace myrocks {

class EncryptionInfoStorage {
public:
    virtual void StoreCurrentMasterKeyId(uint32_t Id) = 0;

    // returns 0 if no id stored
    virtual uint32_t GetCurrentMasterKeyId() = 0;

    virtual std::string GetServerUuid() = 0;

};

} // namespace myrocks