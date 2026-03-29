/* system/wolfssl/include/wolfssl/wolfcrypt/coding.h
 *
 * Base64 encode / decode API used by AzureIoTCrypto.cpp.
 *
 * **STUB FILE** – replace with the real wolfSSL header when vendoring the
 * full wolfSSL source tree.  See system/wolfssl/README.md for instructions.
 */

#ifndef WOLF_CRYPT_CODING_H
#define WOLF_CRYPT_CODING_H

#include "wolfssl/wolfcrypt/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Standard Base64 encode (output includes '\\n' line-breaks every 64 chars).
 * @param in      Input bytes.
 * @param inLen   Number of input bytes.
 * @param out     Output buffer.
 * @param outLen  [in] capacity of out; [out] actual bytes written (including
 *                the final NUL appended by this function).
 * @return 0 on success, negative on error.
 */
WOLFSSL_API int Base64_Encode(const byte* in, word32 inLen,
                               byte* out, word32* outLen);

/**
 * Base64 encode without embedded newlines.  This is what Azure IoT SAS
 * token signatures require.
 */
WOLFSSL_API int Base64_Encode_NoNl(const byte* in, word32 inLen,
                                    byte* out, word32* outLen);

/**
 * Base64 decode.
 * @param in      Base64-encoded input (may contain '\\n' and '\\r').
 * @param inLen   Length of input.
 * @param out     Output buffer.
 * @param outLen  [in] capacity of out; [out] actual decoded bytes written.
 * @return 0 on success, negative on error.
 */
WOLFSSL_API int Base64_Decode(const byte* in, word32 inLen,
                               byte* out, word32* outLen);

#ifdef __cplusplus
}
#endif

#endif /* WOLF_CRYPT_CODING_H */
