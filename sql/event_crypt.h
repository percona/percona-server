#ifndef EVENT_ENCRYPTER_H
#define EVENT_ENCRYPTER_H

#include "my_global.h"
#include "my_crypt.h"
#include "rpl_constants.h"
#include "binlog_event.h"
#include "binlog_crypt_data.h"

bool encrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar* buf, uchar *ebuf, size_t buf_len);
bool decrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar* buf, uchar *ebuf, size_t buf_len);

class Event_encrypter
{
public:
  Event_encrypter()
    : event_len(0)
    , ctx(NULL) 
    , crypto(NULL)
  {}

  ~Event_encrypter()
  {
    if (ctx != NULL)
      delete ctx;
  }

  bool init(IO_CACHE *output_cache, uchar* &header, size_t &buf_len);
  bool encrypt_and_write(IO_CACHE *output_cache, const uchar *pos, size_t len);
  bool finish(IO_CACHE *output_cache);

  void enable_encryption(Binlog_crypt_data* crypto)
  {
    DBUG_ASSERT(crypto != NULL);
    this->crypto = crypto;
  }

  bool is_encryption_enabled()
  {
    return crypto != NULL;
  }

private:
  bool maybe_write_event_len(IO_CACHE *output_cache, uchar *pos, size_t len);
  uint event_len;

  MyEncryptionCTX *ctx;
  /**
     Encryption data (key, nonce). Only used if ctx != 0.
  */
  Binlog_crypt_data *crypto;
};

#endif //EVENT_ENCRYPTER_H
