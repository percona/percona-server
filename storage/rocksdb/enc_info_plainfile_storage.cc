#include "./enc_info_plainfile_storage.h"
#include "rocksdb/file_system.h"

namespace myrocks {
EncryptionInfoPlainFileStorage::EncryptionInfoPlainFileStorage(
    const std::string& filePath,
    const std::shared_ptr<rocksdb::FileSystem> fs,
    const std::string& uuidHint)
: filePath_(filePath),
  fs_(fs),
  uuidHint_(uuidHint)
{
    std::unique_lock<std::mutex> lock(fileAccessMtx_);
    deserialize();
}

void EncryptionInfoPlainFileStorage::StoreCurrentMasterKeyId(uint32_t Id)
{
    std::unique_lock<std::mutex> lock(fileAccessMtx_);
    encryptionInfo_->CurrentMasterKeyId = Id;
    serialize();
}

    // returns 0 if no id stored
uint32_t EncryptionInfoPlainFileStorage::GetCurrentMasterKeyId()
{
    std::unique_lock<std::mutex> lock(fileAccessMtx_);
    if (!encryptionInfo_) deserialize();
    return encryptionInfo_->CurrentMasterKeyId;
}

std::string EncryptionInfoPlainFileStorage::GetServerUuid()
{
    std::unique_lock<std::mutex> lock(fileAccessMtx_);
    if (!encryptionInfo_) deserialize();
    return encryptionInfo_->ServerUuid;
}

void EncryptionInfoPlainFileStorage::serialize()
{
    if (encryptionInfo_){
      rocksdb::Slice data;
      char buffer[1024];
      std::unique_ptr<rocksdb::FSWritableFile> file;

      snprintf(buffer, 1024, "MasterKeyId:%u\nServerUuid:%s\n",
        encryptionInfo_->CurrentMasterKeyId, encryptionInfo_->ServerUuid.c_str());
      data = rocksdb::Slice(buffer);

      fs_->NewWritableFile(filePath_, rocksdb::FileOptions(), &file, nullptr);
      file->Append(data, rocksdb::IOOptions(), nullptr);
      file->Close(rocksdb::IOOptions(), nullptr);
    }
}

void EncryptionInfoPlainFileStorage::deserialize()
{
    std::unique_ptr<rocksdb::FSSequentialFile> file;
    rocksdb::Slice data;
    char buffer[1024];
    auto newInfo = std::make_unique<EncryptionInfo>();

    auto status = fs_->NewSequentialFile(filePath_, rocksdb::FileOptions(), &file, nullptr);
    if (status.ok()){
        file->Read(1024, rocksdb::IOOptions(), &data, buffer, nullptr);
        uint32_t keyId;
        char uuid[128];
        sscanf(data.data(), "MasterKeyId:%u\nServerUuid:%s\n", &keyId, uuid);
        newInfo->CurrentMasterKeyId = keyId;
        newInfo->ServerUuid = std::string(uuid);
    } else {
      newInfo->CurrentMasterKeyId = 0;
      newInfo->ServerUuid = std::string(uuidHint_);
    }
    encryptionInfo_ = std::move(newInfo);
}

} // namespace myrocks