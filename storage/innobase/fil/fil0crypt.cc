/*****************************************************************************
Copyright (C) 2013, 2015, Google Inc. All Rights Reserved.
Copyright (c) 2014, 2017, MariaDB Corporation.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

*****************************************************************************/
/**************************************************/ /**
 @file fil0crypt.cc
 Innodb file space encrypt/decrypt

 Created            Jonas Oreland Google
 Modified           Jan Lindström jan.lindstrom@mariadb.com
 *******************************************************/

#include <algorithm>
#include "fil0fil.h"
#include "mach0data.h"
#include "mtr0types.h"
#include "mysqld.h"  // server_uuid
#include "page0size.h"
#include "page0zip.h"
#include "system_key.h"
#ifndef UNIV_INNOCHECKSUM
#include <my_crypt.h>
#include "btr0scrub.h"
#include "buf0buf.h"
#include "buf0flu.h"
#include "fil0crypt.h"
#include "fsp0fsp.h"
#include "ha_prototypes.h"  // IB_LOG_
#include "keyring_operations_helper.h"
#include "log0recv.h"
#include "mtr0log.h"
#include "mtr0mtr.h"
#include "srv0srv.h"
#include "srv0start.h"
#include "ut0ut.h"

#include "dict0dd.h"
#include "dict0dict.h"
#include "fts0priv.h"
#include "lock0lock.h"
#include "my_dbug.h"
#include "pars0pars.h"
#include "que0que.h"
#include "row0mysql.h"
#include "row0sel.h"
#include "trx0trx.h"  // for updating data dictionary

#include "os0file.h"

#include <list>
#include "sql_thd_internal_api.h"

#define ENCRYPTION_MASTER_KEY_NAME_MAX_LEN 100

#ifdef UNIV_DEBUG
static int number_of_t1_pages_rotated{0};
// we set it to 100 first - so the space would be considered to
// rotation, later we change it to different value - depends on
// how many pages we do wait.
static int number_of_t1_pages_to_rotate{100};
#endif

/** Mutex for keys */
static ib_mutex_t fil_crypt_key_mutex;

static bool fil_crypt_threads_inited = false;

/** No of key rotation threads requested */
uint srv_n_fil_crypt_threads_requested = 0;

/** At this age or older a space/page will be rotated */
uint srv_fil_crypt_rotate_key_age;

/** Event to signal FROM the key rotation threads. */
static os_event_t fil_crypt_event;

/** Event to signal TO the key rotation threads. */
os_event_t fil_crypt_threads_event;

/** Event for waking up threads throttle. */
static os_event_t fil_crypt_throttle_sleep_event;

/** Mutex for key rotation threads. */
ib_mutex_t fil_crypt_threads_mutex;

/** Mutex for setting cnt of threads */
ib_mutex_t fil_crypt_threads_set_cnt_mutex;

/** Mutex for accessing space_list and key_rotation */
ib_mutex_t fil_crypt_list_mutex;

/** Variable ensuring only 1 thread at time does initial conversion */
static bool fil_crypt_start_converting = false;

/** Variables for throttling */
uint srv_n_fil_crypt_iops = 100;  // 10ms per iop
static uint srv_alloc_time = 3;   // allocate iops for 3s at a time
static uint n_fil_crypt_iops_allocated = 0;

extern uint srv_background_scrub_data_interval;
extern uint srv_background_scrub_data_check_interval;
extern bool srv_background_scrub_data_uncompressed;
extern bool srv_background_scrub_data_compressed;

extern bool mysqld_server_started;

EncryptionKeyId get_global_default_encryption_key_id_value();

static constexpr byte ENCRYPTION_KEYRING_VALIDATION_TAG[] = {
    'E', 'N', 'C', '_', 'V', 'A', 'L', '_',
    'T', 'A', 'G', '_', 'V', '1', '_', '1'};
static constexpr size_t ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE =
    MY_AES_BLOCK_SIZE;
static_assert(sizeof(ENCRYPTION_KEYRING_VALIDATION_TAG) == MY_AES_BLOCK_SIZE,
              "Size of ENCRYPTION_KEYRING_VALIDATION_TAG must be equal to size "
              "of the output of AES crypto, i.e. MY_AES_BLOCK_SIZE");

static constexpr uint ENCRYPTION_SERVER_UUID_HEX_LEN = 16;

#define DEBUG_KEYROTATION_THROTTLING 0

static constexpr uint KERYING_ENCRYPTION_INFO_MAX_SIZE =
    Encryption::MAGIC_SIZE + 1                 // type
    + 4                                        // min_key_version
    + 4                                        // max_key_version
    + 4                                        // key_id
    + 1                                        // encryption
    + CRYPT_SCHEME_1_IV_LEN                    // iv (16 bytes)
    + 1                                        // encryption rotation type
    + Encryption::KEY_LEN                      // tablespace key
    + Encryption::SERVER_UUID_HEX_LEN          // server's UUID written in hex
    + ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE;  // validation tag

static constexpr uint KERYING_ENCRYPTION_INFO_MAX_SIZE_V2 =
    Encryption::MAGIC_SIZE + 1      // type
    + 4                             // min_key_version
    + 4                             // key_id
    + 1                             // encryption
    + CRYPT_SCHEME_1_IV_LEN         // iv (16 bytes)
    + 1                             // encryption rotation type
    + Encryption::KEY_LEN           // tablespace key
    + Encryption::SERVER_UUID_LEN;  // server's UUID

static constexpr uint KERYING_ENCRYPTION_INFO_MAX_SIZE_V1 =
    Encryption::MAGIC_SIZE + 2  // length of iv
    + 4                         // space id
    + 2                         // offset
    + 1                         // type
    + 4                         // min_key_version
    + 4                         // key_id
    + 1                         // encryption
    + CRYPT_SCHEME_1_IV_LEN     // iv (16 bytes)
    + 4                         // encryption rotation type
    + Encryption::KEY_LEN       // tablespace key
    + Encryption::KEY_LEN;      // tablespace iv

/* The size of keyring key encrption header cannot cross the Master
Key header. This is because the bytes followed by MK header are used
by other features like SDI, encryption progress (MK) etc. If we cross
the MK encryption header size ENCRYPTION_INFO_MAX_SIZE, we corrupt the
header and other features will not work */
static_assert(KERYING_ENCRYPTION_INFO_MAX_SIZE < Encryption::INFO_MAX_SIZE,
              "Keyring key encryption header crosses Master Key encryption"
              " header size");

void fil_space_crypt_t::unload_keys_from_local_cache() {
  for (auto item : local_keys_cache) {
    if (item.second != nullptr) {
      memset(item.second, 0, Encryption::KEY_LEN);
      my_free(item.second);
    }
  }
  local_keys_cache.clear();
}

bool fil_space_crypt_t::load_keys_to_local_cache(const uint from_key_version,
                                                 const uint to_key_version) {
  for (uint key_version = from_key_version; key_version <= to_key_version;
       ++key_version) {
    ut_ad(key_version != ENCRYPTION_KEY_VERSION_INVALID &&
          key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED);

    if (local_keys_cache[key_version] == nullptr) {
      size_t key_length{0};

      Encryption::get_tablespace_key(this->key_id, this->uuid, key_version,
                                     &local_keys_cache[key_version],
                                     &key_length);
      if (local_keys_cache[key_version] == nullptr) {
        return false;
      }
    }
  }
  return true;
}

bool fil_space_crypt_t::load_keys_to_local_cache() {
  if (min_key_version == ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED &&
      max_key_version == ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED)
    return true;  // tablespace not encrypted - no keys to fetch

  // in case space is not encrypted we need to only load max_key_version,
  // min_key_version will be just unencrypted. In case it is encrypted
  // min_key_version might be 0, but it's not a valid key version - just
  // a marker that some pages are unencrypted.
  uint start_version = (type == CRYPT_SCHEME_UNENCRYPTED)
                           ? max_key_version
                           : std::max(min_key_version, static_cast<uint>(1));

  ut_ad(start_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED);

  return load_keys_to_local_cache(start_version, max_key_version);
}

/** Statistics variables */
static fil_crypt_stat_t crypt_stat;
static ib_mutex_t crypt_stat_mutex;

/***********************************************************************
Check if a key needs rotation given a key_state
@param[in]	encrypt_mode		Encryption mode
@param[in]	key_version		Current key version
@param[in]	latest_key_version	Latest key version
@param[in]	rotate_key_age		when to rotate
@return true if key needs rotation, false if not */
static bool fil_crypt_needs_rotation(fil_encryption_t encrypt_mode,
                                     uint key_version, uint latest_key_version,
                                     uint rotate_key_age)
    MY_ATTRIBUTE((warn_unused_result));

static bool encrypt_validation_tag(const byte *secret, const size_t secret_size,
                                   const byte *key, byte *encrypted_secret) {
  auto elen =
      my_aes_encrypt(secret, secret_size, encrypted_secret, key,
                     Encryption::KEY_LEN, my_aes_256_ecb, nullptr, false);

  if (elen == MY_AES_BAD_DATA) {
    return false;
  }

  return true;
}

/*********************************************************************
Init space crypt */
void fil_space_crypt_init() {
  mutex_create(LATCH_ID_FIL_CRYPT_MUTEX, &fil_crypt_key_mutex);

  fil_crypt_throttle_sleep_event = os_event_create();

  mutex_create(LATCH_ID_FIL_CRYPT_STAT_MUTEX, &crypt_stat_mutex);
  memset(&crypt_stat, 0, sizeof(crypt_stat));
}

/*********************************************************************
Cleanup space crypt */
void fil_space_crypt_cleanup() {
  os_event_destroy(fil_crypt_throttle_sleep_event);
  mutex_free(&fil_crypt_key_mutex);
  mutex_free(&crypt_stat_mutex);
}

fil_space_crypt_t::fil_space_crypt_t(uint new_min_key_version, uint new_key_id,
                                     const char *new_uuid,
                                     fil_encryption_t new_encryption,
                                     Crypt_key_operation key_operation)
    : min_key_version(new_min_key_version),
      encryption(new_encryption),
      key_found(false),
      rotate_state(),
      tablespace_key(NULL) {
  encryption_rotation =
      (new_encryption == FIL_ENCRYPTION_DEFAULT &&
       srv_default_table_encryption == DEFAULT_TABLE_ENC_ONLINE_TO_KEYRING)
          ? Encryption_rotation::ENCRYPTING
          : Encryption_rotation::NO_ROTATION;

  mutex_create(LATCH_ID_FIL_CRYPT_START_ROTATE_MUTEX, &start_rotate_mutex);
  mutex_create(LATCH_ID_FIL_CRYPT_DATA_MUTEX, &mutex);

  memcpy(encrypted_validation_tag, ENCRYPTION_KEYRING_VALIDATION_TAG,
         ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);

  key_id = new_key_id;
  if (my_random_bytes(iv, sizeof(iv)) != MY_AES_OK)  // TODO:Robert: This can
                                                     // return error and because
                                                     // of that it should not be
                                                     // in constructor
    type = 0;  // TODO:Robert: This is temporary to get rid of unused variable
               // problem
  if (new_uuid != nullptr && strlen(new_uuid) > 0) {
    memcpy(uuid, new_uuid, Encryption::SERVER_UUID_LEN);
    uuid[Encryption::SERVER_UUID_LEN] = '\0';
  } else {
    uuid[0] = '\0';
  }

  if (strlen(new_uuid) == 0) {
    key_found = false;
    min_key_version = max_key_version = ENCRYPTION_KEY_VERSION_INVALID;
    // This was read - because when creating new crypt data - it means that
    // uuid is never empty. type will be overwritten by read function
  } else if (new_encryption == FIL_ENCRYPTION_OFF) {
    type = CRYPT_SCHEME_UNENCRYPTED;
    key_found = false;
    min_key_version = ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;
    max_key_version = ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;
  } else if (Encryption::is_online_encryption_on() == false &&
             new_encryption == FIL_ENCRYPTION_DEFAULT) {
    type = CRYPT_SCHEME_UNENCRYPTED;
    min_key_version = ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;
    max_key_version = ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;
    key_found =
        key_operation == FETCH_OR_GENERATE_KEY
            ? Encryption::
                  tablespace_key_exists_or_create_new_one_if_does_not_exist(
                      key_id, uuid)
            : Encryption::tablespace_key_exists(key_id, uuid);
  } else {
    type = CRYPT_SCHEME_1;
    // key_found = true; // cheat key_get_latest_version that the key exists -
    // if it does not it will return ENCRYPTION_KEY_VERSION_INVALID
    uchar *key = nullptr;
    uint key_version = ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;
    if (key_operation == FETCH_OR_GENERATE_KEY) {
      Encryption::get_latest_key_or_create(key_id, uuid, &key_version, &key);
    } else if (key_operation == FETCH_KEY) {
      Encryption::get_latest_tablespace_key(key_id, uuid, &key_version, &key);
    } else {
      ut_ad(0);
    }
    if (key == nullptr) {
      key_found = false;
      min_key_version = max_key_version = ENCRYPTION_KEY_VERSION_INVALID;
    } else {
      key_found = true;
      min_key_version = max_key_version = key_version;
      // we are creating new encrypted space, we need to encrypt validation tag
      if (key_operation == FETCH_OR_GENERATE_KEY &&
          encrypt_validation_tag(ENCRYPTION_KEYRING_VALIDATION_TAG,
                                 ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE, key,
                                 encrypted_validation_tag) == false) {
        ut_ad(false);
        key_found = false;
      }
    }
    my_free(key);
  }
}

/**
Get latest key version from encryption plugin.
@return key version or ENCRYPTION_KEY_VERSION_INVALID */
uint fil_space_crypt_t::key_get_latest_version(void) {
  uint key_version = ENCRYPTION_KEY_VERSION_INVALID;

  if (is_key_found()) {  // TODO:Robert:This blocks new version from being found
                         // - if it once read - it stays the same
    key_version = Encryption::encryption_get_latest_version(key_id, uuid);
    srv_stats.n_key_requests.inc();
  }

  return key_version;
}

/******************************************************************
Get the latest(key-version), waking the encrypt thread, if needed
@param[in,out]	crypt_data	Crypt data */
static inline uint fil_crypt_get_latest_key_version(
    fil_space_crypt_t *crypt_data) {
  ut_ad(crypt_data != NULL);

  uint key_version = crypt_data->key_get_latest_version();

  if (crypt_data->is_key_found()) {
    if (fil_crypt_needs_rotation(crypt_data->encryption,
                                 crypt_data->min_key_version, key_version,
                                 srv_fil_crypt_rotate_key_age)) {
      /* Below event seen as NULL-pointer at startup
      when new database was created and we create a
      checkpoint. Only seen when debugging. */
      if (fil_crypt_threads_inited) {
        os_event_set(fil_crypt_threads_event);
      }
    }
  }

  return key_version;
}

/******************************************************************
Create a fil_space_crypt_t object
@param[in]	type		CRYPT_SCHEME_UNENCRYPTE or
                                CRYPT_SCHEME_1
@param[in]	encrypt_mode	FIL_ENCRYPTION_DEFAULT or
                                FIL_ENCRYPTION_ON or
                                FIL_ENCRYPTION_OFF
@param[in]	min_key_version key_version or 0
@param[in]	key_id		Used key id
@return crypt object */

static fil_space_crypt_t *fil_space_create_crypt_data(
    fil_encryption_t encrypt_mode, uint min_key_version, uint key_id,
    const char *uuid,
    Crypt_key_operation key_operation =
        Crypt_key_operation::FETCH_OR_GENERATE_KEY) {
  fil_space_crypt_t *crypt_data = NULL;
  if (void *buf = ut_zalloc_nokey(sizeof(fil_space_crypt_t))) {
    crypt_data = new (buf) fil_space_crypt_t(min_key_version, key_id, uuid,
                                             encrypt_mode, key_operation);
  }

  return crypt_data;
}

void fil_space_rotate_state_t::create_flush_observer(space_id_t space_id) {
  destroy_flush_observer();
  trx = trx_allocate_for_background();
  flush_observer = UT_NEW_NOKEY(FlushObserver(space_id, trx, NULL));
  trx_set_flush_observer(trx, flush_observer);
}

/* Forces flush of all pages observed by flush observer
 * and destroys the transaction associated with flush observer.
 * When called crypt_data->mutex cannot be owned because underlying
 * i/o layer need to take this mutex */
void fil_space_rotate_state_t::destroy_flush_observer() {
  if (flush_observer != nullptr) {
    flush_observer->flush();
    UT_DELETE(flush_observer);
    flush_observer = nullptr;
  }
  if (trx != nullptr) {
    trx_free_for_background(trx);
    trx = nullptr;
  }
}

/******************************************************************
Create a fil_space_crypt_t object
@param[in]	encrypt_mode	FIL_ENCRYPTION_DEFAULT or
                                FIL_ENCRYPTION_ON or
                                FIL_ENCRYPTION_OFF

@param[in]	key_id		Encryption key id
@return crypt object */
fil_space_crypt_t *fil_space_create_crypt_data(
    fil_encryption_t encrypt_mode, uint key_id, const char *uuid,
    Crypt_key_operation key_operation) {
  return (fil_space_create_crypt_data(encrypt_mode,
                                      ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED,
                                      key_id, uuid, key_operation));
}

bool is_space_keyring_pre_v3_encrypted(fil_space_t *space) {
  ut_ad(space != nullptr);
  return space->crypt_data != nullptr &&
         (space->crypt_data->private_version == 1 ||
          space->crypt_data->private_version == 2) &&
         space->crypt_data->type != CRYPT_SCHEME_UNENCRYPTED;
}

bool is_space_keyring_pre_v3_encrypted(space_id_t space_id) {
  fil_space_t *space = fil_space_get(space_id);
  return is_space_keyring_pre_v3_encrypted(space);
}

/******************************************************************
Merge fil_space_crypt_t object
@param[in,out]	dst		Destination cryp data
@param[in]	src		Source crypt data */
void fil_space_merge_crypt_data(fil_space_crypt_t *dst,
                                const fil_space_crypt_t *src) {
  mutex_enter(&dst->mutex);

  /* validate that they are mergeable */
  ut_a(src->type == CRYPT_SCHEME_UNENCRYPTED || src->type == CRYPT_SCHEME_1);

  ut_a(dst->type == CRYPT_SCHEME_UNENCRYPTED || dst->type == CRYPT_SCHEME_1);

  dst->encryption = src->encryption;
  dst->type = src->type;
  dst->min_key_version = src->min_key_version;
  dst->keyserver_requests += src->keyserver_requests;

  mutex_exit(&dst->mutex);
}

static ulint fsp_header_get_keyring_encryption_offset(
    const page_size_t &page_size) {
  ulint offset;
#ifdef UNIV_DEBUG
  ulint left_size;
#endif

  offset = XDES_ARR_OFFSET + XDES_SIZE * xdes_arr_size(page_size);
#ifdef UNIV_DEBUG
  left_size =
      page_size.physical() - FSP_HEADER_OFFSET - offset - FIL_PAGE_DATA_END;
  ut_ad(left_size >= KERYING_ENCRYPTION_INFO_MAX_SIZE);
#endif

  return offset;
}

