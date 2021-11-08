/***********************************************************************

Copyright (c) 2019, 2021, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

***********************************************************************/

/** @file include/os0enc.h
 Page encryption infrastructure. */

#ifndef os0enc_h
#define os0enc_h

#include <mysql/components/my_service.h>

#include "keyring_encryption_key_info.h"
#include "template_utils.h"

#include "univ.i"

namespace innobase {
namespace encryption {

bool init_keyring_services(SERVICE_TYPE(registry) * reg_srv);

void deinit_keyring_services(SERVICE_TYPE(registry) * reg_srv);

bool generate_key(const char *key_id, const char *key_type, size_t key_length);
void remove_key(const char *key_id);
bool store_key(const char *key_id, const unsigned char *key, size_t key_length,
               const char *key_type);
int read_key(const char *key_id, unsigned char **key, size_t *key_length,
             char **key_type);

}  // namespace encryption
}  // namespace innobase

// Forward declaration.
class IORequest;
struct Encryption_key;

enum class Encryption_rotation : std::uint8_t {
  NO_ROTATION,
  /** For Master Key encrypted pages use the tablespace key to read.
   * Use the crypt_data's key when writing (encrypting). */
  MASTER_KEY_TO_KEYRING,
  /** Encrypt all the pages that go through I/O level */
  ENCRYPTING,
  /** Do not encrypt pages that go through I/O level.
   * When encryption threads decrypt pages, they just pass I/O level
   * unencrypted (the encryption is disabled). */
  DECRYPTING
};

/** Encryption algorithm. */
class Encryption {
 public:
  /** Algorithm types supported */
  enum Type {

    /** No encryption */
    NONE = 0,

    /** Use AES */
    AES = 1,

    KEYRING = 2
  };

  /** Encryption information format version */
  enum Version {

    /** Version in 5.7.11 */
    VERSION_1 = 0,

    /** Version in > 5.7.11 */
    VERSION_2 = 1,

    /** Version in > 8.0.4 */
    VERSION_3 = 2,
  };

  /** Encryption progress type. */
  enum class Progress {
    /* Space encryption in progress */
    ENCRYPTION,
    /* Space decryption in progress */
    DECRYPTION,
    /* Nothing in progress */
    NONE
  };

  /** Encryption operation resume point after server restart. */
  enum class Resume_point {
    /* Resume from the beginning. */
    INIT,
    /* Resume processing. */
    PROCESS,
    /* Operation has ended. */
    END,
    /* All done. */
    DONE
  };

  /** Encryption magic bytes for 5.7.11, it's for checking the encryption
  information version. */
  static constexpr char KEY_MAGIC_V1[] = "lCA";

  /** Encryption magic bytes for 5.7.12+, it's for checking the encryption
  information version. */
  static constexpr char KEY_MAGIC_V2[] = "lCB";

  /** Encryption magic bytes for 8.0.5+, it's for checking the encryption
  information version. */
  static constexpr char KEY_MAGIC_V3[] = "lCC";

  /** Encryption magic bytes before 8.0.19, it's for checking KEYRING
  redo log information version. */
  static constexpr char KEY_MAGIC_RK_V1[] = "lRK";

  /** Encryption magic bytes for 8.0.19+, it's for checking KEYRING
  redo log information version. */
  static constexpr char KEY_MAGIC_RK_V2[] = "RKB";

  static constexpr char KEY_MAGIC_PS_V1[] = "PSA";

  static constexpr char KEY_MAGIC_PS_V2[] = "PSB";

  static constexpr char KEY_MAGIC_PS_V3[] = "PSC";

  /** Encryption master key prifix */
  static constexpr char MASTER_KEY_PREFIX[] = "INNODBKey";

  /** Encryption key length */
  static constexpr size_t KEY_LEN = 32;

  /** Default master key for bootstrap */
  static constexpr char DEFAULT_MASTER_KEY[] = "DefaultMasterKey";

  /** Encryption magic bytes size */
  static constexpr size_t MAGIC_SIZE = 3;

  static constexpr size_t SERVER_UUID_HEX_LEN = 16;

  /** Encryption master key prifix size */
  static constexpr size_t MASTER_KEY_PRIFIX_LEN = 9;

  static constexpr char ZIP_PAGE_KEYRING_ENCRYPTION_MAGIC[] = "RK";

  static constexpr ulint ZIP_PAGE_KEYRING_ENCRYPTION_MAGIC_LEN = 2;

