#pragma once

#include <string>

#include "rocksdb/env.h"
#include "rocksdb/env_encryption.h"
#include "rocksdb/file_system.h"
#include "rocksdb/rocksdb_namespace.h"

namespace myrocks {

struct ConfigOptions;
class MyRocksEncryptedFileSystem;

class MyRocksEncryptionProvider : public rocksdb::EncryptionProvider {
 public:
  virtual rocksdb::Status CreateThreadSafeCipherStream(
      const std::string &fname, const rocksdb::EnvOptions &options,
      rocksdb::Slice &prefix,
      std::unique_ptr<rocksdb::BlockAccessCipherStream> *result) = 0;
  virtual bool IsPrefixOK(const rocksdb::Slice &prefix) = 0;
};

class MyRocksEncryptedFileSystem : public rocksdb::EncryptedFileSystem {
 public:
  MyRocksEncryptedFileSystem(const std::shared_ptr<FileSystem> &base)
      : EncryptedFileSystem(base) {}
};

std::shared_ptr<MyRocksEncryptedFileSystem> NewEncryptedFS(
    const std::shared_ptr<rocksdb::FileSystem> &base,
    const std::shared_ptr<MyRocksEncryptionProvider> &provider,
    std::atomic_bool &encryptNewFiles,
    std::shared_ptr<rocksdb::Logger> logger);

}  // namespace myrocks
