#ifndef EVENT_ENCRYPTER_H
#define EVENT_ENCRYPTER_H

#include "binlog_crypt_data.h"
#include "my_crypt.h"

// Decrypt Percona binlog encryption encrypted-event
bool decrypt_event(uint32 offs, const Binlog_crypt_data &crypto, uchar *buf,
                   uchar *ebuf, size_t buf_len);

#endif  // EVENT_ENCRYPTER_H
