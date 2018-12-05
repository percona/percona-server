#ifndef EVENT_ENCRYPTER_H
#define EVENT_ENCRYPTER_H

#include "basic_ostream.h"
#include "binlog_crypt_data.h"
#include "binlog_event.h"
#include "my_crypt.h"
#include "rpl_constants.h"

bool encrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar *buf,
                   uchar *ebuf, size_t buf_len);
bool decrypt_event(const Binlog_crypt_data &crypto, uchar *buf, uchar *ebuf,
                   size_t buf_len);

class Event_encrypter final {
 public:
  Event_encrypter() noexcept : event_len(0), ctx(nullptr), crypto(nullptr) {}

  ~Event_encrypter() {
    if (ctx != nullptr) {
      my_aes_crypt_free_ctx(ctx);
      ctx = nullptr;
    }
  }

  bool init(Basic_ostream *ostream, uchar *header, size_t buf_len);
  bool encrypt_and_write(Basic_ostream *ostream, const uchar *pos, size_t len);
  bool finish(Basic_ostream *ostream);

  void enable_encryption(Binlog_crypt_data *crypto) noexcept {
    DBUG_ASSERT(crypto != nullptr);
    this->crypto = crypto;
  }

  bool is_encryption_enabled() const noexcept { return crypto != nullptr; }

 private:
  bool maybe_write_event_len(Basic_ostream *ostream, uchar *pos, size_t len);
  uint event_len;

  MyEncryptionCTX *ctx;
  /**
     Encryption data (key, nonce). Only used if ctx != 0.
  */
  Binlog_crypt_data *crypto;

  Event_encrypter(const Event_encrypter &);
  Event_encrypter &operator=(const Event_encrypter &);
};

#endif  // EVENT_ENCRYPTER_H