  /** Encryption master key prifix */
  // TODO: Change this to percona_innodb_idb
  static constexpr char PERCONA_SYSTEM_KEY_PREFIX[] = "percona_innodb";

  /** Encryption master key prifix size */
  static constexpr ulint PERCONA_SYSTEM_KEY_PREFIX_LEN =
      array_elements(PERCONA_SYSTEM_KEY_PREFIX);

  /** Encryption master key prifix size */
  static constexpr size_t MASTER_KEY_NAME_MAX_LEN = 100;

  /** UUID of server instance, it's needed for composing master key name */
  static constexpr size_t SERVER_UUID_LEN = 36;

  /** Encryption information total size: magic number + master_key_id +
  key + iv + server_uuid + checksum */
  static constexpr size_t INFO_SIZE =
      (MAGIC_SIZE + sizeof(uint32) + (KEY_LEN * 2) + SERVER_UUID_LEN +
       sizeof(uint32));

  /** Maximum size of Encryption information considering all
  formats v1, v2 & v3. */
  static constexpr size_t INFO_MAX_SIZE = INFO_SIZE + sizeof(uint32);

  /** Default master key id for bootstrap */
  static constexpr uint32_t DEFAULT_MASTER_KEY_ID = 0;

  /** (De)Encryption Operation information size */
  static constexpr size_t OPERATION_INFO_SIZE = 1;

  /** Encryption Progress information size */
  static constexpr size_t PROGRESS_INFO_SIZE = sizeof(uint);

  /** Flag bit to indicate if Encryption/Decryption is in progress */
  static constexpr size_t ENCRYPT_IN_PROGRESS = 1 << 0;

  /** Decryption in progress. */
  static constexpr size_t DECRYPT_IN_PROGRESS = 1 << 1;

  /** Tablespaces whose key needs to be reencrypted */
  static std::vector<space_id_t> s_tablespaces_to_reencrypt;

  /** Default constructor */
  Encryption() noexcept
      : m_type(NONE),
        m_key(nullptr),
        m_klen(0),
        m_iv(nullptr),
        m_tablespace_key(nullptr),
        m_key_version(0),
        m_key_id(0),
        m_checksum(0),
        m_encryption_rotation(Encryption_rotation::NO_ROTATION),
        m_key_versions_cache(nullptr) {
    m_key_id_uuid[0] = '\0';
  }

  /** Specific constructor
  @param[in]  type    Algorithm type */
  explicit Encryption(Type type) noexcept
      : m_type(type),
        m_key(nullptr),
        m_klen(0),
        m_iv(nullptr),
        m_tablespace_key(nullptr),
        m_key_version(0),
        m_key_id(0),
        m_checksum(0),
        m_encryption_rotation(Encryption_rotation::NO_ROTATION) {
    m_key_id_uuid[0] = '\0';
#ifdef UNIV_DEBUG
    switch (m_type) {
      case NONE:
      case AES:
      case KEYRING:

      default:
        ut_error;
    }
#endif /* UNIV_DEBUG */
  }

  /** Copy constructor */
  Encryption(const Encryption &other) noexcept;

  Encryption &operator=(const Encryption &other) noexcept {
    Encryption tmp(other);
    swap(tmp);
    return *this;
  }

  void swap(Encryption &other) noexcept {
    std::swap(m_type, other.m_type);
    std::swap(m_key, other.m_key);
    std::swap(m_klen, other.m_klen);
    std::swap(m_iv, other.m_iv);
    std::swap(m_tablespace_key, other.m_tablespace_key);
    std::swap(m_key_version, other.m_key_version);
    std::swap(m_key_id, other.m_key_id);
    std::swap(m_checksum, other.m_checksum);
    std::swap(m_encryption_rotation, other.m_encryption_rotation);
    std::swap(m_key_id_uuid, other.m_key_id_uuid);
    std::swap(m_key_versions_cache, other.m_key_versions_cache);
  }

  ~Encryption();

  void set_key(const byte *key, ulint key_len) noexcept;

  void set_key_versions_cache(
      std::map<uint, byte *> *key_versions_cache) noexcept;

  /** Check if page is encrypted page or not
  @param[in]  page  page which need to check
  @return true if it is an encrypted page */
  [[nodiscard]] static bool is_encrypted_page(const byte *page) noexcept;

  /** Check if a log block is encrypted or not
  @param[in]  block block which need to check
  @return true if it is an encrypted block */
  [[nodiscard]] static bool is_encrypted_log(const byte *block) noexcept;

