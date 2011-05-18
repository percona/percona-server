/* This file implements a 64-bit FNV-1a hash UDF (user-defined function) for
 * MySQL.  The function accepts any number of arguments and returns a 64-bit
 * unsigned integer.  MySQL actually interprets the result as a signed integer,
 * but you should ignore that.  I chose not to return the number as a
 * hexadecimal string because using an integer makes it possible to use it
 * efficiently with BIT_XOR().
 *
 * The function never returns NULL, even when you give it NULL arguments.
 *
 * To compile and install, execute the following commands.  The function name
 * fnv1a_64 in the mysql command is case-sensitive!  (Of course, when you
 * actually call the function, it is case-insensitive just like any other SQL
 * function).
 *
 * gcc -fPIC -Wall -I/usr/include/mysql -shared -o fnv1a_udf.so fnv1a_udf.cc
 * cp fnv1a_udf.so /lib * OR: * cp fnv1a_udf.so /usr/lib
 * mysql mysql -e "CREATE FUNCTION fnv1a_64 RETURNS INTEGER SONAME 'fnv1a_udf.so'"
 *
 * For MySQL version 4.1 or older you must add the following flag to the gcc
 * command above: -DNO_DECIMAL_RESULT
 * Otherwise you will get an error like:
 *   fnv1a_udf.cc:167: `DECIMAL_RESULT' undeclared (first use this function)
 * (See http://code.google.com/p/maatkit/issues/detail?id=89)
 * 
 * If you get the error "ERROR 1126 (HY000): Can't open shared library
 * 'fnv1a_udf.so' (errno: 22 fnv1a_udf.so: cannot open shared object file: No
 * such file or directory)" then you may need to copy the .so file to another
 * location in your system.  Look at your environment's $LD_LIBRARY_PATH
 * variable for clues.  If none is set, you may need to set this variable to
 * something like /lib.
 *
 * If you get the error "ERROR 1126 (HY000): Can't open shared library
 * 'libfnv1a_udf.so' (errno: 22 /lib/libfnv1a_udf.so: undefined symbol: 
 * __gxx_personality_v0)" then you may need to use g++ instead of gcc.
 *
 * Try both /lib and /usr/lib before changing LD_LIBRARY_PATH.
 *
 * On Mac OSX, use -dynamiclib instead of -shared and add -lstdc++ to the
 * compile flags.
 *
 * Once installed successfully, you should be able to call the function.  Here's
 * a faster alternative to MD5 hashing, with the added ability to hash multiple
 * arguments in a single call:
 *
 * mysql> SELECT FNV1A_64('hello', 'world');
 *
 * Here's a way to reduce an entire table to a single order-independent hash:
 *
 * mysql> SELECT BIT_XOR(CAST(FNV1A_64(col1, col2, col3) AS UNSIGNED)) FROM tbl;
 *
 */

/* The following header is from hash_64a.c:
 *
 * hash_64 - 64 bit Fowler/Noll/Vo-0 FNV-1a hash code
 *
 * @(#) $Revision: 5.1 $
 * @(#) $Id: hash_64a.c,v 5.1 2009/06/30 09:01:38 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/hash_64a.c,v $
 *
 ***
 *
 * Fowler/Noll/Vo hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 *
 ***
 *
 * To use the recommended 64 bit FNV-1a hash, pass FNV1A_64_INIT as the
 * Fnv64_t hashval argument to fnv_64a_buf() or fnv_64a_str().
 *
 ***
 *
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *	chongo <Landon Curt Noll> /\oo/\
 *      http://www.isthe.com/chongo/
 *
 * Share and Enjoy!	:-)
 */

#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>
#include <ctype.h>
#include <string.h>

/* On the first call, use this as the initial_value. */
#define FNV1A_64_INIT 0xcbf29ce484222325ULL
/* Default for NULLs, just so the result is never NULL. */
#define HASH_NULL_DEFAULT 0x0a0b0c0d
/* Magic number for the hashing. */
#define FNV_64_PRIME 0x100000001b3ULL

/* Prototypes */

extern "C" {
   ulonglong hash64a(const void *buf, size_t len, ulonglong hval);
   my_bool fnv1a_64_init(UDF_INIT* initid, UDF_ARGS* args, char* message);
   ulonglong fnv1a_64(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error );
}

/* Implementations */

ulonglong hash64a(const void *buf, size_t len, ulonglong hval) {
   const unsigned char *bp = (const unsigned char*)buf;
   const unsigned char *be = bp + len;

   /* FNV-1a hash each octet of the buffer */
   for (; bp != be; ++bp) {
      /* xor the bottom with the current octet */
      hval ^= (ulonglong)*bp;
      /* multiply by the 64 bit FNV magic prime mod 2^64 */
      hval *= FNV_64_PRIME;
   }

   return hval;
}

my_bool
fnv1a_64_init(UDF_INIT* initid, UDF_ARGS* args, char* message) {
   if (args->arg_count == 0 ) {
      strcpy(message,"FNV1A_64 requires at least one argument");
      return 1;
   }
   initid->maybe_null = 0;      /* The result will never be NULL */
   return 0;
}

ulonglong
fnv1a_64(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {

   uint null_default = HASH_NULL_DEFAULT;
   ulonglong result  = FNV1A_64_INIT;
   uint i;

   for (i = 0 ; i < args->arg_count; ++i ) {
      if ( args->args[i] != NULL ) {
         switch ( args->arg_type[i] ) {
         case STRING_RESULT:
         #ifdef NO_DECIMAL_RESULT
         #else
         case DECIMAL_RESULT:
         #endif
            result
               = hash64a((const void*) args->args[i], args->lengths[i], result);
            break;
         case REAL_RESULT:
            {
               double real_val;
               real_val = *((double*) args->args[i]);
               result
                  = hash64a((const void*)&real_val, sizeof(double), result);
            }
            break;
         case INT_RESULT:
            {
               long long int_val;
               int_val = *((long long*) args->args[i]);
               result = hash64a((const void*)&int_val, sizeof(ulonglong), result);
            }
            break;
         default:
            break;
         }
      }
      else {
         result
            = hash64a((const void*)&null_default, sizeof(null_default), result);
      }
   }
   return result;
}
