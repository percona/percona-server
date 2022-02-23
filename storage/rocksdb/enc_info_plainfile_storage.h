#pragma once

#include <string>
#include <memory>
#include <mutex>
#include "./enc_info_storage.h"

namespace rocksdb {
    class FileSystem;
    class Logger;
}

namespace myrocks {

// This implementation uses plain text file to store the information
// The file access is protected by single mutex, but it is not a problem
// at this moment, as the storage is used only when MK rotation happens.
// If we need it to be scalable, consider using db (RocksDB?) as the backend.
class EncryptionInfoPlainFileStorage : public EncryptionInfoStorage {
public:
    EncryptionInfoPlainFileStorage(const std::string& filePath,
    const std::shared_ptr<rocksdb::FileSystem> fs, const std::string &uuidHint,
    std::shared_ptr<rocksdb::Logger> logger);

    void StoreCurrentMasterKeyId(uint32_t Id) override;
    void StoreMasterKeyRotationInProgress(bool flag) override;
    bool GetMasterKeyRotationInProgress() override;

    // returns 0 if no ID stored
    uint32_t GetCurrentMasterKeyId() override;

    std::string GetServerUuid() override;

private:
   struct EncryptionInfo {
       uint32_t CurrentMasterKeyId;
       std::string ServerUuid;
       uint32_t MasterKeyRotationInProgress;
       uint32_t CRC;
   };
   void serialize(const std::string &filePath, std::shared_ptr<EncryptionInfo> info, bool backup = true);
   std::shared_ptr<EncryptionInfo> deserialize(const std::string &filePath);
   void recover();

   std::string filePath_;
   std::string backupFilePath_;
   const std::shared_ptr<rocksdb::FileSystem> fs_;
   std::string uuidHint_;
   std::mutex fileAccessMtx_;
   std::shared_ptr<EncryptionInfo> encryptionInfo_;
   std::shared_ptr<rocksdb::Logger> logger_;
};

} // namespace myrocks