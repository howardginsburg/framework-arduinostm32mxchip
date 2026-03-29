/* system/wolfssl/include/wolfssl/wolfcrypt/types.h
 *
 * Minimal type definitions used across wolfSSL / wolfCrypt APIs.
 *
 * **STUB FILE** – replace with the real wolfSSL header when vendoring the
 * full wolfSSL source tree.  See system/wolfssl/README.md for instructions.
 */

#ifndef WOLF_CRYPT_TYPES_H
#define WOLF_CRYPT_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef unsigned char  byte;
typedef unsigned short word16;
typedef unsigned int   word32;
typedef uint64_t       word64;

/* Sentinel value meaning "no hardware crypto device". */
#ifndef INVALID_DEVID
#  define INVALID_DEVID (-2)
#endif

/* Boolean convenience. */
#ifndef WOLFSSL_API
#  define WOLFSSL_API
#endif

#endif /* WOLF_CRYPT_TYPES_H */
