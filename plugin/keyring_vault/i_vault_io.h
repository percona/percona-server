#ifndef I_VAULTIO_INCLUDED
#define I_VAULTIO_INCLUDED

#include <my_global.h>
#include "i_keyring_io.h"

namespace keyring {

class IVault_io : public IKeyring_io
{
public:
  virtual my_bool retrieve_key_type_and_data(IKey *key) = 0;
  
  virtual ~IVault_io() {}
};

} // namespace keyring

#endif // I_VAULTIO_INCLUDED