static fil_space_crypt_t *fil_space_read_crypt_data_v1(
    const page_size_t &page_size, const byte *page) {
  const ulint offset{fsp_header_get_keyring_encryption_offset(page_size)};

  ut_ad(memcmp(page + offset, Encryption::KEY_MAGIC_PS_V1,
               Encryption::MAGIC_SIZE) == 0);

  ulint bytes_read{Encryption::MAGIC_SIZE};

  uint8_t iv_length = mach_read_from_2(page + offset + bytes_read);
  ut_ad(iv_length == CRYPT_SCHEME_1_IV_LEN);
  bytes_read += 2;

  bytes_read += 4;  // skip space_id
  bytes_read += 2;  // skip offset

  uint8_t type = mach_read_from_1(page + offset + bytes_read);
  bytes_read += 1;

  fil_space_crypt_t *crypt_data;

  if ((type != CRYPT_SCHEME_UNENCRYPTED && type != CRYPT_SCHEME_1) ||
      iv_length != CRYPT_SCHEME_1_IV_LEN) {
    ib::error() << "Found non sensible crypt scheme: " << type
                << " for space: " << page_get_space_id(page)
                << " offset: " << offset << " bytes: ["
                << page[offset + 2 + Encryption::MAGIC_SIZE]
                << page[offset + 3 + Encryption::MAGIC_SIZE]
                << page[offset + 4 + Encryption::MAGIC_SIZE]
                << page[offset + 5 + Encryption::MAGIC_SIZE] << "].";
    return nullptr;
  }

  uint min_key_version = mach_read_from_4(page + offset + bytes_read);
  bytes_read += 4;

  uint key_id = mach_read_from_4(page + offset + bytes_read);
  bytes_read += 4;

  ut_ad(key_id != (uint)(~0));

  fil_encryption_t encryption =
      (fil_encryption_t)mach_read_from_1(page + offset + bytes_read);
  bytes_read += 1;

  crypt_data =
      fil_space_create_crypt_data(encryption, key_id, server_uuid,
                                  Crypt_key_operation::FETCH_OR_GENERATE_KEY);

  /* We need to overwrite these as above function will initialize
  members */
  crypt_data->type = type;
  crypt_data->min_key_version = min_key_version;
  // for encrypted space the upgrade would have failed
  crypt_data->max_key_version = ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;
  // set memory to ENCRYPTION_KEYRING_VALIDATION_TAG
  memcpy(crypt_data->encrypted_validation_tag,
         ENCRYPTION_KEYRING_VALIDATION_TAG,
         ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);
  crypt_data->private_version = 1;
  memcpy(crypt_data->iv, page + offset + bytes_read, CRYPT_SCHEME_1_IV_LEN);
  bytes_read += CRYPT_SCHEME_1_IV_LEN;

  crypt_data->encryption_rotation = static_cast<Encryption_rotation>(
      mach_read_from_4(page + offset + bytes_read));
  bytes_read += 4;

  uchar tablespace_key[Encryption::KEY_LEN];
  memcpy(tablespace_key, page + offset + bytes_read, Encryption::KEY_LEN);
  bytes_read += Encryption::KEY_LEN;

  if (std::search_n(tablespace_key, tablespace_key + Encryption::KEY_LEN,
                    Encryption::KEY_LEN,
                    0) ==
      tablespace_key) {  // tablespace_key is all zeroes which means there is no
                         // tablepsace in mtr log
    crypt_data->set_tablespace_key(nullptr);
  } else {
    crypt_data->set_tablespace_key(
        tablespace_key);  // We are using the same iv for both
                          // MK encryption and KEYRING encryption
  }

  return crypt_data;
}

static fil_space_crypt_t *fil_space_read_crypt_data_v2(
    const page_size_t &page_size, const byte *page) {
  const ulint offset = fsp_header_get_keyring_encryption_offset(page_size);

  if (memcmp(page + offset, Encryption::KEY_MAGIC_PS_V2,
             Encryption::MAGIC_SIZE) != 0) {
    /* Crypt data is not stored. */
    return nullptr;
  }

  ulint bytes_read{Encryption::MAGIC_SIZE};

  uint8_t type = mach_read_from_1(page + offset + bytes_read);

  ut_a(type == CRYPT_SCHEME_UNENCRYPTED ||
       type == CRYPT_SCHEME_1);  // only supported

  bytes_read += 1;

  fil_space_crypt_t *crypt_data;

  if (!(type == CRYPT_SCHEME_UNENCRYPTED || type == CRYPT_SCHEME_1)) {
    ib::error() << "Found non sensible crypt scheme: " << type
                << " for space: " << page_get_space_id(page)
                << " offset: " << offset << " bytes: ["
                << page[offset + 2 + Encryption::MAGIC_SIZE]
                << page[offset + 3 + Encryption::MAGIC_SIZE]
                << page[offset + 4 + Encryption::MAGIC_SIZE]
                << page[offset + 5 + Encryption::MAGIC_SIZE] << "].";
    return nullptr;
  }

  uint min_key_version = mach_read_from_4(page + offset + bytes_read);
  bytes_read += 4;

  uint key_id = mach_read_from_4(page + offset + bytes_read);
  bytes_read += 4;

  ut_ad(key_id != (uint)(~0));

  char uuid[Encryption::SERVER_UUID_LEN];
  memset(uuid, 0, Encryption::SERVER_UUID_LEN);
  memcpy(uuid, page + offset + bytes_read, Encryption::SERVER_UUID_LEN);
  bytes_read += Encryption::SERVER_UUID_LEN;

  ut_ad(strlen(uuid) > 0);

  fil_encryption_t encryption =
      (fil_encryption_t)mach_read_from_1(page + offset + bytes_read);
  bytes_read += 1;

  crypt_data = fil_space_create_crypt_data(encryption, key_id, uuid,
                                           Crypt_key_operation::FETCH_KEY);

  crypt_data->private_version = 2;

  /* We need to overwrite these as above function will initialize
  members */
  crypt_data->type = type;
  crypt_data->min_key_version = min_key_version;
  // for encrypted space the upgrade would have failed
  crypt_data->max_key_version = ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;
  // set memory to ENCRYPTION_KEYRING_VALIDATION_TAG
  memcpy(crypt_data->encrypted_validation_tag,
         ENCRYPTION_KEYRING_VALIDATION_TAG,
         ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);
  memcpy(crypt_data->iv, page + offset + bytes_read, CRYPT_SCHEME_1_IV_LEN);
  bytes_read += CRYPT_SCHEME_1_IV_LEN;

  crypt_data->encryption_rotation = static_cast<Encryption_rotation>(
      mach_read_from_1(page + offset + bytes_read));
  bytes_read += 1;

  uchar tablespace_key[Encryption::KEY_LEN];
  memcpy(tablespace_key, page + offset + bytes_read, Encryption::KEY_LEN);
  bytes_read += Encryption::KEY_LEN;

  if (std::search_n(tablespace_key, tablespace_key + Encryption::KEY_LEN,
                    Encryption::KEY_LEN,
                    0) ==
      tablespace_key) {  // tablespace_key is all zeroes which means there is no
                         // tablepsace in mtr log
    crypt_data->set_tablespace_key(nullptr);
  } else {
    crypt_data->set_tablespace_key(
        tablespace_key);  // We are using the same iv for both
                          // MK encryption and KEYRING encryption
  }

  return crypt_data;
}

static void hex_to_uuid(const uchar *hex, char *uuid) {
  char uuid_string[Encryption::SERVER_UUID_LEN + 1];
  snprintf(
      uuid_string, Encryption::SERVER_UUID_LEN + 1,
      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      hex[0], hex[1], hex[2], hex[3], hex[4], hex[5], hex[6], hex[7], hex[8],
      hex[9], hex[10], hex[11], hex[12], hex[13], hex[14], hex[15]);
  memcpy(uuid, uuid_string, Encryption::SERVER_UUID_LEN);
}

static fil_space_crypt_t *fil_space_read_crypt_data_v3(
    const page_size_t &page_size, const byte *page) {
  const ulint offset = fsp_header_get_keyring_encryption_offset(page_size);

  if (memcmp(page + offset, Encryption::KEY_MAGIC_PS_V3,
             Encryption::MAGIC_SIZE) != 0) {
    /* Crypt data is not stored. */
    return nullptr;
  }

  ulint bytes_read{Encryption::MAGIC_SIZE};

  uint8_t type = mach_read_from_1(page + offset + bytes_read);

  ut_a(type == CRYPT_SCHEME_UNENCRYPTED ||
       type == CRYPT_SCHEME_1);  // only supported

  bytes_read += 1;

  fil_space_crypt_t *crypt_data;

  if (!(type == CRYPT_SCHEME_UNENCRYPTED || type == CRYPT_SCHEME_1)) {
    ib::error() << "Found non sensible crypt scheme: " << type
                << " for space: " << page_get_space_id(page)
                << " offset: " << offset << " bytes: ["
                << page[offset + 2 + Encryption::MAGIC_SIZE]
                << page[offset + 3 + Encryption::MAGIC_SIZE]
                << page[offset + 4 + Encryption::MAGIC_SIZE]
                << page[offset + 5 + Encryption::MAGIC_SIZE] << "].";
    return nullptr;
  }

  uint min_key_version = mach_read_from_4(page + offset + bytes_read);
  bytes_read += 4;

  uint max_key_version = mach_read_from_4(page + offset + bytes_read);
  bytes_read += 4;

  uint key_id = mach_read_from_4(page + offset + bytes_read);
  bytes_read += 4;

  ut_ad(key_id != (uint)(~0));

  static uchar uuid_hex[ENCRYPTION_SERVER_UUID_HEX_LEN];
  memcpy(&uuid_hex, page + offset + bytes_read, ENCRYPTION_SERVER_UUID_HEX_LEN);

  bytes_read += ENCRYPTION_SERVER_UUID_HEX_LEN;

  static char uuid[Encryption::SERVER_UUID_LEN];
  memset(uuid, 0, Encryption::SERVER_UUID_LEN);
  hex_to_uuid(uuid_hex, uuid);

  ut_ad(strlen(uuid) > 0);

  fil_encryption_t encryption =
      (fil_encryption_t)mach_read_from_1(page + offset + bytes_read);
  bytes_read += 1;

  crypt_data = fil_space_create_crypt_data(encryption, key_id, uuid,
                                           Crypt_key_operation::FETCH_KEY);

  /* We need to overwrite these as above function will initialize
  members */
  crypt_data->type = type;
  crypt_data->min_key_version = min_key_version;
  crypt_data->max_key_version = max_key_version;
  // set memory to ENCRYPTION_KEYRING_VALIDATION_TAG
  memcpy(crypt_data->encrypted_validation_tag, page + offset + bytes_read,
         ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);
  bytes_read += ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE;

  memcpy(crypt_data->iv, page + offset + bytes_read, CRYPT_SCHEME_1_IV_LEN);
  bytes_read += CRYPT_SCHEME_1_IV_LEN;

  crypt_data->encryption_rotation = static_cast<Encryption_rotation>(
      mach_read_from_1(page + offset + bytes_read));
  bytes_read += 1;

  uchar tablespace_key[Encryption::KEY_LEN];
  memcpy(tablespace_key, page + offset + bytes_read, Encryption::KEY_LEN);
  bytes_read += Encryption::KEY_LEN;

  if (std::search_n(tablespace_key, tablespace_key + Encryption::KEY_LEN,
                    Encryption::KEY_LEN,
                    0) ==
      tablespace_key) {  // tablespace_key is all zeroes which means there is no
                         // tablepsace in mtr log
    crypt_data->set_tablespace_key(nullptr);
  } else {
    crypt_data->set_tablespace_key(
        tablespace_key);  // We are using the same iv for both
                          // MK encryption and KEYRING encryption
  }

  return crypt_data;
}

fil_space_crypt_t *fil_space_read_crypt_data(const page_size_t &page_size,
                                             const byte *page) {
  const ulint offset{fsp_header_get_keyring_encryption_offset(page_size)};

  if (memcmp(page + offset, Encryption::KEY_MAGIC_PS_V1,
             Encryption::MAGIC_SIZE) == 0) {
    return fil_space_read_crypt_data_v1(page_size, page);
  }

  if (memcmp(page + offset, Encryption::KEY_MAGIC_PS_V2,
             Encryption::MAGIC_SIZE) == 0) {
    return fil_space_read_crypt_data_v2(page_size, page);
  }

  if (memcmp(page + offset, Encryption::KEY_MAGIC_PS_V3,
             Encryption::MAGIC_SIZE) == 0) {
    return fil_space_read_crypt_data_v3(page_size, page);
  }

  /* Crypt data is not stored. */
  return nullptr;
}

/******************************************************************
Free a crypt data object
@param[in,out] crypt_data	crypt data to be freed */
void fil_space_destroy_crypt_data(fil_space_crypt_t **crypt_data) {
  if (crypt_data != NULL && (*crypt_data) != NULL) {
    fil_space_crypt_t *c;
    if (UNIV_LIKELY(fil_crypt_threads_inited)) {
      mutex_enter(&fil_crypt_threads_mutex);
      c = *crypt_data;
      *crypt_data = NULL;
      mutex_exit(&fil_crypt_threads_mutex);
    } else {
      // ut_ad(srv_read_only_mode || srv_is_being_started ||
      // srv_is_being_shutdown);
      // destroy_crypt_data can be also called on bootstrap,
      // I have not found any boostrap variable I could use here,
      // TODO: check if there is some boostrap variable or introduce one
      c = *crypt_data;
      *crypt_data = NULL;
    }
    if (c) {
      c->~fil_space_crypt_t();
      ut_free(c);
    } else {
      ut_ad(0);
    }
  }
}

static void uuid_to_hex(const char *uuid, byte *uuid_hex) {
  sscanf(uuid,
         "%2hhx%2hhx%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-"
         "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
         &uuid_hex[0], &uuid_hex[1], &uuid_hex[2], &uuid_hex[3], &uuid_hex[4],
         &uuid_hex[5], &uuid_hex[6], &uuid_hex[7], &uuid_hex[8], &uuid_hex[9],
         &uuid_hex[10], &uuid_hex[11], &uuid_hex[12], &uuid_hex[13],
         &uuid_hex[14], &uuid_hex[15]);
}

/******************************************************************
Write crypt data to a page (0)
@param[in]	space	tablespace
@param[in,out]	page0	first page of the tablespace
@param[in,out]	mtr	mini-transaction */

// TODO: Should be marked as const when PS-5738 is implemented
void fil_space_crypt_t::write_page0(const fil_space_t *space, byte *page,
                                    mtr_t *mtr, uint a_min_key_version,
                                    uint a_max_key_version, uint a_type) {
  ut_ad(this == space->crypt_data);
  const ulint offset =
      fsp_header_get_keyring_encryption_offset(page_size_t(space->flags));

  byte *encrypt_info = new byte[KERYING_ENCRYPTION_INFO_MAX_SIZE];
  byte *encrypt_info_ptr = encrypt_info;

  mlog_write_ulint(page + FSP_HEADER_OFFSET + FSP_SPACE_FLAGS, space->flags,
                   MLOG_4BYTES, mtr);  // done

  memcpy(encrypt_info_ptr, Encryption::KEY_MAGIC_PS_V3, Encryption::MAGIC_SIZE);
  encrypt_info_ptr += Encryption::MAGIC_SIZE;

  mach_write_to_1(encrypt_info_ptr, a_type);
  encrypt_info_ptr += 1;
  mach_write_to_4(encrypt_info_ptr, a_min_key_version);
  encrypt_info_ptr += 4;
  mach_write_to_4(encrypt_info_ptr, a_max_key_version);
  encrypt_info_ptr += 4;
  ut_ad(key_id != (uint)(~0));
  mach_write_to_4(encrypt_info_ptr, key_id);
  encrypt_info_ptr += 4;
  ut_ad(strlen(space->crypt_data->uuid) > 0);

  static uchar uuid_hex[ENCRYPTION_SERVER_UUID_HEX_LEN];
  uuid_to_hex(space->crypt_data->uuid, uuid_hex);

  memcpy(encrypt_info_ptr, uuid_hex, ENCRYPTION_SERVER_UUID_HEX_LEN);
  encrypt_info_ptr += ENCRYPTION_SERVER_UUID_HEX_LEN;
  mach_write_to_1(encrypt_info_ptr, encryption);
  encrypt_info_ptr += 1;

  memcpy(encrypt_info_ptr, space->crypt_data->encrypted_validation_tag,
         ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);
  encrypt_info_ptr += ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE;

  memcpy(encrypt_info_ptr, iv, CRYPT_SCHEME_1_IV_LEN);
  encrypt_info_ptr += CRYPT_SCHEME_1_IV_LEN;

  mach_write_to_1(encrypt_info_ptr, static_cast<byte>(encryption_rotation));
  encrypt_info_ptr += 1;

  if (tablespace_key == nullptr) {
    memset(encrypt_info_ptr, 0, Encryption::KEY_LEN);
    encrypt_info_ptr += Encryption::KEY_LEN;
  } else {
    memcpy(encrypt_info_ptr, tablespace_key, Encryption::KEY_LEN);
    encrypt_info_ptr += Encryption::KEY_LEN;
  }

  mlog_write_string(page + offset, encrypt_info,
                    KERYING_ENCRYPTION_INFO_MAX_SIZE, mtr);

  delete[] encrypt_info;
}

/******************************************************************
Set crypt data for a tablespace
@param[in,out]		space		Tablespace
@param[in,out]		crypt_data	Crypt data to be set
@return crypt_data in tablespace */
static fil_space_crypt_t *fil_space_set_crypt_data(
    fil_space_t *space, fil_space_crypt_t *crypt_data) {
  fil_space_crypt_t *free_crypt_data = NULL;
  fil_space_crypt_t *ret_crypt_data = NULL;

  /* Provided space is protected using fil_space_acquire()
  from concurrent operations. */
  if (space->crypt_data != NULL) {
    /* There is already crypt data present,
    merge new crypt_data */
    fil_space_merge_crypt_data(space->crypt_data, crypt_data);
    ret_crypt_data = space->crypt_data;
    free_crypt_data = crypt_data;
  } else {
    space->crypt_data = crypt_data;
    ret_crypt_data = space->crypt_data;
  }

  if (free_crypt_data != NULL) {
    /* there was already crypt data present and the new crypt
     * data provided as argument to this function has been merged
     * into that => free new crypt data
     */
    fil_space_destroy_crypt_data(&free_crypt_data);
  }

  return ret_crypt_data;
}

