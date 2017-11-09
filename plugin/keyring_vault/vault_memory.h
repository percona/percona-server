#ifndef MYSQL_VAULT_MEMORY
#define MYSQL_VAULT_MEMORY

#include <my_global.h>
#include "keyring_memory.h"

namespace keyring
{
  template <class T> class Secure_allocator : public std::allocator<T>
  {
  public:

    template<class U> struct rebind { typedef Secure_allocator<U> other; };
    Secure_allocator() throw() {}
    Secure_allocator(const Secure_allocator& secure_allocator) : std::allocator<T>(secure_allocator)
    {}
    template <class U> Secure_allocator(const Secure_allocator<U>&) throw() {}

    T* allocate(size_t n)
    {
      if (n == 0)
        return NULL;
      else if (n > INT_MAX)
        throw std::bad_alloc();
      return keyring_malloc<T*>(n*sizeof(T)); 
    }

    void deallocate(T *p, size_t n)
    {
      memset_s(p, n, 0, n);
      my_free(p);
    }
  };
} // namespace keyring

#endif // MYSQL_VAULT_MEMORY
