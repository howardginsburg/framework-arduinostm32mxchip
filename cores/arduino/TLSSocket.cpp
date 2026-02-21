// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// Modified to use IoT Hub SDK-style buffering and polling

#include "TLSSocket.h"
#include "mbedtls/error.h"
#include <stdlib.h>
#include <string.h>

static void tls_log_error(const char* label, int ret)
{
    char buf[128];
    mbedtls_strerror(ret, buf, sizeof(buf));
    printf("[TLS] %s: -0x%04X %s\r\n", label, (unsigned int)(-ret), buf);
}

#define TLS_CUNSTOM "Arduino TLS Socket"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// IoT Hub SDK-style SSL callbacks
// These use the TLSSocket instance pointer to access internal buffer

/**
 * Receive callback for mbed TLS - IoT Hub SDK style
 * 
 * Uses internal buffer and polling loop like tlsio_mbedtls.c on_io_recv()
 */
static int ssl_recv(void *ctx, unsigned char *buf, size_t len) 
{
    TLSSocket *tls = static_cast<TLSSocket *>(ctx);
    TCPSocket *socket = tls->_tcp_socket;
    int pending = 0;
    
    // IoT Hub SDK style: poll socket until we have data in buffer
    while (tls->_recv_buffer_count == 0)
    {
        // Try to receive data from underlying socket
        unsigned char temp_buf[128];
        int recv_result = socket->recv(temp_buf, sizeof(temp_buf));
        
        if (recv_result > 0)
        {
            // Got data - add to internal buffer
            size_t new_size = tls->_recv_buffer_count + recv_result;
            unsigned char *new_buffer = (unsigned char *)realloc(tls->_recv_buffer, new_size);
            if (new_buffer != NULL)
            {
                tls->_recv_buffer = new_buffer;
                memcpy(tls->_recv_buffer + tls->_recv_buffer_count, temp_buf, recv_result);
                tls->_recv_buffer_count = new_size;
            }
            break;  // Got data, exit polling loop
        }
        else if (recv_result == NSAPI_ERROR_WOULD_BLOCK || recv_result == 0)
        {
            // No data available yet
            if (tls->_handshake_complete)
            {
                // After handshake, don't block - return WANT_READ
                break;
            }
            else
            {
                // During handshake: poll with timeout like IoT Hub SDK
                if (pending++ >= HANDSHAKE_TIMEOUT_MS / HANDSHAKE_WAIT_INTERVAL_MS)
                {
                    // Timeout during handshake
                    return MBEDTLS_ERR_SSL_TIMEOUT;
                }
                wait_ms(HANDSHAKE_WAIT_INTERVAL_MS);
            }
        }
        else
        {
            // Real socket error
            return -1;
        }
    }
    
    // Return data from internal buffer (like IoT Hub SDK on_io_recv)
    int result = (int)tls->_recv_buffer_count;
    if (result > (int)len)
    {
        result = (int)len;
    }
    
    if (result > 0)
    {
        // Copy data to caller's buffer
        memcpy(buf, tls->_recv_buffer, result);
        
        // Shift remaining data in buffer
        size_t remaining = tls->_recv_buffer_count - result;
        if (remaining > 0)
        {
            memmove(tls->_recv_buffer, tls->_recv_buffer + result, remaining);
            tls->_recv_buffer_count = remaining;
            
            // Shrink buffer
            unsigned char *new_buffer = (unsigned char *)realloc(tls->_recv_buffer, remaining);
            if (new_buffer != NULL)
            {
                tls->_recv_buffer = new_buffer;
            }
        }
        else
        {
            // Buffer empty
            free(tls->_recv_buffer);
            tls->_recv_buffer = NULL;
            tls->_recv_buffer_count = 0;
        }
        
        return result;
    }
    
    // No data in buffer - tell mbedTLS to try again
    return MBEDTLS_ERR_SSL_WANT_READ;
}

/**
 * Send callback for mbed TLS - IoT Hub SDK style
 * 
 * Loops until all data sent or error (like tlsio_mbedtls.c on_io_send)
 */
static int ssl_send(void *ctx, const unsigned char *buf, size_t len)
{
    TLSSocket *tls = static_cast<TLSSocket *>(ctx);
    TCPSocket *socket = tls->_tcp_socket;
    
    // IoT Hub SDK style: retry loop for send
    for (int i = 0; i < 10; ++i)
    {
        int size = socket->send(buf, len);
        
        if (size > 0)
        {
            return size;
        }
        else if (size == NSAPI_ERROR_WOULD_BLOCK || size == 0)
        {
            // Can't send right now - wait and retry
            wait_ms(100);
        }
        else
        {
            // Real socket error
            return -1;
        }
    }
    
    // Exhausted retries - tell mbedTLS we sent it (like original devkit-sdk)
    // This prevents mbedTLS from treating it as fatal error
    return len;
}

