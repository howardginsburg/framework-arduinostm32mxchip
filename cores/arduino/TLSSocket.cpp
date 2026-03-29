// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// wolfSSL 5.7.6 backed TLS — replaces mbedTLS in open-source code path.

#include "TLSSocket.h"
#include <wolfssl/error-ssl.h>
#include <stdlib.h>
#include <string.h>

static void tls_log_error(const char* label, int err)
{
    printf("[TLS] %s failed: error %d\r\n", label, err);
}

#define TLS_CUSTOM_LABEL "Arduino TLS Socket"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// wolfSSL I/O callbacks (IoT Hub SDK-style buffered polling)

/**
 * wolfSSL receive callback.
 *
 * Mirrors the IoT Hub SDK tlsio_mbedtls.c on_io_recv() pattern: it polls
 * the underlying TCPSocket, buffers any incoming bytes into TLSSocket's
 * internal _recv_buffer, and returns data to wolfSSL from that buffer.
 *
 * During the handshake, if no data is available the callback blocks in a
 * polling loop (bounded by HANDSHAKE_TIMEOUT_MS).  After the handshake it
 * returns WOLFSSL_CBIO_ERR_WANT_READ immediately so wolfSSL can indicate
 * non-blocking WANT_READ to the caller.
 */
static int wolfssl_recv(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
    (void)ssl;
    TLSSocket *tls   = static_cast<TLSSocket *>(ctx);
    TCPSocket *socket = tls->_tcp_socket;
    int pending = 0;

    // Fill the internal buffer if it is empty.
    while (tls->_recv_buffer_count == 0)
    {
        unsigned char temp_buf[128];
        int recv_result = socket->recv(temp_buf, sizeof(temp_buf));

        if (recv_result > 0)
        {
            size_t new_size = tls->_recv_buffer_count + recv_result;
            unsigned char *new_buffer =
                (unsigned char *)realloc(tls->_recv_buffer, new_size);
            if (new_buffer != NULL)
            {
                tls->_recv_buffer = new_buffer;
                memcpy(tls->_recv_buffer + tls->_recv_buffer_count,
                       temp_buf, recv_result);
                tls->_recv_buffer_count = new_size;
            }
            break;
        }
        else if (recv_result == NSAPI_ERROR_WOULD_BLOCK || recv_result == 0)
        {
            if (tls->_handshake_complete)
            {
                // Post-handshake: tell wolfSSL to retry rather than blocking.
                break;
            }
            else
            {
                // During handshake: bounded poll.
                if (pending++ >= HANDSHAKE_TIMEOUT_MS / HANDSHAKE_WAIT_INTERVAL_MS)
                    return WOLFSSL_CBIO_ERR_TIMEOUT;
                wait_ms(HANDSHAKE_WAIT_INTERVAL_MS);
            }
        }
        else
        {
            return WOLFSSL_CBIO_ERR_GENERAL;
        }
    }

    // Serve data from the internal buffer.
    int result = (int)tls->_recv_buffer_count;
    if (result > sz)
        result = sz;

    if (result > 0)
    {
        memcpy(buf, tls->_recv_buffer, result);
        size_t remaining = tls->_recv_buffer_count - result;
        if (remaining > 0)
        {
            memmove(tls->_recv_buffer, tls->_recv_buffer + result, remaining);
            tls->_recv_buffer_count = remaining;
            unsigned char *shrunken =
                (unsigned char *)realloc(tls->_recv_buffer, remaining);
            if (shrunken != NULL)
                tls->_recv_buffer = shrunken;
        }
        else
        {
            free(tls->_recv_buffer);
            tls->_recv_buffer = NULL;
            tls->_recv_buffer_count = 0;
        }
        return result;
    }

    return WOLFSSL_CBIO_ERR_WANT_READ;
}

/**
 * wolfSSL send callback.
 *
 * Loops up to 10 times on NSAPI_ERROR_WOULD_BLOCK, consistent with the
 * IoT Hub SDK tlsio_mbedtls.c on_io_send() retry pattern.
 */
