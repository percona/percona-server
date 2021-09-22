#include "./enc_aes_ctr_stream_factory.h"
#include <openssl/evp.h>
#include <rocksdb/env_encryption.h>
#include <condition_variable>
#include <mutex>
#include "./enc_stream_cipher.h"
#include "rocksdb/env.h"
#include "rocksdb/logging/logging.h"

// Note: This implementation of BlockAccessCipherStream uses AES-CRT encryption
// with SSL backend

// Right now server's keyring_encryption_service does not provide AES CRT mode.
// We introduce this factory layer to be able to change the implementation
// of encryptor/decryptor to the one using server service, but for now
// we will just use the implementation got from binlog cache encryption.

namespace myrocks {

using rocksdb::InfoLogLevel;

class MyRocksBlockAccessCipherStream : public rocksdb::BlockAccessCipherStream {
 public:
  MyRocksBlockAccessCipherStream(std::shared_ptr<rocksdb::Logger> logger)
      : logger_(logger) {}
  virtual bool Init(const std::string &file_key, const std::string &iv) = 0;

 protected:
  void AllocateScratch(std::string &) override {}
  rocksdb::Status EncryptBlock(uint64_t blockIndex, char *data,
                               char *scratch) override {
    return rocksdb::Status::NotSupported();
  }
  rocksdb::Status DecryptBlock(uint64_t blockIndex, char *data, char *scratch) {
    return rocksdb::Status::NotSupported();
  }

  std::shared_ptr<rocksdb::Logger> logger_;
};

/******************************************************************************/

class AesCtrCipherStream final : public MyRocksBlockAccessCipherStream {
 private:
  std::unique_ptr<Stream_cipher> encryptor_;
  std::unique_ptr<Stream_cipher> decryptor_;

 public:
  AesCtrCipherStream(std::shared_ptr<rocksdb::Logger> logger);
  virtual ~AesCtrCipherStream();

  // BlockSize returns the size of each block supported by this cipher stream.
  size_t BlockSize() override;

  // Encrypt one or more (partial) blocks of data at the file offset.
  // Length of data is given in dataSize.
  rocksdb::Status Encrypt(uint64_t fileOffset, char *data,
                          size_t dataSize) override;

  // Decrypt one or more (partial) blocks of data at the file offset.
  // Length of data is given in dataSize.
  rocksdb::Status Decrypt(uint64_t fileOffset, char *data,
                          size_t dataSize) override;

  bool Init(const std::string &file_key, const std::string &iv) override;
};

/******************************************************************************/
AesCtrCipherStream::AesCtrCipherStream(std::shared_ptr<rocksdb::Logger> logger)
    : MyRocksBlockAccessCipherStream(logger) {}

AesCtrCipherStream::~AesCtrCipherStream() {
  encryptor_->close();
  decryptor_->close();
}

bool AesCtrCipherStream::Init(const std::string &file_key,
                              const std::string &iv) {
  // encryptor
  auto encryptor = Aes_ctr::get_encryptor();
  auto openRes =
      encryptor->open(reinterpret_cast<const unsigned char *>(file_key.c_str()),
                      reinterpret_cast<const unsigned char *>(iv.c_str()));
  if (openRes) {
    ROCKS_LOG_ERROR(logger_, "Encryptor open failed");
    return openRes;
  }

  // decryptor
  auto decryptor = Aes_ctr::get_decryptor();
  openRes =
      decryptor->open(reinterpret_cast<const unsigned char *>(file_key.c_str()),
                      reinterpret_cast<const unsigned char *>(iv.c_str()));
  if (openRes) {
    ROCKS_LOG_ERROR(logger_, "Decryptor open failed");
    return openRes;
  }

  encryptor_ = std::move(encryptor);
  decryptor_ = std::move(decryptor);
  return false;
}

size_t AesCtrCipherStream::BlockSize() {
  // this method is not used
  return 0;
}

rocksdb::Status AesCtrCipherStream::Encrypt(uint64_t fileOffset, char *data,
                                            size_t dataSize) {
  // Offset tracking to avoid underlaying cipher reinitialization is done
  // inside encryptor_
  if (encryptor_->set_stream_offset(fileOffset)) {
    ROCKS_LOG_ERROR(logger_, "Set stream offset for encryption failed");
    return rocksdb::Status::IOError();
  }
  if (encryptor_->encrypt((unsigned char *)data, (const unsigned char *)data,
                          dataSize)) {
    ROCKS_LOG_ERROR(logger_, "Encryption failed");
    return rocksdb::Status::IOError();
  }
  return rocksdb::Status::OK();
}

rocksdb::Status AesCtrCipherStream::Decrypt(uint64_t fileOffset, char *data,
                                            size_t dataSize) {
  // Offset tracking to avoid underlaying cipher reinitialization is done
  // inside decryptor_
  if (decryptor_->set_stream_offset(fileOffset)) {
    ROCKS_LOG_ERROR(logger_, "Set stream offset for decryption failed");
    return rocksdb::Status::IOError();
  }
  if (decryptor_->decrypt((unsigned char *)data, (const unsigned char *)data,
                          dataSize)) {
    ROCKS_LOG_ERROR(logger_, "Decryption failed");
    return rocksdb::Status::IOError();
  }
  return rocksdb::Status::OK();
}

/******************************************************************************/
/* thread safe implementation, necessary for RandomReadAccessFile->read() */
class CipherManager {
 public:
  enum CipherType { ENCRYPTOR, DECRYPTOR };

