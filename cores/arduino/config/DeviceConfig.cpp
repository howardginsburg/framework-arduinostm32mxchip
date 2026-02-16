// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

/**
 * @file DeviceConfig.cpp
 * @brief Core device configuration storage layer
 * 
 * This module provides profile definitions and low-level EEPROM read/write
 * operations. For runtime loading and parsing, see DeviceConfigRuntime.cpp.
 */

#define _DEVICE_CONFIG_IMPL
#include "DeviceConfig.h"
#include "DeviceConfigZones.h"
#include "SettingUI.h"
#include "EEPROMInterface.h"
#include <string.h>

// Check for user-provided custom profile definition
#if __has_include("custom_profile.h")
    #include "custom_profile.h"
#endif

// Current active profile
static const ProfileDefinition* s_activeProfile = NULL;

/**
 * @brief Profile definitions for all supported connection types
 * 
 * Zone allocation follows these rules:
 * - WiFi SSID is always zone 3 (120 bytes)
 * - WiFi password is always zone 10 (88 bytes)
 * - No two settings in a profile share the same zone
 * - Large certificates can span multiple zones
 */
static const ProfileDefinition PROFILES[] = {
    // PROFILE_NONE - No EEPROM usage, sketch provides configuration
    {
        PROFILE_NONE,
        "None",
        "No EEPROM storage - configuration provided by sketch code",
        {
            UNUSED_ZONE,            // SETTING_WIFI_SSID
            UNUSED_ZONE,            // SETTING_WIFI_PASSWORD
            UNUSED_ZONE,            // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            UNUSED_ZONE,            // SETTING_CONNECTION_STRING
            UNUSED_ZONE,            // SETTING_DPS_ENDPOINT
            UNUSED_ZONE,            // SETTING_SCOPE_ID
            UNUSED_ZONE,            // SETTING_REGISTRATION_ID
            UNUSED_ZONE,            // SETTING_SYMMETRIC_KEY
            UNUSED_ZONE             // SETTING_DEVICE_CERT
        }
    },
    
    // PROFILE_MQTT_USERPASS - Basic MQTT with username/password
    {
        PROFILE_MQTT_USERPASS,
        "MQTT Username/Password",
        "MQTT broker with username and password authentication",
        {
            ZONE(3, ZONE_3_SIZE),   // SETTING_WIFI_SSID
            ZONE(10, ZONE_10_SIZE), // SETTING_WIFI_PASSWORD
            ZONE(5, ZONE_5_SIZE),   // SETTING_BROKER_URL
            ZONE(6, ZONE_6_SIZE),   // SETTING_DEVICE_ID
            ZONE(7, ZONE_7_SIZE),   // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            UNUSED_ZONE,            // SETTING_CONNECTION_STRING
            UNUSED_ZONE,            // SETTING_DPS_ENDPOINT
            UNUSED_ZONE,            // SETTING_SCOPE_ID
            UNUSED_ZONE,            // SETTING_REGISTRATION_ID
            UNUSED_ZONE,            // SETTING_SYMMETRIC_KEY
            UNUSED_ZONE             // SETTING_DEVICE_CERT
        }
    },
    
    // PROFILE_MQTT_USERPASS_TLS - MQTT with username/password over TLS
    {
        PROFILE_MQTT_USERPASS_TLS,
        "MQTT Username/Password over TLS",
        "MQTT broker with username/password over TLS with server certificate verification",
        {
            ZONE(3, ZONE_3_SIZE),   // SETTING_WIFI_SSID
            ZONE(10, ZONE_10_SIZE), // SETTING_WIFI_PASSWORD
            ZONE(5, ZONE_5_SIZE),   // SETTING_BROKER_URL
            ZONE(2, ZONE_2_SIZE),   // SETTING_DEVICE_ID
            ZONE(6, ZONE_6_SIZE),   // SETTING_DEVICE_PASSWORD
            // CA_CERT spans zones 0+7+8 = 2640 bytes total
            ZONE3(0, ZONE_0_SIZE, 7, ZONE_7_SIZE, 8, ZONE_8_SIZE),  // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            UNUSED_ZONE,            // SETTING_CONNECTION_STRING
            UNUSED_ZONE,            // SETTING_DPS_ENDPOINT
            UNUSED_ZONE,            // SETTING_SCOPE_ID
            UNUSED_ZONE,            // SETTING_REGISTRATION_ID
            UNUSED_ZONE,            // SETTING_SYMMETRIC_KEY
            UNUSED_ZONE             // SETTING_DEVICE_CERT
        }
    },
    
    // PROFILE_MQTT_MTLS - MQTT with mutual TLS
    {
        PROFILE_MQTT_MTLS,
        "MQTT Mutual TLS",
        "MQTT broker with mutual TLS (client certificate authentication)",
        {
            ZONE(3, ZONE_3_SIZE),   // SETTING_WIFI_SSID
            ZONE(10, ZONE_10_SIZE), // SETTING_WIFI_PASSWORD
            ZONE(5, ZONE_5_SIZE),   // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID (extracted from cert CN)
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            ZONE(0, ZONE_0_SIZE),   // SETTING_CA_CERT
            // CLIENT_CERT spans zones 6+7 = 1464 bytes total
            ZONE2(6, ZONE_6_SIZE, 7, ZONE_7_SIZE),  // SETTING_CLIENT_CERT
            ZONE(8, ZONE_8_SIZE),   // SETTING_CLIENT_KEY
            UNUSED_ZONE,            // SETTING_CONNECTION_STRING
            UNUSED_ZONE,            // SETTING_DPS_ENDPOINT
            UNUSED_ZONE,            // SETTING_SCOPE_ID
            UNUSED_ZONE,            // SETTING_REGISTRATION_ID
            UNUSED_ZONE,            // SETTING_SYMMETRIC_KEY
            UNUSED_ZONE             // SETTING_DEVICE_CERT
        }
    },
    
    // PROFILE_IOTHUB_SAS - Azure IoT Hub with SAS authentication
    {
        PROFILE_IOTHUB_SAS,
        "Azure IoT Hub (SAS)",
        "Azure IoT Hub with connection string (SAS token authentication)",
        {
            ZONE(3, ZONE_3_SIZE),   // SETTING_WIFI_SSID
            ZONE(10, ZONE_10_SIZE), // SETTING_WIFI_PASSWORD
            UNUSED_ZONE,            // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            ZONE(5, ZONE_5_SIZE),   // SETTING_CONNECTION_STRING
            UNUSED_ZONE,            // SETTING_DPS_ENDPOINT
            UNUSED_ZONE,            // SETTING_SCOPE_ID
            UNUSED_ZONE,            // SETTING_REGISTRATION_ID
            UNUSED_ZONE,            // SETTING_SYMMETRIC_KEY
            UNUSED_ZONE             // SETTING_DEVICE_CERT
        }
    },
    
    // PROFILE_IOTHUB_CERT - Azure IoT Hub with X.509 certificate
    {
        PROFILE_IOTHUB_CERT,
        "Azure IoT Hub (X.509)",
        "Azure IoT Hub with X.509 certificate authentication",
        {
            ZONE(3, ZONE_3_SIZE),   // SETTING_WIFI_SSID
            ZONE(10, ZONE_10_SIZE), // SETTING_WIFI_PASSWORD
            UNUSED_ZONE,            // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            ZONE(5, ZONE_5_SIZE),   // SETTING_CONNECTION_STRING
            UNUSED_ZONE,            // SETTING_DPS_ENDPOINT
            UNUSED_ZONE,            // SETTING_SCOPE_ID
            UNUSED_ZONE,            // SETTING_REGISTRATION_ID
            UNUSED_ZONE,            // SETTING_SYMMETRIC_KEY
            // DEVICE_CERT spans zones 0+7+8 = 2640 bytes total
            ZONE3(0, ZONE_0_SIZE, 7, ZONE_7_SIZE, 8, ZONE_8_SIZE)  // SETTING_DEVICE_CERT
        }
    },
    
    // PROFILE_DPS_SAS - Azure DPS with symmetric key
    {
        PROFILE_DPS_SAS,
        "Azure DPS (Symmetric Key)",
        "Azure Device Provisioning Service with symmetric key authentication",
        {
            ZONE(3, ZONE_3_SIZE),   // SETTING_WIFI_SSID
            ZONE(10, ZONE_10_SIZE), // SETTING_WIFI_PASSWORD
            UNUSED_ZONE,            // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            UNUSED_ZONE,            // SETTING_CONNECTION_STRING
            ZONE(5, ZONE_5_SIZE),   // SETTING_DPS_ENDPOINT
            ZONE(2, ZONE_2_SIZE),   // SETTING_SCOPE_ID
            ZONE(6, ZONE_6_SIZE),   // SETTING_REGISTRATION_ID
            ZONE(7, ZONE_7_SIZE),   // SETTING_SYMMETRIC_KEY
            UNUSED_ZONE             // SETTING_DEVICE_CERT
        }
    },
    
    // PROFILE_DPS_CERT - Azure DPS with X.509 certificate
    {
        PROFILE_DPS_CERT,
        "Azure DPS (X.509)",
        "Azure Device Provisioning Service with X.509 certificate authentication",
        {
            ZONE(3, ZONE_3_SIZE),   // SETTING_WIFI_SSID
            ZONE(10, ZONE_10_SIZE), // SETTING_WIFI_PASSWORD
            UNUSED_ZONE,            // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            UNUSED_ZONE,            // SETTING_CONNECTION_STRING
            ZONE(2, ZONE_2_SIZE),   // SETTING_DPS_ENDPOINT
            ZONE(5, ZONE_5_SIZE),   // SETTING_SCOPE_ID
            ZONE(6, ZONE_6_SIZE),   // SETTING_REGISTRATION_ID
            UNUSED_ZONE,            // SETTING_SYMMETRIC_KEY
            // DEVICE_CERT spans zones 0+7+8 = 2640 bytes total
            ZONE3(0, ZONE_0_SIZE, 7, ZONE_7_SIZE, 8, ZONE_8_SIZE)  // SETTING_DEVICE_CERT
        }
    },
    
    // PROFILE_DPS_SAS_GROUP - Azure DPS with symmetric key (group enrollment)
    {
        PROFILE_DPS_SAS_GROUP,
        "Azure DPS (Group SAS)",
        "Azure Device Provisioning Service with group symmetric key (derived device key)",
        {
            ZONE(3, ZONE_3_SIZE),   // SETTING_WIFI_SSID
            ZONE(10, ZONE_10_SIZE), // SETTING_WIFI_PASSWORD
            UNUSED_ZONE,            // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            UNUSED_ZONE,            // SETTING_CONNECTION_STRING
            ZONE(5, ZONE_5_SIZE),   // SETTING_DPS_ENDPOINT
            ZONE(2, ZONE_2_SIZE),   // SETTING_SCOPE_ID
            ZONE(6, ZONE_6_SIZE),   // SETTING_REGISTRATION_ID
            ZONE(7, ZONE_7_SIZE),   // SETTING_SYMMETRIC_KEY
            UNUSED_ZONE             // SETTING_DEVICE_CERT
        }
    }
};