/** Parse a MLOG_FILE_WRITE_CRYPT_DATA log entry
@param[in]  space_id  id of space that this log entry refers to
@param[in]  ptr  Log entry start
@param[in]  end_ptr  Log entry end
@param[in]  len  Log entry length
@param[in]  recv_needed_recovery  missing keys will report error
@return position on log buffer */
byte *fil_parse_write_crypt_data_v3(space_id_t space_id, byte *ptr,
                                    const byte *end_ptr, ulint len,
                                    bool recv_needed_recovery, lsn_t lsn) {
  ptr += 4;  // skip offset and len

#ifdef UNIV_DEBUG
  byte *start_ptr = ptr;
#endif

  if (len != KERYING_ENCRYPTION_INFO_MAX_SIZE) {
    recv_sys->set_corrupt_log();
    return nullptr;
  }

  if (ptr + KERYING_ENCRYPTION_INFO_MAX_SIZE > end_ptr) {
    return nullptr;
  }

  // We should only enter this function if ENCRYPTION_KEY_MAGIC_PS_V3 is set
  ut_ad(
      (memcmp(ptr, Encryption::KEY_MAGIC_PS_V3, Encryption::MAGIC_SIZE) == 0));

  fil_space_t *space = fil_space_get(space_id);
  /* If space is already loaded and have header_page_flushed_lsn greater than
  this REDO entry LSN, then skip it because header has the latest
  information. */
  if (space != nullptr && space->m_header_page_flush_lsn > lsn) {
    return ptr + len;
  }

  ptr += Encryption::MAGIC_SIZE;

  uint type = mach_read_from_1(ptr);
  ptr += 1;

  ut_a(type == CRYPT_SCHEME_UNENCRYPTED ||
       type == CRYPT_SCHEME_1);  // only supported

  uint min_key_version = mach_read_from_4(ptr);
  ptr += 4;

  uint max_key_version = mach_read_from_4(ptr);
  ptr += 4;

  uint key_id = mach_read_from_4(ptr);
  ptr += 4;

  static uchar uuid_hex[ENCRYPTION_SERVER_UUID_HEX_LEN];
  memcpy(&uuid_hex, ptr, ENCRYPTION_SERVER_UUID_HEX_LEN);

  ptr += ENCRYPTION_SERVER_UUID_HEX_LEN;

  static char uuid[Encryption::SERVER_UUID_LEN];
  memset(uuid, 0, Encryption::SERVER_UUID_LEN);
  hex_to_uuid(uuid_hex, uuid);

  ut_ad(strlen(uuid) > 0);
  ut_ad(strlen(server_uuid) == 0 ||
        memcmp(uuid, server_uuid, Encryption::SERVER_UUID_LEN) == 0);

  fil_encryption_t encryption = (fil_encryption_t)mach_read_from_1(ptr);
  ptr += 1;

  Crypt_key_operation key_operation =
      (type == CRYPT_SCHEME_UNENCRYPTED)
          ? Crypt_key_operation::FETCH_OR_GENERATE_KEY
          : Crypt_key_operation::FETCH_KEY;

  fil_space_crypt_t *crypt_data =
      fil_space_create_crypt_data(encryption, key_id, uuid, key_operation);
  /* Need to overwrite these as above will initialize fields. */
  crypt_data->type = type;
  assert(min_key_version != ENCRYPTION_KEY_VERSION_INVALID);
  crypt_data->min_key_version = min_key_version;
  crypt_data->max_key_version = max_key_version;
  // set memory to ENCRYPTION_KEYRING_VALIDATION_TAG
  memcpy(crypt_data->encrypted_validation_tag, ptr,
         ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);
  ptr += ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE;
  crypt_data->encryption = encryption;
  memcpy(crypt_data->iv, ptr, CRYPT_SCHEME_1_IV_LEN);
  ptr += CRYPT_SCHEME_1_IV_LEN;
  crypt_data->encryption_rotation =
      static_cast<Encryption_rotation>(mach_read_from_1(ptr));
  ptr += 1;
  uchar tablespace_key[Encryption::KEY_LEN];
  memcpy(tablespace_key, ptr, Encryption::KEY_LEN);
  ptr += Encryption::KEY_LEN;

  if (std::search_n(tablespace_key, tablespace_key + Encryption::KEY_LEN,
                    Encryption::KEY_LEN,
                    0) ==
      tablespace_key) {  // tablespace_key is all zeroes which means there is no
                         // tablepsace in mtr log
    crypt_data->set_tablespace_key(nullptr);
  } else {
    crypt_data->set_tablespace_key(tablespace_key);
  }

  if (recv_needed_recovery) {
    /* Check is used key found from encryption plugin */
    if (crypt_data->should_encrypt() && !crypt_data->is_key_found()) {
      ib::error() << "Key cannot be read for space id = " << space_id;
      recv_sys->set_corrupt_log();
      fil_space_destroy_crypt_data(&crypt_data);
      return nullptr;
    }

    if (crypt_data->type != CRYPT_SCHEME_UNENCRYPTED) {
      // We have encrypted tablespace - validate that encryption key is
      // available and it is the correct one.
      if (crypt_data->key_found == false) {
        ib::warn(ER_REDO_TABLESPACE_ENCRYPTION_MISSING_KEY, space_id,
                 crypt_data->key_id);
        recv_sys->set_corrupt_log();
        fil_space_destroy_crypt_data(&crypt_data);
        return nullptr;
      } else {
        Validation_key_verions_result result{
            crypt_data->validate_encryption_key_versions()};
        if (result != Validation_key_verions_result::SUCCESS) {
          uint error =
              (result == Validation_key_verions_result::MISSING_KEY_VERSIONS)
                  ? ER_REDO_TABLESPACE_ENCRYPTION_MISSING_KEY_VERSIONS
                  : ER_REDO_TABLESPACE_ENCRYPTION_CORRUPTED_KEYS;
          ib::warn(error, space_id, crypt_data->key_id);
          recv_sys->set_corrupt_log();
          fil_space_destroy_crypt_data(&crypt_data);
          return nullptr;
        }
      }
    }
  }

  /* update fil_space memory cache with crypt_data */
  if (space != nullptr) {
    crypt_data = fil_space_set_crypt_data(space, crypt_data);
  } else {
    // crypt_data was created as part of creating a new tablespace
    if (recv_sys->crypt_datas->count(space_id) > 0) {
      fil_space_destroy_crypt_data(&(*recv_sys->crypt_datas)[space_id]);
    }
    (*recv_sys->crypt_datas)[space_id] = crypt_data;
  }

  // We are advancing the ptr pointer while reading crypt_data - make
  // sure that we read exactly len bytes starting from start_ptr.
  ut_ad((ulint)(ptr - start_ptr) == len);

  return ptr;
}

/** Parse a MLOG_FILE_WRITE_CRYPT_DATA log entry
@param[in]  space_id  id of space that this log entry refers to
@param[in]  ptr  Log entry start
@param[in]  end_ptr  Log entry end
@param[in]  len  Log entry length
@return position on log buffer */
byte *fil_parse_write_crypt_data_v2(space_id_t space_id, byte *ptr,
                                    const byte *end_ptr, ulint len, lsn_t lsn) {
  ptr += 4;  // skip offset and len

#ifdef UNIV_DEBUG
  byte *start_ptr = ptr;
#endif

  if (len != KERYING_ENCRYPTION_INFO_MAX_SIZE_V2) {
    recv_sys->set_corrupt_log();
    return nullptr;
  }

  if (ptr + KERYING_ENCRYPTION_INFO_MAX_SIZE_V2 > end_ptr) {
    return nullptr;
  }

  // We should only enter this function if ENCRYPTION_KEY_MAGIC_PS_V2 is set
  ut_ad(
      (memcmp(ptr, Encryption::KEY_MAGIC_PS_V2, Encryption::MAGIC_SIZE) == 0));

  fil_space_t *space = fil_space_get(space_id);
  /* If space is already loaded and have header_page_flushed_lsn greater than
  this REDO entry LSN, then skip it because header has the latest
  information. */
  if (space != nullptr && space->m_header_page_flush_lsn > lsn) {
    return ptr + len;
  }

  ptr += Encryption::MAGIC_SIZE;

  uint type = mach_read_from_1(ptr);
  ptr += 1;

  ut_a(type == CRYPT_SCHEME_UNENCRYPTED ||
       type == CRYPT_SCHEME_1);  // only supported

  uint min_key_version = mach_read_from_4(ptr);
  ptr += 4;

  uint key_id = mach_read_from_4(ptr);
  ptr += 4;

  char uuid[Encryption::SERVER_UUID_LEN];
  memset(uuid, 0, Encryption::SERVER_UUID_LEN);
  memcpy(uuid, ptr, Encryption::SERVER_UUID_LEN);
  ptr += Encryption::SERVER_UUID_LEN;

  fil_encryption_t encryption = (fil_encryption_t)mach_read_from_1(ptr);
  ptr += 1;

  fil_space_crypt_t *crypt_data = fil_space_create_crypt_data(
      encryption, key_id, uuid, Crypt_key_operation::FETCH_OR_GENERATE_KEY);
  /* Need to overwrite these as above will initialize fields. */
  assert(min_key_version != ENCRYPTION_KEY_VERSION_INVALID);
  crypt_data->min_key_version = min_key_version;
  crypt_data->max_key_version = ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;
  // set memory to ENCRYPTION_KEYRING_VALIDATION_TAG
  memcpy(crypt_data->encrypted_validation_tag,
         ENCRYPTION_KEYRING_VALIDATION_TAG,
         ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);
  crypt_data->encryption = encryption;
  crypt_data->private_version = 2;
  memcpy(crypt_data->iv, ptr, CRYPT_SCHEME_1_IV_LEN);
  ptr += CRYPT_SCHEME_1_IV_LEN;
  crypt_data->encryption_rotation =
      static_cast<Encryption_rotation>(mach_read_from_1(ptr));
  ptr += 1;
  uchar tablespace_key[Encryption::KEY_LEN];
  memcpy(tablespace_key, ptr, Encryption::KEY_LEN);
  ptr += Encryption::KEY_LEN;

  if (std::search_n(tablespace_key, tablespace_key + Encryption::KEY_LEN,
                    Encryption::KEY_LEN,
                    0) ==
      tablespace_key) {  // tablespace_key is all zeroes which means there is no
                         // tablepsace in mtr log
    crypt_data->set_tablespace_key(nullptr);
  } else {
    crypt_data->set_tablespace_key(tablespace_key);
  }

  /* Check is used key found from encryption plugin */
  if (crypt_data->should_encrypt() && !crypt_data->is_key_found()) {
    ib::error() << "Key cannot be read for space id = " << space_id;
    recv_sys->set_corrupt_log();
  }

  /* update fil_space memory cache with crypt_data */
  if (space != nullptr) {
    crypt_data = fil_space_set_crypt_data(space, crypt_data);
  } else {
    fil_space_destroy_crypt_data(&crypt_data);
  }

  // We are advancing the ptr pointer while reading crypt_data - make
  // sure that we read exactly len bytes starting from start_ptr.
  ut_ad((ulint)(ptr - start_ptr) == len);

  return ptr;
}
/** Parse a MLOG_FILE_WRITE_CRYPT_DATA log entry
@param[in]  space_id  id of space that this log entry refers to
@param[in]  ptr  Log entry start
@param[in]  end_ptr  Log entry end
@param[in]  len  Log entry length
@return position on log buffer */
byte *fil_parse_write_crypt_data_v1(space_id_t space_id, byte *ptr,
                                    const byte *end_ptr, ulint len, lsn_t lsn) {
  ptr += 4;  // skip offset and len
  ptr += 2;  // skip iv_length

  if (len != KERYING_ENCRYPTION_INFO_MAX_SIZE_V1) {
    recv_sys->set_corrupt_log();
    return nullptr;
  }

  if (ptr + KERYING_ENCRYPTION_INFO_MAX_SIZE_V1 > end_ptr) {
    return nullptr;
  }

  // We should only enter this function if ENCRYPTION_KEY_MAGIC_PS_V1 is set
  ut_ad(
      (memcmp(ptr, Encryption::KEY_MAGIC_PS_V1, Encryption::MAGIC_SIZE) == 0));

  fil_space_t *space = fil_space_get(space_id);
  /* If space is already loaded and have header_page_flushed_lsn greater than
  this REDO entry LSN, then skip it because header has the latest
  information. */
  if (space != nullptr && space->m_header_page_flush_lsn > lsn) {
    return ptr + len;
  }

  ptr += Encryption::MAGIC_SIZE;

  ptr += 4;  // skip space_id
  ptr += 2;  // skip offset

  uint type = mach_read_from_1(ptr);
  ptr += 1;

  ut_a(type == CRYPT_SCHEME_UNENCRYPTED ||
       type == CRYPT_SCHEME_1);  // only supported

  uint min_key_version = mach_read_from_4(ptr);
  ptr += 4;

  uint key_id = mach_read_from_4(ptr);
  ptr += 4;

  fil_encryption_t encryption = (fil_encryption_t)mach_read_from_1(ptr);
  ptr += 1;

  // since we are parsing v1 crypt_data - it means that encryption of this table
  // has not yet started and thus we might need to create a key with uuid in
  // keyring for this table now
  fil_space_crypt_t *crypt_data =
      fil_space_create_crypt_data(encryption, key_id, server_uuid,
                                  Crypt_key_operation::FETCH_OR_GENERATE_KEY);
  /* Need to overwrite these as above will initialize fields. */
  assert(min_key_version != ENCRYPTION_KEY_VERSION_INVALID);
  crypt_data->min_key_version = min_key_version;
  crypt_data->encryption = encryption;
  crypt_data->private_version = 1;
  memcpy(crypt_data->iv, ptr, CRYPT_SCHEME_1_IV_LEN);
  ptr += CRYPT_SCHEME_1_IV_LEN;
  crypt_data->encryption_rotation =
      static_cast<Encryption_rotation>(mach_read_from_4(ptr));
  ptr += 4;
  uchar tablespace_key[Encryption::KEY_LEN];
  memcpy(tablespace_key, ptr, Encryption::KEY_LEN);
  ptr += Encryption::KEY_LEN;

  if (std::search_n(tablespace_key, tablespace_key + Encryption::KEY_LEN,
                    Encryption::KEY_LEN,
                    0) ==
      tablespace_key) {  // tablespace_key is all zeroes which means there is no
                         // tablepsace in mtr log
    crypt_data->set_tablespace_key(nullptr);
    ptr += Encryption::KEY_LEN;  // skip tablespace iv
  } else {
    crypt_data->set_tablespace_key(tablespace_key);
    ptr += Encryption::KEY_LEN;  // skip tablespace iv
  }

  if (crypt_data->encryption_rotation ==
      Encryption_rotation::MASTER_KEY_TO_KEYRING) {
    ib::error()
        << "There is a redo record that would start master key to keyring "
           "re-encryption "
        << "for space = " << space_id
        << ". Such record cannot be proceed by upgrade. "
        << "Please finish off the re-encryption and try again. Re-encryption "
           "is an "
        << "experimental feature in the server version you are upgrading from.";
    recv_sys->set_corrupt_log();
    return ptr;
  }

  /* Check is used key found from encryption plugin */
  if (crypt_data->should_encrypt() && !crypt_data->is_key_found()) {
    ib::error() << "Key cannot be read for space id = " << space_id;
    recv_sys->set_corrupt_log();
  }

  /* update fil_space memory cache with crypt_data */
  if (space != nullptr) {
    crypt_data = fil_space_set_crypt_data(space, crypt_data);
  } else {
    fil_space_destroy_crypt_data(&crypt_data);
  }

  return ptr;
}

/***********************************************************************/

/** A copy of global key state */
struct key_state_t {
  key_state_t()
      : key_id((~0)),
        key_version(ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED),
        rotate_key_age(srv_fil_crypt_rotate_key_age) {}
  bool operator==(const key_state_t &other) const {
    return key_version == other.key_version &&
           rotate_key_age == other.rotate_key_age;
  }
  uint key_id;
  uint key_version;
  uint rotate_key_age;
};

/***********************************************************************
Copy global key state
@param[in,out]	new_state	key state
@param[in]	crypt_data	crypt data */
static void fil_crypt_get_key_state(key_state_t *new_state,
                                    fil_space_crypt_t *crypt_data) {
  if (srv_default_table_encryption ==
      DEFAULT_TABLE_ENC_ONLINE_FROM_KEYRING_TO_UNENCRYPTED) {
    new_state->key_version = ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;
    new_state->rotate_key_age = 0;
  } else {
    new_state->key_version = crypt_data->key_get_latest_version();
    new_state->rotate_key_age = srv_fil_crypt_rotate_key_age;
    ut_a(new_state->key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED);
  }
}

/***********************************************************************
Check if a key needs rotation given a key_state
@param[in]	encrypt_mode		Encryption mode
@param[in]	key_version		Current key version
@param[in]	latest_key_version	Latest key version
@param[in]	rotate_key_age		when to rotate
@return true if key needs rotation, false if not */
static bool fil_crypt_needs_rotation(fil_encryption_t encrypt_mode,
                                     uint key_version, uint latest_key_version,
                                     uint rotate_key_age) {
  if (key_version == ENCRYPTION_KEY_VERSION_INVALID) {
    ut_ad(0);
    return false;
  }

  if (key_version == ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED &&
      latest_key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED &&
      Encryption::is_online_encryption_on()) {
    /* this is rotation unencrypted => encrypted
     * ignore rotate_key_age */
    return true;
  }

  if (key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED &&
      latest_key_version == ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED) {
    if (encrypt_mode == FIL_ENCRYPTION_DEFAULT) {
      // this is rotation encrypted => unencrypted
      return true;
    }
    return false;
  }

  /* this is rotation encrypted => encrypted,
   * only reencrypt if key is sufficiently old */
  /* Do not encrypt tables which are unencrypted when key is rotated */
  if (key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED &&
      rotate_key_age > 0 &&
      (key_version + rotate_key_age <= latest_key_version)) {
    return true;
  }

  return false;
}

/** Read page 0 and possible crypt data from there.
@param[in,out]	space		Tablespace */
static inline void fil_crypt_read_crypt_data(fil_space_t *space) {
  if (space->size != 0) {
    /* When space->size != 0 it means that page0 of the tablespace has
    already been read in order to read the size of the space (i.e. number of the
    pages). When we read page0 - we also read crypt_data information from it -
    thus if space->size != 0 - it means that crypt_data was already read (if
    existed). Also we need to read space->size in order to know how many pages
    we are going to rotate in space. buf_page_get will read space's size if it
    is not yet read */
    return;
  }

  const page_size_t page_size(space->flags);
  mtr_t mtr;
  mtr.start();
  if (buf_block_t *block =
          buf_page_get(page_id_t(space->id, 0), page_size, RW_S_LATCH, &mtr)) {
    fil_lock_shard_by_id(space->id);
    if (!space->crypt_data) {
      fil_space_crypt_t *crypt_data =
          fil_space_read_crypt_data(page_size, block->frame);
      if (crypt_data != nullptr) {
        space->crypt_data = crypt_data;
      }
    }
    fil_unlock_shard_by_id(space->id);
  }
  mtr.commit();
}

/**
Write crypt data belonging to space to page0
@param space tablespace with crypt data to write
*/
static void fil_crypt_write_crypt_data_to_page0(fil_space_t *space) {
  mtr_t mtr;
  mtr.start();

  if (buf_block_t *block = buf_page_get_gen(
          page_id_t(space->id, 0), page_size_t(space->flags), RW_X_LATCH, NULL,
          Page_fetch::NORMAL, __FILE__, __LINE__, &mtr)) {
    space->crypt_data->write_page0(
        space, block->frame, &mtr, space->crypt_data->min_key_version,
        space->crypt_data->max_key_version, space->crypt_data->type);
  }
  mtr.commit();
}

bool fil_crypt_exclude_tablespace_from_rotation_temporarily(
    fil_space_t *space) {
  if (space->exclude_from_rotation) {
    // nothing to do
    return true;
  }

  // We acquire fil_crypt_threads_mutex to stop encryption threads from
  // generating crypt_data for tablespaces that they are about to encrypt.
  // Generating crypt_data is the fist step in encryption process.
  mutex_enter(&fil_crypt_threads_mutex);

  if (space->crypt_data == nullptr) {
    space->exclude_from_rotation = true;
    mutex_exit(&fil_crypt_threads_mutex);
    return true;
  }

  mutex_exit(&fil_crypt_threads_mutex);

  // crypt_data already existed.
  fil_space_crypt_t *crypt_data = space->crypt_data;
  // take a lock on crypt_data to block rotation from starting
  IB_mutex_guard crypt_data_mutex_guard(&crypt_data->mutex);

  // if there are any encryption threads "running" on this tablespace we cannot
  // exclude it from rotation.
  if (crypt_data->rotate_state.active_threads != 0 ||
      crypt_data->rotate_state.starting || crypt_data->rotate_state.flushing) {
    my_error(ER_EXCLUDE_ENCRYPTION_THREADS_RUNNING, MYF(0), space->name);
    return false;
  }

  // it is only possible to exclude tablespace that is unencrypted
  if (crypt_data->type != CRYPT_SCHEME_UNENCRYPTED) {
    my_error(ER_EXCLUDE_ENCRYPTION_TABLE_ENCRYPTED, MYF(0), space->name);
    return false;
  }

  space->exclude_from_rotation = true;

  return true;
}

