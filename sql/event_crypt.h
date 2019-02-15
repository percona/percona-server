#ifndef EVENT_ENCRYPTER_H
#define EVENT_ENCRYPTER_H

#include "basic_ostream.h"
#include "binlog_crypt_data.h"
#include "binlog_event.h"
#include "my_crypt.h"
#include "rpl_constants.h"

bool encrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar *buf,
                   uchar *ebuf, size_t buf_len);
bool decrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar *buf,
                   uchar *ebuf, size_t buf_len);

#endif  // EVENT_ENCRYPTER_H