void DeviceConfig_Init(ConnectionProfile profile)
{
#if __has_include("custom_profile.h")
    if (profile == PROFILE_CUSTOM)
    {
        s_activeProfile = &CUSTOM_PROFILE;
#ifdef CUSTOM_PROFILE_UI_COUNT
        SettingUI_SetCustomUI(CUSTOM_PROFILE_UI, CUSTOM_PROFILE_UI_COUNT);
#endif
        return;
    }
#endif
    if (profile < sizeof(PROFILES) / sizeof(ProfileDefinition))
    {
        s_activeProfile = &PROFILES[profile];
    }
}

const char* DeviceConfig_GetProfileName(void)
{
    if (s_activeProfile != NULL)
    {
        return s_activeProfile->name;
    }
    return "Unknown";
}

bool DeviceConfig_IsSettingAvailable(SettingID setting)
{
    if (s_activeProfile == NULL || setting >= SETTING_COUNT)
    {
        return false;
    }
    // A setting is available if its first zone is not 0xFF
    return s_activeProfile->mappings[setting].zones[0] != 0xFF;
}

int DeviceConfig_GetMaxLen(SettingID setting)
{
    if (!DeviceConfig_IsSettingAvailable(setting))
    {
        return -1;
    }
    // Sum up all zone sizes
    const ZoneMapping* mapping = &s_activeProfile->mappings[setting];
    int totalLen = 0;
    for (int i = 0; i < MAX_ZONES_PER_SETTING && mapping->zones[i] != 0xFF; i++)
    {
        totalLen += mapping->zoneSizes[i];
    }
    return totalLen;
}

