#include "./enc_info_plainfile_storage.h"
#include "rocksdb/file_system.h"
#include "rocksdb/util/crc32c.h"
#include "rocksdb/logging/logging.h"

namespace myrocks {
using namespace rocksdb;

static constexpr uint32_t g_serializtion_format_version = 1;
static constexpr const char *g_serialization_format =
  "Version: %u\nMasterKeyId: %u\nMasterKeyRotationInProgress: %u\nServerUuid: %s\nCRC: %u\n";
static constexpr const char *g_CRC_item = "CRC: ";
static constexpr size_t uint32_t_strlen = strlen("4294967295");  // uint32_t max
static constexpr size_t uuid_strlen = strlen("9ddb9c4d-c4bf-4db3-9303-0e52470a484f");
static constexpr size_t g_serialization_buffer_size = strlen(g_serialization_format)
                                                      + uint32_t_strlen // version
                                                      + uint32_t_strlen // MasterKeyId
                                                      + uint32_t_strlen // MasterKeyRotationInProgress
                                                      + uuid_strlen     // uuid
                                                      + uint32_t_strlen // CRC
                                                      + 1;              // '0'
static char g_serialization_buffer[g_serialization_buffer_size];

EncryptionInfoPlainFileStorage::EncryptionInfoPlainFileStorage(
    const std::string& filePath,
    const std::shared_ptr<rocksdb::FileSystem> fs,
    const std::string& uuidHint,
    std::shared_ptr<rocksdb::Logger> logger)
: filePath_(filePath),
  backupFilePath_(filePath_+".bak"),
  fs_(fs),
  uuidHint_(uuidHint),
  logger_(logger)
{
    std::unique_lock<std::mutex> lock(fileAccessMtx_);
    recover();
    encryptionInfo_ = deserialize(filePath_);
}

void EncryptionInfoPlainFileStorage::StoreCurrentMasterKeyId(uint32_t Id)
{
    std::unique_lock<std::mutex> lock(fileAccessMtx_);
    encryptionInfo_->CurrentMasterKeyId = Id;
    serialize(filePath_, encryptionInfo_);
}

uint32_t EncryptionInfoPlainFileStorage::GetCurrentMasterKeyId()
{
    std::unique_lock<std::mutex> lock(fileAccessMtx_);
    if (!encryptionInfo_) encryptionInfo_ = deserialize(filePath_);
    return encryptionInfo_->CurrentMasterKeyId;
}

std::string EncryptionInfoPlainFileStorage::GetServerUuid()
{
    std::unique_lock<std::mutex> lock(fileAccessMtx_);
    if (!encryptionInfo_) encryptionInfo_ = deserialize(filePath_);
    return encryptionInfo_->ServerUuid;
}

void EncryptionInfoPlainFileStorage::StoreMasterKeyRotationInProgress(bool flag){
    std::unique_lock<std::mutex> lock(fileAccessMtx_);
    encryptionInfo_->MasterKeyRotationInProgress = flag;
    serialize(filePath_, encryptionInfo_);
}

bool EncryptionInfoPlainFileStorage::GetMasterKeyRotationInProgress() {
    std::unique_lock<std::mutex> lock(fileAccessMtx_);
    if (!encryptionInfo_) encryptionInfo_ = deserialize(filePath_);
    return encryptionInfo_->MasterKeyRotationInProgress;
}

void EncryptionInfoPlainFileStorage::serialize(const std::string &filePath,
  std::shared_ptr<EncryptionInfoPlainFileStorage::EncryptionInfo> info, bool backup)
{
    if (info){
      if (backup) {
        // 1st, create the backup of current configuration
        serialize(backupFilePath_, deserialize(filePath_), false);
      }

      rocksdb::Slice data;
      std::unique_ptr<rocksdb::FSWritableFile> file;

      snprintf(g_serialization_buffer, g_serialization_buffer_size, g_serialization_format,
        g_serializtion_format_version,
        info->CurrentMasterKeyId,
        info->MasterKeyRotationInProgress,
        info->ServerUuid.c_str(),
        0);

      // crc
      char* ptr = strstr(g_serialization_buffer, g_CRC_item);
      if (ptr) ptr += strlen(g_CRC_item);
      uint32_t crc = rocksdb::crc32c::Extend(0, g_serialization_buffer,
                                         ptr - g_serialization_buffer);

      sprintf(ptr, "%u\n", crc);
      data = rocksdb::Slice(g_serialization_buffer);

      auto status = fs_->NewWritableFile(filePath, rocksdb::FileOptions(), &file, nullptr);
      if (!status.ok()) {
        ROCKS_LOG_WARN(logger_, "EncryptionInfoPlainFileStorage::serialize(). Failed to create %s file.",
            filePath.c_str());
        return;
      }
      status = file->Append(data, rocksdb::IOOptions(), nullptr);
      if (!status.ok()) {
        ROCKS_LOG_WARN(logger_, "EncryptionInfoPlainFileStorage::serialize(). Failed to append to %s file.",
            filePath.c_str());
        return;
      }
      status = file->Close(rocksdb::IOOptions(), nullptr);
      if (!status.ok()) {
        ROCKS_LOG_WARN(logger_, "EncryptionInfoPlainFileStorage::serialize(). Failed to close %s file.",
            filePath.c_str());
        return;
      }
      if (backup) {
        status = fs_->DeleteFile(backupFilePath_, rocksdb::IOOptions(), nullptr);
        if (!status.ok()) {
          ROCKS_LOG_WARN(logger_, "EncryptionInfoPlainFileStorage::serialize(). Failed to delete %s file.",
            filePath.c_str());
          return;
        }
      }
    }
}

std::shared_ptr<EncryptionInfoPlainFileStorage::EncryptionInfo>
EncryptionInfoPlainFileStorage::deserialize(const std::string &filePath)
{
    std::unique_ptr<rocksdb::FSSequentialFile> file;
    rocksdb::Slice data;
    auto newInfo = std::make_shared<EncryptionInfo>();

    auto status = fs_->NewSequentialFile(filePath, rocksdb::FileOptions(), &file, nullptr);
    if (status.ok()){
        status = file->Read(g_serialization_buffer_size, rocksdb::IOOptions(), &data, g_serialization_buffer, nullptr);
        if (!status.ok()) {
            ROCKS_LOG_WARN(logger_, "EncryptionInfoPlainFileStorage::deserialize(). Reading from file %s failed.",
              filePath.c_str());
            return nullptr;
        }
        char uuid[128];
        uint32_t version;
        sscanf(data.data(), g_serialization_format,
                &version,
                &newInfo->CurrentMasterKeyId,
                &newInfo->MasterKeyRotationInProgress,
                uuid,
                &newInfo->CRC);

        // crc check
        char *ptr = strstr(g_serialization_buffer, g_CRC_item);
        if (!ptr) {
            ROCKS_LOG_WARN(logger_, "EncryptionInfoPlainFileStorage::deserialize(). Failed to read CRC.");
            return nullptr;
        }
        ptr += strlen(g_CRC_item);
        uint32_t crc;
        sscanf(ptr, "%u\n", &crc);
        if (newInfo->CRC != crc) {
            ROCKS_LOG_WARN(logger_, "EncryptionInfoPlainFileStorage::deserialize(). CRC mismatch for file %s",
             filePath.c_str());
            return nullptr;
        }

        newInfo->ServerUuid = std::string(uuid);
        if (g_serializtion_format_version != version) {
            ROCKS_LOG_ERROR(logger_, "EncryptionInfoPlainFileStorage::deserialize() failed. Serialization format mismatch. Current: %u, expected: %u",
              version, g_serializtion_format_version);
            return nullptr;
        }
    } else {
      newInfo->CurrentMasterKeyId = 0;
      newInfo->MasterKeyRotationInProgress = 0;
      newInfo->ServerUuid = std::string(uuidHint_);
    }
    return newInfo;
}


void EncryptionInfoPlainFileStorage::recover() {
    auto status = fs_->FileExists(backupFilePath_, rocksdb::IOOptions(), nullptr);
    if (status.ok()) {
        // Check if main file is OK.
        // If it is - use it as it is not older than backup one.
        // If the main file is not OK, recover from backup.
        auto mainInfo = deserialize(filePath_);
        if (mainInfo) {
            status = fs_->DeleteFile(backupFilePath_, rocksdb::IOOptions(), nullptr);
            if (!status.ok()){
                ROCKS_LOG_ERROR(logger_, "EncryptionInfoPlainFileStorage::recover(). Failed deleting backup file.");
            }
            return;
        }

        // main info is not OK, recover from backup
        auto backupInfo = deserialize(backupFilePath_);
        if (!backupInfo) {
            // This means total disaster, and should never happen,
            // as we are modifying main and backup files independently.
        }
        serialize(filePath_, backupInfo, false);
        status = fs_->DeleteFile(backupFilePath_, rocksdb::IOOptions(), nullptr);
        if (!status.ok()){
            ROCKS_LOG_ERROR(logger_, "EncryptionInfoPlainFileStorage::recover(). Failed deleting backup file.");
        }
    }
}
} // namespace myrocks