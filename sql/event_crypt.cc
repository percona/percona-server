#include "event_crypt.h"

#include "binlog.h"
#include "my_aes.h"
#include "my_byteorder.h"

bool decrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar *buf,
                   uchar *ebuf, size_t buf_len) {
  assert(crypto.is_enabled());
  assert(crypto.get_key() != nullptr);

  uchar iv[binary_log::Start_encryption_event::IV_LENGTH];

  crypto.set_iv(iv, offs);
  memcpy(buf + EVENT_LEN_OFFSET, buf, 4);

  if (my_legacy_aes_cbc_nopad_decrypt(
          buf + 4, buf_len - 4, ebuf + 4, crypto.get_key(),
          crypto.get_keys_length(), iv) != static_cast<int>(buf_len - 4)) {
    memcpy(buf, buf + EVENT_LEN_OFFSET, 4);
    return true;
  }

  memcpy(ebuf, ebuf + EVENT_LEN_OFFSET, 4);
  int4store(ebuf + EVENT_LEN_OFFSET, buf_len);

  return false;
}
