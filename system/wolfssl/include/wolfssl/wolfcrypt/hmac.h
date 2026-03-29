/* system/wolfssl/include/wolfssl/wolfcrypt/hmac.h
 *
 * HMAC-SHA256 API used by AzureIoTCrypto.cpp for SAS token generation and
 * group-key derivation.
 *
 * **STUB FILE** – replace with the real wolfSSL header when vendoring the
 * full wolfSSL source tree.  See system/wolfssl/README.md for instructions.
 */

#ifndef WOLF_CRYPT_HMAC_H
#define WOLF_CRYPT_HMAC_H

#include "wolfssl/wolfcrypt/types.h"

/* Digest algorithm IDs (subset; matches real wolfSSL values). */
#define WC_MD5         0
#define WC_SHA         1
#define WC_SHA256      4
#define WC_SHA384      5
#define WC_SHA512      6

#define WC_SHA256_DIGEST_SIZE 32

/* Internal block size used by the stub for ipad/opad storage. */
#define HMAC_BLOCK_SIZE 64

/*
 * Size of the opaque inner-hash scratch area embedded in Hmac.
 * Must be >= sizeof(SHA256_CTX) defined in wolfcrypt_stub.c (~108 bytes).
 * Sized with margin so the struct is stable across minor internal changes.
 */
#define HMAC_INNER_CTX_SIZE 128

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque HMAC context.  The stub stores type, key material, and a running
 * inner-hash state so that wc_HmacUpdate() can be called multiple times.
 */
typedef struct Hmac {
    int    macType;                    /**< WC_SHA256 etc.          */
    byte   ipad[HMAC_BLOCK_SIZE];      /**< inner-padding key xor   */
    byte   opad[HMAC_BLOCK_SIZE];      /**< outer-padding key xor   */
    byte   inner[HMAC_INNER_CTX_SIZE]; /**< SHA256_CTX (opaque)     */
} Hmac;

/**
 * Initialise an Hmac context.
 * @param hmac   Context to initialise.
 * @param heap   Heap hint (pass NULL for default).
 * @param devId  Hardware device ID (pass INVALID_DEVID for software).
 * @return 0 on success, negative on error.
 */
WOLFSSL_API int wc_HmacInit(Hmac* hmac, void* heap, int devId);

/**
 * Set the HMAC key and algorithm.
 * @param hmac   Initialised context.
 * @param type   Algorithm (WC_SHA256).
 * @param key    Key bytes.
 * @param keySz  Key length in bytes.
 * @return 0 on success.
 */
WOLFSSL_API int wc_HmacSetKey(Hmac* hmac, int type, const byte* key, word32 keySz);

/**
 * Feed data into the HMAC.
 * @param hmac   Context in progress.
 * @param in     Input bytes.
 * @param sz     Number of bytes.
 * @return 0 on success.
 */
WOLFSSL_API int wc_HmacUpdate(Hmac* hmac, const byte* in, word32 sz);

/**
 * Finalise the HMAC and write the digest.
 * @param hmac   Context in progress.
 * @param hash   Output buffer (must be at least WC_SHA256_DIGEST_SIZE bytes).
 * @return 0 on success.
 */
WOLFSSL_API int wc_HmacFinal(Hmac* hmac, byte* hash);

/** Free any resources held by the context. */
WOLFSSL_API void wc_HmacFree(Hmac* hmac);

#ifdef __cplusplus
}
#endif

#endif /* WOLF_CRYPT_HMAC_H */
