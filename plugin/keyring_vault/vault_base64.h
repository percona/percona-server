#ifndef MYSQL_VAULT_BASE64_H
#define MYSQL_VAULT_BASE64_H

#include <my_global.h>
#include "vault_memory.h"

namespace keyring
{
  class Vault_base64
  {
  public :
    enum Base64Format
    {
      SINGLE_LINE,
      MULTI_LINE
    };
    static bool encode(const void *src, size_t src_len, Secure_string *encoded, Base64Format format);
    static bool decode(const Secure_string &src, char **dst, uint64 *dst_length);
    static bool decode(const Secure_string &src, Secure_string *dst);
  };
}

#endif // MYSQL_VAULT_BASE64_H