bool fil_crypt_exclude_tablespace_from_rotation_permanently(
    fil_space_t *space) {
  // We acquire fil_crypt_threads_mutex to stop encryption threads from
  // generating crypt_data for tablespaces that they are about to encrypt.
  // Generating crypt_data is the fist step in encryption process.
  mutex_enter(&fil_crypt_threads_mutex);

  if (space->crypt_data == nullptr) {
    space->crypt_data = fil_space_create_crypt_data(
        FIL_ENCRYPTION_OFF, FIL_DEFAULT_ENCRYPTION_KEY, server_uuid);
    fil_crypt_write_crypt_data_to_page0(space);
    mutex_exit(&fil_crypt_threads_mutex);
    return true;
  }

  mutex_exit(&fil_crypt_threads_mutex);

  // crypt_data already existed.
  fil_space_crypt_t *crypt_data = space->crypt_data;
  IB_mutex_guard crypt_data_mutex_guard(&crypt_data->mutex);

  if (crypt_data->encryption == FIL_ENCRYPTION_OFF) {
    // nothing to do
    return true;
  }

  // if there are any encryption threads "running" on this tablespace we cannot
  // exclude it from rotation.
  if (crypt_data->rotate_state.active_threads != 0 ||
      crypt_data->rotate_state.starting || crypt_data->rotate_state.flushing) {
    my_error(ER_EXCLUDE_ENCRYPTION_THREADS_RUNNING, MYF(0), space->name);
    return false;
  }

  // it is only possible to exclude tablespace that is unencrypted
  if (crypt_data->type != CRYPT_SCHEME_UNENCRYPTED) {
    my_error(ER_EXCLUDE_ENCRYPTION_TABLE_ENCRYPTED, MYF(0), space->name);
    return false;
  }

  crypt_data->encryption = FIL_ENCRYPTION_OFF;
  crypt_data->key_id = FIL_DEFAULT_ENCRYPTION_KEY;

  fil_crypt_write_crypt_data_to_page0(space);

  return true;
}

void fil_crypt_readd_space_to_rotation(space_id_t space_id) {
  fil_space_t *space = fil_space_acquire_silent(space_id);

  if (space != nullptr) {
    space->exclude_from_rotation = false;
    fil_space_release(space);
  }
}

static bool decrypt_validation_tag(const byte *encrypted_validation_tag,
                                   const byte *key,
                                   byte *decrypted_validation_tag) {
  auto len = my_aes_decrypt(encrypted_validation_tag, MY_AES_BLOCK_SIZE,
                            decrypted_validation_tag, key, Encryption::KEY_LEN,
                            my_aes_256_ecb, nullptr, false);

  /* If decryption failed, return error. */
  return len != MY_AES_BAD_DATA;
}

Validation_key_verions_result
fil_space_crypt_t::validate_encryption_key_versions() {
  // unencrypted space, thus no keys needed to decrypt
  if (type == CRYPT_SCHEME_UNENCRYPTED)
    return Validation_key_verions_result::SUCCESS;

  if (load_keys_to_local_cache() == false)
    return Validation_key_verions_result::MISSING_KEY_VERSIONS;

  byte decrypted_validation_tag[ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE] = {0};
  byte current_validation_tag[ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE];
  memcpy(current_validation_tag, this->encrypted_validation_tag,
         ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);

  // in case we are validating a space for which only a subset of pages is
  // encrypted, we may be in a situation that there are muliple encryption keys
  // and they do not fully fill the range [min_key_version, max_key_version],
  // since min_key_version == 0 is a marker that there are some unencrypted
  // pages, without any version. The encryption keys might be in some range [n,
  // max_key_version], where n > min_key_version and min_key_version = 0. Thus
  // for this situation (min_key_version = 0) we validate the tag after each
  // decryption. If the tag matches after any decryption it means we have all
  // the valid keys we need to decrypt space.
  bool check_tag_for_each_version{this->min_key_version ==
                                  ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED};

  for (uint key_version = max_key_version;
       key_version >= std::max(min_key_version, static_cast<uint>(1));
       --key_version) {
    ut_ad(local_keys_cache[key_version] != nullptr);

    if (!decrypt_validation_tag(current_validation_tag,
                                local_keys_cache[key_version],
                                decrypted_validation_tag))
      return Validation_key_verions_result::CORRUPTED_OR_WRONG_KEY_VERSIONS;

    if (check_tag_for_each_version &&
        memcmp(current_validation_tag, ENCRYPTION_KEYRING_VALIDATION_TAG,
               ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE) == 0)
      return Validation_key_verions_result::SUCCESS;

    memcpy(current_validation_tag, decrypted_validation_tag,
           ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);
  }

  return memcmp(current_validation_tag, ENCRYPTION_KEYRING_VALIDATION_TAG,
                ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE) == 0
             ? Validation_key_verions_result::SUCCESS
             : Validation_key_verions_result::CORRUPTED_OR_WRONG_KEY_VERSIONS;
}

/***********************************************************************
Start encrypting a space
@param[in,out]		space		Tablespace
@return true if a recheck is needed */
static bool fil_crypt_start_encrypting_space(fil_space_t *space) {
  bool recheck = false;

  mutex_enter(&fil_crypt_threads_mutex);

  fil_space_crypt_t *crypt_data = space->crypt_data;

  /* If space is not encrypted and encryption is not enabled, then
  do not continue encrypting the space. */
  if (!crypt_data && Encryption::is_online_encryption_on() == false) {
    mutex_exit(&fil_crypt_threads_mutex);
    return false;
  }

  if (crypt_data != NULL || fil_crypt_start_converting) {
    /* someone beat us to it */
    if (fil_crypt_start_converting) {
      recheck = true;
    }

    mutex_exit(&fil_crypt_threads_mutex);
    return recheck;
  }

  /* NOTE: we need to write and flush page 0 before publishing
   * the crypt data. This so that after restart there is no
   * risk of finding encrypted pages without having
   * crypt data in page 0 */

  crypt_data = fil_space_create_crypt_data(
      FIL_ENCRYPTION_DEFAULT, get_global_default_encryption_key_id_value(),
      server_uuid, Crypt_key_operation::FETCH_OR_GENERATE_KEY);

  if (crypt_data == nullptr || crypt_data->key_found == false) {
    mutex_exit(&fil_crypt_threads_mutex);
    return false;
  }

  crypt_data->type = CRYPT_SCHEME_UNENCRYPTED;
  crypt_data->min_key_version =
      ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;  // all pages are unencrypted
  crypt_data->max_key_version = crypt_data->key_get_latest_version();
  ut_ad(crypt_data->max_key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED);
  ut_ad(crypt_data->max_key_version != ENCRYPTION_KEY_VERSION_INVALID);
  crypt_data->rotate_state.start_time = time(0);
  crypt_data->rotate_state.starting = true;
  crypt_data->rotate_state.active_threads = 1;

  if (space->encryption_type == Encryption::AES) {  // We are re-encrypting
                                                    // space from MK encryption
                                                    // to RK encryption
    // TODO: assert that space->encryption_key is all zeroes here
    crypt_data->encryption_rotation =
        Encryption_rotation::MASTER_KEY_TO_KEYRING;
    crypt_data->set_tablespace_key(space->encryption_key);
    crypt_data->set_iv(
        space->encryption_iv);  // using the same iv for reading MK encrypted
                                // pages and encrypting KEYRING encrypted
                                // pages
  }

  if (crypt_data->key_found == false ||
      crypt_data->load_keys_to_local_cache() == false) {
    // This should not happen, we have locked the keyring before encryption
    // threads could have even started unless something realy strange have
    // happend like removing keyring file from under running server.
    ib::error() << "Encryption thread could not retrieve a key from a keyring "
                   "for tablespace "
                << space->name
                << " . Removing space from encrypting. Please make sure "
                   "keyring is functional and try restarting the server";
    space->exclude_from_rotation = true;
    mutex_exit(&fil_crypt_threads_mutex);
    fil_space_destroy_crypt_data(&crypt_data);
    return false;
  }
  mutex_enter(&crypt_data->mutex);
  crypt_data = fil_space_set_crypt_data(space, crypt_data);
  mutex_exit(&crypt_data->mutex);

  space->encryption_type = Encryption::KEYRING;  // This works like this - if
                                                 // Encryption::KEYRING is set -
                                                 // it means that
  fil_crypt_start_converting = true;
  mutex_exit(&fil_crypt_threads_mutex);

  do {
    trx_t *trx = trx_allocate_for_background();
    FlushObserver flush_observer(space->id, trx, nullptr);
    trx_set_flush_observer(trx, &flush_observer);

    mtr_t mtr;
    mtr.start();
    // mtr.set_named_space(space);
    mtr.set_log_mode(MTR_LOG_NO_REDO);  // We do not need page 0 to be redo log.
                                        // If we fail to update page 0 we will
                                        // rotate those pages again after
                                        // restart - when encryption threads
                                        // discover that there is work to do.
    mtr.set_flush_observer(&flush_observer);
    /* 2 - get page 0 */

    buf_block_t *block = buf_page_get_gen(
        page_id_t(space->id, 0), page_size_t(space->flags), RW_X_LATCH, nullptr,
        Page_fetch::NORMAL, __FILE__, __LINE__, &mtr);

    /* 3 - write crypt data to page 0 */
    byte *frame = buf_block_get_frame(block);
    crypt_data->type = CRYPT_SCHEME_1;
    crypt_data->write_page0(space, frame, &mtr, crypt_data->min_key_version,
                            crypt_data->max_key_version, crypt_data->type);

    mtr.commit();
    /* 4 - sync tablespace before publishing crypt data */
    // buf_flush_request_force(LSN_MAX);
    // buf_flush_wait_flushed(LSN_MAX);

    flush_observer.flush();
    trx_free_for_background(trx);
    /* 5 - publish crypt data */
    mutex_enter(&fil_crypt_threads_mutex);
    mutex_enter(&crypt_data->mutex);
    crypt_data->type = CRYPT_SCHEME_1;
    ut_a(crypt_data->rotate_state.active_threads == 1);
    crypt_data->rotate_state.active_threads = 0;
    crypt_data->rotate_state.starting = false;

    fil_crypt_start_converting = false;
    mutex_exit(&crypt_data->mutex);
    mutex_exit(&fil_crypt_threads_mutex);

    return recheck;
  } while (0);

  mutex_enter(&crypt_data->mutex);
  ut_a(crypt_data->rotate_state.active_threads == 1);
  crypt_data->rotate_state.active_threads = 0;
  mutex_exit(&crypt_data->mutex);

  mutex_enter(&fil_crypt_threads_mutex);
  fil_crypt_start_converting = false;
  mutex_exit(&fil_crypt_threads_mutex);

  return recheck;
}

/** State of a rotation thread */
struct rotate_thread_t {
  rotate_thread_t(uint no, THD *thd) {
    ut_ad(thd != nullptr);
    memset(this, 0, sizeof(*this));
    thread_no = no;
    first = true;
    estimated_max_iops = 20;
    this->thd = thd;
  }

  uint thread_no;
  bool first;                 /*!< is position before first space */
  fil_space_t *space;         /*!< current space or NULL */
  page_no_t offset;           /*!< current offset */
  page_no_t batch;            /*!< #pages to rotate */
  uint min_key_version_found; /*!< min key version found but not rotated */
  lsn_t end_lsn;              /*!< max lsn when rotating this space */

  uint estimated_max_iops; /*!< estimation of max iops */
  uint allocated_iops;     /*!< allocated iops */
  ulint cnt_waited;        /*!< #times waited during this slot */
  uintmax_t sum_waited_us; /*!< wait time during this slot */

  fil_crypt_stat_t crypt_stat;  // statistics
  btr_scrub_t scrub_data; /*< thread local data used by btr_scrub-functions
                              when iterating pages of tablespace */

  THD *thd;  // We need THD object to be able to update
             // tablespace's DD encryption flag

  /** @return whether this thread should terminate */
  bool should_shutdown() const {
    switch (srv_shutdown_state) {
      case SRV_SHUTDOWN_NONE:
        return thread_no >= srv_n_fil_crypt_threads_requested;
      case SRV_SHUTDOWN_EXIT_THREADS:
      /* srv_init_abort() must have been invoked */
      case SRV_SHUTDOWN_PRE_DD_AND_SYSTEM_TRANSACTIONS:
        return true;
      case SRV_SHUTDOWN_PURGE:
      case SRV_SHUTDOWN_CLEANUP:
      case SRV_SHUTDOWN_DD:
      case SRV_SHUTDOWN_FLUSH_PHASE:
      case SRV_SHUTDOWN_LAST_PHASE:
      case SRV_SHUTDOWN_MASTER_STOP:
      case SRV_SHUTDOWN_RECOVERY_ROLLBACK:
        break;
    }
    ut_ad(0);
    return true;
  }
};

#ifdef UNIV_DEBUG
static bool is_unenc_to_enc_rotation(const fil_space_crypt_t &crypt_data) {
  return crypt_data.type == CRYPT_SCHEME_UNENCRYPTED &&
         !crypt_data.is_encryption_disabled();
}
#endif

/***********************************************************************
Check if space needs rotation given a key_state
@param[in,out]		state		Key rotation state
@param[in,out]		key_state	Key state
@param[in,out]		recheck		needs recheck ?
@return true if space needs key rotation */
static bool fil_crypt_space_needs_rotation(rotate_thread_t *state,
                                           key_state_t *key_state,
                                           bool *recheck) {
  fil_space_t *space = state->space;

  DBUG_EXECUTE_IF("rotate_only_first_x_pages_from_t1",
                  if (strcmp(space->name, "test/t1") == 0 &&
                      number_of_t1_pages_rotated >=
                          number_of_t1_pages_to_rotate) return false;);

  /* Make sure that tablespace is normal tablespace */
  if (space->purpose != FIL_TYPE_TABLESPACE &&
      space->purpose != FIL_TYPE_TEMPORARY) {
    return false;
  }

  ut_ad(space->n_pending_ops > 0);

  fil_space_crypt_t *crypt_data = space->crypt_data;

  if (crypt_data == NULL) {
    /**
     * space has no crypt data
     *   start encrypting it...
     */
    key_state->key_id = get_global_default_encryption_key_id_value();

    *recheck = fil_crypt_start_encrypting_space(space);
    crypt_data = space->crypt_data;

    if (crypt_data == NULL) {
      return false;
    }

    key_state->key_version = crypt_data->max_key_version;
    ut_ad(key_state->key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED);
  }

  mutex_enter(&crypt_data->mutex);

  /* If used key_id is not found from encryption plugin we can't
  continue to rotate the tablespace */

  if (!crypt_data->is_key_found()) {
    // We can end up here in case we try to encrypt tablespace but the key used
    // by this tablespace is no longer in keyring. This can happen when keyring
    // was changed or crypt_data is in version 1 or 2 and key's uuid is empty.
    // Also when tablespace was encrypted, then decrypted and server uuid was
    // changed. Then the crypt_data uuid will not match the server_uuid. The
    // change of server_uuid is done in some of the MTR tests (for instance
    // encryption.innodb-missing-key).
    if (crypt_data->rotate_state.active_threads == 0 &&
        crypt_data->encryption == FIL_ENCRYPTION_DEFAULT) {
      ut_ad(((crypt_data->private_version == 1 ||
              crypt_data->private_version == 2 ||
              strlen(crypt_data->uuid) == 0) ||
             (crypt_data->private_version == 3 &&
              memcmp(crypt_data->uuid, server_uuid,
                     Encryption::SERVER_UUID_LEN) != 0)) &&
            is_unenc_to_enc_rotation(*crypt_data));

      crypt_data->key_found =
          Encryption::tablespace_key_exists_or_create_new_one_if_does_not_exist(
              crypt_data->key_id, server_uuid);

      if (!crypt_data->key_found) {
        mutex_exit(&crypt_data->mutex);
        return false;  // failed to fetch or create the key - skip the
                       // tablespace
      }

      key_state->key_version = 1;
      // We assing here uuid to crypt_data key's uuid. If crypt_data
      // does not make it to page0 (due to crash) we will do the same
      // after the restart, because we will end up here again -
      // encryption key will not be found.
      ut_ad(strlen(server_uuid) > 0);
      memcpy(crypt_data->uuid, server_uuid, Encryption::SERVER_UUID_LEN);
      crypt_data->uuid[Encryption::SERVER_UUID_LEN] = '\0';
      // fix private_version - it might have been 1 or 2
      crypt_data->private_version = 3;
    } else {
      mutex_exit(&crypt_data->mutex);
      return false;
    }
  }

  const bool space_compressed = space->compression_type != Compression::NONE;

  do {
    /* prevent threads from starting to rotate space */
    if (crypt_data->rotate_state.starting) {
      /* recheck this space later */
      *recheck = true;
      break;
    }

    /* prevent threads from starting to rotate space */
    if (space->is_stopping()) {
      break;
    }

    if (crypt_data->rotate_state.flushing) {
      break;
    }

    /* No need to rotate space if encryption is disabled
     * permanently or temporarily */
    if (crypt_data->is_encryption_disabled() || space->exclude_from_rotation) {
      break;
    }

    if (crypt_data->key_id != key_state->key_id) {
      key_state->key_id = crypt_data->key_id;
      fil_crypt_get_key_state(key_state, crypt_data);
    }

    bool need_key_rotation = fil_crypt_needs_rotation(
        crypt_data->encryption, crypt_data->min_key_version,
        key_state->key_version, key_state->rotate_key_age);

    if (need_key_rotation && crypt_data->rotate_state.active_threads != 0 &&
        crypt_data->rotate_state.next_offset >
            crypt_data->rotate_state.max_offset) {
      break;  // the space is already being processed and there are no more
              // pages to rotate
    }

    crypt_data->rotate_state.scrubbing.is_active =
        btr_scrub_start_space(space->id, &state->scrub_data, space_compressed);
    time_t diff =
        time(0) - crypt_data->rotate_state.scrubbing.last_scrub_completed;
    btr_scrub_start_space(space->id, &state->scrub_data, space_compressed);

    bool need_scrubbing = (srv_background_scrub_data_uncompressed ||
                           srv_background_scrub_data_compressed) &&
                          crypt_data->rotate_state.scrubbing.is_active &&
                          diff >= 0 &&
                          ulint(diff) >= srv_background_scrub_data_interval;
    if (need_key_rotation == false && need_scrubbing == false) {
      break;
    }

    mutex_exit(&crypt_data->mutex);

    return true;
  } while (0);

  mutex_exit(&crypt_data->mutex);

  return false;
}

/***********************************************************************
Update global statistics with thread statistics
@param[in,out]	state		key rotation statistics */
static void fil_crypt_update_total_stat(rotate_thread_t *state) {
  mutex_enter(&crypt_stat_mutex);
  crypt_stat.pages_read_from_cache += state->crypt_stat.pages_read_from_cache;
  crypt_stat.pages_read_from_disk += state->crypt_stat.pages_read_from_disk;
  crypt_stat.pages_modified += state->crypt_stat.pages_modified;
  crypt_stat.pages_flushed += state->crypt_stat.pages_flushed;
  // remote old estimate
  crypt_stat.estimated_iops -= state->crypt_stat.estimated_iops;
  // add new estimate
  crypt_stat.estimated_iops += state->estimated_max_iops;
  mutex_exit(&crypt_stat_mutex);

  // make new estimate "current" estimate
  memset(&state->crypt_stat, 0, sizeof(state->crypt_stat));
  // record our old (current) estimate
  state->crypt_stat.estimated_iops = state->estimated_max_iops;
}

