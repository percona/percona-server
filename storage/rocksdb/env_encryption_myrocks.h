//  Copyright (c) 2016-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#pragma once

#include <string>

#include "rocksdb/env.h"
#include "rocksdb/file_system.h"
#include "rocksdb/rocksdb_namespace.h"
#include "rocksdb/env_encryption.h"

namespace myrocks {

struct ConfigOptions;
class MyRocksEncryptedFileSystem;

class MyRocksEncryptionProvider : public rocksdb::EncryptionProvider {
  public:
    virtual rocksdb::Status Feed(rocksdb::Slice& prefix) = 0;
    virtual rocksdb::Status ReencryptPrefix(rocksdb::Slice& prefix) const = 0;
};

class MyRocksEncryptedFileSystem : public rocksdb::EncryptedFileSystem {
  public:
    MyRocksEncryptedFileSystem(const std::shared_ptr<FileSystem>& base)
      : EncryptedFileSystem(base) {}
    virtual rocksdb::Status Init(const std::string dir) = 0;
    virtual rocksdb::Status RotateEncryptionMasterKey(const std::string dir) = 0;
};

std::shared_ptr<MyRocksEncryptedFileSystem> NewEncryptedFS(
    const std::shared_ptr<rocksdb::FileSystem>& base,
    const std::shared_ptr<MyRocksEncryptionProvider>& provider,
    bool encryptNewFiles, const std::string& dir);

}  // namespace