  /** Check the encryption option and set it
  @param[in]      option      encryption option
  @param[in,out]  type        The encryption type
  @return DB_SUCCESS or DB_UNSUPPORTED */
  [[nodiscard]] dberr_t set_algorithm(const char *option,
                                      Encryption *type) noexcept;

  /** Validate the algorithm string.
  @param[in]  option  Encryption option
  @return DB_SUCCESS or error code */
  [[nodiscard]] static dberr_t validate(const char *option) noexcept;

  /** Validate the algorithm string for tablespace
  @param[in]	option		Encryption option
  @return DB_SUCCESS or error code */
  MY_NODISCARD static dberr_t validate_for_tablespace(
      const char *option) noexcept;

  /** Convert to a "string".
  @param[in]  type  The encryption type
  @return the string representation */
  [[nodiscard]] static const char *to_string(Type type) noexcept;

  /** Check if the string is "" or "n".
  @param[in]  algorithm  Encryption algorithm to check
  @return true if no algorithm requested */
  [[nodiscard]] static bool is_none(const char *algorithm) noexcept;

  /** Check if the NO algorithm was explicitly specified.
  @param[in]      explicit_encryption was ENCRYPTION clause
                  specified explicitly
  @param[in]      algorithm       Encryption algorithm to check
  @return true if no algorithm explicitly requested */
  static bool none_explicitly_specified(
      bool explicit_encryption,
      const char *algorithm) noexcept MY_ATTRIBUTE((warn_unused_result));

  /** Check if the string is "y" or "Y".
  @param[in]      algorithm       Encryption algorithm to check
  @return true if no algorithm requested */
  static bool is_master_key_encryption(
      const char *algorithm) noexcept MY_ATTRIBUTE((warn_unused_result));

  static bool is_empty(const char *algorithm) noexcept MY_ATTRIBUTE(
      (warn_unused_result));

  static bool is_keyring(const char *algoritm) noexcept MY_ATTRIBUTE(
      (warn_unused_result));

  static bool is_online_encryption_on() noexcept MY_ATTRIBUTE(
      (warn_unused_result));

  static bool should_be_keyring_encrypted(
      bool explicit_encryption,
      const char *algorithm) noexcept MY_ATTRIBUTE((warn_unused_result));

  /** Generate random encryption value for key and iv.
  @param[in,out]  value Encryption value */
  static void random_value(byte *value) noexcept;

  /** Create tablespace key
  @param[in,out]	tablespace_key	tablespace key - null if failure
  @param[in]		key_id		tablespace key id
  @param[in]  uuid tablespace key uuid */
  static void create_tablespace_key(byte **tablespace_key, uint key_id,
                                    const char *uuid);

  /** Create new master key for key rotation.
  @param[in,out]  master_key  master key */
  static void create_master_key(byte **master_key) noexcept;

  static bool tablespace_key_exists_or_create_new_one_if_does_not_exist(
      uint key_id, const char *uuid);

  static bool tablespace_key_exists(uint key_id, const char *uuid);

  static bool is_encrypted_and_compressed(const byte *page);

  static uint encryption_get_latest_version(uint key_id, const char *uuid);

  static void get_latest_tablespace_key(uint key_id, const char *uuid,
                                        uint *tablespace_key_version,
                                        byte **tablespace_key);

  static void get_latest_key_or_create(uint tablespace_key_id, const char *uuid,
                                       uint *tablespace_key_version,
                                       byte **tablespace_key);

  static bool get_tablespace_key(uint key_id, const char *uuid,
                                 uint tablespace_key_version,
                                 byte **tablespace_key, size_t *key_len);

  /** Get master key by key id.
  @param[in]      master_key_id master key id
  @param[in]      srv_uuid      uuid of server instance
  @param[in,out]  master_key    master key */
  static void get_master_key(uint32_t master_key_id, char *srv_uuid,
                             byte **master_key) noexcept;

  /** Get current master key and key id.
  @param[in,out]  master_key_id master key id
  @param[in,out]  master_key    master key */
  static void get_master_key(uint32_t *master_key_id,
                             byte **master_key) noexcept;

  /** Checks if keyring is installed and it is operational.
   *  This is done by trying to fetch/create
   *  dummy percona_keyring_test key
  @return true if success */
  static bool is_keyring_alive();

  static bool can_page_be_keyring_encrypted(ulint page_type);
  static bool can_page_be_keyring_encrypted(byte *page);

