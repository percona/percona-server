#include <algorithm>
#include "vault_base64.h"
#include "base64.h"

namespace keyring
{
  bool Vault_base64::encode(const void *src, size_t src_len, Secure_string *encoded, Base64Format format)
  {
    uint64 memory_needed = base64_needed_encoded_length(src_len);
    char *base64_encoded_text = new char[memory_needed];
    if (::base64_encode(src, src_len, base64_encoded_text) != 0)
    {
      memset_s(base64_encoded_text, 0, memory_needed, memory_needed);
      delete[] base64_encoded_text;
      return true;
    }
    if (format == SINGLE_LINE)
    {
      char* new_end = std::remove(base64_encoded_text, base64_encoded_text + memory_needed, '\n');
      memory_needed = new_end - base64_encoded_text;
    }
    encoded->assign(base64_encoded_text, memory_needed-1); // base64 encode returns data with NULL terminating string - which we do not care about
    memset_s(base64_encoded_text, 0, memory_needed, memory_needed);
    delete[] base64_encoded_text;

    return false;
  }

  bool Vault_base64::decode(const Secure_string &src, Secure_string *dst)
  {
    char *data;
    uint64 data_length;
    if (decode(src, &data, &data_length))
      return true;    
    dst->assign(data, data_length);
    memset_s(data, 0, data_length, data_length);
    delete[] data;
    return false;
  }

  bool Vault_base64::decode(const Secure_string &src, char **dst, uint64 *dst_length)
  {
    uint64 base64_length_of_memory_needed_for_decode = base64_needed_decoded_length(src.length());
    char *data = new char[base64_length_of_memory_needed_for_decode];

    int64 decoded_length = ::base64_decode(src.c_str(), src.length(), data, NULL, 0);
    if (decoded_length <= 0)
    { 
      memset_s(data, 0, base64_length_of_memory_needed_for_decode, base64_length_of_memory_needed_for_decode);
      delete[] data;
      return true;
    }
    *dst = data;
    *dst_length = decoded_length;

    return false;
  }
}
