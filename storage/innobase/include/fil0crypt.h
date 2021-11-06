/*****************************************************************************
Copyright (C) 2013, 2015, Google Inc. All Rights Reserved.
Copyright (c) 2015, 2017, MariaDB Corporation.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA

*****************************************************************************/

/**************************************************/ /**
 @file include/fil0crypt.h
 The low-level file system encryption support functions

 Created 04/01/2015 Jan Lindström
 *******************************************************/

#ifndef fil0crypt_h
#define fil0crypt_h

#ifndef UNIV_INNOCHECKSUM

#include "log0types.h"
#include "my_crypt.h"
#include "os0event.h"
// TODO: Robert: This is temporary for fil_encryption_t
#include "fil0fil.h"
#include "ha_prototypes.h"

#endif /*! UNIV_INNOCHECKSUM */

#include "log0types.h"
#include "sql/system_variables.h"

/**
 * Magic pattern in start of crypt data on page 0
 */
#define MAGIC_SZ 6

struct trx_t;

static const unsigned char CRYPT_MAGIC[MAGIC_SZ] = {'s', 0xE, 0xC,
                                                    'R', 'E', 't'};

// static const char ENCRYPTION_PERCONA_SYSTEM_KEY_PREFIX[] = "percona_innodb";

#ifdef UNIV_INNOCHECKSUM  // TODO:Robert INNOCHECKSUM ENCRYPTION_KEY_LEN is not
                          // defined - and probably all of this file should not
                          // be
static const ulint ENCRYPTION_KEY_LEN = 32;  // TODO:Robert kind of workaround
#endif                                       // UNIV_INNOCHECKSUM

/* This key will be used if nothing else is given */
//#define FIL_DEFAULT_ENCRYPTION_KEY 0
//#define ENCRYPTION_KEY_VERSION_INVALID        (~(unsigned int)0)
//#define ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED  (~(unsigned int)0) - 1
//#define ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED  0

extern os_event_t fil_crypt_threads_event;

/**
 * CRYPT_SCHEME_UNENCRYPTED
 *
 * Used as intermediate state when convering a space from unencrypted
 * to encrypted
 */
/**
 * CRYPT_SCHEME_1
 *
 * xxx is AES_CTR or AES_CBC (or another block cypher with the same key and iv
 * lengths) L = AES_ECB(KEY, IV) CRYPT(PAGE) = xxx(KEY=L, IV=C, PAGE)
 */

//#define CRYPT_SCHEME_1 1
#define CRYPT_SCHEME_1_IV_LEN 16
//#define CRYPT_SCHEME_UNENCRYPTED 0

// TODO:Robert:Those are mine
//#define MY_AES_MAX_KEY_LENGTH 16
//#define ENCRYPTION_SCHEME_BLOCK_LENGTH 16

/* Cached L or key for given key_version */
struct key_struct {
  uint key_version;                       /*!< Version of the key */
  uint key_length;                        /*!< Key length */
  unsigned char key[Encryption::KEY_LEN]; /*!< Cached key
                                          (that is L in CRYPT_SCHEME_1) */
};

// enum fil_encryption_t {
//[>* Encrypted if innodb_encrypt_tables=ON (srv_encrypt_tables) <]
// FIL_ENCRYPTION_DEFAULT,
//[>* Encrypted <]
// FIL_ENCRYPTION_ON,
//[>* Not encrypted <]
// FIL_ENCRYPTION_OFF
//};

struct st_encryption_scheme_key {
  unsigned int version;
  // unsigned char key[ENCRYPTION_SCHEME_BLOCK_LENGTH];
  uchar *key;
};

struct Cached_key {
  byte *key;
  uint key_version;
  size_t key_len;

  ~Cached_key() {
    if (key != NULL) {
      memset_s(key, Encryption::KEY_LEN, 0, Encryption::KEY_LEN);
      my_free(key);
    }
  }
};

/** is encryption enabled */
extern enum_default_table_encryption srv_default_table_encryption;