  CipherManager(size_t maxCiphers, CipherType type, const std::string &key,
                const std::string &iv)
      : max_ciphers_(maxCiphers),
        ciphers_count_(0),
        cipher_type_(type),
        key_(key),
        iv_(iv),
        ciphers_mutex_(),
        cv_() {}

  ~CipherManager() {
    std::unique_lock<std::mutex> lock(ciphers_mutex_);
    ciphers_.clear();
  }

  std::unique_ptr<Stream_cipher> CreateNewCipher() {
    std::unique_ptr<Stream_cipher> cipher;

    if (cipher_type_ == ENCRYPTOR) {
      cipher = Aes_ctr::get_encryptor();
    } else {
      cipher = Aes_ctr::get_decryptor();
    }
    auto openRes =
        cipher->open(reinterpret_cast<const unsigned char *>(key_.c_str()),
                     reinterpret_cast<const unsigned char *>(iv_.c_str()));
    if (openRes) return nullptr;
    return cipher;
  }

  std::unique_ptr<Stream_cipher> AllocateCipher(uint64_t offsetHint) {
    std::unique_lock<std::mutex> lock(ciphers_mutex_);
    // try to find the cipher which is at the requested offset
    auto it = ciphers_.find(offsetHint);
    if (it != ciphers_.end()) {
      // free cipher at the requested position found
      auto res = std::move(it->second);
      ciphers_.erase(it);
      return res;
    }

    // if there is no cipher at the requested position, use the 1st one
    it = ciphers_.begin();
    if (it != ciphers_.end()) {
      auto res = std::move(it->second);
      ciphers_.erase(it);
      return res;
    }

    // If there are not free ciphers, check if we already have max number
    // of ciphers allocated. If not - alllocate new one
    if (ciphers_count_ < max_ciphers_) {
      auto res = CreateNewCipher();
      if (res != nullptr) {
        // new cipher allocated
        ciphers_count_++;
        return res;
      }
    }

    // if we got here it means that max count of ciphers were already allocated
    // and there is no free cipher to be used. We just need to wait

    cv_.wait(lock, [this] { return ciphers_.size() > 0; });
    // a cipher has been deallocated. Use the first one
    it = ciphers_.begin();
    auto res = std::move(it->second);
    ciphers_.erase(it);
    return res;
  }

  void DeallocateCipher(std::unique_ptr<Stream_cipher> cipher,
                        uint64_t offsetHint) {
    std::unique_lock<std::mutex> lock(ciphers_mutex_);
    ciphers_.insert(std::pair<uint64_t, std::unique_ptr<Stream_cipher>>(
        offsetHint, std::move(cipher)));
    cv_.notify_one();
  }

 private:
  const size_t max_ciphers_;
  size_t ciphers_count_;
  CipherType cipher_type_;
  const std::string key_;
  const std::string iv_;
  std::mutex ciphers_mutex_;
  std::condition_variable cv_;
  std::multimap<uint64_t, std::unique_ptr<Stream_cipher>> ciphers_;
};

class AesCtrCipherStreamTS final : public MyRocksBlockAccessCipherStream {
 private:
  std::unique_ptr<CipherManager> encryptorManager_;
  std::unique_ptr<CipherManager> decryptorManager_;

 public:
  AesCtrCipherStreamTS(std::shared_ptr<rocksdb::Logger> logger);
  virtual ~AesCtrCipherStreamTS();

  // BlockSize returns the size of each block supported by this cipher stream.
  size_t BlockSize() override;

  // Encrypt one or more (partial) blocks of data at the file offset.
  // Length of data is given in dataSize.
  rocksdb::Status Encrypt(uint64_t fileOffset, char *data,
                          size_t dataSize) override;

  // Decrypt one or more (partial) blocks of data at the file offset.
  // Length of data is given in dataSize.
  rocksdb::Status Decrypt(uint64_t fileOffset, char *data,
                          size_t dataSize) override;

