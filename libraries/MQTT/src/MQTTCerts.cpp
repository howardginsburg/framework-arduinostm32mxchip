// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include <stddef.h>
#include "MQTTCerts.h"

/**
 * @file MQTTCerts.cpp
 * @brief Default (empty) certificate definitions
 * 
 * These are set to NULL by default. To use compile-time certificates,
 * override these in your sketch by creating a local MQTTCerts.cpp file
 * with your certificate definitions.
 * 
 * Alternatively, use EEPROMInterface to store certificates securely.
 */

// Default: No CA certificate (must be provided at runtime or from EEPROM)
const char* MQTT_CA_CERT = nullptr;

// Default: No client certificate (use username/password or EEPROM)
const char* MQTT_CLIENT_CERT = nullptr;

// Default: No client private key
const char* MQTT_CLIENT_KEY = nullptr;

// Default: No broker host (must be specified in connect call)
const char* MQTT_BROKER_HOST = nullptr;

// Default: Standard MQTT TLS port
const int MQTT_BROKER_PORT = 8883;