struct fil_space_rotate_state_t {
  fil_space_rotate_state_t() : trx(nullptr), flush_observer(nullptr) {}

  time_t start_time;          /*!< time when rotation started */
  ulint active_threads;       /*!< active threads in space */
  page_no_t next_offset;      /*!< next "free" offset */
  page_no_t max_offset;       /*!< max offset needing to be rotated */
  uint min_key_version_found; /*!< min key version found but not
                              rotated */
  lsn_t end_lsn;              /*!< max lsn created when rotating this
                              space */
  bool starting;              /*!< initial write of IV */
  bool flushing;              /*!< space is being flushed at end of rotate */
  struct {
    bool is_active;              /*!< is scrubbing active in this space */
    time_t last_scrub_completed; /*!< when was last scrub
                                 completed */
  } scrubbing;

  trx_t *trx;
  FlushObserver *flush_observer;

  void create_flush_observer(space_id_t space_id);

  void destroy_flush_observer();
};

#ifndef UNIV_INNOCHECKSUM

enum Crypt_key_operation { FETCH_KEY, FETCH_OR_GENERATE_KEY };
enum class Validation_key_verions_result {
  MISSING_KEY_VERSIONS,
  CORRUPTED_OR_WRONG_KEY_VERSIONS,
  SUCCESS
};

struct fil_space_crypt_t {
 public:
  /** Constructor. Does not initialize the members!
  The object is expected to be placed in a buffer that
  has been zero-initialized. */
  fil_space_crypt_t(uint new_min_key_version, uint new_key_id, const char *uuid,
                    fil_encryption_t new_encryption,
                    Crypt_key_operation key_operation);

  /** Destructor */
  ~fil_space_crypt_t() {
    mutex_free(&mutex);
    mutex_free(&start_rotate_mutex);
    if (tablespace_key != nullptr) ut::free(tablespace_key);

    unload_keys_from_local_cache();

    rotate_state.destroy_flush_observer();
  }

  /** Get latest key version from encryption plugin
  @retval key_version or
  @retval ENCRYPTION_KEY_VERSION_INVALID if used key_id
  is not found from encryption plugin. */
  uint key_get_latest_version(void);

  /** Returns true if key was found from encryption plugin
  and false if not. */
  bool is_key_found() const {
    // return found_key_version != ENCRYPTION_KEY_VERSION_INVALID;
    return key_found;
  }

  /** Returns true if tablespace should be encrypted */
  bool should_encrypt() const {
    return (
        (encryption == FIL_ENCRYPTION_ON) ||
        (srv_default_table_encryption == DEFAULT_TABLE_ENC_ONLINE_TO_KEYRING &&
         encryption == FIL_ENCRYPTION_DEFAULT));
  }

  /** Return true if encryption for this table is disabled. */
  bool is_encryption_disabled() const {
    return (encryption == FIL_ENCRYPTION_OFF);
  }

  /** Return true if default tablespace encryption is used, */
  bool is_default_encryption() const {
    return (encryption == FIL_ENCRYPTION_DEFAULT);
  }

  /** Write crypt data to a page (0)
  @param[in]	space	tablespace
  @param[in,out]	page0	first page of the tablespace
  @param[in,out]	mtr	mini-transaction
  @param[in]	        a_min_key_verion min key version used in encryption
  @param[in]            a_max_key_verion max key version used in encryption
  @param[in]            a_type encryption type */
  void write_page0(const fil_space_t *space, byte *page0, mtr_t *mtr,
                   uint a_min_key_version, uint a_max_key_version, uint a_type);

  void set_tablespace_key(const uchar *tablespace_key) {
    if (tablespace_key == NULL) {
      if (this->tablespace_key != NULL) ut::free(this->tablespace_key);
      this->tablespace_key = NULL;
    } else {
      if (this->tablespace_key == NULL)
        this->tablespace_key = (byte *)ut_malloc_nokey(Encryption::KEY_LEN);
      memcpy(this->tablespace_key, tablespace_key, Encryption::KEY_LEN);
    }
  }

