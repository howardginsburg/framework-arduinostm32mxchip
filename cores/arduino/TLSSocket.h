// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __TLS_SOCKET_H__
#define __TLS_SOCKET_H__

#include "mbed.h"

/*
 * wolfSSL replaces mbedTLS for all TLS operations owned by this class.
 *
 * The pre-compiled system binaries (libdevkit-sdk-core-lib.a, libstsafe.a)
 * still use mbedTLS internally; their symbols are satisfied by the
 * pre-compiled mbedTLS contained in those libraries.  Our open-source code
 * only calls wolfSSL APIs from here forward.
 */
#ifndef WOLFSSL_USER_SETTINGS
#  define WOLFSSL_USER_SETTINGS
#endif
#include "wolfssl/wolfcrypt/settings.h"
#include "wolfssl/ssl.h"

// IoT Hub SDK-style configuration
#define TLSIO_RECV_BUFFER_SIZE    256
#define HANDSHAKE_TIMEOUT_MS      5000
#define HANDSHAKE_WAIT_INTERVAL_MS 10

class TLSSocket
{
public:
    /**
     * @brief Construct TLSSocket for server-only TLS (one-way authentication).
     * @param ssl_ca_pem  CA certificate in PEM format (null-terminated string).
     * @param net_iface   Network interface to use.
     */
    TLSSocket(const char *ssl_ca_pem, NetworkInterface* net_iface);

    /**
     * @brief Construct TLSSocket for mutual TLS (two-way authentication).
     * @param ssl_ca_pem       CA certificate in PEM format (null-terminated string).
     * @param ssl_client_cert  Client certificate in PEM format, or NULL.
     * @param ssl_client_key   Client private key in PEM format, or NULL.
     * @param net_iface        Network interface to use.
     */
    TLSSocket(const char *ssl_ca_pem, const char *ssl_client_cert,
              const char *ssl_client_key, NetworkInterface* net_iface);

    virtual ~TLSSocket();

    nsapi_error_t connect(const char *host, uint16_t port);
    nsapi_error_t close();
    nsapi_size_or_error_t send(const void *data, nsapi_size_t size);
    nsapi_size_or_error_t recv(void *data, nsapi_size_t size);

    /**
     * @brief Check whether mutual TLS (client certificate) is configured.
     * @return true if a client certificate has been set.
     */
    bool isMutualTLS() const { return _ssl_client_cert != NULL; }

    // IoT Hub SDK-style receive buffer (accessed by the wolfSSL I/O callbacks).
    unsigned char *_recv_buffer;
    size_t         _recv_buffer_count;
    bool           _handshake_complete;
    TCPSocket     *_tcp_socket;

private:
    void init_common(NetworkInterface* net_iface);

    WOLFSSL_CTX *_ctx;   ///< wolfSSL context (one per connection attempt)
    WOLFSSL     *_ssl;   ///< wolfSSL session object

    const char *_ssl_ca_pem;
    const char *_ssl_client_cert;
    const char *_ssl_client_key;
};

#endif  // __TLS_SOCKET_H__