  /** Fill the encryption information.
  @param[in]      key           encryption key
  @param[in]      iv            encryption iv
  @param[in,out]  encrypt_info  encryption information
  @param[in]      is_boot       if it's for bootstrap
  @param[in]      encrypt_key   encrypt with master key
  @return true if success. */
  static bool fill_encryption_info(const byte *key, const byte *iv,
                                   byte *encrypt_info, bool is_boot,
                                   bool encrypt_key) noexcept;

  static bool fill_encryption_info(uint key_version, byte *iv,
                                   byte *encrypt_info);

  /** Get master key from encryption information
  @param[in]      encrypt_info  encryption information
  @param[in]      version       version of encryption information
  @param[in,out]  m_key_id      master key id
  @param[in,out]  srv_uuid      server uuid
  @param[in,out]  master_key    master key
  @return position after master key id or uuid, or the old position
  if can't get the master key. */
  static byte *get_master_key_from_info(byte *encrypt_info, Version version,
                                        uint32_t *m_key_id, char *srv_uuid,
                                        byte **master_key) noexcept;

  /** Decoding the encryption info from the first page of a tablespace.
  @param[in]      space_id        Tablespace id
  @param[in,out]  e_key           key, iv
  @param[in]      encryption_info encryption info
  @param[in]      decrypt_key     decrypt key using master key
  @return true if success */
  static bool decode_encryption_info(space_id_t space_id, Encryption_key &e_key,
                                     byte *encryption_info,
                                     bool decrypt_key) noexcept;

  /** Encrypt the redo log block.
  @param[in]      type      IORequest
  @param[in,out]  src_ptr   log block which need to encrypt
  @param[in,out]  dst_ptr   destination area
  @return true if success. */
  bool encrypt_log_block(const IORequest &type, byte *src_ptr,
                         byte *dst_ptr) noexcept;

  /** Encrypt the redo log data contents.
  @param[in]      type      IORequest
  @param[in,out]  src       page data which need to encrypt
  @param[in]      src_len   size of the source in bytes
  @param[in,out]  dst       destination area
  @param[in,out]  dst_len   size of the destination in bytes
  @return buffer data, dst_len will have the length of the data */
  byte *encrypt_log(const IORequest &type, byte *src, ulint src_len, byte *dst,
                    ulint *dst_len) noexcept;

  /** Encrypt the page data contents. Page type can't be
  FIL_PAGE_ENCRYPTED, FIL_PAGE_COMPRESSED_AND_ENCRYPTED,
  FIL_PAGE_ENCRYPTED_RTREE.
  @param[in]      type      IORequest
  @param[in,out]  src       page data which need to encrypt
  @param[in]      src_len   size of the source in bytes
  @param[in,out]  dst       destination area
  @param[in,out]  dst_len   size of the destination in bytes
  @return buffer data, dst_len will have the length of the data */
  [[nodiscard]] byte *encrypt(const IORequest &type, byte *src, ulint src_len,
                              byte *dst, ulint *dst_len) noexcept;

  /** Decrypt the log block.
  @param[in]      type  IORequest
  @param[in,out]  src   data read from disk, decrypted data
                        will be copied to this page
  @param[in,out]  dst   scratch area to use for decryption
  @return DB_SUCCESS or error code */
  dberr_t decrypt_log_block(const IORequest &type, byte *src,
                            byte *dst) noexcept;

  /** Decrypt the log data contents.
  @param[in]      type      IORequest
  @param[in,out]  src       data read from disk, decrypted data
                            will be copied to this page
  @param[in]      src_len   source data length
  @param[in,out]  dst       scratch area to use for decryption
  @param[in]      dst_len   size of the scratch area in bytes
  @return DB_SUCCESS or error code */
  dberr_t decrypt_log(const IORequest &type, byte *src, ulint src_len,
                      byte *dst, ulint dst_len) noexcept;

  /** Decrypt the page data contents. Page type must be
  FIL_PAGE_ENCRYPTED, FIL_PAGE_COMPRESSED_AND_ENCRYPTED,
  FIL_PAGE_ENCRYPTED_RTREE, if not then the source contents are
  left unchanged and DB_SUCCESS is returned.
  @param[in]      type    IORequest
  @param[in,out]  src     data read from disk, decrypt
                          data will be copied to this page
  @param[in]      src_len source data length
  @param[in,out]  dst     scratch area to use for decrypt
  @param[in]  dst_len     size of the scratch area in bytes
  @return DB_SUCCESS or error code */
  [[nodiscard]] dberr_t decrypt(const IORequest &type, byte *src, ulint src_len,
                                byte *dst, ulint dst_len) noexcept;