  void set_iv(const uchar *iv) { memcpy(this->iv, iv, CRYPT_SCHEME_1_IV_LEN); }

  uint min_key_version;         // min key version for this space
  uint max_key_version;         // max key version for this space
  fil_encryption_t encryption;  // Encryption setup

  using Key_map = std::map<uint, byte *>;
  Key_map local_keys_cache;

  /** Load needed keys for encryption/decryption to local cache
  @return true - success, false - failure */
  bool load_keys_to_local_cache();
  /** Remove keys from local cache */
  void unload_keys_from_local_cache();

  ib_mutex_t
      start_rotate_mutex;  // mutex protecting starting of rotation of the space
  ib_mutex_t mutex;        // mutex protecting following variables

  /** Return code from encryption_key_get_latest_version.
  If ENCRYPTION_KEY_VERSION_INVALID encryption plugin
  could not find the key and there is no need to call
  get_latest_key_version again as keys are read only
  at startup. */
  // uint key_found;
  // uint found_key_version;
  bool key_found;

  // false if we are holding a mutex in backgroud thread
  bool mutex_lock_needed{false};

  fil_space_rotate_state_t rotate_state;

  Encryption_rotation encryption_rotation;

  uchar *tablespace_key;  // TODO:Make it private ?

  // In Oracle's tablespace encryption iv is ENCRYPTION_KEY_LEN long,
  // which is incorrect value - it should be always 128 bits.
  // In case of MK to KEYRING re-encryption we re-use MK iv for
  // Keyring encryption. Since only 128 bits is really used
  // by AES we only store the needed 128 bits of this iv.
  // During re-encryption we use this iv to decrypt MK encrypted
  // pages and encrypt pages with KEYRING.
  unsigned char iv[CRYPT_SCHEME_1_IV_LEN];

  unsigned int keyserver_requests;
  unsigned int key_id;
  unsigned int type;

  // Internally we have three versions of crypt_data written to page 0.
  // One starting with magic PSA, the second one starting with PSB and
  // the last onw tih PSC.
  // Here we store which magic we read : 1 - PSA, 2 - PSB, 3 - PSC
  size_t private_version{3};

  char uuid[Encryption::SERVER_UUID_LEN + 1];

  // A fix text that we encrypt with range of key versions: from min_key_version
  // to max_key_version. When we validate that key versions loaded from keyring
  // are the valid keys to decrypt space, we decrypt the validation tag starting
  // with max key version till min key version.
  byte encrypted_validation_tag[MY_AES_BLOCK_SIZE];

  /** Validate that encrypted_validation_tag can be decrypted by keys in range
  [min_key_version, max_key_Version] */
  Validation_key_verions_result validate_encryption_key_versions();

  /** Re encrypt validation tag with key versions from
  from_key_version to to_key_version
  @param[in]	        from_key_version - starting version
  @param[in]	        to_key_version   - ending version
  @return false - error, true - success */
  bool re_encrypt_validation_tag(const uint from_key_version,
                                 const uint to_key_version);

 private:
  /** load copies of keys from keyring to local cache.
  @param[in]            from_key_version - starting version
  @param[in]            to_key_version   - ending version
  @return false - error, true - success */
  bool load_keys_to_local_cache(const uint from_key_version,
                                const uint to_key_version);
};

/** Status info about encryption */
struct fil_space_crypt_status_t {
  space_id_t space;                  /*!< tablespace id */
  ulint scheme;                      /*!< encryption scheme */
  uint min_key_version;              /*!< min key version */
  uint max_key_version;              /*!< max key version */
  uint current_key_version;          /*!< current key version */
  uint keyserver_requests;           /*!< no of key requests to key server */
  uint key_id;                       /*!< current key_id */
  bool rotating;                     /*!< is key rotation ongoing */
  bool flushing;                     /*!< is flush at end of rotation ongoing */
  page_no_t rotate_next_page_number; /*!< next page if key rotating */
  page_no_t rotate_max_page_number;  /*!< max page if key rotating */
};

