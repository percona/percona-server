#include <my_global.h>
#include "event_crypt.h"

static bool encrypt_event(uint32 offs, int flags, const Binlog_crypt_data &crypto, uchar* buf, uchar *ebuf, size_t buf_len) 
{
  DBUG_ASSERT(crypto.is_enabled());
  DBUG_ASSERT(crypto.get_key() != NULL);

  size_t elen;
  uchar iv[Binlog_crypt_data::BINLOG_IV_LENGTH];

  crypto.set_iv(iv, offs);
  memcpy(buf + EVENT_LEN_OFFSET, buf, 4);

  if (my_aes_crypt(MY_AES_CBC, flags | ENCRYPTION_FLAG_NOPAD,
                   buf + 4, buf_len - 4, ebuf + 4, &elen,
                   crypto.get_key(), crypto.get_keys_length(), iv, sizeof(iv)))
  {
    memcpy(buf, buf + EVENT_LEN_OFFSET, 4);
    return true;
  }
  DBUG_ASSERT(elen == buf_len - 4);

  memcpy(ebuf, ebuf + EVENT_LEN_OFFSET, 4);
  int4store(ebuf + EVENT_LEN_OFFSET, buf_len);

  return false;
}

bool encrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar* buf, uchar *ebuf, size_t buf_len) 
{
  return encrypt_event(offs, ENCRYPTION_FLAG_ENCRYPT, crypto, buf, ebuf, buf_len);
}

bool decrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar* buf, uchar *ebuf, size_t buf_len) 
{
  return encrypt_event(offs, ENCRYPTION_FLAG_DECRYPT, crypto, buf, ebuf, buf_len);
}


bool Event_encrypter::init(IO_CACHE *output_cache, uchar* &header, size_t &buf_len)
{
  uchar iv[Binlog_crypt_data::BINLOG_IV_LENGTH];
  crypto->set_iv(iv, my_b_safe_tell(output_cache));
  if (ctx != NULL)
  {
      my_aes_crypt_free_ctx(ctx);
      ctx = NULL;
  }
  if (my_aes_crypt_init(ctx, MY_AES_CBC, ENCRYPTION_FLAG_ENCRYPT | ENCRYPTION_FLAG_NOPAD,
                        crypto->get_key(), crypto->get_keys_length(), iv, sizeof(iv)))
    return true;

  DBUG_ASSERT(buf_len >= LOG_EVENT_HEADER_LEN);
  event_len = uint4korr(header + EVENT_LEN_OFFSET);
  DBUG_ASSERT(event_len >= buf_len);
  memcpy(header + EVENT_LEN_OFFSET, header, 4);
  header += 4;
  buf_len -= 4;

  return false;
}

bool Event_encrypter::maybe_write_event_len(IO_CACHE *output_cache, uchar *pos, size_t len)
{
  if (len && event_len)
  {
    DBUG_ASSERT(len >= EVENT_LEN_OFFSET);
    if (my_b_safe_write(output_cache, pos + EVENT_LEN_OFFSET - 4, 4)) 
      return true;
    int4store(pos + EVENT_LEN_OFFSET - 4, event_len);
    event_len = 0;
  }
  return false;
}

bool Event_encrypter::encrypt_and_write(IO_CACHE *output_cache, const uchar *pos, size_t len)
{
  DBUG_ASSERT(output_cache != NULL);

  uchar *dst = NULL;
  size_t dstsize = 0;

  if (crypto)
  {
    dstsize = my_aes_crypt_get_size(MY_AES_ECB, len);
    if (!(dst = reinterpret_cast<uchar*>(my_safe_alloca(dstsize, 512))))
      return true;

    uint dstlen;
    if (my_aes_crypt_update(ctx, pos, len, dst, (size_t*)&dstlen))
      goto err;

    if (maybe_write_event_len(output_cache, dst, dstlen))
      return true;
    pos = dst;
    len = dstlen;
  }
  else
  {
    dst = 0;
  }

  if (my_b_safe_write(output_cache, pos, len))
    goto err;

  my_safe_afree(dst, dstsize, 512);
  return false;
err:
  my_safe_afree(dst, dstsize, 512);
  return true;
}

bool Event_encrypter::finish(IO_CACHE *output_cache)
{
  DBUG_ASSERT(output_cache != NULL);
  DBUG_ASSERT(ctx != NULL);

  size_t dstlen;
  uchar dst[MY_AES_BLOCK_SIZE*2];
  if (my_aes_crypt_finish(ctx, dst, &dstlen) ||
      maybe_write_event_len(output_cache, dst, dstlen) ||
      my_b_safe_write(output_cache, dst, dstlen))
    return true;
  return false;
}
