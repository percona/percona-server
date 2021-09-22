#pragma once

#include <memory>
#include "./enc_cipher_stream_factory.h"

namespace rocksdb {
class Logger;
}

namespace myrocks {

class AesCtrStreamFactory : public CipherStreamFactory {
 public:
  AesCtrStreamFactory(std::shared_ptr<rocksdb::Logger> logger);
  virtual std::unique_ptr<rocksdb::BlockAccessCipherStream> CreateCipherStream(
      const std::string &fileKey, const std::string &iv) override;
  virtual std::unique_ptr<rocksdb::BlockAccessCipherStream>
  CreateThreadSafeCipherStream(const std::string &fileKey,
                               const std::string &iv) override;

 private:
  std::shared_ptr<rocksdb::Logger> logger_;
};

}  // namespace myrocks