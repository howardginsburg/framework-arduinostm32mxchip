// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __DEVICE_CONFIG_H__
#define __DEVICE_CONFIG_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Connection profile types supported by the framework
 */
typedef enum {
    PROFILE_MQTT_USERPASS = 0,      // MQTT with username/password
    PROFILE_MQTT_USERPASS_TLS,      // MQTT with username/password over TLS (server CA cert)
    PROFILE_MQTT_MTLS,              // MQTT with mutual TLS (client cert + key + CA cert)
    PROFILE_IOTHUB_SAS,             // Azure IoT Hub with SAS key (connection string)
    PROFILE_IOTHUB_CERT,            // Azure IoT Hub with X.509 certificate
    PROFILE_DPS_SAS,                // Azure DPS with symmetric key
    PROFILE_DPS_CERT                // Azure DPS with X.509 certificate
} ConnectionProfile;

/**
 * @brief Setting identifiers for device configuration
 */
typedef enum {
    SETTING_WIFI_SSID = 0,          // WiFi SSID (always zone 3)
    SETTING_WIFI_PASSWORD,          // WiFi password (always zone 10)
    SETTING_BROKER_URL,             // MQTT broker address
    SETTING_DEVICE_ID,              // MQTT client/device ID
    SETTING_DEVICE_PASSWORD,        // Device password
    SETTING_CA_CERT,                // CA/server certificate (can span multiple zones)
    SETTING_CLIENT_CERT,            // Client certificate for mTLS
    SETTING_CLIENT_KEY,             // Client private key for mTLS
    SETTING_CONNECTION_STRING,      // IoT Hub connection string
    SETTING_DPS_ENDPOINT,           // DPS endpoint URL
    SETTING_SCOPE_ID,               // DPS scope ID
    SETTING_REGISTRATION_ID,        // DPS registration ID
    SETTING_SYMMETRIC_KEY,          // DPS symmetric key
    SETTING_DEVICE_CERT,            // Large device certificate spanning multiple zones
    SETTING_COUNT                   // Sentinel value - number of settings
} SettingID;

/**
 * @brief Zone mapping structure
 */
typedef struct {
    uint8_t zoneIndex;              // STSAFE zone index (0xFF = not available)
    uint16_t maxLen;                // Maximum length for this setting
    uint16_t offset;                // Offset within zone (0 for single zone, >0 for multi-zone)
} ZoneMapping;

/**
 * @brief Profile definition structure
 */
typedef struct {
    ConnectionProfile profile;
    const char* name;
    const char* description;
    ZoneMapping mappings[SETTING_COUNT];
} ProfileDefinition;

// Check for user-provided device_config.h
#if __has_include("device_config.h")
    #include "device_config.h"
#else
    #include "device_config_default.h"
#endif

// Validate that CONNECTION_PROFILE is defined
#ifndef CONNECTION_PROFILE
    #error "CONNECTION_PROFILE must be defined in device_config.h or device_config_default.h"
#endif

/**
 * @brief Initialize the device configuration system
 * 
 * @param profile Connection profile to use
 */
void DeviceConfig_Init(ConnectionProfile profile);

/**
 * @brief Get the name of the active connection profile
 * 
 * @return Profile name string
 */
const char* DeviceConfig_GetProfileName(void);

/**
 * @brief Check if a setting is available in the active profile
 * 
 * @param setting Setting ID to check
 * @return true if setting is available, false otherwise
 */
bool DeviceConfig_IsSettingAvailable(SettingID setting);

/**
 * @brief Get the maximum length for a setting
 * 
 * @param setting Setting ID
 * @return Maximum length in bytes, or -1 if setting not available
 */
int DeviceConfig_GetMaxLen(SettingID setting);

/**
 * @brief Save a setting value to EEPROM
 * 
 * @param setting Setting ID
 * @param value Value to save (null-terminated string)
 * @return 0 on success, -1 on failure
 */
int DeviceConfig_Save(SettingID setting, const char* value);

/**
 * @brief Read a setting value from EEPROM
 * 
 * @param setting Setting ID
 * @param buffer Buffer to store the value
 * @param bufferSize Size of the buffer
 * @return Number of bytes read on success, -1 on failure
 */
int DeviceConfig_Read(SettingID setting, char* buffer, int bufferSize);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_CONFIG_H__ */
