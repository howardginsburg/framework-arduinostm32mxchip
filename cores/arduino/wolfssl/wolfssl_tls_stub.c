/*
 * cores/arduino/wolfssl/wolfssl_tls_stub.c
 *
 * Stub implementations of the wolfSSL TLS API used by TLSSocket.cpp.
 *
 * PURPOSE
 * -------
 * These stubs allow the framework to compile and link without the real
 * wolfSSL source tree.  The handshake and I/O functions delegate directly
 * to the registered I/O callbacks so that the overall plumbing (TLSSocket →
 * wolfSSL → tcp_socket) is exercised, but no actual TLS crypto is performed.
 *
 * REPLACING WITH REAL wolfSSL
 * ---------------------------
 * 1. Download wolfSSL from https://www.wolfssl.com/download/ (5.x or later).
 * 2. Copy the wolfSSL source tree into  system/wolfssl/wolfssl-<version>/.
 * 3. Delete  cores/arduino/wolfssl/wolfssl_tls_stub.c  (this file).
 * 4. Update platform.txt to compile the real wolfSSL .c sources from
 *    system/wolfssl/wolfssl-<version>/src/ and add the include paths.
 * 5. Keep  system/wolfssl/user_settings.h  — it is picked up automatically
 *    when WOLFSSL_USER_SETTINGS is defined (already set in platform.txt).
 */

#include "wolfssl/ssl.h"
#include <stdlib.h>
#include <string.h>

/* ── Method singletons ───────────────────────────────────────────────────── */

static WOLFSSL_METHOD _method_v23   = { 0x0304 }; /* TLS 1.3 preferred */
static WOLFSSL_METHOD _method_tls12 = { 0x0303 };
static WOLFSSL_METHOD _method_tls13 = { 0x0304 };

int wolfSSL_Init(void)    { return WOLFSSL_SUCCESS; }
int wolfSSL_Cleanup(void) { return WOLFSSL_SUCCESS; }

WOLFSSL_METHOD* wolfSSLv23_client_method(void)  { return &_method_v23;   }
WOLFSSL_METHOD* wolfTLS_client_method(void)     { return &_method_v23;   }
WOLFSSL_METHOD* wolfTLSv1_2_client_method(void) { return &_method_tls12; }
WOLFSSL_METHOD* wolfTLSv1_3_client_method(void) { return &_method_tls13; }

/* ── Context lifecycle ────────────────────────────────────────────────────── */

WOLFSSL_CTX* wolfSSL_CTX_new(WOLFSSL_METHOD* method)
{
    WOLFSSL_CTX *ctx = (WOLFSSL_CTX *)calloc(1, sizeof(WOLFSSL_CTX));
    (void)method;
    return ctx;
}

void wolfSSL_CTX_free(WOLFSSL_CTX* ctx)
{
    if (ctx)
        free(ctx);
}

/* ── Certificate / key loading (stubs accept any buffer) ─────────────────── */

int wolfSSL_CTX_load_verify_buffer(WOLFSSL_CTX* ctx,
        const unsigned char* in, long sz, int format)
{
    (void)ctx; (void)in; (void)sz; (void)format;
    return WOLFSSL_SUCCESS;
}

int wolfSSL_CTX_use_certificate_buffer(WOLFSSL_CTX* ctx,
        const unsigned char* in, long sz, int format)
{
    (void)ctx; (void)in; (void)sz; (void)format;
    return WOLFSSL_SUCCESS;
}

int wolfSSL_CTX_use_PrivateKey_buffer(WOLFSSL_CTX* ctx,
        const unsigned char* in, long sz, int format)
{
    (void)ctx; (void)in; (void)sz; (void)format;
    return WOLFSSL_SUCCESS;
}

/* ── I/O callbacks ────────────────────────────────────────────────────────── */

void wolfSSL_CTX_SetIORecv(WOLFSSL_CTX* ctx, CallbackIORecv callback)
{
    if (ctx)
        ctx->recv_cb = callback;
}

void wolfSSL_CTX_SetIOSend(WOLFSSL_CTX* ctx, CallbackIOSend callback)
{
    if (ctx)
        ctx->send_cb = callback;
}

void wolfSSL_SetIOReadCtx(WOLFSSL* ssl, void* ctx)
{
    if (ssl)
        ssl->read_ctx = ctx;
}

void wolfSSL_SetIOWriteCtx(WOLFSSL* ssl, void* ctx)
{
    if (ssl)
        ssl->write_ctx = ctx;
}

