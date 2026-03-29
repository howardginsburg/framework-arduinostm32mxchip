/* system/wolfssl/include/wolfssl/ssl.h
 *
 * Minimal wolfSSL TLS client API used by TLSSocket.cpp.
 *
 * **STUB FILE** – replace with the real wolfSSL header tree when vendoring
 * the full wolfSSL source.  See system/wolfssl/README.md for instructions.
 *
 * Struct definitions here are intentionally simple so that the code compiles
 * and links without the real wolfSSL sources.  When the real wolfSSL is
 * dropped in, delete this file and let the genuine wolfssl/ssl.h take over.
 */

#ifndef WOLFSSL_SSL_H
#define WOLFSSL_SSL_H

#include <stddef.h>
#include <stdint.h>

/* Pull in wolfSSL_API macro and basic types. */
#include "wolfssl/wolfcrypt/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Opaque context / session types ──────────────────────────────────────── */

/* I/O callback signatures (must match before the struct definitions below). */
struct WOLFSSL;
typedef int (*CallbackIORecv)(struct WOLFSSL* ssl, char* buf, int sz, void* ctx);
typedef int (*CallbackIOSend)(struct WOLFSSL* ssl, char* buf, int sz, void* ctx);

/** Per-context configuration (cipher suites, certificates, I/O callbacks). */
typedef struct WOLFSSL_CTX {
    CallbackIORecv recv_cb;
    CallbackIOSend send_cb;
} WOLFSSL_CTX;

/** Per-connection TLS session. */
typedef struct WOLFSSL {
    WOLFSSL_CTX *ctx;
    void        *read_ctx;   /**< user pointer forwarded to recv_cb */
    void        *write_ctx;  /**< user pointer forwarded to send_cb */
    int          nonblock;   /**< non-zero when underlying I/O is non-blocking */
    int          last_error; /**< cached wolfSSL_get_error() value */
} WOLFSSL;

/** TLS version selection method. */
typedef struct WOLFSSL_METHOD {
    int version;
} WOLFSSL_METHOD;

/** Certificate-store context (used only in verify callbacks). */
typedef struct WOLFSSL_X509_STORE_CTX {
    int dummy;
} WOLFSSL_X509_STORE_CTX;

/* ── Return codes ─────────────────────────────────────────────────────────── */
#define WOLFSSL_SUCCESS       1
#define WOLFSSL_FAILURE       0
#define WOLFSSL_FATAL_ERROR  (-1)

/* wolfSSL_get_error() codes (match real wolfSSL values). */
#define SSL_ERROR_NONE        0
#define SSL_ERROR_WANT_READ   2
#define SSL_ERROR_WANT_WRITE  3
#define WOLFSSL_ERROR_WANT_READ  SSL_ERROR_WANT_READ
#define WOLFSSL_ERROR_WANT_WRITE SSL_ERROR_WANT_WRITE

/* ── I/O callback return codes ────────────────────────────────────────────── */
#define WOLFSSL_CBIO_ERR_GENERAL    (-1)
#define WOLFSSL_CBIO_ERR_WANT_READ  (-2)
#define WOLFSSL_CBIO_ERR_WANT_WRITE (-2)
#define WOLFSSL_CBIO_ERR_CONN_RST   (-3)
#define WOLFSSL_CBIO_ERR_ISR        (-4)
#define WOLFSSL_CBIO_ERR_CONN_CLOSE (-5)
#define WOLFSSL_CBIO_ERR_TIMEOUT    (-6)

/* ── Certificate / key format constants ──────────────────────────────────── */
#define SSL_FILETYPE_ASN1       2
#define SSL_FILETYPE_PEM        1
#define WOLFSSL_FILETYPE_ASN1   SSL_FILETYPE_ASN1
#define WOLFSSL_FILETYPE_PEM    SSL_FILETYPE_PEM

/* ── Peer-verification modes ─────────────────────────────────────────────── */
#define WOLFSSL_VERIFY_NONE                 0
#define WOLFSSL_VERIFY_PEER                 1
#define WOLFSSL_VERIFY_FAIL_IF_NO_PEER_CERT 2

/* ── SNI extension ────────────────────────────────────────────────────────── */
#define WOLFSSL_SNI_HOST_NAME 0

/* ── Library lifecycle ────────────────────────────────────────────────────── */

/** One-time initialisation of the wolfSSL library. */
WOLFSSL_API int wolfSSL_Init(void);

/** Clean up library resources. */
WOLFSSL_API int wolfSSL_Cleanup(void);

/* ── Method selectors ─────────────────────────────────────────────────────── */

/** Negotiate the highest mutually supported TLS version (client). */
WOLFSSL_API WOLFSSL_METHOD* wolfSSLv23_client_method(void);

/** Alias accepted by some wolf SSL versions. */
WOLFSSL_API WOLFSSL_METHOD* wolfTLS_client_method(void);

WOLFSSL_API WOLFSSL_METHOD* wolfTLSv1_2_client_method(void);
WOLFSSL_API WOLFSSL_METHOD* wolfTLSv1_3_client_method(void);

