// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef _MQTT_CERTS_H_
#define _MQTT_CERTS_H_

/**
 * @file MQTTCerts.h
 * @brief Compile-time certificate storage for MQTT TLS connections
 * 
 * This file provides a way to embed certificates at compile time.
 * To use your own certificates, create a copy of MQTTCerts.cpp in your
 * sketch folder and define your certificates there.
 * 
 * The certificates should be in PEM format (null-terminated strings).
 * 
 * Example:
 * @code
 * // In your sketch's MQTTCerts.cpp:
 * #include "MQTTCerts.h"
 * 
 * const char* MQTT_CA_CERT = 
 *     "-----BEGIN CERTIFICATE-----\n"
 *     "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQsFADBs\n"
 *     // ... rest of certificate ...
 *     "-----END CERTIFICATE-----\n";
 * 
 * const char* MQTT_CLIENT_CERT = 
 *     "-----BEGIN CERTIFICATE-----\n"
 *     // ... your client certificate ...
 *     "-----END CERTIFICATE-----\n";
 * 
 * const char* MQTT_CLIENT_KEY = 
 *     "-----BEGIN RSA PRIVATE KEY-----\n"
 *     // ... your private key ...
 *     "-----END RSA PRIVATE KEY-----\n";
 * @endcode
 */

/**
 * @brief CA certificate for server verification (PEM format)
 * 
 * Common CA certificates for cloud MQTT brokers:
 * - Azure IoT Hub: DigiCert Global Root G2
 * - AWS IoT Core: Amazon Root CA 1
 * - Mosquitto: Varies by server configuration
 * 
 * Set to NULL to use EEPROM-stored certificate instead.
 */
extern const char* MQTT_CA_CERT;

/**
 * @brief Client certificate for mutual TLS (PEM format)
 * 
 * Used for client authentication in place of username/password.
 * Set to NULL if using username/password authentication or EEPROM storage.
 */
extern const char* MQTT_CLIENT_CERT;

/**
 * @brief Client private key for mutual TLS (PEM format)
 * 
 * Must correspond to MQTT_CLIENT_CERT.
 * WARNING: Embedding private keys in firmware has security implications.
 * Consider using EEPROM secure storage for production deployments.
 */
extern const char* MQTT_CLIENT_KEY;

/**
 * @brief Default MQTT broker hostname
 * 
 * Set to NULL to require explicit hostname in connect calls.
 */
extern const char* MQTT_BROKER_HOST;

/**
 * @brief Default MQTT broker port
 * 
 * Common ports:
 * - 1883: MQTT (unencrypted)
 * - 8883: MQTT over TLS
 * - 443: MQTT over TLS (for firewall traversal)
 */
extern const int MQTT_BROKER_PORT;

#endif // _MQTT_CERTS_H_