/** Statistics about encryption key rotation */
struct fil_crypt_stat_t {
  ulint pages_read_from_cache;
  ulint pages_read_from_disk;
  ulint pages_modified;
  ulint pages_flushed;
  ulint estimated_iops;
};

/** Status info about scrubbing */
struct fil_space_scrub_status_t {
  space_id_t space;                    /*!< tablespace id */
  bool compressed;                     /*!< is space compressed  */
  time_t last_scrub_completed;         /*!< when was last scrub completed */
  bool scrubbing;                      /*!< is scrubbing ongoing */
  time_t current_scrub_started;        /*!< when started current scrubbing */
  ulint current_scrub_active_threads;  /*!< current scrub active threads */
  ulint current_scrub_page_number;     /*!< current scrub page no */
  ulint current_scrub_max_page_number; /*!< current scrub max page no */
};

struct redo_log_key final {
  uint version;
  char key[Encryption::KEY_LEN];
  ulint read_count;
  ulint write_count;
  bool present;
};

/** Handles the fetching/generation/storing/etc of keyring redo log keys.

This class is *NOT* thread safe, as thread safety is not required.
Data is only accessed/modified on the following points:
* When the redo space is created, at startup
* During redo log recovery, at startup
* When the server UUID is generated, at startup
* When the user requests a new key version, checked periodically in the
   master thread

As these can't happen in parallel, no lock is used. */
class redo_log_keys final {
 public:
  /** Loads the latest redo log key from the keyring.
  @param[in]	generate If true, a key is generated if an existing key can't
  be loaded. */
  MY_NODISCARD
  redo_log_key *load_latest_key(THD *thd, bool generate);
  MY_NODISCARD
  redo_log_key *load_key_version(THD *thd, const char *uuid, uint version);

  MY_NODISCARD
  redo_log_key *generate_and_store_new_key(THD *thd);

  /** Fetch if exists default percona_redo key, in case it does not
  exist - generate it in keyring. Should be used when server_uuid is not
  yet available
  @param[in] thd - connection thread
  @return percona_redo default key */
  MY_NODISCARD
  redo_log_key *fetch_or_generate_default_key(THD *thd);

 private:
  /**
  Get KEYRING encryption redo key name
  @param[in] - uuid key's UUID
  @param[in] - key_version key's version
  @return KEYRING encryption redo key name */
  std::string get_key_name(const char *uuid, uint key_version);
  /**
  Get KEYRING encryption redo key name
  @param[in] uuid - key's UUID
  @return KEYRING encryption redo key name */
  std::string get_key_name(const char *uuid);
  /**
  Get KEYRING encryption redo key name
  @param[in,out]  oss - output string stream
  @param[in] uuid - key's UUID */
  void get_key_name(std::ostringstream &oss, const char *uuid);

  using key_map = std::map<ulint, redo_log_key>;
  key_map m_keys;
};

extern redo_log_keys redo_log_key_mgr;

/**
Exclude tablespace from encryption threads rotation. This is "permament"
exclusion, i.e. ENCRYPTION of a tablespace was explicitly set to 'N'.
@param[in] space tablespace to exclude
@return false tablespace cannot be excluded because there are encryption
              threads currently operating on it.
        true  success
*/
bool fil_crypt_exclude_tablespace_from_rotation_permanently(fil_space_t *space);

/**
Exclude tablespace from encryption threads rotation. This is "temporary"
exclusion, i.e. tablespace ENCRYPTION is set to Y for Master Key encryption
or to N for Master Key decryption. While the Master Key encryption/decryption
is running, encryption threads should not interfere.
@param[in] space tablespace to exclude
@return false tablespace cannot be excluded because there are encryption
              threads currently operating on it.
        true  success
*/
bool fil_crypt_exclude_tablespace_from_rotation_temporarily(fil_space_t *space);

/**
Re-adds temporarily excluded tablespace to rotation threads
@param[in] space tablespace to re-added
*/
void fil_crypt_readd_space_to_rotation(space_id_t space_id);

