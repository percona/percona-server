#include "./enc_aes_ctr_stream_factory.h"
#include <openssl/evp.h>
#include <rocksdb/env_encryption.h>
#include "./enc_stream_cipher.h"

// Note: This implementation of BlockAccessCipherStream uses AES-CRT encryption
// with SSL backend

// Right now server's keyring_encryption_service does not provide AES CRT mode.
// We introduce this factory layer to be able to change the implementation
// of encryptor/decryptor to the one using server service, but for now
// we will just use the implementation got from binlog cache encryption.

namespace myrocks {

class AesCtrCipherStream final : public rocksdb::BlockAccessCipherStream {
 private:
  std::unique_ptr<Stream_cipher> encryptor_;
  std::unique_ptr<Stream_cipher> decryptor_;

 public:
  AesCtrCipherStream();
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

  bool Init(const std::string &file_key, const std::string &iv);

 protected:
  // Allocate scratch space which is passed to EncryptBlock/DecryptBlock.
  void AllocateScratch(std::string &) override;

  // Encrypt a block of data at the given block index.
  // Length of data is equal to BlockSize();
  rocksdb::Status EncryptBlock(uint64_t blockIndex, char *data,
                               char *scratch) override;

  // Decrypt a block of data at the given block index.
  // Length of data is equal to BlockSize();
  rocksdb::Status DecryptBlock(uint64_t blockIndex, char *data,
                               char *scratch) override;
};

/******************************************************************************/
AesCtrCipherStream::AesCtrCipherStream() {}

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
  if (openRes) return openRes;

  // decryptor
  auto decryptor = Aes_ctr::get_decryptor();
  openRes =
      decryptor->open(reinterpret_cast<const unsigned char *>(file_key.c_str()),
                      reinterpret_cast<const unsigned char *>(iv.c_str()));
  if (openRes) return openRes;

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
    return rocksdb::Status::IOError();
  }
  if (encryptor_->encrypt((unsigned char *)data, (const unsigned char *)data,
                          dataSize)) {
    return rocksdb::Status::IOError();
  }
  return rocksdb::Status::OK();
}

rocksdb::Status AesCtrCipherStream::Decrypt(uint64_t fileOffset, char *data,
                                            size_t dataSize) {
  // Offset tracking to avoid underlaying cipher reinitialization is done
  // inside decryptor_
  if (decryptor_->set_stream_offset(fileOffset)) {
    return rocksdb::Status::IOError();
  }
  if (decryptor_->decrypt((unsigned char *)data, (const unsigned char *)data,
                          dataSize)) {
    return rocksdb::Status::IOError();
  }
  return rocksdb::Status::OK();
}

void AesCtrCipherStream::AllocateScratch(std::string &) {}

rocksdb::Status AesCtrCipherStream::EncryptBlock(uint64_t blockIndex,
                                                 char *data, char *scratch) {
  return rocksdb::Status::NotSupported();
}

rocksdb::Status AesCtrCipherStream::DecryptBlock(uint64_t blockIndex,
                                                 char *data, char *scratch) {
  return rocksdb::Status::NotSupported();
}

/******************************************************************************/

std::unique_ptr<rocksdb::BlockAccessCipherStream>
AesCtrStreamFactory::CreateCipherStream(const std::string &fileKey,
                                        const std::string &iv) {
  auto cipher = std::make_unique<AesCtrCipherStream>();
  if (cipher->Init(fileKey, iv)) {
    return nullptr;
  }
  return cipher;
}

}  // namespace myrocks