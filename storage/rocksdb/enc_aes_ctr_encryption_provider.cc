#include "./enc_aes_ctr_encryption_provider.h"
#include "./enc_cipher_stream_factory.h"
#include "./enc_master_key_manager.h"
#include "rocksdb/system_clock.h"
#include "rocksdb/util/random.h"

namespace myrocks {

/******************************************************************************/
static constexpr char kKeyMagic[] = "rdbe001";

static constexpr int KEY_MAGIC_SIZE = strlen(kKeyMagic);
static constexpr int MASTER_KEY_ID_SIZE = sizeof(uint32_t);
static constexpr int S_UUID_SIZE = 36;
static constexpr int CRC_SIZE = sizeof(uint32_t);
static constexpr int FILE_KEY_SIZE = 32;
static constexpr int IV_SIZE = 16;

static constexpr int KEY_MAGIC_OFFSET = 0;
static constexpr int MASTER_KEY_ID_OFFSET = KEY_MAGIC_OFFSET + KEY_MAGIC_SIZE;
static constexpr int S_UUID_OFFSET = MASTER_KEY_ID_OFFSET + MASTER_KEY_ID_SIZE;
static constexpr int CRC_OFFSET = S_UUID_OFFSET + S_UUID_SIZE;
static constexpr int FILE_KEY_OFFSET = CRC_OFFSET + CRC_SIZE;
static constexpr int IV_OFFSET = FILE_KEY_OFFSET + FILE_KEY_SIZE;

static constexpr char iv[IV_SIZE] = {0};
static const std::string kHeaderIV(iv, IV_SIZE);

/******************************************************************************/
const char *AesCtrEncryptionProvider::kCTRAesProviderName = "AES_CTR";

AesCtrEncryptionProvider::~AesCtrEncryptionProvider() {}

AesCtrEncryptionProvider::AesCtrEncryptionProvider(
    std::shared_ptr<MasterKeyManager> mmm,
    std::unique_ptr<CipherStreamFactory> csf)
    : masterKeyManager_(mmm), cipherStreamFactory_(std::move(csf)) {}

rocksdb::Status AesCtrEncryptionProvider::Feed(rocksdb::Slice &prefix) {
  // here we get the whole prefix of the encrypted file
  uint32_t masterKeyId = 0;
  memcpy(&masterKeyId, prefix.data() + MASTER_KEY_ID_OFFSET,
         MASTER_KEY_ID_SIZE);

  std::string serverUuid(prefix.data() + S_UUID_OFFSET, S_UUID_SIZE);

  masterKeyManager_->RegisterMasterKeyId(masterKeyId, serverUuid);

  return rocksdb::Status::OK();
}

const char *AesCtrEncryptionProvider::Name() const {
  return kCTRAesProviderName;
}

size_t AesCtrEncryptionProvider::GetPrefixLength() const {
  return defaultPrefixLength;
}

/*
Encryption prefix:
 4 bytes		KEY_MAGIC_V1 (e001) 		not encrypted
 4 bytes		master_key_id			not encrypted
36 bytes		s_uuid				not encrypted
 4 bytes		CRC (unencrypted key + iv)	not encrypted
32 bytes		key				encrypted
16 bytes		iv				encrypted
*/
rocksdb::Status AesCtrEncryptionProvider::CreateNewPrefix(
    const std::string &fname, char *prefix, size_t prefixLength) const {
  memcpy((void *)(&prefix[KEY_MAGIC_OFFSET]), kKeyMagic, KEY_MAGIC_SIZE);

  std::string masterKey;
  uint32_t masterKeyId = 0;

  masterKeyManager_->GetMostRecentMasterKey(&masterKey, &masterKeyId);
  // todo: do something like mach_write_to_4
  // store the master key id
  memcpy((void *)(&prefix[MASTER_KEY_ID_OFFSET]), &masterKeyId,
         MASTER_KEY_ID_SIZE);

  // store server uuid
  std::string serverUuid;
  masterKeyManager_->GetServerUuid(&serverUuid);
  memcpy((void *)(&prefix[S_UUID_OFFSET]), serverUuid.data(), S_UUID_SIZE);

  // Create & seed rnd.
  // todo: maybe openssl would be better for random numbers?
  rocksdb::Random rnd((uint32_t)rocksdb::SystemClock::Default()->NowMicros());
  // Fill the not clear-text part of the prefix with random values.
  // file key and IV are generated here as well
  for (size_t i = FILE_KEY_OFFSET; i < prefixLength; i++) {
    prefix[i] = rnd.Uniform(256) & 0xFF;
  }

#if 0
  memset((void*)(&prefix[FILE_KEY_OFFSET]), 'K', FILE_KEY_SIZE);
  memset((void*)(&prefix[IV_OFFSET]), 'V', IV_SIZE);
#endif

  // calculate and store CRC of not encrypted file key and IV
  // todo: skip calculation for now
  uint32_t crc = 0xABCDABCD;
  memcpy((void *)(&prefix[CRC_OFFSET]), &crc, CRC_SIZE);

  // encrypt file key and IV with master key
  auto encryptor =
      cipherStreamFactory_->CreateCipherStream(masterKey, kHeaderIV);
  char *dataToEncrypt = &prefix[FILE_KEY_OFFSET];
  encryptor->Encrypt(0, dataToEncrypt, FILE_KEY_SIZE + IV_SIZE);

#if 0
  memset((void*)(&prefix[MASTER_KEY_ID_OFFSET]), 'M', MASTER_KEY_ID_SIZE);
  memset((void*)(&prefix[S_UUID_OFFSET]), 'U', S_UUID_SIZE);
  memset((void*)(&prefix[FILE_KEY_OFFSET]), 'K', FILE_KEY_SIZE);
  memset((void*)(&prefix[IV_OFFSET]), 'V', IV_SIZE);
  memset((void*)(&prefix[CRC_OFFSET]), 'C', CRC_SIZE);
#endif
  return rocksdb::Status::OK();
}

// prefix is encrypted, reencrypt with new master key if needed.
rocksdb::Status AesCtrEncryptionProvider::ReencryptPrefix(
    rocksdb::Slice &prefix) const {
  // todo: introduce GetMostRecentMasterKeyId, to avoid getting it over and
  // over from keyring component
  std::string newestMasterKey;
  uint32_t newestMasterKeyId;
  masterKeyManager_->GetMostRecentMasterKey(&newestMasterKey,
                                            &newestMasterKeyId);

  uint32_t fileMasterKeyId;
  memcpy(&fileMasterKeyId, prefix.data() + MASTER_KEY_ID_OFFSET,
         MASTER_KEY_ID_SIZE);

  if (newestMasterKeyId == fileMasterKeyId) {
    return rocksdb::Status::OK();
  }

  // decrypt the header using old MK
  std::string suuid(prefix.data() + S_UUID_OFFSET, S_UUID_SIZE);
  std::string fileMasterKey;

  masterKeyManager_->GetMasterKey(fileMasterKeyId, suuid, &fileMasterKey);

  auto cipher =
      cipherStreamFactory_->CreateCipherStream(fileMasterKey, kHeaderIV);

  auto data = (char *)(prefix.data() + FILE_KEY_OFFSET);
  cipher->Decrypt(0, data, FILE_KEY_SIZE + IV_SIZE);

  // encrypt using the new master key
  cipher = cipherStreamFactory_->CreateCipherStream(newestMasterKey, kHeaderIV);
  cipher->Encrypt(0, data, FILE_KEY_SIZE + IV_SIZE);

  // update CRC
  // todo: skip calculation for now
  uint32_t crc = 0xABCDABCD;
  memcpy((void *)(prefix.data() + CRC_OFFSET), &crc, CRC_SIZE);

  // update MK id
  memcpy((void *)(prefix.data() + MASTER_KEY_ID_OFFSET), &newestMasterKeyId,
         MASTER_KEY_ID_SIZE);

  return rocksdb::Status::OK();
}

rocksdb::Status AesCtrEncryptionProvider::CreateCipherStream(
    const std::string &fname, const rocksdb::EnvOptions &options,
    rocksdb::Slice &prefix,
    std::unique_ptr<rocksdb::BlockAccessCipherStream> *result) {
  if (0 != memcmp(prefix.data(), kKeyMagic, KEY_MAGIC_SIZE)) {
    fprintf(stderr, "KH: not encrypted file detected\n");
    return rocksdb::Status::OK();
  }

  rocksdb::Slice masterKeyIdSlice(prefix.data() + MASTER_KEY_ID_OFFSET,
                                  MASTER_KEY_ID_SIZE);
  rocksdb::Slice suuidSlice(prefix.data() + S_UUID_OFFSET, S_UUID_SIZE);
  uint32_t masterKeyId = 0;
  memcpy(&masterKeyId, masterKeyIdSlice.data(), masterKeyIdSlice.size());
  std::string suuid(suuidSlice.data(), suuidSlice.size());

  std::string masterKey;
  masterKeyManager_->GetMasterKey(masterKeyId, suuid, &masterKey);

  // Decrypt key and iv with master key
  auto cipher = cipherStreamFactory_->CreateCipherStream(masterKey, kHeaderIV);

  char dataToDecrypt[FILE_KEY_SIZE + IV_SIZE];
  memcpy(dataToDecrypt, prefix.data() + FILE_KEY_OFFSET,
         FILE_KEY_SIZE + IV_SIZE);

  cipher->Decrypt(0, dataToDecrypt, FILE_KEY_SIZE + IV_SIZE);

  // TODO: Calculate and validate CRC
  uint32_t crc = 0;
  memcpy(&crc, prefix.data() + CRC_OFFSET, CRC_SIZE);
  if (crc != 0xABCDABCD) {
    fprintf(stderr, "WRONG CRC!\n");
  }

  rocksdb::Slice fileKey((char *)dataToDecrypt, FILE_KEY_SIZE);
  rocksdb::Slice fileIV((char *)(dataToDecrypt + FILE_KEY_SIZE), IV_SIZE);

  // Create cipher stream
  return CreateCipherStreamFromPrefix(fileKey, fileIV, result);
}

rocksdb::Status AesCtrEncryptionProvider::AddCipher(
    const std::string & /*descriptor*/, const char * /*cipher*/, size_t /*len*/,
    bool /*for_write*/) {
  // We do not use this method
  return rocksdb::Status::NotSupported();
}

rocksdb::Status AesCtrEncryptionProvider::CreateCipherStreamFromPrefix(
    const rocksdb::Slice &key, const rocksdb::Slice &iv,
    std::unique_ptr<rocksdb::BlockAccessCipherStream> *result) {
  std::string skey(key.data(), key.size());
  std::string siv(iv.data(), iv.size());
  (*result) = cipherStreamFactory_->CreateCipherStream(skey, siv);
  return rocksdb::Status::OK();
}

std::string AesCtrEncryptionProvider::GetMarker() const { return kKeyMagic; }
}  // namespace myrocks