  /** Check if keyring plugin loaded. */
  MY_NODISCARD static bool check_keyring() noexcept;

  /** Get encryption type
  @return encryption type **/
  Type get_type() const;

  /** Check if the encryption algorithm is NONE.
  @return true if no algorithm is set, false otherwise. */
  [[nodiscard]] bool is_none() const noexcept { return m_type == NONE; }

  /** Set encryption type
  @param[in]  type  encryption type **/
  void set_type(Type type);

  std::map<uint, byte *> *get_key_versions_cache() const;

  /** Set encryption key
  @param[in]  key  encryption key **/
  void set_key(const byte *key);

  bool has_key() const noexcept { return m_key != nullptr; }

  /** Get key length
  @return  key length **/
  ulint get_key_length() const;

  /** Set key length
  @param[in]  klen  key length **/
  void set_key_length(ulint klen);

  /** Set initial vector
  @param[in]  iv  initial_vector **/
  void set_initial_vector(const byte *iv);

  /** Get tablespace encryption key
  @return tablespace encryption key **/
  byte *get_tablespace_key() const;

  /** Set tablespace encryption key
  @param[in]  tablespace_key  tablespace encryption key **/
  void set_tablespace_key(byte *tablespace_key);

  /** Get key id UUID
  @return key id UUID **/
  const char *get_key_id_uuid() const;

  /** Set key id UUID
  @param[in]  key_id_uuid  key id UUID **/
  void set_key_id_uuid(const char *key_id_uuid);

  /** Get key version
  @return  key version **/
  ulint get_key_version() const;

  /** Set key version
  @param[in]  key_version  key version **/
  void set_key_version(ulint key_version);

  /** Get key id
  @return  key id **/
  ulint get_key_id() const;

  /** Set key id
  @param[in]  key_id  key id **/
  void set_key_id(ulint key_id);

  /** Get encryption rotation
  @return  encryption rotation **/
  Encryption_rotation get_encryption_rotation() const;

  /** Set encryption rotation
  @param[in]  encryption_rotation  encryption rotation **/
  void set_encryption_rotation(Encryption_rotation encryption_rotation);

  /** Get master key id
  @return master key id **/
  static uint32_t get_master_key_id();

 private:
  /** Encrypt the page data contents. Page type can't be
  FIL_PAGE_ENCRYPTED, FIL_PAGE_COMPRESSED_AND_ENCRYPTED,
  FIL_PAGE_ENCRYPTED_RTREE.
  @param[in]  type      IORequest
  @param[in]  src       page data which need to encrypt
  @param[in]  src_len   size of the source in bytes
  @param[in,out]  dst       destination area
  @param[in,out]  dst_len   size of the destination in bytes
  @return true if operation successful, false otherwise. */
  [[nodiscard]] bool encrypt_low(const IORequest &type, byte *src,
                                 ulint src_len, byte *dst,
                                 ulint *dst_len) noexcept;

  /** Encrypt type */
  Type m_type;

  /** Encrypt key */
  const byte *m_key;

  /** Encrypt key length*/
  ulint m_klen;

  /** Encrypt initial vector */
  const byte *m_iv;

  byte *m_tablespace_key;

  char m_key_id_uuid[SERVER_UUID_LEN + 1];  // uuid that is part of
                                            // the full key id of a
                                            // percona system key
  uint m_key_version;

  uint m_key_id;

  uint32 m_checksum;

  Encryption_rotation m_encryption_rotation;

  std::map<uint, byte *> *m_key_versions_cache;

  /** Current master key id */
  static uint32_t s_master_key_id;

  /** Current uuid of server instance */
  static char s_uuid[SERVER_UUID_LEN + 1];

  // TODO: Robert: Is it needed here?
  static void get_keyring_key(const char *key_name, byte **key,
                              size_t *key_len);

  static void get_latest_system_key(const char *system_key_name, byte **key,
                                    uint *key_version, size_t *key_length);

  static void fill_key_name(char *key_name, uint key_id, const char *uuid);

  static void fill_key_name(char *key_name, uint key_id, const char *uuid,
                            uint key_version);
};

struct Encryption_key {
  /** Encrypt key */
  byte *m_key;

  /** Encrypt initial vector */
  byte *m_iv;

  /** Master key id */
  uint32_t m_master_key_id{Encryption::DEFAULT_MASTER_KEY_ID};
};
#endif /* os0enc_h */