  bool Init(const std::string &file_key, const std::string &iv) override;
};

AesCtrCipherStreamTS::AesCtrCipherStreamTS(
    std::shared_ptr<rocksdb::Logger> logger)
    : MyRocksBlockAccessCipherStream(logger) {}

AesCtrCipherStreamTS::~AesCtrCipherStreamTS() {
  encryptorManager_.release();
  decryptorManager_.release();
}

bool AesCtrCipherStreamTS::Init(const std::string &file_key,
                                const std::string &iv) {
  // encryptor
  encryptorManager_ = std::make_unique<CipherManager>(
      10, CipherManager::ENCRYPTOR, file_key, iv);

  // decryptor
  decryptorManager_ = std::make_unique<CipherManager>(
      10, CipherManager::DECRYPTOR, file_key, iv);

  return false;
}

size_t AesCtrCipherStreamTS::BlockSize() {
  // this method is not used
  return 0;
}

// #define VALIDATE_ENCRYPT
rocksdb::Status AesCtrCipherStreamTS::Encrypt(uint64_t fileOffset, char *data,
                                              size_t dataSize) {
#ifdef VALIDATE_ENCRYPT
  std::vector<unsigned char> originalData;
  originalData.reserve(dataSize);
  memcpy(originalData.data(), data, dataSize);
#endif
  // Offset tracking to avoid underlaying cipher reinitialization is done
  // inside encryptor_
  auto encryptor = encryptorManager_->AllocateCipher(fileOffset);
  if (encryptor->set_stream_offset(fileOffset)) {
    encryptorManager_->DeallocateCipher(std::move(encryptor), (size_t)-1);
    ROCKS_LOG_ERROR(logger_, "Set stream offset for encryption failed");
    return rocksdb::Status::IOError();
  }
  if (encryptor->encrypt((unsigned char *)data, (const unsigned char *)data,
                         dataSize)) {
    encryptorManager_->DeallocateCipher(std::move(encryptor), (size_t)-1);
    ROCKS_LOG_ERROR(logger_, "Encryption failed");
    return rocksdb::Status::IOError();
  }
  encryptorManager_->DeallocateCipher(std::move(encryptor), fileOffset);

#ifdef VALIDATE_ENCRYPT
  std::vector<unsigned char> vec;
  vec.reserve(dataSize);
  auto decryptor = decryptorManager_->AllocateCipher(fileOffset);
  decryptor->set_stream_offset(fileOffset);
  auto decrypted = vec.data();
  decryptor->decrypt(decrypted, (const unsigned char *)data, dataSize);
  decryptorManager_->DeallocateCipher(std::move(decryptor), fileOffset);
  if (memcmp(decrypted, originalData.data(), dataSize)) {
    ROCKS_LOG_ERROR(logger_, "Encryption failed");
  }
#endif
  return rocksdb::Status::OK();
}

rocksdb::Status AesCtrCipherStreamTS::Decrypt(uint64_t fileOffset, char *data,
                                              size_t dataSize) {
  // Offset tracking to avoid underlaying cipher reinitialization is done
  // inside decryptor_
  auto decryptor = decryptorManager_->AllocateCipher(fileOffset);
  if (decryptor->set_stream_offset(fileOffset)) {
    decryptorManager_->DeallocateCipher(std::move(decryptor), (size_t)-1);
    ROCKS_LOG_ERROR(logger_, "Set stream offset for decryption failed");
    return rocksdb::Status::IOError();
  }
  if (decryptor->decrypt((unsigned char *)data, (const unsigned char *)data,
                         dataSize)) {
    decryptorManager_->DeallocateCipher(std::move(decryptor), (size_t)-1);
    ROCKS_LOG_ERROR(logger_, "Decryption failed");
    return rocksdb::Status::IOError();
  }

  decryptorManager_->DeallocateCipher(std::move(decryptor), fileOffset);

  return rocksdb::Status::OK();
}

/******************************************************************************/

AesCtrStreamFactory::AesCtrStreamFactory(
    std::shared_ptr<rocksdb::Logger> logger)
    : logger_(logger) {}

std::unique_ptr<rocksdb::BlockAccessCipherStream>
AesCtrStreamFactory::CreateThreadSafeCipherStream(const std::string &fileKey,
                                                  const std::string &iv) {
  std::unique_ptr<MyRocksBlockAccessCipherStream> cipher =
      std::make_unique<AesCtrCipherStreamTS>(logger_);

  if (cipher == nullptr || cipher->Init(fileKey, iv)) {
    ROCKS_LOG_ERROR(logger_, "Thread safe cipher creation failed");
    return nullptr;
  }
  return cipher;
}

std::unique_ptr<rocksdb::BlockAccessCipherStream>
AesCtrStreamFactory::CreateCipherStream(const std::string &fileKey,
                                        const std::string &iv) {
  std::unique_ptr<MyRocksBlockAccessCipherStream> cipher =
      std::make_unique<AesCtrCipherStream>(logger_);

  if (cipher == nullptr || cipher->Init(fileKey, iv)) {
    ROCKS_LOG_ERROR(logger_, "Cipher creation failed");
    return nullptr;
  }
  return cipher;
}

}  // namespace myrocks