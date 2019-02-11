#include "event_crypt.h"

#include "binlog.h"
#include "my_byteorder.h"

static bool encrypt_event(uint32 offs, int flags,
                          const Binlog_crypt_data &crypto, uchar *buf,
                          uchar *ebuf, size_t buf_len) {
  DBUG_ASSERT(crypto.is_enabled());
  DBUG_ASSERT(crypto.get_key() != nullptr);

  size_t elen;
  uchar iv[binary_log::Start_encryption_event::IV_LENGTH];

  crypto.set_iv(iv, offs);
  memcpy(buf + EVENT_LEN_OFFSET, buf, 4);

  if (my_aes_crypt(my_aes_mode::CBC, flags | ENCRYPTION_FLAG_NOPAD, buf + 4,
                   buf_len - 4, ebuf + 4, &elen, crypto.get_key(),
                   crypto.get_keys_length(), iv, sizeof(iv))) {
    memcpy(buf, buf + EVENT_LEN_OFFSET, 4);
    return true;
  }
  DBUG_ASSERT(elen == buf_len - 4);

  memcpy(ebuf, ebuf + EVENT_LEN_OFFSET, 4);
  int4store(ebuf + EVENT_LEN_OFFSET, buf_len);

  return false;
}

bool encrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar *buf,
                   uchar *ebuf, size_t buf_len) {
  return encrypt_event(offs, ENCRYPTION_FLAG_ENCRYPT, crypto, buf, ebuf,
                       buf_len);
}

bool decrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar *buf,
                   uchar *ebuf, size_t buf_len) {
  return encrypt_event(offs, ENCRYPTION_FLAG_DECRYPT, crypto, buf, ebuf,
                       buf_len);
}
