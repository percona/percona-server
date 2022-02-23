#pragma once

#include <string>

namespace myrocks {

class MasterKeyManager {
 public:
  MasterKeyManager(){};
  virtual ~MasterKeyManager(){};

  virtual int GetMostRecentMasterKey(std::string *masterKey,
                                     uint32_t *masterKeyId) = 0;
  virtual int GetMasterKey(uint32_t masterKeyId, const std::string &suuid,
                           std::string *masterKey) = 0;
  virtual void GetServerUuid(std::string *serverUuid) = 0;
  virtual int GenerateNewMasterKey() = 0;
};

}  // namespace myrocks