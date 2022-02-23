#pragma once
#include <memory>

// Right now keyring_encryption_service does not provide AES CRT mode
// We introduce this abstraction layer to be able to provide custom
// implementation right now, but still be open to switch to server encryption
// service if when it provides AES CRT.
namespace rocksdb {
class BlockAccessCipherStream;
}

namespace myrocks {

class CipherStreamFactory {
 public:
  // Create non thread-safe cipher stream.
  virtual std::unique_ptr<rocksdb::BlockAccessCipherStream> CreateCipherStream(
      const std::string &fileKey, const std::string &iv) = 0;

  // Create thread-safe cipher stream.
  // Multiple threads can use it like stream->Decrypt()/stream->Encrypt()
  // concurently.
  virtual std::unique_ptr<rocksdb::BlockAccessCipherStream>
  CreateThreadSafeCipherStream(const std::string &fileKey,
                               const std::string &iv) = 0;
};

}  // namespace myrocks