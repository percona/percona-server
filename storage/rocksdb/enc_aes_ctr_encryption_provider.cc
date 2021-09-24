#include "./enc_aes_ctr_encryption_provider.h"
#include "./enc_cipher_stream_factory.h"
#include "./enc_master_key_manager.h"
#include "rocksdb/env.h"
#include "rocksdb/logging/logging.h"
#include "rocksdb/system_clock.h"
#include "rocksdb/util/coding_lean.h"
#include "rocksdb/util/crc32c.h"
#include "rocksdb/util/random.h"

namespace myrocks {

using rocksdb::InfoLogLevel;

/******************************************************************************/
static constexpr char kKeyMagic[] = "rdbe001";

static constexpr int KEY_MAGIC_SIZE = strlen(kKeyMagic);
static constexpr int MASTER_KEY_ID_SIZE = sizeof(uint32_t);
static constexpr int S_UUID_SIZE = 36;
static constexpr int FILE_KEY_SIZE = 32;
static constexpr int IV_SIZE = 16;
static constexpr int KEY_CRC_SIZE = sizeof(uint32_t);
static constexpr int HEADER_CRC_SIZE = sizeof(uint32_t);
static constexpr int HEADER_SIZE = KEY_MAGIC_SIZE + MASTER_KEY_ID_SIZE +
                                   S_UUID_SIZE + FILE_KEY_SIZE + IV_SIZE +
                                   KEY_CRC_SIZE;

static constexpr int KEY_MAGIC_OFFSET = 0;
static constexpr int MASTER_KEY_ID_OFFSET = KEY_MAGIC_OFFSET + KEY_MAGIC_SIZE;
static constexpr int S_UUID_OFFSET = MASTER_KEY_ID_OFFSET + MASTER_KEY_ID_SIZE;
static constexpr int FILE_KEY_OFFSET = S_UUID_OFFSET + S_UUID_SIZE;
static constexpr int IV_OFFSET = FILE_KEY_OFFSET + FILE_KEY_SIZE;
static constexpr int KEY_CRC_OFFSET = IV_OFFSET + IV_SIZE;
static constexpr int HEADER_CRC_OFFSET = KEY_CRC_OFFSET + KEY_CRC_SIZE;

static constexpr char iv[IV_SIZE] = {0};
static const std::string kHeaderIV(iv, IV_SIZE);

/******************************************************************************/
const char *AesCtrEncryptionProvider::kCTRAesProviderName = "AES_CTR";

AesCtrEncryptionProvider::~AesCtrEncryptionProvider() {}

AesCtrEncryptionProvider::AesCtrEncryptionProvider(
    std::shared_ptr<MasterKeyManager> mmm,
    std::unique_ptr<CipherStreamFactory> csf,
    std::shared_ptr<rocksdb::Logger> logger)
    : masterKeyManager_(mmm),
      cipherStreamFactory_(std::move(csf)),
      logger_(logger) {}

rocksdb::Status AesCtrEncryptionProvider::Feed(rocksdb::Slice &prefix) {
  // here we get the whole prefix of the encrypted file
  uint32_t masterKeyId =
      rocksdb::DecodeFixed32(prefix.data() + MASTER_KEY_ID_OFFSET);

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
 4 bytes		master_key_id			    not encrypted
36 bytes		s_uuid				        not encrypted
32 bytes		key				            encrypted
16 bytes		iv				            encrypted
 4 bytes		CRC (unencrypted key + iv)	encrypted
 4 bytes        CRC (all above)             not encrypted
*/
rocksdb::Status AesCtrEncryptionProvider::CreateNewPrefix(
    const std::string &fname, char *prefix, size_t prefixLength) const {
  memcpy((void *)(&prefix[KEY_MAGIC_OFFSET]), kKeyMagic, KEY_MAGIC_SIZE);

  std::string masterKey;
  uint32_t masterKeyId = 0;

  if (masterKeyManager_->GetMostRecentMasterKey(&masterKey, &masterKeyId)) {
    ROCKS_LOG_ERROR(logger_, "Failed to get the most recent master key");
    return rocksdb::Status::IOError();
  }
  // store the master key id
  rocksdb::EncodeFixed32(&prefix[MASTER_KEY_ID_OFFSET], masterKeyId);

  // store server uuid
  std::string serverUuid;
  masterKeyManager_->GetServerUuid(&serverUuid);
  memcpy((void *)(&prefix[S_UUID_OFFSET]), serverUuid.data(), S_UUID_SIZE);

  // Create & seed rnd.
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
  uint32_t crc = rocksdb::crc32c::Extend(0, &prefix[FILE_KEY_OFFSET],
                                         FILE_KEY_SIZE + IV_SIZE);
  rocksdb::EncodeFixed32(&prefix[KEY_CRC_OFFSET], crc);

  // encrypt file key + IV + CRC with master key
  auto encryptor =
      cipherStreamFactory_->CreateCipherStream(masterKey, kHeaderIV);
  if (!encryptor) {
    ROCKS_LOG_ERROR(logger_, "Failed to create cipher stream");
    return rocksdb::Status::IOError();
  }
  char *dataToEncrypt = &prefix[FILE_KEY_OFFSET];
  auto res = encryptor->Encrypt(0, dataToEncrypt,
                                FILE_KEY_SIZE + IV_SIZE + KEY_CRC_SIZE);
  if (res != rocksdb::Status::OK()) {
    ROCKS_LOG_ERROR(logger_, "Encryption failed");
  }

  // calculate and store CRC of the whole header
  crc = rocksdb::crc32c::Extend(0, prefix, HEADER_CRC_SIZE);
  rocksdb::EncodeFixed32(&prefix[HEADER_CRC_OFFSET], crc);

  return res;
}

bool AesCtrEncryptionProvider::IsPrefixOK(const rocksdb::Slice &prefix) {
  if (prefix.size() != defaultPrefixLength) {
    return false;
  }

  if (memcmp(prefix.data() + KEY_MAGIC_OFFSET, kKeyMagic, KEY_MAGIC_SIZE)) {
    return false;
  }

  uint32_t crc = rocksdb::DecodeFixed32(prefix.data() + HEADER_CRC_OFFSET);
  uint32_t crcCalculated =
      rocksdb::crc32c::Extend(0, prefix.data(), HEADER_CRC_SIZE);

  return crc == crcCalculated;
}

// prefix is encrypted, reencrypt with new master key if needed.
rocksdb::Status AesCtrEncryptionProvider::ReencryptPrefix(
    rocksdb::Slice &prefix) const {
  std::string newestMasterKey;
  uint32_t newestMasterKeyId;

  if (masterKeyManager_->GetMostRecentMasterKey(&newestMasterKey,
                                                &newestMasterKeyId)) {
    ROCKS_LOG_ERROR(logger_, "Failed to get the most recent master key");
    return rocksdb::Status::IOError();
  }

  uint32_t fileMasterKeyId =
      rocksdb::DecodeFixed32(prefix.data() + MASTER_KEY_ID_OFFSET);

  if (newestMasterKeyId == fileMasterKeyId) {
    ROCKS_LOG_INFO(logger_,
                   "Newest master key already used. Reencryption skipped.");
    return rocksdb::Status::OK();
  }

  // decrypt the header using old MK
  std::string suuid(prefix.data() + S_UUID_OFFSET, S_UUID_SIZE);
  std::string fileMasterKey;

  if (masterKeyManager_->GetMasterKey(fileMasterKeyId, suuid, &fileMasterKey)) {
    ROCKS_LOG_ERROR(logger_, "Failed to get master key");
    return rocksdb::Status::IOError();
  }

  auto cipher =
      cipherStreamFactory_->CreateCipherStream(fileMasterKey, kHeaderIV);
  if (!cipher) {
    ROCKS_LOG_ERROR(logger_,
                    "Failed to create cipher stream for header decryption");
    return rocksdb::Status::IOError();
  }
  auto data = (char *)(prefix.data() + FILE_KEY_OFFSET);
  auto res = cipher->Decrypt(0, data, FILE_KEY_SIZE + IV_SIZE + KEY_CRC_SIZE);
  if (res != rocksdb::Status::OK()) {
    ROCKS_LOG_ERROR(logger_, "Header decryption failed");
    return rocksdb::Status::IOError();
  }
  // Calculate and validate CRC
  uint32_t crcRead = rocksdb::DecodeFixed32(prefix.data() + KEY_CRC_OFFSET);

  uint32_t crcCalculated = rocksdb::crc32c::Extend(
      0, &prefix.data()[FILE_KEY_OFFSET], FILE_KEY_SIZE + IV_SIZE);
  if (crcRead != crcCalculated) {
    ROCKS_LOG_ERROR(logger_, "Wrong CRC in header");
    return rocksdb::Status::Corruption();
  }

  // no need to update CRC as it is calculated from plain text it didn't change

  // encrypt using the new master key
  cipher = cipherStreamFactory_->CreateCipherStream(newestMasterKey, kHeaderIV);
  if (!cipher) {
    ROCKS_LOG_ERROR(logger_,
                    "Failed to create cipher stream for header encryption");
    return rocksdb::Status::IOError();
  }
  res = cipher->Encrypt(0, data, FILE_KEY_SIZE + IV_SIZE + KEY_CRC_SIZE);
  if (res != rocksdb::Status::OK()) {
    ROCKS_LOG_ERROR(logger_, "Header encryption failed");
    return rocksdb::Status::IOError();
  }

  // update MK id
  rocksdb::EncodeFixed32((char *)prefix.data() + MASTER_KEY_ID_OFFSET,
                         newestMasterKeyId);

  // calculate and store CRC of the whole header
  uint32_t crc = rocksdb::crc32c::Extend(0, prefix.data(), HEADER_CRC_SIZE);
  rocksdb::EncodeFixed32((char *)prefix.data() + HEADER_CRC_OFFSET, crc);

  return res;
}

rocksdb::Status AesCtrEncryptionProvider::CreateCipherStream(
    const std::string &fname, const rocksdb::EnvOptions &options,
    rocksdb::Slice &prefix,
    std::unique_ptr<rocksdb::BlockAccessCipherStream> *result) {
  return CreateCipherStreamCommon(fname, options, prefix, result, false);
}

rocksdb::Status AesCtrEncryptionProvider::CreateThreadSafeCipherStream(
    const std::string &fname, const rocksdb::EnvOptions &options,
    rocksdb::Slice &prefix,
    std::unique_ptr<rocksdb::BlockAccessCipherStream> *result) {
  return CreateCipherStreamCommon(fname, options, prefix, result, true);
}

rocksdb::Status AesCtrEncryptionProvider::CreateCipherStreamCommon(
    const std::string &fname, const rocksdb::EnvOptions &options,
    rocksdb::Slice &prefix,
    std::unique_ptr<rocksdb::BlockAccessCipherStream> *result,
    bool threadSafe) {
  if (0 != memcmp(prefix.data(), kKeyMagic, KEY_MAGIC_SIZE)) {
    ROCKS_LOG_INFO(
        logger_,
        "Header corruption detected or file is not encrypted. file: %s",
        fname.c_str());
    return rocksdb::Status::OK();
  }

  uint32_t masterKeyId =
      rocksdb::DecodeFixed32(prefix.data() + MASTER_KEY_ID_OFFSET);
  std::string suuid(prefix.data() + S_UUID_OFFSET, S_UUID_SIZE);

  std::string masterKey;
  if (masterKeyManager_->GetMasterKey(masterKeyId, suuid, &masterKey)) {
    ROCKS_LOG_ERROR(logger_, "Failed to get master key");
    return rocksdb::Status::IOError();
  }

  // Decrypt key+iv+crc with master key
  auto cipher = cipherStreamFactory_->CreateCipherStream(masterKey, kHeaderIV);
  if (!cipher) {
    ROCKS_LOG_ERROR(logger_,
                    "Failed to create cipher stream for header decryption");
    return rocksdb::Status::IOError();
  }
  char dataToDecrypt[FILE_KEY_SIZE + IV_SIZE + KEY_CRC_SIZE];
  memcpy(dataToDecrypt, prefix.data() + FILE_KEY_OFFSET,
         FILE_KEY_SIZE + IV_SIZE + KEY_CRC_SIZE);

  auto res =
      cipher->Decrypt(0, dataToDecrypt, FILE_KEY_SIZE + IV_SIZE + KEY_CRC_SIZE);
  if (res != rocksdb::Status::OK()) {
    ROCKS_LOG_ERROR(logger_, "Decryption of file header failed");
    return rocksdb::Status::IOError();
  }
  // Calculate and validate CRC
  uint32_t crcRead =
      rocksdb::DecodeFixed32(&dataToDecrypt[FILE_KEY_SIZE + IV_SIZE]);

  uint32_t crcCalculated =
      rocksdb::crc32c::Extend(0, dataToDecrypt, FILE_KEY_SIZE + IV_SIZE);
  if (crcRead != crcCalculated) {
    ROCKS_LOG_ERROR(logger_, "Wrong CRC in header");
    return rocksdb::Status::Corruption();
  }

  rocksdb::Slice fileKey((char *)dataToDecrypt, FILE_KEY_SIZE);
  rocksdb::Slice fileIV((char *)(dataToDecrypt + FILE_KEY_SIZE), IV_SIZE);

  // Create cipher stream
  return CreateCipherStreamFromPrefix(fileKey, fileIV, result, threadSafe);
}

rocksdb::Status AesCtrEncryptionProvider::AddCipher(
    const std::string & /*descriptor*/, const char * /*cipher*/, size_t /*len*/,
    bool /*for_write*/) {
  // We do not use this method
  return rocksdb::Status::NotSupported();
}

rocksdb::Status AesCtrEncryptionProvider::CreateCipherStreamFromPrefix(
    const rocksdb::Slice &key, const rocksdb::Slice &iv,
    std::unique_ptr<rocksdb::BlockAccessCipherStream> *result,
    bool threadSafe) {
  std::string skey(key.data(), key.size());
  std::string siv(iv.data(), iv.size());
  if (threadSafe) {
    (*result) = cipherStreamFactory_->CreateThreadSafeCipherStream(skey, siv);
  } else {
    (*result) = cipherStreamFactory_->CreateCipherStream(skey, siv);
  }
  if (!*result) {
    ROCKS_LOG_ERROR(logger_, "Failed to create cipher stream from prefix");
    return rocksdb::Status::IOError();
  }
  return rocksdb::Status::OK();
}

std::string AesCtrEncryptionProvider::GetMarker() const { return kKeyMagic; }
}  // namespace myrocks