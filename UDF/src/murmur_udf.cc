/* License: This code is in the public domain.
 *
 * See http://murmurhash.googlepages.com/ for more about the Murmur hash.  The
 * Murmur hash is by Austin Appleby.
 *
 * This file implements a 64-bit Murmur-2 hash UDF (user-defined function) for
 * MySQL.  The function accepts any number of arguments and returns a 64-bit
 * unsigned integer.  MySQL actually interprets the result as a signed integer,
 * but you should ignore that.  I chose not to return the number as a
 * hexadecimal string because using an integer makes it possible to use it
 * efficiently with BIT_XOR().
 *
 * The function never returns NULL, even when you give it NULL arguments.
 *
 * To compile and install, execute the following commands.  The function name
 * murmur_hash in the mysql command is case-sensitive!  (Of course, when you
 * actually call the function, it is case-insensitive just like any other SQL
 * function).
 *
 * g++ -fPIC -Wall -I/usr/include/mysql -shared -o murmur_udf.so murmur_udf.cc
 * cp murmur_udf.so /lib
 * mysql mysql -e "CREATE FUNCTION murmur_hash RETURNS INTEGER SONAME 'murmur_udf.so'"
 *
 * Once installed successfully, you should be able to call the function.  Here's
 * a faster alternative to MD5 hashing, with the added ability to hash multiple
 * arguments in a single call:
 *
 * mysql> SELECT MURMUR_HASH('hello', 'world');
 *
 * Here's a way to reduce an entire table to a single order-independent hash:
 *
 * mysql> SELECT BIT_XOR(CAST(MURMUR_HASH(col1, col2, col3) AS UNSIGNED)) FROM tbl1;
 *
 * Note - This code makes a few assumptions about how your machine behaves -
 * 1. We can read a 4-byte value from any address without crashing
 * 2. sizeof(int) == 4
 *
 * And it has a few limitations:
 * 1. It will not work incrementally.
 * 2. It will not produce the same results on little-endian and big-endian machines.
 */

#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>
#include <ctype.h>
#include <string.h>

/* On the first call, use this as the initial_value. */
#define HASH_64_INIT 0x84222325cbf29ce4ULL
/* Default for NULLs, just so the result is never NULL. */
#define HASH_NULL_DEFAULT 0x0a0b0c0d

/* Prototypes */

extern "C" {
   ulonglong MurmurHash2( const void *key, int len, unsigned int seed );
   my_bool murmur_hash_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
   ulonglong murmur_hash( UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error );
}

/* Implementations */

/*
 * MurmurHash2, by Austin Appleby
 * This is the 64 bit version of MurmurHash2 for 32-bit platforms.
 */

ulonglong
MurmurHash2( const void *key, int len, unsigned int seed ) {

   const unsigned int m = 0x5bd1e995;
   const int r = 24;

   unsigned int h1 = seed ^ len;
   unsigned int h2 = 0;

   const unsigned int *data = (const unsigned int *)key;

   while (len >= 8) {
      unsigned int k1 = *data++;
      k1 *= m; k1 ^= k1 >> r; k1 *= m;
      h1 *= m; h1 ^= k1;
      len -= 4;

      unsigned int k2 = *data++;
      k2 *= m; k2 ^= k2 >> r; k2 *= m;
      h2 *= m; h2 ^= k2;
      len -= 4;
   }

   if (len >= 4) {
      unsigned int k1 = *data++;
      k1 *= m; k1 ^= k1 >> r; k1 *= m;
      h1 *= m; h1 ^= k1;
      len -= 4;
   }

   switch (len) {
      case 3: h2 ^= ((unsigned char*)data)[2] << 16;
      case 2: h2 ^= ((unsigned char*)data)[1] << 8;
      case 1: h2 ^= ((unsigned char*)data)[0];
              h2 *= m;
   };

   h1 ^= h2 >> 18; h1 *= m;
   h2 ^= h1 >> 22; h2 *= m;
   h1 ^= h2 >> 17; h1 *= m;

   ulonglong h = h1;

   h = (h << 32) | h2;
   return h;
}

my_bool 
murmur_hash_init( UDF_INIT* initid, UDF_ARGS* args, char* message) {
   if (args->arg_count == 0 ) {
      strcpy(message,"MURMUR_HASH requires at least one argument");
      return 1;
   }
   initid->maybe_null = 0;      /* The result will never be NULL */
   return 0;
}

ulonglong 
murmur_hash( UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error ) {

   uint null_default = HASH_NULL_DEFAULT;
   ulonglong result  = HASH_64_INIT;
   uint i;

   for (i = 0 ; i < args->arg_count; ++i ) {
      if ( args->args[i] != NULL ) {
         switch ( args->arg_type[i] ) {
         case STRING_RESULT:
         case DECIMAL_RESULT:
            result
               = MurmurHash2((const void*) args->args[i], args->lengths[i], result);
            break;
         case REAL_RESULT:
            {
               double real_val;
               real_val = *((double*) args->args[i]);
               result
                  = MurmurHash2((const void*)&real_val, sizeof(double), result);
            }
            break;
         case INT_RESULT:
            {
               long long int_val;
               int_val = *((long long*) args->args[i]);
               result = MurmurHash2((const void*)&int_val, sizeof(ulonglong), result);
            }
            break;
         default:
            break;
         }
      }
      else {
         result
            = MurmurHash2((const void*)&null_default, sizeof(null_default), result);
      }
   }
   return result;
}
