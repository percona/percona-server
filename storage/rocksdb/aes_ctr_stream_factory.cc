#include "./aes_ctr_stream_factory.h"
#include "./stream_cipher.h"
#include <rocksdb/env_encryption.h>
#include <openssl/evp.h>

// Note: This implementation of BlockAccessCipherStream uses AES-CRT encryption
// with SSL backend

// Right now server's keyring_encryption_service does not provide AES CRT mode.
// We introducd this factory layer to be able to change the implementation
// of encryptor/decryptor to the one using srver service, but for now
// we will just use the implementation got from binlog cache encryption.

namespace myrocks {

// CTRCipherStream implements BlockAccessCipherStream using an
// Counter operations mode.
// See https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation
//
class CTRAesCipherStream final : public rocksdb::BlockAccessCipherStream {
 private:
  std::unique_ptr<Stream_cipher> m_encryptor;
  std::unique_ptr<Stream_cipher> m_decryptor;
  uint64_t m_encryptPosition;
  uint64_t m_decryptPosition;

 public:
  CTRAesCipherStream();
  virtual ~CTRAesCipherStream();

  // BlockSize returns the size of each block supported by this cipher stream.
  size_t BlockSize() override;

  // Encrypt one or more (partial) blocks of data at the file offset.
  // Length of data is given in dataSize.
  rocksdb::Status Encrypt(uint64_t fileOffset, char* data, size_t dataSize) override;

  // Decrypt one or more (partial) blocks of data at the file offset.
  // Length of data is given in dataSize.
  rocksdb::Status Decrypt(uint64_t fileOffset, char* data, size_t dataSize) override;

  bool Init(const std::string& file_key, const std::string& iv);

 protected:
  // Allocate scratch space which is passed to EncryptBlock/DecryptBlock.
  void AllocateScratch(std::string&) override;

  // Encrypt a block of data at the given block index.
  // Length of data is equal to BlockSize();
  rocksdb::Status EncryptBlock(uint64_t blockIndex, char* data, char* scratch) override;

  // Decrypt a block of data at the given block index.
  // Length of data is equal to BlockSize();
  rocksdb::Status DecryptBlock(uint64_t blockIndex, char* data, char* scratch) override;
};



/******************************************************************************/
CTRAesCipherStream::CTRAesCipherStream() {
}

CTRAesCipherStream::~CTRAesCipherStream() {
    m_encryptor->close();
    m_decryptor->close();
}

bool CTRAesCipherStream::Init(const std::string& file_key, const std::string& iv) {
  // encryptor
  auto encryptor = Aes_ctr::get_encryptor();
  auto openRes = encryptor->open(reinterpret_cast<const unsigned char*>(file_key.c_str()),
                                 reinterpret_cast<const unsigned char*>(iv.c_str()));
  if (openRes) return openRes;

  //decryptor
  auto decryptor = Aes_ctr::get_decryptor();
  openRes = decryptor->open(reinterpret_cast<const unsigned char*>(file_key.c_str()),
                            reinterpret_cast<const unsigned char*>(iv.c_str()));
  if (openRes) return openRes;

  m_encryptor = std::move(encryptor);
  m_decryptor = std::move(decryptor);
  return false;
}

size_t CTRAesCipherStream::BlockSize()
{
    return 0;
}

rocksdb::Status CTRAesCipherStream::Encrypt(uint64_t fileOffset, char* data, size_t dataSize)
{
  // todo: add offset tracking to avoid underlaying cipher reinitialization if not needed
  m_encryptor->set_stream_offset(fileOffset);
  m_encryptor->encrypt((unsigned char*)data, (const unsigned char*)data, dataSize);
  return rocksdb::Status::OK();
}

rocksdb::Status CTRAesCipherStream::Decrypt(uint64_t fileOffset, char* data, size_t dataSize)
{
  // todo: add offset tracking to avoid underlaying cipher reinitialization if not needed
  m_decryptor->set_stream_offset(fileOffset);
  m_decryptor->decrypt((unsigned char*)data, (const unsigned char*)data, dataSize);
  return rocksdb::Status::OK();
}

void CTRAesCipherStream::AllocateScratch(std::string&)
{

}

rocksdb::Status CTRAesCipherStream::EncryptBlock(uint64_t blockIndex, char* data, char* scratch)
{
  return rocksdb::Status::OK();
}

rocksdb::Status CTRAesCipherStream::DecryptBlock(uint64_t blockIndex, char* data, char* scratch)
{
  return rocksdb::Status::OK();
}

/******************************************************************************/

std::unique_ptr<rocksdb::BlockAccessCipherStream>
  AESCtrStreamFactory::CreateCipherStream(const std::string& fileKey, const std::string& iv) {
  auto cipher = std::make_unique<CTRAesCipherStream>();
  if (cipher->Init(fileKey, iv)) {
      return nullptr;
  }
  return cipher;
}



}  // namespace