/***********************************************************************
Allocate iops to thread from global setting,
used before starting to rotate a space.
@param[in,out]		state		Rotation state
@return true if allocation succeeded, false if failed */
static bool fil_crypt_alloc_iops(rotate_thread_t *state) {
  ut_ad(state->allocated_iops == 0);

  /* We have not yet selected the space to rotate, thus
  state might not contain space and we can't check
  its status yet. */

  uint max_iops = state->estimated_max_iops;
  mutex_enter(&fil_crypt_threads_mutex);

  if (n_fil_crypt_iops_allocated >= srv_n_fil_crypt_iops) {
    /* this can happen when user decreases srv_fil_crypt_iops */
    mutex_exit(&fil_crypt_threads_mutex);
    return false;
  }

  uint alloc = srv_n_fil_crypt_iops - n_fil_crypt_iops_allocated;

  if (alloc > max_iops) {
    alloc = max_iops;
  }

  n_fil_crypt_iops_allocated += alloc;
  mutex_exit(&fil_crypt_threads_mutex);

  state->allocated_iops = alloc;

  return alloc > 0;
}

/***********************************************************************
Reallocate iops to thread,
used when inside a space
@param[in,out]		state		Rotation state */
static void fil_crypt_realloc_iops(rotate_thread_t *state) {
  ut_a(state->allocated_iops > 0);

  if (10 * state->cnt_waited > state->batch) {
    /* if we waited more than 10% re-estimate max_iops */
    ulint avg_wait_time_us = ulint(state->sum_waited_us / state->cnt_waited);

    if (avg_wait_time_us == 0) {
      avg_wait_time_us = 1;  // prevent division by zero
    }

    DBUG_PRINT(
        "ib_crypt",
        ("thr_no: %u - update estimated_max_iops from %u to " ULINTPF ".",
         state->thread_no, state->estimated_max_iops,
         1000000 / avg_wait_time_us));

    state->estimated_max_iops = uint(1000000 / avg_wait_time_us);
    state->cnt_waited = 0;
    state->sum_waited_us = 0;
  } else {
    DBUG_PRINT("ib_crypt",
               ("thr_no: %u only waited " ULINTPF "%% skip re-estimate.",
                state->thread_no,
                (100 * state->cnt_waited) / (state->batch ? state->batch : 1)));
  }

  if (state->estimated_max_iops <= state->allocated_iops) {
    /* return extra iops */
    uint extra = state->allocated_iops - state->estimated_max_iops;

    if (extra > 0) {
      mutex_enter(&fil_crypt_threads_mutex);
      if (n_fil_crypt_iops_allocated < extra) {
        /* unknown bug!
         * crash in debug
         * keep n_fil_crypt_iops_allocated unchanged
         * in release */
        ut_ad(0);
        extra = 0;
      }
      n_fil_crypt_iops_allocated -= extra;
      state->allocated_iops -= extra;

      if (state->allocated_iops == 0) {
        /* no matter how slow io system seems to be
         * never decrease allocated_iops to 0... */
        state->allocated_iops++;
        n_fil_crypt_iops_allocated++;
      }

      os_event_set(fil_crypt_threads_event);
      mutex_exit(&fil_crypt_threads_mutex);
    }
  } else {
    /* see if there are more to get */
    mutex_enter(&fil_crypt_threads_mutex);
    if (n_fil_crypt_iops_allocated < srv_n_fil_crypt_iops) {
      /* there are extra iops free */
      uint extra = srv_n_fil_crypt_iops - n_fil_crypt_iops_allocated;
      if (state->allocated_iops + extra > state->estimated_max_iops) {
        /* but don't alloc more than our max */
        extra = state->estimated_max_iops - state->allocated_iops;
      }
      n_fil_crypt_iops_allocated += extra;
      state->allocated_iops += extra;

      DBUG_PRINT("ib_crypt",
                 ("thr_no: %u increased iops from %u to %u.", state->thread_no,
                  state->allocated_iops - extra, state->allocated_iops));
    }
    mutex_exit(&fil_crypt_threads_mutex);
  }

  fil_crypt_update_total_stat(state);
}

/***********************************************************************
Return allocated iops to global
@param[in,out] state                       rotation state
@param[in]     set_fil_crypt_threads_event should fil_crypt_threads_event be set
                                           so to notify other events that there
                                           might be some work to do. */
static void fil_crypt_return_iops(rotate_thread_t *state,
                                  bool set_fil_crypt_threads_event = true) {
  if (state->allocated_iops > 0) {
    uint iops = state->allocated_iops;
    mutex_enter(&fil_crypt_threads_mutex);
    if (n_fil_crypt_iops_allocated < iops) {
      /* unknown bug!
       * crash in debug
       * keep n_fil_crypt_iops_allocated unchanged
       * in release */
      ut_ad(0);
      iops = 0;
    }

    n_fil_crypt_iops_allocated -= iops;
    state->allocated_iops = 0;
    if (set_fil_crypt_threads_event) os_event_set(fil_crypt_threads_event);
    mutex_exit(&fil_crypt_threads_mutex);
  }

  fil_crypt_update_total_stat(state);
}

/***********************************************************************
Search for a space needing rotation
@param[in,out]		key_state		Key state
@param[in,out]		state			Rotation state
@param[in,out]		recheck			recheck ? */
static bool fil_crypt_find_space_to_rotate(key_state_t *key_state,
                                           rotate_thread_t *state,
                                           bool *recheck) {
  /* we need iops to start rotating */
  while (!state->should_shutdown() && !fil_crypt_alloc_iops(state)) {
    os_event_reset(fil_crypt_threads_event);
    os_event_wait_time(fil_crypt_threads_event, 100000);
  }

  if (state->should_shutdown()) {
    if (state->space) {
      fil_space_release(state->space);
      state->space = NULL;
    }
    return false;
  }

  if (state->first) {
    state->first = false;
    if (state->space) {
      fil_space_release(state->space);
    }
    state->space = NULL;
  }

  /* If key rotation is enabled (default) we iterate all tablespaces.
  If key rotation is not enabled we iterate only the tablespaces
  added to keyrotation list. */
  if (srv_fil_crypt_rotate_key_age) {
    state->space = fil_space_next(state->space);
  } else {
    state->space = fil_space_keyrotate_next(state->space);
  }

  while (!state->should_shutdown() && state->space) {
    /* If there is no crypt data and we have not yet read
    page 0 for this tablespace, we need to read it before
    we can continue. */
    fil_crypt_read_crypt_data(state->space);

    ut_ad(state->space->size);

    // TODO: What about excluding spaces that are (has?) SPATIAL INDEXES ?

    // if space is marked as encrytped this means some of the pages are
    // encrypted and space should be skipped size must be set - i.e. tablespace
    // has been read
    if (!state->space->is_space_encrypted &&
        !state->space->exclude_from_rotation &&
        fil_crypt_space_needs_rotation(state, key_state, recheck)) {
      ut_ad(key_state->key_id != ENCRYPTION_KEY_VERSION_INVALID);
      /* init state->min_key_version_found before
       * starting on a space */
      state->min_key_version_found = key_state->key_version;

      return true;
    }

    DBUG_EXECUTE_IF(
        "wait_for_ts1_to_be_considered_for_rotation",
        if (strcmp(state->space->name, "ts1") == 0) {
          // artifical key_id = 10 for system space to let MTR test know that
          // t1 was already proccessed
          fil_space_t *sys_space = fil_space_get(0);
          EncryptionKeyId key_id = sys_space->crypt_data->key_id;
          sys_space->crypt_data->key_id = 10;
          while (DBUG_EVALUATE_IF("wait_for_ts1_to_be_considered_for_rotation",
                                  true, false))
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
          sys_space->crypt_data->key_id = key_id;
        });

    if (srv_fil_crypt_rotate_key_age) {
      state->space = fil_space_next(state->space);
    } else {
      state->space = fil_space_keyrotate_next(state->space);
    }
  }

  /* If we didn't find any space return iops. Do not set
     fil_crypt_threads_event. We do not want to notify other threads that there
     is some work to do - since
     we went through all spaces and did not find any space that needs rotation.
   */
  fil_crypt_return_iops(state, false);

  return false;
}

bool fil_space_crypt_t::re_encrypt_validation_tag(const uint from_key_version,
                                                  const uint to_key_version) {
  if (from_key_version > to_key_version)
    return true;  // re-encryption not needed

  // load key_versions that we might be missing in the cache
  if (load_keys_to_local_cache(from_key_version, to_key_version) == false) {
    // generate error
    return false;
  }

  byte re_encrypted_validation_tag[ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE] = {
      0};
  byte copy_encrypted_validation_tag[ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE];
  memcpy(copy_encrypted_validation_tag, encrypted_validation_tag,
         ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);

  for (uint key_version = from_key_version; key_version <= to_key_version;
       ++key_version) {
    ut_ad(local_keys_cache[key_version] != nullptr);

    if (encrypt_validation_tag(copy_encrypted_validation_tag,
                               ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE,
                               local_keys_cache[key_version],
                               re_encrypted_validation_tag) == false) {
      // generate error
      return false;
    }

    memcpy(copy_encrypted_validation_tag, re_encrypted_validation_tag,
           ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);
  }

  // only update encrypted_validation_tag if the whole re-encryption was
  // successful
  memcpy(encrypted_validation_tag, copy_encrypted_validation_tag,
         ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);

  return true;
}

/***********************************************************************
Start rotating a space
@param[in]	key_state		Key state
@param[in,out]	state			Rotation state */
static bool fil_crypt_start_rotate_space(const key_state_t *key_state,
                                         rotate_thread_t *state) {
  fil_space_crypt_t *crypt_data = state->space->crypt_data;

  ut_ad(crypt_data);

  mutex_enter(&crypt_data->start_rotate_mutex);
  // flush observer needs to be created outside crypt_data->mutex;
  // active threads is increased only in this function - thus once it's 0 under
  // start_rotate_mutex it will stay 0.
  if (crypt_data->rotate_state.active_threads == 0)
    crypt_data->rotate_state.create_flush_observer(state->space->id);

  mutex_enter(&crypt_data->mutex);

  // Check of crypt_data->is_encryption_disabled:
  // It is possible that tablespace was excluded from rotation in the meantime
  // ("permanently") I.e. fil_crypt_exclude_tablespace_from_rotation was called,
  // because ENCRYPTION='N' was set. Check it here. Check for
  // state->space->exclude_from_rotation: Check that in the meantime space was
  // not excluded from rotation (temporarly). This could be the case when ALTER
  // ENCRYPTION='Y/N' is done on a tablespace. When ENCRYPTION of general
  // tablespace is changed the page0 is periodically updated with the progress
  // of encryption/decryption of general tablespace. We do not want encryption
  // threads to interfere with that.

  if (crypt_data->is_encryption_disabled() ||
      state->space->exclude_from_rotation) {
    mutex_exit(&crypt_data->mutex);
    crypt_data->rotate_state.destroy_flush_observer();
    mutex_exit(&crypt_data->start_rotate_mutex);
    return false;
  }

  ut_ad(key_state->key_id == crypt_data->key_id);

  if (crypt_data->rotate_state.active_threads == 0) {
    bool validation_tag_re_encryption_failure{false};
    bool load_key_failure{false};
    uint org_max_key_version = crypt_data->max_key_version;
    crypt_data->rotate_state.active_threads = 1;

    if (crypt_data->load_keys_to_local_cache() == false) {
      load_key_failure = true;
    } else if (key_state->key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED) {
      if (crypt_data->re_encrypt_validation_tag(crypt_data->max_key_version + 1,
                                                key_state->key_version) ==
          false) {
        validation_tag_re_encryption_failure = true;
      } else {
        // If we are doing unencrypted=>encrypted rotation - set online
        // encryption to true. Such tables - in case the rotation is
        // not finished before server shutdowns/crashes - will be validated
        // and server will check that needed encryption keys are loaded.
        if (crypt_data->min_key_version == 0) {
          mutex_exit(&crypt_data->mutex);

          DBUG_EXECUTE_IF(
              "hang_on_ts_hang_rotation",
              if (strcmp(state->space->name, "ts_hang") == 0) {
                // artifical key_id = 10 to let MTR test know that we are
                // hanging
                static EncryptionKeyId key_id = crypt_data->key_id;
                crypt_data->key_id = 10;
                while (
                    DBUG_EVALUATE_IF("hang_on_ts_hang_rotation", true, false))
                  std::this_thread::sleep_for(std::chrono::microseconds(1000));
                crypt_data->key_id = key_id;
              });

          if (dd_set_online_encryption(state->thd, state->space->name,
                                       &state->space->stop_new_ops)) {
            // should not happen
            ib::error() << "Could not update DD for tablespace "
                        << state->space->name
                        << " with information on online keyring encryption."
                        << " Removing space from online keyring encryption.";
            state->space->exclude_from_rotation = true;
            crypt_data->rotate_state.destroy_flush_observer();
            mutex_enter(&crypt_data->mutex);
            crypt_data->rotate_state.active_threads = 0;
            mutex_exit(&crypt_data->mutex);
            mutex_exit(&crypt_data->start_rotate_mutex);
            return false;
          }

          mutex_enter(&crypt_data->mutex);
        }
        crypt_data->max_key_version = key_state->key_version;
        ut_ad(crypt_data->min_key_version < crypt_data->max_key_version);
      }
    }
    if (load_key_failure || validation_tag_re_encryption_failure) {
      ib::error() << "Encryption thread could not retrieve a key from a "
                     "keyring for tablespace "
                  << state->space->name
                  << " . Removing space from encrypting. Please make sure "
                     "keyring is functional and try restarting the server";
      state->space->exclude_from_rotation = true;
      crypt_data->max_key_version = org_max_key_version;
      crypt_data->rotate_state.active_threads = 0;
      mutex_exit(&crypt_data->mutex);
      crypt_data->rotate_state.destroy_flush_observer();
      mutex_exit(&crypt_data->start_rotate_mutex);
      return false;
    }

    if (key_state->key_version == ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED)
      crypt_data->encryption_rotation = Encryption_rotation::DECRYPTING;
    else if (crypt_data->encryption_rotation !=
             Encryption_rotation::MASTER_KEY_TO_KEYRING)
      crypt_data->encryption_rotation = Encryption_rotation::ENCRYPTING;

    /* only first thread needs to init */
    crypt_data->rotate_state.next_offset = 1;  // skip page 0
    /* no need to rotate beyond current max
     * if space extends, it will be encrypted with newer version */

    ut_ad(state->space->size > 0);
    crypt_data->rotate_state.max_offset = state->space->size;
    crypt_data->rotate_state.end_lsn = 0;
    crypt_data->rotate_state.min_key_version_found = key_state->key_version;

    crypt_data->rotate_state.start_time = time(0);

    if (crypt_data->type == CRYPT_SCHEME_UNENCRYPTED &&
        !crypt_data->is_encryption_disabled() &&
        key_state->key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED) {
      /* this is rotation unencrypted => encrypted */
      crypt_data->type = CRYPT_SCHEME_1;
    }

    fil_crypt_write_crypt_data_to_page0(state->space);

    DBUG_EXECUTE_IF(
        "set_number_of_t1_pages_to_rotate_to_20",
        if (strcmp(state->space->name, "test/t1") == 0) {
          number_of_t1_pages_to_rotate = 20;
        });
  } else {
    /* count active threads in space */
    crypt_data->rotate_state.active_threads++;
  }

  /* Initialize thread local state */
  state->min_key_version_found = crypt_data->rotate_state.min_key_version_found;

  mutex_exit(&crypt_data->mutex);
  mutex_exit(&crypt_data->start_rotate_mutex);

  return true;
}

/***********************************************************************
Search for batch of pages needing rotation
@param[in]	key_state		Key state
@param[in,out]	state			Rotation state
@return true if page needing key rotation found, false if not found */
static bool fil_crypt_find_page_to_rotate(const key_state_t *key_state,
                                          rotate_thread_t *state) {
  ulint batch = srv_alloc_time * state->allocated_iops;
  fil_space_t *space = state->space;

  ut_ad(!space || space->n_pending_ops > 0);

  /* If space is marked to be dropped stop rotation. */
  if (!space || space->is_stopping()) {
    return false;
  }

  fil_space_crypt_t *crypt_data = space->crypt_data;

  mutex_enter(&crypt_data->mutex);
  ut_ad(key_state->key_id == crypt_data->key_id);

  bool found = crypt_data->rotate_state.max_offset >=
               crypt_data->rotate_state.next_offset;

  if (found) {
    state->offset = crypt_data->rotate_state.next_offset;
    ulint remaining = crypt_data->rotate_state.max_offset -
                      crypt_data->rotate_state.next_offset;

    if (batch <= remaining) {
      state->batch = batch;
    } else {
      state->batch = remaining;
    }
  }

  crypt_data->rotate_state.next_offset += batch;
  mutex_exit(&crypt_data->mutex);
  return found;
}

#define fil_crypt_get_page_throttle(state, offset, mtr, sleeptime_ms)          \
  fil_crypt_get_page_throttle_func(state, offset, mtr, sleeptime_ms, __FILE__, \
                                   __LINE__)

/***********************************************************************
Get a page and compute sleep time
@param[in,out]		state		Rotation state
@param[in]		offset		Page offset
@param[in,out]		mtr		Minitransaction
@param[out]		sleeptime_ms	Sleep time
@param[in]		file		File where called
@param[in]		line		Line where called
@return page or NULL*/
static buf_block_t *fil_crypt_get_page_throttle_func(rotate_thread_t *state,
                                                     ulint offset, mtr_t *mtr,
                                                     ulint *sleeptime_ms,
                                                     const char *file,
                                                     unsigned line) {
  fil_space_t *space = state->space;

  const page_size_t page_size = page_size_t(space->flags);
  const page_id_t page_id(space->id, offset);
  ut_ad(space->n_pending_ops > 0);

  /* Before reading from tablespace we need to make sure that
  the tablespace is not about to be dropped or truncated. */
  if (space->is_stopping()) {
    return NULL;
  }

  buf_block_t *block =
      buf_page_get_gen(page_id, page_size, RW_X_LATCH, NULL,
                       Page_fetch::PEEK_IF_IN_POOL, file, line, mtr);

  if (block != NULL) {
    /* page was in buffer pool */
    state->crypt_stat.pages_read_from_cache++;
    return block;
  }

  if (space->is_stopping()) {
    return NULL;
  }

  state->crypt_stat.pages_read_from_disk++;

  const auto start = ut_time_monotonic_us();
  block = buf_page_get_gen(page_id, page_size, RW_X_LATCH, NULL,
                           Page_fetch::POSSIBLY_FREED, file, line, mtr, false);
  const auto end = ut_time_monotonic_us();

  state->cnt_waited++;
  state->sum_waited_us += (end - start);

  /* average page load */
  ulint add_sleeptime_ms = 0;
  ulint avg_wait_time_us = ulint(state->sum_waited_us / state->cnt_waited);
  ulint alloc_wait_us = 1000000 / state->allocated_iops;

  if (avg_wait_time_us < alloc_wait_us) {
    /* we reading faster than we allocated */
    add_sleeptime_ms = (alloc_wait_us - avg_wait_time_us) / 1000;
  } else {
    /* if page load time is longer than we want, skip sleeping */
  }

  *sleeptime_ms += add_sleeptime_ms;

  return block;
}

/***********************************************************************
Get block and allocation status
 note: innodb locks fil_space_latch and then block when allocating page
but locks block and then fil_space_latch when freeing page.
 @param[in,out]          state           Rotation state
@param[in]              offset          Page offset
@param[in,out]          mtr             Minitransaction
@param[out]             allocation_status Allocation status
@param[out]             sleeptime_ms    Sleep time
@return block or NULL
*/
static buf_block_t *btr_scrub_get_block_and_allocation_status(
    rotate_thread_t *state, fseg_header_t *seg, ulint offset, mtr_t *mtr,
    btr_scrub_page_allocation_status_t *allocation_status,
    ulint *sleeptime_ms) {
  mtr_t local_mtr;
  buf_block_t *block = NULL;
  mtr_start(&local_mtr);
  mtr_commit(&local_mtr);
  block = fil_crypt_get_page_throttle(state, offset, mtr, sleeptime_ms);
  *allocation_status = block->page.state == BUF_BLOCK_NOT_USED
                           ? BTR_SCRUB_PAGE_FREE
                           : BTR_SCRUB_PAGE_ALLOCATED;
  return block;
}

