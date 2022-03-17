#include "enc_uuid_provider.h"
#include "rocksdb/file_system.h"
#include "rocksdb/logging/logging.h"
#include "rocksdb/slice.h"

namespace myrocks {
using namespace rocksdb;

static constexpr size_t uuid_strlen = strlen("9ddb9c4d-c4bf-4db3-9303-0e52470a484f");

EncryptionUuidProvider::EncryptionUuidProvider(std::shared_ptr<rocksdb::Logger> logger)
  : logger_(logger) {
}

bool EncryptionUuidProvider::Init(const std::string &filePath,
  const std::shared_ptr<rocksdb::FileSystem> fs,
  const std::string &uuidHint) {

  auto status = fs->FileExists(filePath, rocksdb::IOOptions(), nullptr);
  if (!status.ok()) {
    ROCKS_LOG_INFO(logger_, "EncryptionUuidProvider::Init().  %s file not found.", filePath.c_str());

    std::unique_ptr<rocksdb::FSWritableFile> file;
    status = fs->NewWritableFile(filePath, rocksdb::FileOptions(), &file, nullptr);
    if (!status.ok()) {
      ROCKS_LOG_WARN(logger_, "EncryptionUuidProvider::Init(). Failed to create %s file.",
            filePath.c_str());
      return false;
    }
    auto data = rocksdb::Slice(uuidHint);
    status = file->Append(data, rocksdb::IOOptions(), nullptr);
    if (!status.ok()) {
      ROCKS_LOG_WARN(logger_, "EncryptionUuidProvider::Init(). Failed to append to %s file.",
        filePath.c_str());
      return false;
    }
    status = file->Close(rocksdb::IOOptions(), nullptr);
    if (!status.ok()) {
    ROCKS_LOG_WARN(logger_, "EncryptionUuidProvider::Init(). Failed to close %s file.",
        filePath.c_str());
      return false;
    }

    serverUuid_ = uuidHint;
  } else {
    ROCKS_LOG_INFO(logger_, "EncryptionUuidProvider::Init().  %s file found.", filePath.c_str());

    std::unique_ptr<rocksdb::FSSequentialFile> file;
    rocksdb::Slice data;
    auto status = fs->NewSequentialFile(filePath, rocksdb::FileOptions(), &file, nullptr);
    if (!status.ok()){
      ROCKS_LOG_WARN(logger_, "EncryptionUuidProvider::Init(). Failed to open %s file.", filePath.c_str());
      return false;
    }
    char buf[uuid_strlen];
    status = file->Read(uuid_strlen, rocksdb::IOOptions(), &data, buf, nullptr);
    if (!status.ok()) {
      ROCKS_LOG_WARN(logger_, "EncryptionUuidProvider::Init(). Reading from file %s failed.",
        filePath.c_str());
      return false;
    }
    serverUuid_ = data.ToString();
  }

  return true;
}

const std::string& EncryptionUuidProvider::GetServerUuid() const {
    return serverUuid_;
}

}  // namespace myrocks