/* ── Context lifecycle ────────────────────────────────────────────────────── */

/** Allocate a new SSL context. */
WOLFSSL_API WOLFSSL_CTX* wolfSSL_CTX_new(WOLFSSL_METHOD* method);

/** Free a context and all associated resources. */
WOLFSSL_API void wolfSSL_CTX_free(WOLFSSL_CTX* ctx);

/* ── Certificate / key loading ────────────────────────────────────────────── */

/**
 * Load one or more trusted CA certificates from a memory buffer.
 * @param format  SSL_FILETYPE_PEM or SSL_FILETYPE_ASN1.
 */
WOLFSSL_API int wolfSSL_CTX_load_verify_buffer(WOLFSSL_CTX* ctx,
        const unsigned char* in, long sz, int format);

/** Load the client certificate from a memory buffer. */
WOLFSSL_API int wolfSSL_CTX_use_certificate_buffer(WOLFSSL_CTX* ctx,
        const unsigned char* in, long sz, int format);

/** Load the client private key from a memory buffer. */
WOLFSSL_API int wolfSSL_CTX_use_PrivateKey_buffer(WOLFSSL_CTX* ctx,
        const unsigned char* in, long sz, int format);

/* ── I/O callbacks ────────────────────────────────────────────────────────── */

/** Set the receive callback for all SSL objects created from this context. */
WOLFSSL_API void wolfSSL_CTX_SetIORecv(WOLFSSL_CTX* ctx, CallbackIORecv callback);

/** Set the send callback for all SSL objects created from this context. */
WOLFSSL_API void wolfSSL_CTX_SetIOSend(WOLFSSL_CTX* ctx, CallbackIOSend callback);

/** Set the opaque user pointer passed to the receive callback. */
WOLFSSL_API void wolfSSL_SetIOReadCtx(WOLFSSL* ssl, void* ctx);

/** Set the opaque user pointer passed to the send callback. */
WOLFSSL_API void wolfSSL_SetIOWriteCtx(WOLFSSL* ssl, void* ctx);

/* ── Peer-verification ────────────────────────────────────────────────────── */

/** Set the verification mode and optional callback for this context. */
WOLFSSL_API void wolfSSL_CTX_set_verify(WOLFSSL_CTX* ctx, int mode,
        int (*verify_cb)(int, WOLFSSL_X509_STORE_CTX*));

/* ── SNI extension ────────────────────────────────────────────────────────── */

/**
 * Enable Server Name Indication for the handshake.
 * @param type  WOLFSSL_SNI_HOST_NAME.
 * @param data  Hostname string (not NUL-terminated; length given by size).
 * @param size  Length of hostname in bytes.
 */
WOLFSSL_API int wolfSSL_UseSNI(WOLFSSL* ssl, unsigned char type,
        const void* data, unsigned short size);

/* ── SSL session lifecycle ────────────────────────────────────────────────── */

/** Create a new SSL session from a context. */
WOLFSSL_API WOLFSSL* wolfSSL_new(WOLFSSL_CTX* ctx);

/** Free an SSL session. */
WOLFSSL_API void wolfSSL_free(WOLFSSL* ssl);

/** Inform wolfSSL that the underlying transport is non-blocking. */
WOLFSSL_API void wolfSSL_set_using_nonblock(WOLFSSL* ssl, int nonblock);

/* ── Handshake / shutdown ─────────────────────────────────────────────────── */

/**
 * Perform the TLS handshake (client side).
 * @return WOLFSSL_SUCCESS on completion; WOLFSSL_FATAL_ERROR if an error
 *         occurred (check wolfSSL_get_error()).
 */
WOLFSSL_API int wolfSSL_connect(WOLFSSL* ssl);

/**
 * Send a close_notify alert and tear down the TLS session.
 * @return WOLFSSL_SUCCESS on success.
 */
WOLFSSL_API int wolfSSL_shutdown(WOLFSSL* ssl);

/* ── Data transfer ────────────────────────────────────────────────────────── */

/**
 * Read application data from the TLS record layer.
 * @return Number of bytes read (>0), 0 on graceful close, or
 *         WOLFSSL_FATAL_ERROR on error / WANT_READ — check wolfSSL_get_error().
 */
WOLFSSL_API int wolfSSL_read(WOLFSSL* ssl, void* data, int sz);

/**
 * Write application data into the TLS record layer.
 * @return Number of bytes written (>0) or WOLFSSL_FATAL_ERROR.
 */
WOLFSSL_API int wolfSSL_write(WOLFSSL* ssl, const void* data, int sz);

/* ── Error reporting ──────────────────────────────────────────────────────── */

/**
 * Translate a WOLFSSL_FATAL_ERROR return into a detailed error code.
 * Common return values: SSL_ERROR_WANT_READ, SSL_ERROR_WANT_WRITE,
 * SSL_ERROR_NONE (clean close).
 */
WOLFSSL_API int wolfSSL_get_error(WOLFSSL* ssl, int ret);

#ifdef __cplusplus
}
#endif

#endif /* WOLFSSL_SSL_H */