#if DEBUG_LEVEL > 0
static void my_debug(void *ctx, int level, const char *file_name, int line, const char *str)
{
    char tmp[32];
    const char *p, *basename;
    
    if (file_name != NULL)
    {
        /* Extract basename from file */
        basename = file_name;
        for (p = basename; *p != '\0'; p++)
        {
            if(*p == '/' || *p == '\\')
            {
                basename = p + 1;
            }
        }
        
        INFO(basename);
    }
    sprintf(tmp, " %04d: |%d| ", line, level);
    INFO(tmp);
    INFO("\r\n");
}

static int my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
{
    const uint32_t buf_size = 1024;
    char *buf = new char[buf_size];
    (void) data;

    
    mbedtls_x509_crt_info(buf, buf_size - 1, "  ", crt);
    

    if (*flags == 0)
    {
        INFO("No verification issue for this certificate");
    }
    else
    {
        mbedtls_x509_crt_verify_info(buf, buf_size, "  ! ", *flags);
        INFO(buf);
    }

    delete[] buf;
    return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Class

void TLSSocket::init_common(NetworkInterface* net_iface)
{
    // IoT Hub SDK-style: initialize receive buffer
    _recv_buffer = NULL;
    _recv_buffer_count = 0;
    _handshake_complete = false;
    
    if (net_iface)
    {
        _tcp_socket = new TCPSocket(net_iface);
    }
    else
    {
        _tcp_socket = NULL;
    }

    if (_ssl_ca_pem)
    {
        // SSL
        mbedtls_entropy_init(&_entropy);
        mbedtls_ctr_drbg_init(&_ctr_drbg);
        mbedtls_x509_crt_init(&_cacert);
        mbedtls_x509_crt_init(&_clientcert);
        mbedtls_pk_init(&_clientkey);
        mbedtls_ssl_init(&_ssl);
        mbedtls_ssl_config_init(&_ssl_conf);
    }
}

TLSSocket::TLSSocket(const char *ssl_ca_pem, NetworkInterface* net_iface)
{
    _ssl_ca_pem = ssl_ca_pem;
    _ssl_client_cert = NULL;
    _ssl_client_key = NULL;
    init_common(net_iface);
}

TLSSocket::TLSSocket(const char *ssl_ca_pem, const char *ssl_client_cert,
                     const char *ssl_client_key, NetworkInterface* net_iface)
{
    _ssl_ca_pem = ssl_ca_pem;
    _ssl_client_cert = ssl_client_cert;
    _ssl_client_key = ssl_client_key;
    init_common(net_iface);
}

TLSSocket::~TLSSocket()
{
    // Free receive buffer
    if (_recv_buffer != NULL)
    {
        free(_recv_buffer);
        _recv_buffer = NULL;
    }
    
    if (_ssl_ca_pem)
    {
        mbedtls_entropy_free(&_entropy);
        mbedtls_ctr_drbg_free(&_ctr_drbg);
        mbedtls_x509_crt_free(&_cacert);
        mbedtls_x509_crt_free(&_clientcert);
        mbedtls_pk_free(&_clientkey);
        mbedtls_ssl_free(&_ssl);
        mbedtls_ssl_config_free(&_ssl_conf);
    }
    
    if (_tcp_socket)
    {
        _tcp_socket->close();
        delete _tcp_socket;
    }
}

nsapi_error_t TLSSocket::connect(const char *host, uint16_t port)
{
    if (_tcp_socket == NULL)
    {
        return NSAPI_ERROR_NO_SOCKET;
    }
    
    if (_ssl_ca_pem == NULL)
    {
        // No SSL
        return _tcp_socket->connect(host, port);
    }
    
    // Initialize TLS-related stuf.
    int ret;
    if ((ret = mbedtls_ctr_drbg_seed(&_ctr_drbg, mbedtls_entropy_func, &_entropy,
                      (const unsigned char *) TLS_CUNSTOM,
                      sizeof (TLS_CUNSTOM))) != 0)
    {
        tls_log_error("drbg_seed", ret);
        return -1;
    }

    if ((ret = mbedtls_x509_crt_parse(&_cacert, (const unsigned char *)_ssl_ca_pem,
                       strlen(_ssl_ca_pem) + 1)) != 0)
    {
        tls_log_error("CA cert parse", ret);
        return -1;
    }

    if ((ret = mbedtls_ssl_config_defaults(&_ssl_conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        tls_log_error("ssl_config_defaults", ret);
        return -1;
    }

    mbedtls_ssl_conf_ca_chain(&_ssl_conf, &_cacert, NULL);
    mbedtls_ssl_conf_rng(&_ssl_conf, mbedtls_ctr_drbg_random, &_ctr_drbg);

    /* It is possible to disable authentication by passing
     * MBEDTLS_SSL_VERIFY_NONE in the call to mbedtls_ssl_conf_authmode()
     */
    mbedtls_ssl_conf_authmode(&_ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    
    // Configure client certificate for mutual TLS if provided
    if (_ssl_client_cert != NULL && _ssl_client_key != NULL)
    {
        if ((ret = mbedtls_x509_crt_parse(&_clientcert, (const unsigned char *)_ssl_client_cert,
                           strlen(_ssl_client_cert) + 1)) != 0)
        {
            tls_log_error("client cert parse", ret);
            return -1;
        }
        
        if ((ret = mbedtls_pk_parse_key(&_clientkey, (const unsigned char *)_ssl_client_key,
                         strlen(_ssl_client_key) + 1, NULL, 0)) != 0)
        {
            tls_log_error("private key parse", ret);
            return -1;
        }
        
        if ((ret = mbedtls_ssl_conf_own_cert(&_ssl_conf, &_clientcert, &_clientkey)) != 0)
        {
            tls_log_error("ssl_conf_own_cert", ret);
            return -1;
        }
    }

#if DEBUG_LEVEL > 0
    mbedtls_ssl_conf_verify(&_ssl_conf, my_verify, NULL);
    mbedtls_ssl_conf_dbg(&_ssl_conf, my_debug, NULL);
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

    if ((ret = mbedtls_ssl_setup(&_ssl, &_ssl_conf)) != 0)
    {
        tls_log_error("ssl_setup", ret);
        return -1;
    }
    
    mbedtls_ssl_set_hostname(&_ssl, host);
    
    // IoT Hub SDK style: pass TLSSocket pointer to callbacks for buffer access
    mbedtls_ssl_set_bio(&_ssl, static_cast<void *>(this), ssl_send, ssl_recv, NULL);
    
    /* Connect to the server */
    ret = _tcp_socket->connect(host, port);
    if (ret != NSAPI_ERROR_OK)
    {
        printf("[TLS] TCP connect failed: %d\r\n", ret);
        return ret;
    }
    printf("[TLS] TCP connected, starting handshake...\r\n");
    
    // IoT Hub SDK style: set socket to non-blocking for polling
    _tcp_socket->set_blocking(false);
    _tcp_socket->set_timeout(100);  // Short timeout for polling

    /* Start the handshake - IoT Hub SDK style: loop until complete */
    _handshake_complete = false;
    do
    {
        ret = mbedtls_ssl_handshake(&_ssl);
    } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);
    
    if (ret < 0) 
    {
        tls_log_error("handshake", ret);
        uint32_t flags = mbedtls_ssl_get_verify_result(&_ssl);
        if (flags != 0)
        {
            char vrfy[512];
            mbedtls_x509_crt_verify_info(vrfy, sizeof(vrfy), "  ! ", flags);
            printf("[TLS] verify flags=0x%08X\r\n%s\r\n", (unsigned int)flags, vrfy);
        }
        return -1;
    }
    
    printf("[TLS] Handshake complete.\r\n");
    _handshake_complete = true;
    return NSAPI_ERROR_OK;
}

nsapi_error_t TLSSocket::close()
{
    if (_tcp_socket == NULL)
    {
        return NSAPI_ERROR_NO_SOCKET;
    }
    return _tcp_socket->close();
}

nsapi_size_or_error_t TLSSocket::send(const void *data, nsapi_size_t size)
{
    if (_tcp_socket == NULL)
    {
        return NSAPI_ERROR_NO_SOCKET;
    }
    
    if (_ssl_ca_pem == NULL)
    {
        // No SSL - use direct TCP send with retry
        const unsigned char *ptr = (const unsigned char *)data;
        size_t total_sent = 0;
        
        while (total_sent < size)
        {
            int result = _tcp_socket->send(ptr + total_sent, size - total_sent);
            if (result > 0)
            {
                total_sent += result;
            }
            else if (result == NSAPI_ERROR_WOULD_BLOCK || result == 0)
            {
                wait_ms(100);
            }
            else
            {
                return result;  // Real error
            }
        }
        return (nsapi_size_or_error_t)size;
    }

    // IoT Hub SDK style: loop until all data sent
    const unsigned char *ptr = (const unsigned char *)data;
    int out_left = (int)size;
    
    do
    {
        int ret = mbedtls_ssl_write(&_ssl, ptr + (size - out_left), out_left);
        
        if (ret > 0)
        {
            out_left -= ret;
        }
        else if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            // Non-fatal: retry
            wait_ms(10);
        }
        else
        {
            // Real error
            return ret;
        }
    } while (out_left > 0);
    
    return (nsapi_size_or_error_t)size;
}

nsapi_size_or_error_t TLSSocket::recv(void *data, nsapi_size_t size)
{
    if (_tcp_socket == NULL)
    {
        return NSAPI_ERROR_NO_SOCKET;
    }
    
    if (_ssl_ca_pem == NULL)
    {
        // No SSL
        return _tcp_socket->recv(data, size);
    }

    // IoT Hub SDK style: decode received bytes with retry
    int ret = mbedtls_ssl_read(&_ssl, (unsigned char*)data, size);
    
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
    {
        // No data available - return 0 (not error)
        return 0;
    }
    else if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
    {
        // Graceful close
        return 0;
    }
    
    return ret;
}
