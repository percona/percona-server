#pragma once

#include <memory>
#include <string>

namespace rocksdb {
    class FileSystem;
    class Logger;
}

namespace myrocks {

class EncryptionUuidProvider {
public:
  EncryptionUuidProvider(std::shared_ptr<rocksdb::Logger> logger);

  bool Init(const std::string &filePath,
    const std::shared_ptr<rocksdb::FileSystem> fs,
    const std::string &uuidHint);

  const std::string& GetServerUuid() const;

private:
  std::string serverUuid_;
  std::shared_ptr<rocksdb::Logger> logger_;
};

}  // namespace myrocks