/*********************************************************************
Init space crypt */
void fil_space_crypt_init();

/*********************************************************************
Cleanup space crypt */
void fil_space_crypt_cleanup();

/**
Create a fil_space_crypt_t object
@param[in]	encrypt_mode	FIL_ENCRYPTION_DEFAULT or
                                FIL_ENCRYPTION_ON or
                                FIL_ENCRYPTION_OFF

@param[in]	key_id		Encryption key id
@return crypt object */
fil_space_crypt_t *fil_space_create_crypt_data(
    fil_encryption_t encrypt_mode, uint key_id, const char *uuid,
    Crypt_key_operation key_operation =
        Crypt_key_operation::FETCH_OR_GENERATE_KEY)
    MY_ATTRIBUTE((warn_unused_result));

/******************************************************************
Merge fil_space_crypt_t object
@param[in,out]	dst		Destination cryp data
@param[in]	src		Source crypt data */
void fil_space_merge_crypt_data(fil_space_crypt_t *dst,
                                const fil_space_crypt_t *src);

/** Initialize encryption parameters from a tablespace header page.
@param[in]	page_size	page size of the tablespace
@param[in]	page		first page of the tablespace
@return crypt data from page 0
@retval	NULL	if not present or not valid */
fil_space_crypt_t *fil_space_read_crypt_data(const page_size_t &page_size,
                                             const byte *page);

// bool fil_space_read_crypt_data(const page_size_t& page_size, const byte*
// page, ulint space_id);

/**
Free a crypt data object
@param[in,out] crypt_data	crypt data to be freed */
void fil_space_destroy_crypt_data(fil_space_crypt_t **crypt_data);

/** Parse a MLOG_FILE_WRITE_CRYPT_DATA log entry
@param[in]  space_id  id of space that this log entry refers to
@param[in]  ptr  Log entry start
@param[in]  end_ptr  Log entry end
@param[in]  len  Log entry length
@return position on log buffer */
byte *fil_parse_write_crypt_data_v1(space_id_t space_id, byte *ptr,
                                    const byte *end_ptr, ulint len, lsn_t lsn)
    MY_ATTRIBUTE((warn_unused_result));

/** Parse a MLOG_FILE_WRITE_CRYPT_DATA log entry
@param[in]  space_id  id of space that this log entry refers to
@param[in]  ptr  Log entry start
@param[in]  end_ptr  Log entry end
@param[in]  len  Log entry length
@return position on log buffer */
byte *fil_parse_write_crypt_data_v2(space_id_t space_id, byte *ptr,
                                    const byte *end_ptr, ulint len, lsn_t lsn)
    MY_ATTRIBUTE((warn_unused_result));

/** Parse a MLOG_FILE_WRITE_CRYPT_DATA log entry
@param[in]  space_id  id of space that this log entry refers to
@param[in]  ptr  Log entry start
@param[in]  end_ptr  Log entry end
@param[in]  len  Log entry length
@param[in]  recv_needed_recovery  Missing keys will report an error
@return position on log buffer */
byte *fil_parse_write_crypt_data_v3(space_id_t space_id, byte *ptr,
                                    const byte *end_ptr, ulint len,
                                    bool recv_needed_recovery, lsn_t lsn)
    MY_ATTRIBUTE((warn_unused_result));

/**
Decrypt a page.
@param[in,out]	crypt_data		crypt_data
@param[in]	tmp_frame		Temporary buffer
@param[in]	page_size		Page size
@param[in,out]	src_frame		Page to decrypt
@param[out]	err			DB_SUCCESS or error
@return true if page decrypted, false if not.*/
bool fil_space_decrypt(fil_space_crypt_t *crypt_data, byte *tmp_frame,
                       const page_size_t &page_size, byte *src_frame,
                       dberr_t *err);