/* ── Peer verification ────────────────────────────────────────────────────── */

void wolfSSL_CTX_set_verify(WOLFSSL_CTX* ctx, int mode,
        int (*verify_cb)(int, WOLFSSL_X509_STORE_CTX*))
{
    (void)ctx; (void)mode; (void)verify_cb;
}

/* ── SNI ──────────────────────────────────────────────────────────────────── */

int wolfSSL_UseSNI(WOLFSSL* ssl, unsigned char type,
        const void* data, unsigned short size)
{
    (void)ssl; (void)type; (void)data; (void)size;
    return WOLFSSL_SUCCESS;
}

/* ── SSL session lifecycle ────────────────────────────────────────────────── */

WOLFSSL* wolfSSL_new(WOLFSSL_CTX* ctx)
{
    WOLFSSL *ssl = (WOLFSSL *)calloc(1, sizeof(WOLFSSL));
    if (ssl)
        ssl->ctx = ctx;
    return ssl;
}

void wolfSSL_free(WOLFSSL* ssl)
{
    if (ssl)
        free(ssl);
}

void wolfSSL_set_using_nonblock(WOLFSSL* ssl, int nonblock)
{
    if (ssl)
        ssl->nonblock = nonblock;
}

/* ── Handshake / shutdown ─────────────────────────────────────────────────── */

/*
 * Stub: the handshake "succeeds" immediately without real TLS negotiation.
 * Replace with real wolfSSL to get genuine certificate verification and
 * encrypted communication.
 */
int wolfSSL_connect(WOLFSSL* ssl)
{
    (void)ssl;
    return WOLFSSL_SUCCESS;
}

int wolfSSL_shutdown(WOLFSSL* ssl)
{
    (void)ssl;
    return WOLFSSL_SUCCESS;
}

/* ── Data transfer ────────────────────────────────────────────────────────── */

/*
 * Stub: pass data through the registered I/O callbacks without TLS framing.
 * The callback return-code translation below matches the contract that
 * TLSSocket.cpp expects from wolfSSL.
 */
int wolfSSL_read(WOLFSSL* ssl, void* data, int sz)
{
    if (!ssl || !ssl->ctx || !ssl->ctx->recv_cb)
    {
        if (ssl) ssl->last_error = WOLFSSL_CBIO_ERR_GENERAL;
        return WOLFSSL_FATAL_ERROR;
    }

    int ret = ssl->ctx->recv_cb(ssl, (char *)data, sz, ssl->read_ctx);

    if (ret > 0)
    {
        ssl->last_error = SSL_ERROR_NONE;
        return ret;
    }
    else if (ret == 0 || ret == WOLFSSL_CBIO_ERR_CONN_CLOSE)
    {
        ssl->last_error = SSL_ERROR_NONE;
        return 0; /* graceful close */
    }
    else if (ret == WOLFSSL_CBIO_ERR_WANT_READ)
    {
        ssl->last_error = SSL_ERROR_WANT_READ;
        return WOLFSSL_FATAL_ERROR;
    }
    else
    {
        ssl->last_error = WOLFSSL_CBIO_ERR_GENERAL;
        return WOLFSSL_FATAL_ERROR;
    }
}

int wolfSSL_write(WOLFSSL* ssl, const void* data, int sz)
{
    if (!ssl || !ssl->ctx || !ssl->ctx->send_cb)
    {
        if (ssl) ssl->last_error = WOLFSSL_CBIO_ERR_GENERAL;
        return WOLFSSL_FATAL_ERROR;
    }

    int ret = ssl->ctx->send_cb(ssl, (char *)data, sz, ssl->write_ctx);

    if (ret > 0)
    {
        ssl->last_error = SSL_ERROR_NONE;
        return ret;
    }
    else if (ret == WOLFSSL_CBIO_ERR_WANT_WRITE)
    {
        ssl->last_error = SSL_ERROR_WANT_WRITE;
        return WOLFSSL_FATAL_ERROR;
    }
    else
    {
        ssl->last_error = WOLFSSL_CBIO_ERR_GENERAL;
        return WOLFSSL_FATAL_ERROR;
    }
}

/* ── Error reporting ──────────────────────────────────────────────────────── */

int wolfSSL_get_error(WOLFSSL* ssl, int ret)
{
    (void)ret;
    if (!ssl)
        return WOLFSSL_CBIO_ERR_GENERAL;
    return ssl->last_error;
}
