// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef _MQTTNETWORKTLS_H_
#define _MQTTNETWORKTLS_H_

#include "NetworkInterface.h"
#include "MQTTmbed.h"
#include "SystemWiFi.h"
#include "TLSSocket.h"

/**
 * @class MQTTNetworkTLS
 * @brief TLS-enabled network transport for MQTT Client
 * 
 * This class provides a secure network transport layer for use with MQTT::Client.
 * It supports both server-only TLS (one-way authentication) and mutual TLS 
 * (two-way authentication with client certificates).
 * 
 * Usage with MQTT::Client:
 *   MQTTNetworkTLS network(caCert);  // Server-only TLS
 *   // or
 *   MQTTNetworkTLS network(caCert, clientCert, clientKey);  // Mutual TLS
 *   
 *   MQTT::Client<MQTTNetworkTLS, Countdown> client(network);
 *   network.connect("broker.example.com", 8883);
 *   client.connect(options);
 */
class MQTTNetworkTLS
{
public:
    /**
     * @brief Construct MQTTNetworkTLS for server-only TLS (one-way authentication)
     * @param ssl_ca_pem  CA certificate in PEM format (null-terminated string)
     *                    Use NULL for insecure connection (not recommended)
     */
    MQTTNetworkTLS(const char *ssl_ca_pem = NULL)
    {
        _ssl_ca_pem = ssl_ca_pem;
        _ssl_client_cert = NULL;
        _ssl_client_key = NULL;
        _tlsSocket = NULL;
    }

    /**
     * @brief Construct MQTTNetworkTLS for mutual TLS (two-way authentication)
     * @param ssl_ca_pem        CA certificate in PEM format (null-terminated string)
     * @param ssl_client_cert   Client certificate in PEM format (null-terminated string)
     * @param ssl_client_key    Client private key in PEM format (null-terminated string)
     */
    MQTTNetworkTLS(const char *ssl_ca_pem, const char *ssl_client_cert, 
                   const char *ssl_client_key)
    {
        _ssl_ca_pem = ssl_ca_pem;
        _ssl_client_cert = ssl_client_cert;
        _ssl_client_key = ssl_client_key;
        _tlsSocket = NULL;
    }

    ~MQTTNetworkTLS()
    {
        disconnect();
    }

    /**
     * @brief Read data from the TLS socket
     * @param buffer  Buffer to store received data
     * @param len     Maximum number of bytes to read
     * @param timeout Timeout in milliseconds
     * @return Number of bytes read, or negative error code
     */
    int read(unsigned char *buffer, int len, int timeout)
    {
        if (_tlsSocket == NULL)
        {
            return NSAPI_ERROR_NO_SOCKET;
        }

        // TLSSocket doesn't have set_timeout, so we need to handle timeout differently
        // For now, perform a simple blocking read
        // TODO: Consider implementing timeout with Timer class
        (void)timeout;  // timeout not directly supported by TLSSocket
        return _tlsSocket->recv(buffer, len);
    }

    /**
     * @brief Write data to the TLS socket
     * @param buffer  Buffer containing data to send
     * @param len     Number of bytes to send
     * @param timeout Timeout in milliseconds
     * @return Number of bytes written, or negative error code
     */
    int write(unsigned char *buffer, int len, int timeout)
    {
        if (_tlsSocket == NULL)
        {
            return NSAPI_ERROR_NO_SOCKET;
        }

        (void)timeout;  // timeout not directly supported by TLSSocket
        return _tlsSocket->send(buffer, len);
    }

    /**
     * @brief Connect to MQTT broker over TLS
     * @param hostname  Broker hostname
     * @param port      Broker port (typically 8883 for MQTT over TLS)
     * @return 0 on success, negative error code on failure
     */
    int connect(const char *hostname, int port)
    {
        if (_tlsSocket != NULL)
        {
            // Already connected
            return NSAPI_ERROR_OK;
        }

        NetworkInterface* netIface = WiFiInterface();
        if (netIface == NULL)
        {
            return NSAPI_ERROR_NO_CONNECTION;
        }

        // Create TLS socket based on configuration
        if (_ssl_client_cert != NULL && _ssl_client_key != NULL)
        {
            // Mutual TLS
            _tlsSocket = new TLSSocket(_ssl_ca_pem, _ssl_client_cert, 
                                       _ssl_client_key, netIface);
        }
        else
        {
            // Server-only TLS
            _tlsSocket = new TLSSocket(_ssl_ca_pem, netIface);
        }

        if (_tlsSocket == NULL)
        {
            return NSAPI_ERROR_NO_MEMORY;
        }

        int ret = _tlsSocket->connect(hostname, (uint16_t)port);
        if (ret != NSAPI_ERROR_OK)
        {
            delete _tlsSocket;
            _tlsSocket = NULL;
            return ret;
        }

        return NSAPI_ERROR_OK;
    }

    /**
     * @brief Disconnect from the MQTT broker
     * @return 0 on success
     */
    int disconnect()
    {
        if (_tlsSocket != NULL)
        {
            _tlsSocket->close();
            delete _tlsSocket;
            _tlsSocket = NULL;
        }
        return NSAPI_ERROR_OK;
    }

    /**
     * @brief Check if connected
     * @return true if connected
     */
    bool isConnected() const
    {
        return _tlsSocket != NULL;
    }

    /**
     * @brief Check if mutual TLS is configured
     * @return true if client certificate is set
     */
    bool isMutualTLS() const
    {
        return _ssl_client_cert != NULL && _ssl_client_key != NULL;
    }

private:
    TLSSocket *_tlsSocket;
    const char *_ssl_ca_pem;
    const char *_ssl_client_cert;
    const char *_ssl_client_key;
};

#endif // _MQTTNETWORKTLS_H_