/******************************************************************
Decrypt a page
@param[in]	space			Tablespace
@param[in]	tmp_frame		Temporary buffer used for decrypting
@param[in,out]	src_frame		Page to decrypt
@param[out]	decrypted		true if page was decrypted
@return decrypted page, or original not encrypted page if decryption is
not needed.*/
byte *fil_space_decrypt(const fil_space_t *space, byte *tmp_frame,
                        byte *src_frame, bool *decrypted)
    MY_ATTRIBUTE((warn_unused_result));

/******************************************************************
Calculate post encryption checksum
@param[in]	page_size	    page size
@param[in]	page	            page where checksum is calculated
@param[in]      is_zip_compressed   is page compressed with old schema
@return page checksum or BUF_NO_CHECKSUM_MAGIC
not needed. */
uint32_t fil_crypt_calculate_checksum(const ulint page_size, const byte *page,
                                      const bool is_zip_compressed)
    MY_ATTRIBUTE((warn_unused_result));

/**
Verify that post encryption checksum match calculated checksum.
This function should be called only if tablespace contains crypt_data
metadata (this is strong indication that tablespace is encrypted).
Function also verifies that traditional checksum does not match
calculated checksum as if it does page could be valid unencrypted,
encrypted, or corrupted.

@param[in,out]	page		page frame (checksum is temporarily modified)
@param[in]	page_size	page size
@return true if page is encrypted AND OK, false otherwise */
bool fil_space_verify_crypt_checksum(byte *page, ulint page_size,
                                     bool is_zip_compressed,
                                     bool is_new_schema_compressed)
    MY_ATTRIBUTE((warn_unused_result));

/*********************************************************************
Adjust thread count for key rotation
@param[in]	enw_cnt		Number of threads to be used */
void fil_crypt_set_thread_cnt(uint new_cnt);

/*********************************************************************
Adjust max key age
@param[in]	val		New max key age */
void fil_crypt_set_rotate_key_age(uint val);

/*********************************************************************
Adjust rotation iops
@param[in]	val		New max roation iops */
void fil_crypt_set_rotation_iops(uint val);

/*********************************************************************
Adjust encrypt tables
@param[in]	val		New setting for innodb-encrypt-tables
@param[in]      is_server_starting - true if server is starting */
bool fil_crypt_set_encrypt_tables(enum_default_table_encryption val, bool is_server_starting);

/*********************************************************************
Init threads for key rotation */
void fil_crypt_threads_init();

/*********************************************************************
Clean up key rotation threads resources */
void fil_crypt_threads_cleanup();

/*********************************************************************
Wait for crypt threads to stop accessing space
@param[in]	space		Tablespace */
void fil_space_crypt_close_tablespace(const fil_space_t *space);

/*********************************************************************
Get crypt status for a space (used by information_schema)
@param[in]	space		Tablespace
@param[out]	status		Crypt status
return 0 if crypt data present */
void fil_space_crypt_get_status(const fil_space_t *space,
                                struct fil_space_crypt_status_t *status);

/*********************************************************************
Return crypt statistics
@param[out]	stat		Crypt statistics */
void fil_crypt_total_stat(fil_crypt_stat_t *stat);

/**
Get scrub status for a space (used by information_schema)

@param[in]	space		Tablespace
@param[out]	status		Scrub status
return 0 if data found */
void fil_space_get_scrub_status(const fil_space_t *space,
                                fil_space_scrub_status_t *status);

/**
Checks if tablespace is encrypted with KEYRING encryption v1

@param[in] space Tablespace
return true - fully or partially encrypted with keyring
              encryption v1
       false - is not encrypted, fully or partially with
              keyring encryption v1 */
bool is_space_keyring_pre_v3_encrypted(fil_space_t *space);

/**
Checks if tablespace is encrypted with KEYRING encryption v1

@param[in] space_id Tablespace's id
return true - fully or partially encrypted with keyring
              encryption v1
       false - is not encrypted, fully or partially with
              keyring encryption v1 */
bool is_space_keyring_pre_v3_encrypted(space_id_t space_id);

#endif /* !UNIV_INNOCHECKSUM */

#endif /* fil0crypt_h */
