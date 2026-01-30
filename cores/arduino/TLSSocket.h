// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __TLS_SOCKET_H__
#define __TLS_SOCKET_H__

#include "mbed.h"

#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/pk.h"

#if DEBUG_LEVEL > 0
#include "mbedtls/debug.h"
#endif

class TLSSocket
{
public:
    /**
     * @brief Construct TLSSocket for server-only TLS (one-way authentication)
     * @param ssl_ca_pem    CA certificate in PEM format (null-terminated string)
     * @param net_iface     Network interface to use
     */
    TLSSocket(const char *ssl_ca_pem, NetworkInterface* net_iface);
    
    /**
     * @brief Construct TLSSocket for mutual TLS (two-way authentication)
     * @param ssl_ca_pem        CA certificate in PEM format (null-terminated string)
     * @param ssl_client_cert   Client certificate in PEM format (null-terminated string), or NULL
     * @param ssl_client_key    Client private key in PEM format (null-terminated string), or NULL
     * @param net_iface         Network interface to use
     */
    TLSSocket(const char *ssl_ca_pem, const char *ssl_client_cert, 
              const char *ssl_client_key, NetworkInterface* net_iface);
    
    virtual ~TLSSocket();

    nsapi_error_t connect(const char *host, uint16_t port);
    nsapi_error_t close();
    nsapi_size_or_error_t send(const void *data, nsapi_size_t size);
    nsapi_size_or_error_t recv(void *data, nsapi_size_t size);
    
    /**
     * @brief Check if mutual TLS (client certificate) is configured
     * @return true if client certificate is set
     */
    bool isMutualTLS() const { return _ssl_client_cert != NULL; }

private:
    void init_common(NetworkInterface* net_iface);
    
    mbedtls_entropy_context _entropy;
    mbedtls_ctr_drbg_context _ctr_drbg;
    mbedtls_x509_crt _cacert;
    mbedtls_x509_crt _clientcert;     // Client certificate for mutual TLS
    mbedtls_pk_context _clientkey;     // Client private key for mutual TLS
    mbedtls_ssl_context _ssl;
    mbedtls_ssl_config _ssl_conf;
    
    const char *_ssl_ca_pem;
    const char *_ssl_client_cert;
    const char *_ssl_client_key;
    TCPSocket *_tcp_socket;
    bool check_mbedtls_ssl_write(int ret);
};


#endif  // __TLS_SOCKET_H__
