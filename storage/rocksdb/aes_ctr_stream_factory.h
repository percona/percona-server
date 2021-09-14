#pragma once

#include "./cipher_stream_factory.h"

namespace myrocks {

class AESCtrStreamFactory : public CipherStreamFactory {
  public:
    virtual std::unique_ptr<rocksdb::BlockAccessCipherStream>
      CreateCipherStream(const std::string& fileKey, const std::string& iv) override;
};

} // namespace