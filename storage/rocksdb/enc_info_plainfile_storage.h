#pragma once

#include <string>
#include <memory>
#include <mutex>
#include "./enc_info_storage.h"

namespace rocksdb {
    class FileSystem;
}

// This is naive implementation of such a storage. We need something fault-tolerant.
// rdb database instance would be ok for this purpose.
namespace myrocks {
class EncryptionInfoPlainFileStorage : public EncryptionInfoStorage {
public:
    EncryptionInfoPlainFileStorage(const std::string& filePath,
    const std::shared_ptr<rocksdb::FileSystem> fs, const std::string &uuidHint);

    void StoreCurrentMasterKeyId(uint32_t Id) override;

    // returns 0 if no id stored
    uint32_t GetCurrentMasterKeyId() override;

    std::string GetServerUuid() override;

private:
   struct EncryptionInfo {
       uint32_t CurrentMasterKeyId;
       std::string ServerUuid;
   };
   void serialize();
   void deserialize();

   std::string filePath_;
   const std::shared_ptr<rocksdb::FileSystem> fs_;
   std::string uuidHint_;
   std::mutex fileAccessMtx_;
   std::unique_ptr<EncryptionInfo> encryptionInfo_;
};

} // namespace myrocks