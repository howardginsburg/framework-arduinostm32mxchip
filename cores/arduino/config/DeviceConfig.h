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
    PROFILE_NONE = 0,               // No EEPROM usage - configuration provided by sketch
    PROFILE_MQTT_USERPASS,          // MQTT with username/password
    PROFILE_MQTT_USERPASS_TLS,      // MQTT with username/password over TLS (server CA cert)
    PROFILE_MQTT_MTLS,              // MQTT with mutual TLS (client cert + key + CA cert)
    PROFILE_IOTHUB_SAS,             // Azure IoT Hub with SAS key (connection string)
    PROFILE_IOTHUB_CERT,            // Azure IoT Hub with X.509 certificate
    PROFILE_DPS_SAS,                // Azure DPS with symmetric key (individual enrollment)
    PROFILE_DPS_CERT,               // Azure DPS with X.509 certificate
    PROFILE_DPS_SAS_GROUP,          // Azure DPS with symmetric key (group enrollment)
    PROFILE_CUSTOM                  // User-defined profile via custom_profile.h
} ConnectionProfile;

/**
 * @brief Preprocessor macros matching enum values for #if directive support
 * 
 * C/C++ enum values cannot be evaluated by the preprocessor in #if directives.
 * These macros shadow the enum names with integer constants so that user code
 * can write: #if CONNECTION_PROFILE == PROFILE_IOTHUB_SAS
 * 
 * Framework implementation files define _DEVICE_CONFIG_IMPL before including
 * this header to suppress these macros and use the type-safe enum values instead.
 */
#ifndef _DEVICE_CONFIG_IMPL
#define PROFILE_NONE              0
#define PROFILE_MQTT_USERPASS     1
#define PROFILE_MQTT_USERPASS_TLS 2
#define PROFILE_MQTT_MTLS         3
#define PROFILE_IOTHUB_SAS        4
#define PROFILE_IOTHUB_CERT       5
#define PROFILE_DPS_SAS           6
#define PROFILE_DPS_CERT          7
#define PROFILE_DPS_SAS_GROUP     8
#define PROFILE_CUSTOM            9
#endif

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
 * @brief Maximum number of zones a setting can span
 */
#define MAX_ZONES_PER_SETTING 3

/**
 * @brief Zone mapping structure - supports multi-zone settings
 * 
 * For single-zone settings, only zones[0] and zoneSizes[0] are used.
 * For multi-zone settings, data is written/read sequentially across zones.
 * A zone index of 0xFF marks end of zone list.
 */
typedef struct {
    uint8_t zones[MAX_ZONES_PER_SETTING];      // STSAFE zone indices (0xFF = unused/end)
    uint16_t zoneSizes[MAX_ZONES_PER_SETTING]; // Size for each zone
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

// Check for user-provided device_config.h to define CONNECTION_PROFILE
#if __has_include("device_config.h")
    #include "device_config.h"
#endif

// Default to PROFILE_NONE if not defined
#ifndef CONNECTION_PROFILE
    #define CONNECTION_PROFILE PROFILE_NONE
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
 * @brief Get the active connection profile type
 * 
 * @return Active ConnectionProfile enum value, or PROFILE_NONE if not initialized
 */
ConnectionProfile DeviceConfig_GetActiveProfile(void);

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

/**
 * @brief Load all configuration values from EEPROM into internal buffers
 * 
 * This function reads WiFi credentials, broker URL, and certificates
 * (based on the active profile) into static buffers that can be accessed
 * via the getter functions.
 * 
 * @return true if all required settings loaded successfully, false otherwise
 */
bool DeviceConfig_LoadAll(void);

/**
 * @brief Get WiFi SSID
 * @return Pointer to WiFi SSID string (valid after DeviceConfig_LoadAll)
 */
const char* DeviceConfig_GetWifiSsid(void);

/**
 * @brief Get WiFi password
 * @return Pointer to WiFi password string (valid after DeviceConfig_LoadAll)
 */
const char* DeviceConfig_GetWifiPassword(void);

/**
 * @brief Get broker/server host
 * @return Pointer to broker host string (valid after DeviceConfig_LoadAll)
 */
const char* DeviceConfig_GetBrokerHost(void);

/**
 * @brief Get broker/server port
 * @return Port number (valid after DeviceConfig_LoadAll)
 */
int DeviceConfig_GetBrokerPort(void);

/**
 * @brief Get CA certificate
 * @return Pointer to CA certificate string (valid after DeviceConfig_LoadAll)
 */
const char* DeviceConfig_GetCACert(void);

/**
 * @brief Get client certificate
 * @return Pointer to client certificate string (valid after DeviceConfig_LoadAll)
 */
const char* DeviceConfig_GetClientCert(void);

/**
 * @brief Get client private key
 * @return Pointer to client key string (valid after DeviceConfig_LoadAll)
 */
const char* DeviceConfig_GetClientKey(void);

/**
 * @brief Get device ID
 * 
 * For mTLS profiles, this is extracted from the CN of the client certificate.
 * For IoT Hub SAS profiles, this is extracted from the connection string.
 * For other profiles, this may be read directly from EEPROM or be empty.
 * 
 * @return Pointer to device ID string (valid after DeviceConfig_LoadAll)
 */
const char* DeviceConfig_GetDeviceId(void);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_CONFIG_H__ */