int DeviceConfig_Save(SettingID setting, const char* value)
{
    if (!DeviceConfig_IsSettingAvailable(setting) || value == NULL)
    {
        return -1;
    }
    
    EEPROMInterface eeprom;
    const ZoneMapping* mapping = &s_activeProfile->mappings[setting];
    int len = strlen(value) + 1;  // Include null terminator
    int maxLen = DeviceConfig_GetMaxLen(setting);
    
    if (len > maxLen)
    {
        return -1;
    }
    
    // Write data across all configured zones
    int written = 0;
    int remaining = len;
    
    for (int i = 0; i < MAX_ZONES_PER_SETTING && mapping->zones[i] != 0xFF && remaining > 0; i++)
    {
        int zoneSize = mapping->zoneSizes[i];
        int toWrite = (remaining > zoneSize) ? zoneSize : remaining;
        
        if (eeprom.write((uint8_t*)(value + written), toWrite, mapping->zones[i]) != 0)
        {
            return -1;
        }
        
        written += toWrite;
        remaining -= toWrite;
    }
    
    return 0;
}

int DeviceConfig_Read(SettingID setting, char* buffer, int bufferSize)
{
    if (!DeviceConfig_IsSettingAvailable(setting) || buffer == NULL || bufferSize <= 0)
    {
        return -1;
    }
    
    EEPROMInterface eeprom;
    const ZoneMapping* mapping = &s_activeProfile->mappings[setting];
    
    // Read data from all configured zones
    int totalRead = 0;
    int remaining = bufferSize;
    
    for (int i = 0; i < MAX_ZONES_PER_SETTING && mapping->zones[i] != 0xFF && remaining > 0; i++)
    {
        int zoneSize = mapping->zoneSizes[i];
        int toRead = (remaining > zoneSize) ? zoneSize : remaining;
        
        if (eeprom.read((uint8_t*)(buffer + totalRead), toRead, 0, mapping->zones[i]) == -1)
        {
            return -1;
        }
        
        totalRead += toRead;
        remaining -= toRead;
    }
    
    // Ensure null termination
    buffer[bufferSize - 1] = '\0';
    return totalRead;
}

ConnectionProfile DeviceConfig_GetActiveProfile(void)
{
    if (s_activeProfile != NULL)
    {
        return s_activeProfile->profile;
    }
    return PROFILE_NONE;
}
