#ifndef MYSQL_VAULT_KEYS_H
#define MYSQL_VAULT_KEYS_H

#include <boost/core/noncopyable.hpp>
#include <list>
#include "plugin/keyring/common/i_serialized_object.h"

namespace keyring {

class Vault_keys_list final : public ISerialized_object,
                              private boost::noncopyable {
 public:
  bool get_next_key(IKey **key) override;
  bool has_next_key() override;
  void push_back(IKey *key);
  size_t size() const;

  ~Vault_keys_list() override;

 private:
  typedef std::list<IKey *> Keys_list;
  Keys_list keys;
};

}  // namespace keyring

#endif  // MYSQL_VAULT_KEYS_H