/***********************************************************************
Rotate one page
@param[in,out]		key_state		Key state
@param[in,out]		state			Rotation state */
static void fil_crypt_rotate_page(const key_state_t *key_state,
                                  rotate_thread_t *state) {
  fil_space_t *space = state->space;
  const auto space_id = space->id;
  const auto offset = state->offset;
  ulint sleeptime_ms = 0;
  fil_space_crypt_t *crypt_data = space->crypt_data;

  ut_ad(space->n_pending_ops > 0);
  ut_ad(offset > 0);

  /* In fil_crypt_thread where key rotation is done we have
  acquired space and checked that this space is not yet
  marked to be dropped. Similarly, in fil_crypt_find_page_to_rotate().
  Check here also to give DROP TABLE or similar a change. */
  if (space->is_stopping()) {
    return;
  }

  if (space_id == TRX_SYS_SPACE && offset == TRX_SYS_PAGE_NO) {
    /* don't encrypt this as it contains address to dblwr buffer */
    return;
  }

  mtr_t mtr;
  mtr.start();
  mtr.set_log_mode(MTR_LOG_NO_REDO);  // We do not need those pages to be redo
                                      // log. Before we flush page 0, we make
                                      // sure that all pages have been flushed
                                      // to disk. If we fail to update page 0 we
                                      // will rotate those pages again after
                                      // restart - when encryption threads
                                      // discover that there is work to do.

  int needs_scrubbing = BTR_SCRUB_SKIP_PAGE;

  if (buf_block_t *block =
          fil_crypt_get_page_throttle(state, offset, &mtr, &sleeptime_ms)) {
    byte *frame = buf_block_get_frame(block);

    // We always assume that page needs to be encrypted with RK when rotating
    // from MK encryption This might not be the case if the rotation was aborted
    // (due to server crash) and some of the pages might be already encrypted
    // with RK. We re-encrypt them anyways. We could be calculating post -
    // encryption checksum here and decide based on them if the page is RK
    // encrypted or MK encrypted, but this should be very rare case and some
    // extra-re-encryption will do no harm - and we safe on calculating
    // checksums in normal execution
    //
    // This is now also true for all the other encryption rotations
    //
    // We will rotate the pages from the begining if there was a crash
    uint kv = space->crypt_data->encryption_rotation ==
                      Encryption_rotation::MASTER_KEY_TO_KEYRING
                  ? ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED
                  : mach_read_from_4(frame + FIL_PAGE_ENCRYPTION_KEY_VERSION);

    if (space->is_stopping()) {
      /* The tablespace is closing (in DROP TABLE or
      TRUNCATE TABLE or similar): avoid further access */
    } else if (!*reinterpret_cast<uint32_t *>(FIL_PAGE_OFFSET + frame)) {
      /* It looks like this page was never
      allocated. Because key rotation is accessing
      pages in a pattern that is unlike the normal
      B-tree and undo log access pattern, we cannot
      invoke fseg_page_is_free() here, because that
      could result in a deadlock. If we invoked
      fseg_page_is_free() and released the
      tablespace latch before acquiring block->lock,
      then the fseg_page_is_free() information
      could be stale already. */
      ut_ad(page_get_space_id(frame) == 0);
    } else if (fil_crypt_needs_rotation(crypt_data->encryption, kv,
                                        key_state->key_version,
                                        key_state->rotate_key_age)) {
      // mtr.set_named_space(space);
      mtr.set_flush_observer(crypt_data->rotate_state.flush_observer);

      /* force rotation by dummy updating page */
      mlog_write_ulint(frame + FIL_PAGE_SPACE_ID, space_id, MLOG_4BYTES, &mtr);
      // assign key version to a page in a buffer - so it would not be rotated
      // more times
      mlog_write_ulint(frame + FIL_PAGE_ENCRYPTION_KEY_VERSION,
                       key_state->key_version, MLOG_4BYTES, &mtr);

      /* statistics */
      state->crypt_stat.pages_modified++;
    } else if (mach_read_from_4(frame + FIL_PAGE_LSN) == 0) {
      /* LSN == 0 means that the page was never flushed to disk. Thus it never
       * had the key version assigned. We assign the key version to this page
       * here, as it only exists in buffer */
      mlog_write_ulint(frame + FIL_PAGE_ENCRYPTION_KEY_VERSION,
                       key_state->key_version, MLOG_4BYTES, &mtr);
    } else {
      if (!crypt_data->is_encryption_disabled()) {
        if (kv == ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED ||
            kv < state->min_key_version_found) {
          state->min_key_version_found = kv;
        }
      }
      needs_scrubbing = btr_page_needs_scrubbing(
          &state->scrub_data, block, BTR_SCRUB_PAGE_ALLOCATION_UNKNOWN);
    }

    mtr.commit();
  } else {
    /* If block read failed mtr memo and log should be empty. */
    ut_ad(!mtr.has_modifications());
    ut_ad(!mtr.is_dirty());
    ut_ad(mtr.get_memo()->size() == 0);
    ut_ad(mtr.get_log()->size() == 0);
    mtr.commit();
  }

  if (needs_scrubbing == BTR_SCRUB_PAGE) {
    mtr.start();
    btr_scrub_page_allocation_status_t allocated =
        BTR_SCRUB_PAGE_ALLOCATION_UNKNOWN;
    buf_block_t *block = btr_scrub_get_block_and_allocation_status(
        state, NULL, offset, &mtr, &allocated, &sleeptime_ms);
    if (block) {
      // mtr.set_named_space(space);
      /* get required table/index and index-locks */
      needs_scrubbing =
          btr_scrub_recheck_page(&state->scrub_data, block, allocated, &mtr);
      if (needs_scrubbing == BTR_SCRUB_PAGE) {
        /* we need to refetch it once more now that we have
         * index locked */
        block = btr_scrub_get_block_and_allocation_status(
            state, NULL, offset, &mtr, &allocated, &sleeptime_ms);
        needs_scrubbing =
            btr_scrub_page(&state->scrub_data, block, allocated, &mtr);
      }
      /* NOTE: mtr is committed inside btr_scrub_recheck_page()
       * and/or btr_scrub_page. This is to make sure that
       * locks & pages are latched in corrected order,
       * the mtr is in some circumstances restarted.
       * (mtr_commit() + mtr_start())
       */
    }
  }
  if (needs_scrubbing != BTR_SCRUB_PAGE) {
    /* if page didn't need scrubbing it might be that cleanups
       are needed. do those outside of any mtr to prevent deadlocks.
       the information what kinds of cleanups that are needed are
       encoded inside the needs_scrubbing, but this is opaque to
       this function (except the value BTR_SCRUB_PAGE) */
    btr_scrub_skip_page(&state->scrub_data, needs_scrubbing);
  }
  if (needs_scrubbing == BTR_SCRUB_TURNED_OFF) {
    /* if we just detected that scrubbing was turned off
     * update global state to reflect this */
    ut_ad(crypt_data);
    mutex_enter(&crypt_data->mutex);
    crypt_data->rotate_state.scrubbing.is_active = false;
    mutex_exit(&crypt_data->mutex);
  }

  if (sleeptime_ms) {
    os_event_reset(fil_crypt_throttle_sleep_event);
    os_event_wait_time(fil_crypt_throttle_sleep_event, 1000 * sleeptime_ms);
  }
}

/***********************************************************************
Rotate a batch of pages
@param[in,out]		key_state		Key state
@param[in,out]		state			Rotation state */
static void fil_crypt_rotate_pages(const key_state_t *key_state,
                                   rotate_thread_t *state) {
  const auto space = state->space->id;
  const auto end =
      std::min(state->offset + state->batch, state->space->free_limit);

  ut_ad(state->space->n_pending_ops > 0);

  for (; state->offset < end && !state->space->is_space_encrypted;
       state->offset++) {
    /* we can't rotate pages in dblwr buffer as
     * it's not possible to read those due to lots of asserts
     * in buffer pool.
     *
     * However since these are only (short-lived) copies of
     * real pages, they will be updated anyway when the
     * real page is updated
     */
    if (space == TRX_SYS_SPACE && dblwr::v1::is_inside(state->offset)) {
      continue;
    }

    DBUG_EXECUTE_IF(
        "rotate_only_first_x_pages_from_t1",
        if (strcmp(state->space->name, "test/t1") == 0) {
          if (number_of_t1_pages_rotated >= number_of_t1_pages_to_rotate) {
            state->offset = end;
            return;
          } else
            ++number_of_t1_pages_rotated;
        });

    fil_crypt_rotate_page(key_state, state);
  }
}

/******************************************************************/ /**
 Callback that sets a hex formatted FTS table's flags2 in
 SYS_TABLES. The flags is stored in MIX_LEN column.
 @return FALSE if all OK */
// static
// ibool
// fts_set_encrypted_flag_for_table(
// void*		row,		// in: sel_node_t*
// void*		user_arg) {	// in: bool set/unset flag

// sel_node_t*	node = static_cast<sel_node_t*>(row);
// dfield_t*	dfield = que_node_get_val(node->select_list);

// ut_ad(dtype_get_mtype(dfield_get_type(dfield)) == DATA_INT);
// ut_ad(dfield_get_len(dfield) == sizeof(ib_uint32_t));
//[> There should be at most one matching record. So the value
// must be the default value. */
// ut_ad(mach_read_from_4(static_cast<byte*>(user_arg))
//== ULINT32_UNDEFINED);

// ulint flags2 = mach_read_from_4(
// static_cast<byte*>(dfield_get_data(dfield)));

// flags2 |= DICT_TF2_ENCRYPTION;

// mach_write_to_4(static_cast<byte*>(user_arg), flags2);

// return(FALSE);
//}

// static
// ibool
// fts_unset_encrypted_flag_for_table(
// void*		row,		// in: sel_node_t*
// void*		user_arg) {	// in: bool set/unset flag

// sel_node_t*	node = static_cast<sel_node_t*>(row);
// dfield_t*	dfield = que_node_get_val(node->select_list);

// ut_ad(dtype_get_mtype(dfield_get_type(dfield)) == DATA_INT);

// ulint flags = mach_read_from_4(
// static_cast<byte*>(dfield_get_data(dfield)));

// flags &= ~DICT_TF2_ENCRYPTION;
// mach_write_to_4(static_cast<byte*>(user_arg), flags);

// return(FALSE);
//}

// static
// dberr_t
// fts_update_encrypted_tables_flag(
// trx_t*		trx,		[> in/out: transaction that
// covers the update */
// table_id_t	table_id,
// bool	set) {			[> in: Table for which we want
// to set the root table->flags2 */
// pars_info_t*		info;
// ib_uint32_t		flags2;

// static const char	sql[] =
//"PROCEDURE UPDATE_ENCRYPTED_FLAG() IS\n"
//"DECLARE FUNCTION my_func;\n"
//"DECLARE CURSOR c IS\n"
//" SELECT MIX_LEN"
//" FROM SYS_TABLES"
//" WHERE ID = :table_id FOR UPDATE;"
//"\n"
//"BEGIN\n"
//"OPEN c;\n"
//"WHILE 1 = 1 LOOP\n"
//"  FETCH c INTO my_func();\n"
//"  IF c % NOTFOUND THEN\n"
//"    EXIT;\n"
//"  END IF;\n"
//"END LOOP;\n"
//"UPDATE SYS_TABLES"
//" SET MIX_LEN = :flags2"
//" WHERE ID = :table_id;\n"
//"CLOSE c;\n"
//"END;\n";

// flags2 = ULINT32_UNDEFINED;

// info = pars_info_create();

// pars_info_add_ull_literal(info, "table_id", table_id);
// pars_info_bind_int4_literal(info, "flags2", &flags2);

// pars_info_bind_function(
// info, "my_func", set ? fts_set_encrypted_flag_for_table
//: fts_unset_encrypted_flag_for_table, &flags2);

// if (trx_get_dict_operation(trx) == TRX_DICT_OP_NONE) {
// trx_set_dict_operation(trx, TRX_DICT_OP_INDEX);
//}

// dberr_t err = que_eval_sql(info, sql, false, trx);

// ut_a(flags2 != ULINT32_UNDEFINED);

// return(err);
//}

// static
// ibool
// fts_unset_encrypted_flag_for_tablespace(
// void*		row,		// in: sel_node_t*
// void*		user_arg) {	// in: bool set/unset flag

// sel_node_t*	node = static_cast<sel_node_t*>(row);
// dfield_t*	dfield = que_node_get_val(node->select_list);

// ut_ad(dtype_get_mtype(dfield_get_type(dfield)) == DATA_INT);
// ut_ad(dfield_get_len(dfield) == sizeof(ib_uint32_t));
//// There should be at most one matching record. So the value
//// must be the default value.
// ut_ad(mach_read_from_4(static_cast<byte*>(user_arg))
//== ULINT32_UNDEFINED);

// ulint  flags = mach_read_from_4(
// static_cast<byte*>(dfield_get_data(dfield)));

// flags &= ~(1U << FSP_FLAGS_POS_ENCRYPTION);

// mach_write_to_4(static_cast<byte*>(user_arg), flags);

// return(FALSE);
//}

// static
// ibool
// fts_set_encrypted_flag_for_tablespace(
// void*		row,		// in: sel_node_t*
// void*		user_arg) {	// in: bool set/unset flag

// sel_node_t*	node = static_cast<sel_node_t*>(row);
// dfield_t*	dfield = que_node_get_val(node->select_list);

// ut_ad(dtype_get_mtype(dfield_get_type(dfield)) == DATA_INT);
// ut_ad(dfield_get_len(dfield) == sizeof(ib_uint32_t));
//// There should be at most one matching record. So the value
//// must be the default value.
// ut_ad(mach_read_from_4(static_cast<byte*>(user_arg))
//== ULINT32_UNDEFINED);

// ulint  flags = mach_read_from_4(
// static_cast<byte*>(dfield_get_data(dfield)));

// flags |= (1U << FSP_FLAGS_POS_ENCRYPTION);

// mach_write_to_4(static_cast<byte*>(user_arg), flags);

// return(FALSE);
//}

// static
// ibool
// read_table_id(
//[>============<]
// void*		row,		[>!< in: sel_node_t* <]
// void*		user_arg)	[>!< in: pointer to ib_vector_t <]
//{
// ib_vector_t*	tables_ids = static_cast<ib_vector_t*>(user_arg);

// sel_node_t*	node = static_cast<sel_node_t*>(row);
// dfield_t*	dfield = que_node_get_val(node->select_list);

// ut_ad(dfield_get_len(dfield) == 8);

// table_id_t *table_id = static_cast<table_id_t*>(ib_vector_push(tables_ids,
// NULL));

//*table_id = mach_read_from_8(static_cast<byte*>(dfield_get_data(dfield)));

// return(TRUE);
//}

// static
// dberr_t
// get_table_ids_in_space_sql(
// trx_t*		trx,		// in/out: transaction that
// fil_space_t *space,
// ib_vector_t* tables_ids
//) {
// pars_info_t *info = pars_info_create();

// static const char	sql[] =
//"PROCEDURE GET_TABLES_IDS() IS\n"
//"DECLARE FUNCTION my_func;\n"
//"DECLARE CURSOR c IS"
//" SELECT ID"
//" FROM SYS_TABLES"
//" WHERE SPACE=:space_id;\n"
//"BEGIN\n"
//"\n"
//"OPEN c;\n"
//"WHILE 1 = 1 LOOP\n"
//"  FETCH c INTO my_func();\n"
//"  IF c % NOTFOUND THEN\n"
//"    EXIT;\n"
//"  END IF;\n"
//"END LOOP;\n"
//"CLOSE c;\n"
//"END;\n";

// pars_info_bind_function(info, "my_func", read_table_id, tables_ids);
// pars_info_add_int4_literal(info, "space_id", space->id);

// dberr_t err = que_eval_sql(info, sql, false, trx);

// return(err);
//}
/*
static
void
fil_revert_encryption_flag_updates(ib_vector_t* tables_ids_to_revert_if_error,
bool set) {

        while (!ib_vector_is_empty(tables_ids_to_revert_if_error)) {
                table_id_t *table_id = static_cast<table_id_t*>(
                        ib_vector_pop(tables_ids_to_revert_if_error));

                dict_table_t *table = dict_table_open_on_id(*table_id, TRUE,
                                DICT_TABLE_OP_NORMAL);

                ut_ad(table != NULL);

                if (set) {
                        DICT_TF2_FLAG_SET(table, DICT_TF2_ENCRYPTION);
                } else
                        DICT_TF2_FLAG_UNSET(table, DICT_TF2_ENCRYPTION);

                dict_table_close(table, TRUE, FALSE);
        }
}*/

class TransactionAndHeapGuard {
 public:
  TransactionAndHeapGuard()
      : dict_operation_locked(false),
        trx(NULL),
        dict_sys_mutex_entered(false),
        heap(NULL),
        heap_alloc(NULL),
        table_ids(NULL),
        table_ids_to_revert(NULL),
        do_rollback(true) {}

  bool lock_x_dict_operation_lock(fil_space_t *space) {
    ut_ad(!dict_operation_locked);
    ut_ad(trx == nullptr);
    ut_ad(!dict_sys_mutex_entered);
    ut_ad(heap == nullptr);
    ut_ad(heap_alloc == nullptr);
    ut_ad(table_ids == nullptr);
    ut_ad(table_ids_to_revert == nullptr);

    // This should only wait in rare cases
    while (!rw_lock_x_lock_nowait(dict_operation_lock)) {
      std::this_thread::sleep_for(std::chrono::microseconds(6));
      if (space->stop_new_ops)  // space is about to be dropped
        return false;           // do not try to lock the DD
    }
    dict_operation_locked = true;
    return true;
  }

  bool allocate_trx() {
    ut_ad(dict_operation_locked);
    ut_ad(trx == nullptr);
    ut_ad(!dict_sys_mutex_entered);
    ut_ad(heap == nullptr);
    ut_ad(heap_alloc == nullptr);
    ut_ad(table_ids == nullptr);
    ut_ad(table_ids_to_revert == nullptr);

    trx = trx_allocate_for_background();
    if (trx == NULL) return false;
    trx->op_info = "setting encrypted flag";
    trx->dict_operation_lock_mode = RW_X_LATCH;
    return true;
  }

  void enter_dict_sys_mutex() {
    ut_ad(dict_operation_locked);
    ut_ad(trx != nullptr);
    ut_ad(!dict_sys_mutex_entered);
    ut_ad(heap == nullptr);
    ut_ad(heap_alloc == nullptr);
    ut_ad(table_ids == nullptr);
    ut_ad(table_ids_to_revert == nullptr);

    dict_mutex_enter_for_mysql();
    dict_sys_mutex_entered = true;
  }

  bool create_heap() {
    ut_ad(dict_operation_locked);
    ut_ad(trx != nullptr);
    ut_ad(dict_sys_mutex_entered);
    ut_ad(heap == nullptr);
    ut_ad(heap_alloc == nullptr);
    ut_ad(table_ids == nullptr);
    ut_ad(table_ids_to_revert == nullptr);

    // TODO: consider moving expensive operation out of dict_sys->mutex
    heap = mem_heap_create(1024);
    return heap != NULL;
  }

  bool create_allocator() {
    ut_ad(dict_operation_locked);
    ut_ad(trx != nullptr);
    ut_ad(dict_sys_mutex_entered);
    ut_ad(heap != nullptr);
    ut_ad(heap_alloc == nullptr);
    ut_ad(table_ids == nullptr);
    ut_ad(table_ids_to_revert == nullptr);

    heap_alloc = ib_heap_allocator_create(heap);
    return heap_alloc != NULL;
  }

