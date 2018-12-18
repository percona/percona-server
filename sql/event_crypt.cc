#include "event_crypt.h"

#include "my_byteorder.h"
#include "sql/binlog_ostream.h"

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

bool Event_encrypter::init(Basic_ostream *ostream, uchar *&header,
                           size_t &buf_len) {
  uchar iv[binary_log::Start_encryption_event::IV_LENGTH];
  crypto->set_iv(iv, ostream->position());
  if (ctx != nullptr) {
    my_aes_crypt_free_ctx(ctx);
    ctx = nullptr;
  }
  if (my_aes_crypt_init(ctx, my_aes_mode::CBC,
                        ENCRYPTION_FLAG_ENCRYPT | ENCRYPTION_FLAG_NOPAD,
                        crypto->get_key(), crypto->get_keys_length(), iv,
                        sizeof(iv)))
    return true;

  DBUG_ASSERT(buf_len >= LOG_EVENT_HEADER_LEN);
  event_len = uint4korr(header + EVENT_LEN_OFFSET);
  DBUG_ASSERT(event_len >= buf_len);
  memcpy(header + EVENT_LEN_OFFSET, header, 4);

  header += 4;   // We moved first 4 bytes in place of event size
  buf_len -= 4;  // We will add event size in its proper offset later on

  return false;
}

bool Event_encrypter::maybe_write_event_len(Basic_ostream *ostream, uchar *pos,
                                            size_t len) {
  if (len && event_len) {
    DBUG_ASSERT(len >= EVENT_LEN_OFFSET);
    if (ostream->write(pos + EVENT_LEN_OFFSET - 4, 4)) return true;
    int4store(pos + EVENT_LEN_OFFSET - 4, event_len);
    event_len = 0;
  }
  return false;
}

bool Event_encrypter::encrypt_and_write(Basic_ostream *ostream,
                                        const uchar *pos, size_t len) {
  DBUG_ASSERT(ostream != nullptr);

  uchar *dst = NULL;
  size_t dstsize = 0;

  if (crypto) {
    dstsize = my_aes_crypt_get_size(my_aes_mode::ECB, len);
    if (!(dst = reinterpret_cast<uchar *>(my_safe_alloca(dstsize, 512))))
      return true;

    uint dstlen;
    if (my_aes_crypt_update(ctx, pos, len, dst, (size_t *)&dstlen)) goto err;

    if (maybe_write_event_len(ostream, dst, dstlen)) return true;
    pos = dst;
    len = dstlen;
  } else {
    dst = 0;
  }

  if (ostream->write(pos, len)) goto err;

  my_safe_afree(dst, dstsize, 512);
  return false;
err:
  my_safe_afree(dst, dstsize, 512);
  return true;
}

bool Event_encrypter::finish(Basic_ostream *ostream) {
  DBUG_ASSERT(ostream != nullptr && ctx != nullptr);

  size_t dstlen;
  uchar dst[MY_AES_BLOCK_SIZE * 2];
  if (my_aes_crypt_finish(ctx, dst, &dstlen) ||
      maybe_write_event_len(ostream, dst, dstlen) ||
      ostream->write(dst, dstlen))
    return true;
  return false;
}