static int wolfssl_send(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
    (void)ssl;
    TLSSocket *tls   = static_cast<TLSSocket *>(ctx);
    TCPSocket *socket = tls->_tcp_socket;

    for (int i = 0; i < 10; ++i)
    {
        int result = socket->send((const unsigned char *)buf, sz);

        if (result > 0)
            return result;
        else if (result == NSAPI_ERROR_WOULD_BLOCK || result == 0)
            wait_ms(100);
        else
            return WOLFSSL_CBIO_ERR_GENERAL;
    }

    // Exhausted retries: pretend the data was sent to avoid a fatal error.
    return sz;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Class implementation

void TLSSocket::init_common(NetworkInterface* net_iface)
{
    _recv_buffer       = NULL;
    _recv_buffer_count = 0;
    _handshake_complete = false;
    _ctx = NULL;
    _ssl = NULL;

    if (net_iface)
        _tcp_socket = new TCPSocket(net_iface);
    else
        _tcp_socket = NULL;
}

TLSSocket::TLSSocket(const char *ssl_ca_pem, NetworkInterface* net_iface)
{
    _ssl_ca_pem      = ssl_ca_pem;
    _ssl_client_cert = NULL;
    _ssl_client_key  = NULL;
    init_common(net_iface);
}

TLSSocket::TLSSocket(const char *ssl_ca_pem, const char *ssl_client_cert,
                     const char *ssl_client_key, NetworkInterface* net_iface)
{
    _ssl_ca_pem      = ssl_ca_pem;
    _ssl_client_cert = ssl_client_cert;
    _ssl_client_key  = ssl_client_key;
    init_common(net_iface);
}

TLSSocket::~TLSSocket()
{
    if (_recv_buffer != NULL)
    {
        free(_recv_buffer);
        _recv_buffer = NULL;
    }

    if (_ssl)
    {
        wolfSSL_shutdown(_ssl);
        wolfSSL_free(_ssl);
        _ssl = NULL;
    }

    if (_ctx)
    {
        wolfSSL_CTX_free(_ctx);
        _ctx = NULL;
    }

    wolfSSL_Cleanup();

    if (_tcp_socket)
    {
        _tcp_socket->close();
        delete _tcp_socket;
    }
}

nsapi_error_t TLSSocket::connect(const char *host, uint16_t port)
{
    if (_tcp_socket == NULL)
        return NSAPI_ERROR_NO_SOCKET;

    if (_ssl_ca_pem == NULL)
    {
        // Plain TCP — no TLS.
        return _tcp_socket->connect(host, port);
    }

    // ── Initialise wolfSSL library ────────────────────────────────────────
    wolfSSL_Init();

    _ctx = wolfSSL_CTX_new(wolfSSLv23_client_method());
    if (_ctx == NULL)
    {
        tls_log_error("CTX_new", 0);
        return -1;
    }

    // Register custom I/O callbacks so wolfSSL uses our TCPSocket layer.
    wolfSSL_CTX_SetIORecv(_ctx, wolfssl_recv);
    wolfSSL_CTX_SetIOSend(_ctx, wolfssl_send);

    // ── Load CA certificate for server authentication ─────────────────────
    int ret = wolfSSL_CTX_load_verify_buffer(
                  _ctx,
                  (const unsigned char *)_ssl_ca_pem,
                  (long)(strlen(_ssl_ca_pem) + 1),
                  SSL_FILETYPE_PEM);
    if (ret != WOLFSSL_SUCCESS)
    {
        tls_log_error("load_verify_buffer", ret);
        return -1;
    }

    wolfSSL_CTX_set_verify(_ctx, WOLFSSL_VERIFY_PEER, NULL);

    // ── Load client certificate + key for mutual TLS ──────────────────────
    if (_ssl_client_cert != NULL && _ssl_client_key != NULL)
    {
        ret = wolfSSL_CTX_use_certificate_buffer(
                  _ctx,
                  (const unsigned char *)_ssl_client_cert,
                  (long)(strlen(_ssl_client_cert) + 1),
                  SSL_FILETYPE_PEM);
        if (ret != WOLFSSL_SUCCESS)
        {
            tls_log_error("use_certificate_buffer", ret);
            return -1;
        }

        ret = wolfSSL_CTX_use_PrivateKey_buffer(
                  _ctx,
                  (const unsigned char *)_ssl_client_key,
                  (long)(strlen(_ssl_client_key) + 1),
                  SSL_FILETYPE_PEM);
        if (ret != WOLFSSL_SUCCESS)
        {
            tls_log_error("use_PrivateKey_buffer", ret);
            return -1;
        }
    }

    // ── Create SSL session ────────────────────────────────────────────────
    _ssl = wolfSSL_new(_ctx);
    if (_ssl == NULL)
    {
        tls_log_error("wolfSSL_new", 0);
        return -1;
    }

    // SNI: send the server hostname in the ClientHello.
    wolfSSL_UseSNI(_ssl, WOLFSSL_SNI_HOST_NAME,
                   host, (unsigned short)strlen(host));

    // Bind the TLSSocket instance as the context pointer for I/O callbacks.
    wolfSSL_SetIOReadCtx(_ssl,  static_cast<void *>(this));
    wolfSSL_SetIOWriteCtx(_ssl, static_cast<void *>(this));

    // Tell wolfSSL that the underlying transport is non-blocking.
    wolfSSL_set_using_nonblock(_ssl, 1);

    // ── TCP connect ───────────────────────────────────────────────────────
    ret = _tcp_socket->connect(host, port);
    if (ret != NSAPI_ERROR_OK)
    {
        printf("[TLS] TCP connect failed: %d\r\n", ret);
        return ret;
    }
    printf("[TLS] TCP connected, starting TLS handshake...\r\n");

    // Short timeout on reads so the handshake poll loop doesn't stall.
    _tcp_socket->set_blocking(false);
    _tcp_socket->set_timeout(100);

    // ── TLS handshake ─────────────────────────────────────────────────────
    _handshake_complete = false;
    do
    {
        ret = wolfSSL_connect(_ssl);
        if (ret != WOLFSSL_SUCCESS)
        {
            int err = wolfSSL_get_error(_ssl, ret);
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
            {
                wait_ms(HANDSHAKE_WAIT_INTERVAL_MS);
            }
            else
            {
                tls_log_error("handshake", err);
                return -1;
            }
        }
    } while (ret != WOLFSSL_SUCCESS);

    printf("[TLS] Handshake complete.\r\n");
    _handshake_complete = true;
    return NSAPI_ERROR_OK;
}

nsapi_error_t TLSSocket::close()
{
    if (_tcp_socket == NULL)
        return NSAPI_ERROR_NO_SOCKET;
    return _tcp_socket->close();
}

nsapi_size_or_error_t TLSSocket::send(const void *data, nsapi_size_t size)
{
    if (_tcp_socket == NULL)
        return NSAPI_ERROR_NO_SOCKET;

    if (_ssl_ca_pem == NULL)
    {
        // Plain TCP send with retry.
        const unsigned char *ptr  = (const unsigned char *)data;
        size_t total_sent = 0;
        while (total_sent < size)
        {
            int result = _tcp_socket->send(ptr + total_sent, size - total_sent);
            if (result > 0)
                total_sent += result;
            else if (result == NSAPI_ERROR_WOULD_BLOCK || result == 0)
                wait_ms(100);
            else
                return result;
        }
        return (nsapi_size_or_error_t)size;
    }

    // TLS send: loop until all bytes are written.
    const unsigned char *ptr = (const unsigned char *)data;
    int out_left = (int)size;

    do
    {
        int ret = wolfSSL_write(_ssl, ptr + (size - out_left), out_left);

        if (ret > 0)
        {
            out_left -= ret;
        }
        else
        {
            int err = wolfSSL_get_error(_ssl, ret);
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
                wait_ms(10);
            else
                return WOLFSSL_FATAL_ERROR;
        }
    } while (out_left > 0);

    return (nsapi_size_or_error_t)size;
}

nsapi_size_or_error_t TLSSocket::recv(void *data, nsapi_size_t size)
{
    if (_tcp_socket == NULL)
        return NSAPI_ERROR_NO_SOCKET;

    if (_ssl_ca_pem == NULL)
    {
        // Plain TCP receive.
        return _tcp_socket->recv(data, size);
    }

    int ret = wolfSSL_read(_ssl, data, (int)size);

    if (ret > 0)
        return (nsapi_size_or_error_t)ret;

    if (ret == 0)
        return 0;  // graceful close

    int err = wolfSSL_get_error(_ssl, ret);
    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
        return 0;  // no data yet — caller should retry

    return WOLFSSL_FATAL_ERROR;
}
