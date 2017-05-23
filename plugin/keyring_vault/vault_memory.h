#ifndef MYSQL_VAULT_MEMORY
#define MYSQL_VAULT_MEMORY

#include <my_global.h>
#include <string.h>
#include "keyring_memory.h"

namespace keyring
{
#if !defined(HAVE_MEMSET_S)
  inline void memset_s(void *dest, size_t dest_max, int c, size_t n)
  {
#if defined(WIN32)
    SecureZeroMemory(dest, n);
#else
    volatile unsigned char *p = reinterpret_cast<unsigned char*>(dest);
    while (dest_max-- && n--) {
      *p++ = c;
    }
#endif
  } 
#endif

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
      if (n == 0 || n > INT_MAX)
        return NULL;
      return keyring_malloc<T*>(n*sizeof(T)); 
    }

    void deallocate(T *p, size_t n)
    {
      memset_s(p, n, 0, n);
      my_free(p);
    }
  };

  typedef std::basic_string<char, std::char_traits<char>, Secure_allocator<char> > Secure_string;
} // namespace keyring

#endif // MYSQL_VAULT_MEMORY
