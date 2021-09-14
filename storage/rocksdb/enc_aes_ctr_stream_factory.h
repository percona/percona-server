#pragma once

#include "./enc_cipher_stream_factory.h"

namespace myrocks {

class AesCtrStreamFactory : public CipherStreamFactory {
 public:
  virtual std::unique_ptr<rocksdb::BlockAccessCipherStream> CreateCipherStream(
      const std::string &fileKey, const std::string &iv) override;
};

}  // namespace myrocks