  bool create_table_ids_vector() {
    ut_ad(dict_operation_locked);
    ut_ad(trx != nullptr);
    ut_ad(dict_sys_mutex_entered);
    ut_ad(heap != nullptr);
    ut_ad(heap_alloc != nullptr);
    ut_ad(table_ids == nullptr);
    ut_ad(table_ids_to_revert == nullptr);

    table_ids = ib_vector_create(heap_alloc, sizeof(table_id_t), 128);
    return table_ids != NULL;
  }

  bool create_table_ids_to_revert_vector() {
    ut_ad(dict_operation_locked);
    ut_ad(trx != nullptr);
    ut_ad(dict_sys_mutex_entered);
    ut_ad(heap != nullptr);
    ut_ad(heap_alloc != nullptr);
    ut_ad(table_ids != nullptr);
    ut_ad(table_ids_to_revert == nullptr);

    table_ids_to_revert = ib_vector_create(heap_alloc, sizeof(table_id_t), 128);
    return table_ids_to_revert != NULL;
  }

  bool is_table_ids_empty() const {
    ut_ad(dict_operation_locked);
    ut_ad(trx != nullptr);
    ut_ad(dict_sys_mutex_entered);
    ut_ad(heap != nullptr);
    ut_ad(heap_alloc != nullptr);
    ut_ad(table_ids != nullptr);
    ut_ad(table_ids_to_revert != nullptr);

    return ib_vector_is_empty(table_ids);
  }

  table_id_t pop_from_table_ids() {
    ut_ad(table_ids != NULL);

    return *static_cast<table_id_t *>(ib_vector_pop(table_ids));
  }

  void push_to_table_ids_to_revert(table_id_t table_id) {
    ut_ad(table_ids_to_revert != NULL);

    ib_vector_push(table_ids_to_revert, &table_id);
  }

  trx_t *get_trx() {
    ut_ad(trx != NULL);
    return trx;
  }

  ib_vector_t *get_table_ids() {
    ut_ad(table_ids != NULL);
    return table_ids;
  }

  ib_vector_t *get_table_ids_to_revert() {
    ut_ad(table_ids_to_revert != NULL);
    return table_ids_to_revert;
  }

  void commit() {
    ut_ad(dict_operation_locked);
    ut_ad(trx != NULL);
    ut_ad(dict_sys_mutex_entered);
    ut_ad(heap != NULL);
    ut_ad(heap_alloc != NULL);
    ut_ad(table_ids != NULL);
    ut_ad(table_ids_to_revert != NULL);

    fts_sql_commit(trx);
    do_rollback = false;
  }

  ~TransactionAndHeapGuard() {
    if (trx && do_rollback) fts_sql_rollback(trx);

    /*
    if (table_ids_to_revert != NULL)
            ib_vector_free(table_ids_to_revert);

    if (table_ids != NULL)
            ib_vector_free(table_ids);

    if (heap_alloc != NULL)
            ib_heap_allocator_free(heap_alloc);
    */

    if (heap != NULL) mem_heap_free(heap);

    if (dict_sys_mutex_entered) dict_mutex_exit_for_mysql();

    if (dict_operation_locked) {
      rw_lock_x_unlock(dict_operation_lock);
      trx->dict_operation_lock_mode = 0;
    }

    if (trx != NULL) trx_free_for_background(trx);
  }

 private:
  TransactionAndHeapGuard(const TransactionAndHeapGuard &);
  TransactionAndHeapGuard &operator=(const TransactionAndHeapGuard &);

  bool dict_operation_locked;
  trx_t *trx;
  bool dict_sys_mutex_entered;
  mem_heap_t *heap;
  ib_alloc_t *heap_alloc;
  ib_vector_t *table_ids;
  ib_vector_t *table_ids_to_revert;

  bool do_rollback;
};

enum class UpdateEncryptedFlagOperation : char { SET, CLEAR };

static dberr_t fil_update_encrypted_flag(
    const char *space_name, UpdateEncryptedFlagOperation update_operation,
    volatile bool *is_space_being_removed, THD *thd) {
  DBUG_EXECUTE_IF(
      "fail_encryption_flag_update_on_t3",
      if (strcmp(space_name, "test/t3") == 0) { return DB_ERROR; });

  // we set DD's online_encryption flag to N in case we have decrypted
  bool failure =
      (update_operation == UpdateEncryptedFlagOperation::SET
           ? dd_set_encryption_flag(thd, space_name, is_space_being_removed)
           : dd_clear_encryption_flag(
                 thd, space_name, is_space_being_removed,
                 update_operation == UpdateEncryptedFlagOperation::CLEAR));

  return (failure ? DB_ERROR : DB_SUCCESS);
}

/***********************************************************************
Flush rotated pages and then update page 0

@param[in,out]		state	rotation state */
static dberr_t fil_crypt_flush_space(rotate_thread_t *state) {
  fil_space_t *space = state->space;
  fil_space_crypt_t *crypt_data = space->crypt_data;

  ut_ad(space->n_pending_ops > 0);

  /* flush tablespace pages so that there are no pages left with old key */

  ulint number_of_pages_flushed_so_far =
      crypt_data->rotate_state.flush_observer->get_number_of_pages_flushed();

  if (space->is_stopping()) {
    crypt_data->rotate_state.destroy_flush_observer();
    return DB_SUCCESS;
  }

  ulint number_of_pages_flushed_now = 0;
  log_free_check();
  const auto start = ut_time_monotonic_us();

  crypt_data->rotate_state.flush_observer->flush();

  const auto end = ut_time_monotonic_us();

  number_of_pages_flushed_now =
      crypt_data->rotate_state.flush_observer->get_number_of_pages_flushed() -
      number_of_pages_flushed_so_far;

  if (number_of_pages_flushed_now > 0 && end > start) {
    state->cnt_waited += number_of_pages_flushed_now;
    state->sum_waited_us += (end - start);

    /* statistics */
    state->crypt_stat.pages_flushed += number_of_pages_flushed_now;
  }

  crypt_data->rotate_state.destroy_flush_observer();

  // We do not assign the type to crypt_data just yet. We do it after
  // write_page0 so the in-memory crypt_data would be in sync with the
  // crypt_data on disk
  ut_ad(crypt_data->rotate_state.flushing);

  uint current_type = crypt_data->rotate_state.min_key_version_found ==
                              ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED
                          ? CRYPT_SCHEME_UNENCRYPTED
                          : crypt_data->type;

  // update DD flags in case we are doing rotation unencrypted => encrypted
  // or encrypted => unnecrypted. For encrypted => encrypted rotation
  // i.e. re-encryption, DD flags do not need to be updated.
  if ((current_type == CRYPT_SCHEME_1 &&
       crypt_data->min_key_version == ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED) ||
      (current_type == CRYPT_SCHEME_UNENCRYPTED &&
       crypt_data->min_key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED)) {
    UpdateEncryptedFlagOperation update_enc_flag_op =
        (current_type == CRYPT_SCHEME_UNENCRYPTED)
            ? UpdateEncryptedFlagOperation::CLEAR
            : UpdateEncryptedFlagOperation::SET;

    if (DB_SUCCESS != fil_update_encrypted_flag(space->name, update_enc_flag_op,
                                                &space->stop_new_ops,
                                                state->thd)) {
      ut_ad(DBUG_EVALUATE_IF("fail_encryption_flag_update_on_t3", 1, 0) ||
            state->space->stop_new_ops);
      return (DB_ERROR);
    }

    fil_lock_shard_by_id(space->id);
    if (update_enc_flag_op == UpdateEncryptedFlagOperation::SET) {
      space->flags |= (1U << FSP_FLAGS_POS_ENCRYPTION);
    } else {
      ut_ad(update_enc_flag_op == UpdateEncryptedFlagOperation::CLEAR);
      space->flags &= ~(1U << FSP_FLAGS_POS_ENCRYPTION);
    }
    fil_unlock_shard_by_id(space->id);
  }

  DBUG_EXECUTE_IF("crash_on_t1_flush_after_dd_update",
                  if (strcmp(state->space->name, "test/t1") == 0)
                      DBUG_SUICIDE(););

  // encrypt encryption_validation_tag with just max_key_version or leave it
  // unencrypted for unencrypted tablespace
  if (current_type == CRYPT_SCHEME_UNENCRYPTED) {
    memcpy(crypt_data->encrypted_validation_tag,
           ENCRYPTION_KEYRING_VALIDATION_TAG,
           ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE);
  } else {
    // we do not need to obtain crypt_data->mutex here, as flushing flag is set
    // - which will stop other threads from starting rotating this space and
    // since we are flushing - we are the only thread which is currently
    // operating on this space.
    ut_ad(crypt_data->rotate_state.active_threads == 1 &&
          crypt_data->local_keys_cache[crypt_data->max_key_version] != nullptr);
    encrypt_validation_tag(
        ENCRYPTION_KEYRING_VALIDATION_TAG,
        ENCRYPTION_KEYRING_VALIDATION_TAG_SIZE,
        crypt_data->local_keys_cache[crypt_data->max_key_version],
        crypt_data->encrypted_validation_tag);
  }

  if (crypt_data->encryption_rotation ==
      Encryption_rotation::MASTER_KEY_TO_KEYRING) {
    crypt_data->set_tablespace_key(nullptr);
    crypt_data->encryption_rotation = Encryption_rotation::ENCRYPTING;
  }

  /* update page 0 */
  mtr_t mtr;
  mtr.start();

  if (buf_block_t *block = buf_page_get_gen(
          page_id_t(space->id, 0), page_size_t(space->flags), RW_X_LATCH, NULL,
          Page_fetch::NORMAL, __FILE__, __LINE__, &mtr)) {
    // mtr.set_named_space(space);
    crypt_data->write_page0(space, block->frame, &mtr,
                            crypt_data->rotate_state.min_key_version_found,
                            current_type == CRYPT_SCHEME_UNENCRYPTED
                                ? ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED
                                : crypt_data->max_key_version,
                            current_type);
  }

  mtr.commit();
  return DB_SUCCESS;
}

/***********************************************************************
Complete rotating a space
@param[in,out]		key_state		Key state
@param[in,out]		state			Rotation state */
static void fil_crypt_complete_rotate_space(const key_state_t *key_state,
                                            rotate_thread_t *state) {
  fil_space_crypt_t *crypt_data = state->space->crypt_data;

  ut_ad(crypt_data);
  ut_ad(state->space->n_pending_ops > 0);

  /* Space might already be dropped */
  if (!state->space->is_stopping()) {
    mutex_enter(&crypt_data->mutex);

    /**
     * Update crypt data state with state from thread
     */
    if (state->min_key_version_found == ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED ||
        state->min_key_version_found <
            crypt_data->rotate_state.min_key_version_found) {
      crypt_data->rotate_state.min_key_version_found =
          state->min_key_version_found;
    }

    ut_a(crypt_data->rotate_state.active_threads > 0);
    bool last = crypt_data->rotate_state.active_threads - 1 == 0;

    /**
     * check if space is fully done
     * this as when threads shutdown, it could be that we "complete"
     * iterating before we have scanned the full space.
     */
    bool done = crypt_data->rotate_state.next_offset >=
                crypt_data->rotate_state.max_offset;

    /**
     * we should flush space if we're last thread AND
     * the iteration is done
     */
    bool should_flush = last && done;

    /* In case we simulate only 100 pages being rotated - we stop ourselves from
     * writting to page0. Pages should be flushed in mtr test with FLUSH FOR
     * EXPORT - this will make sure that buffers will get flushed * In MTR we
     * can check if we reached this point by checking flushing field - it should
     * be 1 if we are here */
    DBUG_EXECUTE_IF(
        "rotate_only_first_x_pages_from_t1",
        if (strcmp(state->space->name, "test/t1") == 0 &&
            number_of_t1_pages_rotated >= number_of_t1_pages_to_rotate) {
          crypt_data->rotate_state.flushing = true;
          should_flush = false;
        });

    if (should_flush) {
      /* we're the last active thread */
      ut_ad(crypt_data->rotate_state.flushing == false);
      crypt_data->rotate_state.flushing = true;
    }

    DBUG_EXECUTE_IF(
        "crash_on_t1_flush_after_dd_update",
        if (strcmp(state->space->name, "test/t1") == 0 &&
            number_of_t1_pages_rotated >= 100) should_flush = true;);

    /* inform scrubbing */
    crypt_data->rotate_state.scrubbing.is_active = false;

    mutex_exit(&crypt_data->mutex);

    if (state->scrub_data.scrubbing) {
      btr_scrub_complete_space(&state->scrub_data);
      if (should_flush) {
        // only last thread updates last_scrub_completed
        ut_ad(crypt_data);
        mutex_enter(&crypt_data->mutex);
        crypt_data->rotate_state.scrubbing.last_scrub_completed = time(0);
        mutex_exit(&crypt_data->mutex);
      }
    }

    if (should_flush) {
      if (fil_crypt_flush_space(state) == DB_SUCCESS) {
        uint current_type = crypt_data->rotate_state.min_key_version_found ==
                                    ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED
                                ? CRYPT_SCHEME_UNENCRYPTED
                                : crypt_data->type;
        mutex_enter(&crypt_data->mutex);
        crypt_data->min_key_version =
            crypt_data->rotate_state.min_key_version_found;
        if (current_type == CRYPT_SCHEME_UNENCRYPTED) {
          crypt_data->max_key_version = ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED;
        }
        crypt_data->type = current_type;
        crypt_data->rotate_state.flushing = false;
      } else {
        mutex_enter(&crypt_data->mutex);
        crypt_data->rotate_state.flushing = false;
        if (!state->space->stop_new_ops) {  // Flag updated failed not due to
                                            // tablespace being dropped
          ib::error() << "Encryption thread failed to flush encryption "
                         "information for tablespace "
                      << state->space->name
                      << ". This should not happen and could indicate problem "
                         "with OS or filesystem. Excluding "
                      << state->space->name << " from encryption rotation. "
                      << "You can try decrypting/encrypting with alter "
                         "statement for this table or restarting the server.";
          state->space->exclude_from_rotation =
              true;  // This will stop encryption threads from picking up this
                     // tablespace for rotation
        }
      }
    }

    if (!should_flush)  // If we are flushing we have already optained the mutex
      mutex_enter(&crypt_data->mutex);

    ut_a(crypt_data->rotate_state.active_threads > 0);
    crypt_data->rotate_state.active_threads--;
    mutex_exit(&crypt_data->mutex);
  } else {
    mutex_enter(&crypt_data->mutex);
    ut_a(crypt_data->rotate_state.active_threads > 0);
    crypt_data->rotate_state.active_threads--;
    if (crypt_data->rotate_state.active_threads == 0) {
      crypt_data->rotate_state.flushing = true;
      mutex_exit(&crypt_data->mutex);
      crypt_data->rotate_state.destroy_flush_observer();
      mutex_enter(&crypt_data->mutex);
      crypt_data->rotate_state.flushing = false;
    }
    mutex_exit(&crypt_data->mutex);
  }
}

/*********************************************************************/ /**
 A thread which monitors global key state and rotates tablespaces accordingly
 @return a dummy parameter */
