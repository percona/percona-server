//  Copyright (c) 2016-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#pragma once

#include "./enc_env_encryption_myrocks.h"

namespace myrocks {

class MasterKeyManager;
class CipherStreamFactory;


class AesCtrEncryptionProvider : public MyRocksEncryptionProvider {
 private:

 protected:
  // For optimal performance when using direct IO, the prefix length should be a
  // multiple of the page size. This size is to ensure the first real data byte
  // is placed at largest known alignment point for direct io.
  const static size_t defaultPrefixLength = 4096;

  std::shared_ptr<MasterKeyManager> masterKeyManager_;
  std::unique_ptr<CipherStreamFactory> cipherStreamFactory_;

 public:
  AesCtrEncryptionProvider(std::shared_ptr<MasterKeyManager> mmm,
                           std::unique_ptr<CipherStreamFactory> csf);
  ~AesCtrEncryptionProvider() override;

  static const char* kCTRAesProviderName;

  const char* Name() const override;

  // GetPrefixLength returns the length of the prefix that is added to every
  // file
  // and used for storing encryption options.
  // For optimal performance when using direct IO, the prefix length should be a
  // multiple of the page size.
  size_t GetPrefixLength() const override;

  // CreateNewPrefix initialized an allocated block of prefix memory
  // for a new file.
  rocksdb::Status CreateNewPrefix(const std::string& fname, char* prefix,
                         size_t prefixLength) const override;

  rocksdb::Status ReencryptPrefix(rocksdb::Slice& prefix) const override;

  // CreateCipherStream creates a block access cipher stream for a file given
  // given name and options.
  rocksdb::Status CreateCipherStream(
      const std::string& fname, const rocksdb::EnvOptions& options, rocksdb::Slice& prefix,
      std::unique_ptr<rocksdb::BlockAccessCipherStream>* result) override;

  rocksdb::Status AddCipher(const std::string& descriptor, const char* /*cipher*/,
                   size_t /*len*/, bool /*for_write*/) override;

  std::string GetMarker() const override;

  rocksdb::Status Feed(rocksdb::Slice& prefix) override;

 protected:
  // CreateCipherStreamFromPrefix creates a block access cipher stream for a
  // file given
  // given name and options. The given prefix is already decrypted.
  virtual rocksdb::Status CreateCipherStreamFromPrefix(
      const rocksdb::Slice& key, const rocksdb::Slice& iv,
      std::unique_ptr<rocksdb::BlockAccessCipherStream>* result);
};
}  // namespace