void fil_crypt_thread() {
  /* TODO: Add this later */
  //#ifdef UNIV_PFS_THREAD
  // pfs_register_thread(page_cleaner_thread_key);
  //#endif
  THD *thd = create_thd(false, true, true, 0);

  mutex_enter(&fil_crypt_threads_mutex);
  uint thread_no = srv_threads.m_crypt_threads_n;
  srv_threads.m_crypt_threads_n++;
  os_event_set(fil_crypt_event); /* signal that we started */
  mutex_exit(&fil_crypt_threads_mutex);

  /* Wait for server to be fully started */
  while (!mysqld_server_started) {
    if (srv_shutdown_state.load() != SRV_SHUTDOWN_NONE) {
      return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  /* state of this thread */
  rotate_thread_t thr(thread_no, thd);

  /* if we find a space that is starting, skip over it and recheck it later */
  bool recheck = false;

  while (!thr.should_shutdown()) {
    key_state_t new_state;
    time_t wait_start = time(0);

    while (!thr.should_shutdown()) {
      /* wait for key state changes
       * i.e either new key version of change or
       * new rotate_key_age */

      /* if there was a timeout on fil_crypt_threads_event - do not reset
       * fil_crypt_threds_event before we start another wait on it. */
      if (os_event_wait_time(fil_crypt_threads_event, 1000000) == 0) {
        os_event_reset(fil_crypt_threads_event);
        break;
      }

      if (recheck) {
        /* check recheck here, after sleep, so
         * that we don't busy loop while when one thread is starting
         * a space*/
        break;
      }

      time_t waited = time(0) - wait_start;
      if (waited >= 0 &&
          ulint(waited) >= srv_background_scrub_data_check_interval &&
          (srv_background_scrub_data_uncompressed ||
           srv_background_scrub_data_compressed)) {
        break;
      }
    }

    recheck = false;
    thr.first = true;  // restart from first tablespace

    /* iterate all spaces searching for those needing rotation */
    while (!thr.should_shutdown() &&
           fil_crypt_find_space_to_rotate(&new_state, &thr, &recheck)) {
      bool rotation_started = fil_crypt_start_rotate_space(&new_state, &thr);

      /* we found a space to rotate */
      if (rotation_started) {
        /* iterate all pages (cooperativly with other threads) */
        while (!thr.should_shutdown() &&
               fil_crypt_find_page_to_rotate(&new_state, &thr)) {
          if (!thr.space->is_stopping()) {
            /* rotate a (set) of pages */
            fil_crypt_rotate_pages(&new_state, &thr);
          }

          if (thr.space->is_space_encrypted) {
            /* There were some pages that were corrupted or could not have been
             * decrypted - abort rotating space */
            mutex_enter(&thr.space->crypt_data->mutex);
            thr.space->crypt_data->rotate_state.flushing = true;
            mutex_exit(&thr.space->crypt_data->mutex);
            thr.space->crypt_data->rotate_state.destroy_flush_observer();
            mutex_enter(&thr.space->crypt_data->mutex);
            thr.space->crypt_data->rotate_state.flushing = false;
            mutex_exit(&thr.space->crypt_data->mutex);
            fil_space_release(thr.space);
            thr.space = NULL;
            break;
          }

          /* If space is marked as stopping, release
          space and stop rotation. */
          if (thr.space->is_stopping()) {
            fil_crypt_complete_rotate_space(&new_state, &thr);
            fil_space_release(thr.space);
            thr.space = NULL;
            break;
          }

          /* realloc iops */
          fil_crypt_realloc_iops(&thr);
        }
        /* complete rotation */
        if (thr.space) {
          fil_crypt_complete_rotate_space(&new_state, &thr);
        }
      }

      /* force key state refresh */
      new_state.key_id = (~0);  // Marking key_id in new_state invalid - so it
                                // will have to be read from crypt_data

      /* return iops */
      fil_crypt_return_iops(&thr);
    }
  }

  /* return iops if shutting down */
  fil_crypt_return_iops(&thr);

  /* release current space if shutting down */
  if (thr.space) {
    fil_space_release(thr.space);
    thr.space = NULL;
  }

  /* We count the number of threads in os_thread_exit(). A created
  thread should always use that to exit and not use return() to exit. */

  thr.thd = nullptr;
  destroy_thd(thd);

  mutex_enter(&fil_crypt_threads_mutex);
  srv_threads.m_crypt_threads_n--;
  os_event_set(fil_crypt_event); /* signal that we stopped */
  mutex_exit(&fil_crypt_threads_mutex);
}

/*********************************************************************
Adjust thread count for key rotation
@param[in]	enw_cnt		Number of threads to be used */

void fil_crypt_set_thread_cnt(const uint new_cnt) {
  if (!fil_crypt_threads_inited) {
    fil_crypt_threads_init();
  }

  mutex_enter(&fil_crypt_threads_set_cnt_mutex);
  mutex_enter(&fil_crypt_threads_mutex);

  if (new_cnt > srv_n_fil_crypt_threads_requested) {
    uint add = new_cnt - srv_n_fil_crypt_threads_requested;
    srv_n_fil_crypt_threads_requested = new_cnt;
    for (uint i = 0; i < add; i++) {
      auto thread = os_thread_create(PSI_NOT_INSTRUMENTED, fil_crypt_thread);
      ib::info() << "Creating #" << i + 1 << " encryption thread"
                 << " total threads " << new_cnt << ".";
      thread.start();
    }
  } else if (new_cnt < srv_n_fil_crypt_threads_requested) {
    srv_n_fil_crypt_threads_requested = new_cnt;
    os_event_set(fil_crypt_threads_event);
  }

  mutex_exit(&fil_crypt_threads_mutex);

  while (srv_threads.m_crypt_threads_n != srv_n_fil_crypt_threads_requested) {
    os_event_reset(fil_crypt_event);
    os_event_wait_time(fil_crypt_event, 100000);
  }

  /* Send a message to encryption threads that there could be
  something to do. */
  if (srv_n_fil_crypt_threads_requested) {
    os_event_set(fil_crypt_threads_event);
  }

  mutex_exit(&fil_crypt_threads_set_cnt_mutex);
}

/*********************************************************************
Adjust max key age
@param[in]	val		New max key age */

void fil_crypt_set_rotate_key_age(uint val) {
  srv_fil_crypt_rotate_key_age = val;
  os_event_set(fil_crypt_threads_event);
}

/*********************************************************************
Adjust rotation iops
@param[in]	val		New max roation iops */

void fil_crypt_set_rotation_iops(uint val) {
  srv_n_fil_crypt_iops = val;
  os_event_set(fil_crypt_threads_event);
}

/*********************************************************************
Adjust encrypt tables
@param[in]	val		New setting for innodb-encrypt-tables */

bool fil_crypt_set_encrypt_tables(enum_default_table_encryption val,
                                  bool is_server_starting) {
  // It is always OK to set default_table_encryption on server
  // startup
  if (!is_server_starting && (srv_n_fil_crypt_threads_requested != 0)) {
    return false;
  }
  srv_default_table_encryption = val;
  os_event_set(fil_crypt_threads_event);
  return true;
}

/*********************************************************************
Init threads for key rotation */

void fil_crypt_threads_init() {
  if (!fil_crypt_threads_inited) {
    fil_crypt_event = os_event_create();
    fil_crypt_threads_event = os_event_create();
    mutex_create(LATCH_ID_FIL_CRYPT_THREADS_MUTEX, &fil_crypt_threads_mutex);
    mutex_create(LATCH_ID_FIL_CRYPT_THREADS_SET_CNT_MUTEX,
                 &fil_crypt_threads_set_cnt_mutex);
    mutex_create(LATCH_ID_FIL_CRYPT_LIST_MUTEX, &fil_crypt_list_mutex);

    uint cnt = srv_n_fil_crypt_threads_requested;
    srv_n_fil_crypt_threads_requested = 0;
    fil_crypt_threads_inited = true;
    fil_crypt_set_thread_cnt(cnt);
  }
}

/*********************************************************************
Clean up key rotation threads resources */

void fil_crypt_threads_cleanup() {
  if (!fil_crypt_threads_inited) {
    return;
  }
  ut_a(!srv_threads.m_crypt_threads_n);
  os_event_destroy(fil_crypt_event);
  os_event_destroy(fil_crypt_threads_event);
  mutex_free(&fil_crypt_threads_mutex);
  mutex_destroy(&fil_crypt_threads_set_cnt_mutex);
  mutex_destroy(&fil_crypt_list_mutex);
  fil_crypt_threads_inited = false;
}

/*********************************************************************
Wait for crypt threads to stop accessing space
@param[in]	space		Tablespace */

void fil_space_crypt_close_tablespace(const fil_space_t *space) {
  fil_space_crypt_t *crypt_data = space->crypt_data;

  if (!crypt_data) {
    return;
  }

  mutex_enter(&fil_crypt_threads_mutex);

  time_t start = time(0);
  time_t last = start;

  mutex_enter(&crypt_data->mutex);
  mutex_exit(&fil_crypt_threads_mutex);

  ulint cnt = crypt_data->rotate_state.active_threads;
  bool flushing = crypt_data->rotate_state.flushing;

  while (cnt > 0 || flushing) {
    mutex_exit(&crypt_data->mutex);
    /* release dict mutex so that scrub threads can release their
     * table references */
    // dict_mutex_exit_for_mysql();

    /* wakeup throttle (all) sleepers */
    os_event_set(fil_crypt_throttle_sleep_event);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    // dict_mutex_enter_for_mysql();

    mutex_enter(&crypt_data->mutex);
    cnt = crypt_data->rotate_state.active_threads;
    flushing = crypt_data->rotate_state.flushing;

    time_t now = time(0);

    if (now >= last + 30) {
      ib::warn() << "Waited " << now - start
                 << " seconds to drop space: " << space->name << " ("
                 << space->id << ") active threads " << cnt
                 << "flushing=" << flushing << ".";
      last = now;
    }
  }

  mutex_exit(&crypt_data->mutex);
}

/*********************************************************************
Get crypt status for a space (used by information_schema)
@param[in]	space		Tablespace
@param[out]	status		Crypt status */

void fil_space_crypt_get_status(const fil_space_t *space,
                                struct fil_space_crypt_status_t *status) {
  memset(status, 0, sizeof(*status));

  ut_ad(space->n_pending_ops > 0);

  /* If there is no crypt data and we have not yet read
  page 0 for this tablespace, we need to read it before
  we can continue. */
  if (!space->crypt_data) {
    fil_crypt_read_crypt_data(const_cast<fil_space_t *>(space));
  }

  status->space = UINT32_UNDEFINED;

  if (fil_space_crypt_t *crypt_data = space->crypt_data) {
    status->space = space->id;
    mutex_enter(&crypt_data->mutex);
    status->scheme = crypt_data->type;
    status->keyserver_requests = crypt_data->keyserver_requests;
    status->min_key_version = crypt_data->min_key_version;
    status->max_key_version = crypt_data->max_key_version;
    status->key_id = crypt_data->key_id;

    if (crypt_data->rotate_state.active_threads > 0 ||
        crypt_data->rotate_state.flushing) {
      status->rotating = true;
      status->flushing = crypt_data->rotate_state.flushing;
      status->rotate_next_page_number = crypt_data->rotate_state.next_offset;
      status->rotate_max_page_number = crypt_data->rotate_state.max_offset;
    }

    mutex_exit(&crypt_data->mutex);

    if (Encryption::is_online_encryption_on() ||
        crypt_data->min_key_version != ENCRYPTION_KEY_VERSION_NOT_ENCRYPTED) {
      status->current_key_version =
          fil_crypt_get_latest_key_version(crypt_data);
    }
  }
}

/*********************************************************************
 Get scrub status for a space (used by information_schema)

 @param[in]     space           Tablespace
 @param[out]    status          Scrub status */

void fil_space_get_scrub_status(const fil_space_t *space,
                                struct fil_space_scrub_status_t *status) {
  memset(status, 0, sizeof(*status));

  fil_space_crypt_t *crypt_data = space->crypt_data;

  status->space = space->id;

  if (crypt_data != NULL) {
    status->compressed = FSP_FLAGS_GET_ZIP_SSIZE(space->flags) > 0;
    mutex_enter(&crypt_data->mutex);
    status->last_scrub_completed =
        crypt_data->rotate_state.scrubbing.last_scrub_completed;
    if (crypt_data->rotate_state.active_threads > 0 &&
        crypt_data->rotate_state.scrubbing.is_active) {
      status->scrubbing = true;
      status->current_scrub_started = crypt_data->rotate_state.start_time;
      status->current_scrub_active_threads =
          crypt_data->rotate_state.active_threads;
      status->current_scrub_page_number = crypt_data->rotate_state.next_offset;
      status->current_scrub_max_page_number =
          crypt_data->rotate_state.max_offset;
    }

    mutex_exit(&crypt_data->mutex);
  }
}

/*********************************************************************
Return crypt statistics
@param[out]	stat		Crypt statistics */

void fil_crypt_total_stat(fil_crypt_stat_t *stat) {
  mutex_enter(&crypt_stat_mutex);
  *stat = crypt_stat;
  mutex_exit(&crypt_stat_mutex);
}

#endif /* UNIV_INNOCHECKSUM */

/******************************************************************
Calculate post encryption checksum
@param[in]	page_size	page size
@param[in]	dst_frame	Block where checksum is calculated
@return page checksum
not needed. */
uint32_t fil_crypt_calculate_checksum(const ulint page_size, const byte *page,
                                      const bool is_zip_compressed) {
  /* For encrypted tables we use only crc32 and strict_crc32 */
  if (is_zip_compressed) {
    page_size_t page_size_dummy(
        page_size, page_size,
        true);  // we do not care if those are correct values, as we
                // are using calc_zip_checksum directly
    BlockReporter reporter(false, page, page_size_dummy, false);
    return reporter.calc_zip_checksum(page, page_size,
                                      SRV_CHECKSUM_ALGORITHM_CRC32);
  } else {
    return buf_calc_page_crc32_encrypted_with_keyring(page, page_size);
  }
}

/**
Verify that post encryption checksum match calculated checksum.
This function should be called only if tablespace contains crypt_data
metadata (this is strong indication that tablespace is encrypted).
Function also verifies that traditional checksum does not match
calculated checksum as if it does page could be valid unencrypted,
encrypted, or corrupted.

@param[in,out]	page		page frame (checksum is temporarily modified)
@param[in]	page_size	page size
@param[in]	space		tablespace identifier
@return true if page is encrypted AND OK, false otherwise */
bool fil_space_verify_crypt_checksum(byte *page, ulint page_size,
                                     bool is_zip_compressed,
                                     bool is_new_schema_compressed) {
  if (is_new_schema_compressed) {
    page_size = static_cast<uint16_t>(
        mach_read_from_2(page + FIL_PAGE_COMPRESS_SIZE_V1));
  }

  /* Read stored post encryption checksum. */
  uint32_t checksum = 0;
  if (is_new_schema_compressed) {
    checksum = mach_read_from_4(page + FIL_PAGE_DATA);
    memset(page + FIL_PAGE_DATA, 0, 4);  // those bits were 0s before the
                                         // checksum was calcualted thus -- need
                                         // to calculate checksum with those
  } else if (!is_zip_compressed) {
    // page_size can be smaller than UNIV_PAGE_SIZE for row compressed tables
    checksum = mach_read_from_4(page + page_size - 4);
  } else if (is_zip_compressed) {
    checksum = mach_read_from_4(page + FIL_PAGE_LSN + 4);
  }

  uint32 cchecksum1, cchecksum2;

  /* Calculate checksums */
  if (is_zip_compressed) {
    page_size_t page_size_dummy(
        page_size, page_size,
        true);  // we do not care if those are correct values, as we
                // are using calc_zip_checksum directly
    BlockReporter reporter(false, page, page_size_dummy, false);

    cchecksum1 = reporter.calc_zip_checksum(page, page_size,
                                            SRV_CHECKSUM_ALGORITHM_CRC32);

    cchecksum2 = (cchecksum1 == checksum)
                     ? 0
                     : reporter.calc_zip_checksum(
                           page, page_size, SRV_CHECKSUM_ALGORITHM_INNODB);
  } else {
    cchecksum1 = buf_calc_page_crc32_encrypted_with_keyring(page, page_size);
    cchecksum2 =
        (cchecksum1 == checksum) ? 0 : buf_calc_page_new_checksum(page);
  }

  if (is_new_schema_compressed) mach_write_to_4(page + FIL_PAGE_DATA, checksum);

  bool encrypted = (checksum == cchecksum1 || checksum == cchecksum2 ||
                    checksum == BUF_NO_CHECKSUM_MAGIC);

  return (encrypted);
}

redo_log_key *redo_log_keys::load_latest_key(THD *thd, bool generate) {
  size_t klen = 0;
  char *key_type = nullptr;
  byte *rkey = nullptr;

  std::string key_name = get_key_name(server_uuid);

  if (innobase::encryption::read_key(key_name.c_str(), &rkey, &klen,
                                     &key_type) != 1 ||
      rkey == nullptr || strncmp(key_type, "AES", 4) != 0) {
    /* There is no key yet, we'll try to generate one */
    my_free(rkey);
    return generate ? generate_and_store_new_key(thd) : nullptr;
  }

  uint version = 0;
  byte *rkey2 = nullptr;
  size_t klen2 = 0;
  const bool err = (parse_system_key(rkey, klen, &version, &rkey2, &klen2) ==
                    reinterpret_cast<uchar *>(NullS));
  if (err) {
    my_free(rkey);
    my_free(rkey2);
    my_free(key_type);
    return nullptr;
  }

  ut_ad(klen2 == Encryption::KEY_LEN);

  auto it = m_keys.find(version);

  if (it != m_keys.end() && it->second.present) {
    ut_ad(memcmp(it->second.key, rkey2, Encryption::KEY_LEN) == 0);
    my_free(rkey);
    my_free(rkey2);
    my_free(key_type);
    return &it->second;
  }

  redo_log_key *rk = &m_keys[version];
  rk->version = version;
  rk->present = true;
  memcpy(rk->key, rkey2, Encryption::KEY_LEN);

  my_free(rkey);
  my_free(rkey2);
  my_free(key_type);

  return rk;
}

redo_log_key *redo_log_keys::load_key_version(THD *thd, const char *uuid,
                                              uint version) {
  auto it = m_keys.find(version);

  if (it != m_keys.end() && it->second.present) {
    return &it->second;
  }

  size_t klen = 0;
  char *key_type = nullptr;
  byte *rkey = nullptr;

  std::string redo_key_with_ver{get_key_name(
      version != REDO_LOG_ENCRYPT_NO_VERSION ? uuid : "", version)};
  if (innobase::encryption::read_key(redo_key_with_ver.c_str(), &rkey, &klen,
                                     &key_type) != 1 ||
      rkey == nullptr || strncmp(key_type, "AES", 4) != 0) {
    my_free(rkey);
    my_free(key_type);
    ib::error(ER_REDO_ENCRYPTION_CANT_LOAD_KEY_VERSION, version);
    if (thd) {
      ib_senderrf(thd, IB_LOG_LEVEL_WARN,
                  ER_DA_REDO_ENCRYPTION_CANT_LOAD_KEY_VERSION, version);
    }
    return nullptr;
  }

  ut_ad(klen == Encryption::KEY_LEN);

  redo_log_key *rk = &m_keys[version];
  rk->version = version;
  rk->present = true;
  memcpy(rk->key, rkey, Encryption::KEY_LEN);

  my_free(rkey);
  my_free(key_type);

  return rk;
}

std::string redo_log_keys::get_key_name(const char *uuid, uint key_version) {
  std::ostringstream oss;
  get_key_name(oss, uuid);
  oss << ":" << key_version;
  return oss.str();
}

std::string redo_log_keys::get_key_name(const char *uuid) {
  std::ostringstream oss;
  get_key_name(oss, uuid);
  return oss.str();
}

void redo_log_keys::get_key_name(std::ostringstream &oss, const char *uuid) {
  oss << PERCONA_REDO_KEY_NAME;
  if (strlen(uuid) > 0) oss << '-' << uuid;
}

redo_log_key *redo_log_keys::generate_and_store_new_key(THD *thd) {
  std::string key_name = get_key_name(server_uuid);

  if (!innobase::encryption::generate_key(key_name.c_str(), "AES",
                                          Encryption::KEY_LEN)) {
    ib::error(ER_REDO_ENCRYPTION_CANT_GENERATE_KEY);
    if (thd) {
      ib_senderrf(thd, IB_LOG_LEVEL_WARN,
                  ER_DA_REDO_ENCRYPTION_CANT_GENERATE_KEY);
    }
    return nullptr;
  }

  char *redo_key_type = nullptr;
  byte *rkey = nullptr;
  size_t klen = 0;

  if (innobase::encryption::read_key(key_name.c_str(), &rkey, &klen,
                                     &redo_key_type) != 1) {
    ib::error(ER_REDO_ENCRYPTION_CANT_FETCH_KEY);
    if (thd) {
      ib_senderrf(thd, IB_LOG_LEVEL_WARN, ER_DA_REDO_ENCRYPTION_CANT_FETCH_KEY);
    }
    my_free(redo_key_type);
    my_free(rkey);
    return nullptr;
  }

  ut_ad(rkey != nullptr);
  byte *rkey2 = nullptr;
  size_t klen2 = 0;
  uint version = 0;

  bool err = (parse_system_key(rkey, klen, &version, &rkey2, &klen2) ==
              reinterpret_cast<uchar *>(NullS));

  ut_ad(klen2 == Encryption::KEY_LEN);

  if (err) {
    ib::error(ER_REDO_ENCRYPTION_CANT_PARSE_KEY, rkey);
    if (thd != nullptr) {
      ib_senderrf(thd, IB_LOG_LEVEL_WARN, ER_DA_REDO_ENCRYPTION_CANT_PARSE_KEY,
                  rkey);
    }
    my_free(redo_key_type);
    my_free(rkey);
    return nullptr;
  }

  redo_log_key *rk = &m_keys[version];
  rk->version = version;
  memcpy(rk->key, rkey2, Encryption::KEY_LEN);
  rk->present = true;

  my_free(redo_key_type);
  my_free(rkey);
  my_free(rkey2);

  return rk;
}

redo_log_key *redo_log_keys::fetch_or_generate_default_key(THD *thd) {
  ut_ad(m_keys.empty());
  std::string default_key_name{get_key_name("", 0)};
  ut_ad(strlen(server_uuid) != 0);
  ut_ad(default_key_name.length() == strlen("percona_redo:0") &&
        memcmp(default_key_name.c_str(), "percona_redo:0",
               default_key_name.length()) == 0);

  char *default_redo_key_type = nullptr;
  byte *default_rkey = nullptr;
  size_t default_klen = 0;

  auto ret =
      innobase::encryption::read_key(default_key_name.c_str(), &default_rkey,
                                     &default_klen, &default_redo_key_type);

  if (ret == -1) {
    ib::error(ER_REDO_ENCRYPTION_CANT_FETCH_DEFAULT_KEY);
    if (thd != nullptr) {
      ib_senderrf(thd, IB_LOG_LEVEL_WARN,
                  ER_REDO_ENCRYPTION_CANT_FETCH_DEFAULT_KEY);
    }
    my_free(default_redo_key_type);
    my_free(default_rkey);
    return nullptr;
  }

  if (default_rkey != nullptr) {
    redo_log_key *rk = &m_keys[0];
    rk->version = 0;
    memcpy(rk->key, default_rkey, Encryption::KEY_LEN);
    rk->present = true;
    my_free(default_redo_key_type);
    my_free(default_rkey);
    return rk;
  }

  // we use store instead of generate because we want to store system key
  // with illegal version - percona_redo:0.
  Encryption::random_value(reinterpret_cast<byte *>(&m_keys[0].key));

  if (!innobase::encryption::store_key(default_key_name.c_str(),
                                       reinterpret_cast<byte *>(m_keys[0].key),
                                       Encryption::KEY_LEN, "AES")) {
    return nullptr;
  }

  redo_log_key *rk = &m_keys[0];
  rk->version = 0;
  rk->present = true;
  return rk;
}

redo_log_keys redo_log_key_mgr;
