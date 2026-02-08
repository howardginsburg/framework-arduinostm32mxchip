// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "DeviceConfig.h"
#include "EEPROMInterface.h"
#include <string.h>
#include <stdlib.h>

// Marker for unused zones in profile mappings
#define UNUSED_ZONE {0xFF, 0, 0}

// STSAFE zone sizes (hardware constants)
#define ZONE_0_SIZE  976
#define ZONE_2_SIZE  192
#define ZONE_3_SIZE  120
#define ZONE_5_SIZE  584
#define ZONE_6_SIZE  680
#define ZONE_7_SIZE  784
#define ZONE_8_SIZE  880
#define ZONE_10_SIZE 88

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
    // PROFILE_MQTT_USERPASS - Basic MQTT with username/password
    {
        PROFILE_MQTT_USERPASS,
        "MQTT Username/Password",
        "MQTT broker with username and password authentication",
        {
            {3, ZONE_3_SIZE, 0},    // SETTING_WIFI_SSID
            {10, ZONE_10_SIZE, 0},  // SETTING_WIFI_PASSWORD
            {5, ZONE_5_SIZE, 0},    // SETTING_BROKER_URL
            {6, ZONE_6_SIZE, 0},    // SETTING_DEVICE_ID
            {7, ZONE_7_SIZE, 0},    // SETTING_DEVICE_PASSWORD
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
            {3, ZONE_3_SIZE, 0},    // SETTING_WIFI_SSID
            {10, ZONE_10_SIZE, 0},  // SETTING_WIFI_PASSWORD
            {5, ZONE_5_SIZE, 0},    // SETTING_BROKER_URL
            {2, ZONE_2_SIZE, 0},    // SETTING_DEVICE_ID
            {6, ZONE_6_SIZE, 0},    // SETTING_DEVICE_PASSWORD
            // CA_CERT spans zones 0+7+8 = 2640 bytes total
            {0, ZONE_0_SIZE + ZONE_7_SIZE + ZONE_8_SIZE, 0},  // SETTING_CA_CERT
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
            {3, ZONE_3_SIZE, 0},    // SETTING_WIFI_SSID
            {10, ZONE_10_SIZE, 0},  // SETTING_WIFI_PASSWORD
            {5, ZONE_5_SIZE, 0},    // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID (not needed for mTLS)
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            {0, ZONE_0_SIZE, 0},    // SETTING_CA_CERT
            // CLIENT_CERT spans zones 6+7 = 1464 bytes total
            {6, ZONE_6_SIZE + ZONE_7_SIZE, 0},  // SETTING_CLIENT_CERT
            {8, ZONE_8_SIZE, 0},    // SETTING_CLIENT_KEY
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
            {3, ZONE_3_SIZE, 0},    // SETTING_WIFI_SSID
            {10, ZONE_10_SIZE, 0},  // SETTING_WIFI_PASSWORD
            UNUSED_ZONE,            // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            {5, ZONE_5_SIZE, 0},    // SETTING_CONNECTION_STRING
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
            {3, ZONE_3_SIZE, 0},    // SETTING_WIFI_SSID
            {10, ZONE_10_SIZE, 0},  // SETTING_WIFI_PASSWORD
            UNUSED_ZONE,            // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            {5, ZONE_5_SIZE, 0},    // SETTING_CONNECTION_STRING
            UNUSED_ZONE,            // SETTING_DPS_ENDPOINT
            UNUSED_ZONE,            // SETTING_SCOPE_ID
            UNUSED_ZONE,            // SETTING_REGISTRATION_ID
            UNUSED_ZONE,            // SETTING_SYMMETRIC_KEY
            // DEVICE_CERT spans zones 0+7+8 = 2640 bytes total
            {0, ZONE_0_SIZE + ZONE_7_SIZE + ZONE_8_SIZE, 0}   // SETTING_DEVICE_CERT
        }
    },
    
    // PROFILE_DPS_SAS - Azure DPS with symmetric key
    {
        PROFILE_DPS_SAS,
        "Azure DPS (Symmetric Key)",
        "Azure Device Provisioning Service with symmetric key authentication",
        {
            {3, ZONE_3_SIZE, 0},    // SETTING_WIFI_SSID
            {10, ZONE_10_SIZE, 0},  // SETTING_WIFI_PASSWORD
            UNUSED_ZONE,            // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            UNUSED_ZONE,            // SETTING_CONNECTION_STRING
            {5, ZONE_5_SIZE, 0},    // SETTING_DPS_ENDPOINT
            {2, ZONE_2_SIZE, 0},    // SETTING_SCOPE_ID
            {6, ZONE_6_SIZE, 0},    // SETTING_REGISTRATION_ID
            {7, ZONE_7_SIZE, 0},    // SETTING_SYMMETRIC_KEY
            UNUSED_ZONE             // SETTING_DEVICE_CERT
        }
    },
    
    // PROFILE_DPS_CERT - Azure DPS with X.509 certificate
    {
        PROFILE_DPS_CERT,
        "Azure DPS (X.509)",
        "Azure Device Provisioning Service with X.509 certificate authentication",
        {
            {3, ZONE_3_SIZE, 0},    // SETTING_WIFI_SSID
            {10, ZONE_10_SIZE, 0},  // SETTING_WIFI_PASSWORD
            UNUSED_ZONE,            // SETTING_BROKER_URL
            UNUSED_ZONE,            // SETTING_DEVICE_ID
            UNUSED_ZONE,            // SETTING_DEVICE_PASSWORD
            UNUSED_ZONE,            // SETTING_CA_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_CERT
            UNUSED_ZONE,            // SETTING_CLIENT_KEY
            UNUSED_ZONE,            // SETTING_CONNECTION_STRING
            {2, ZONE_2_SIZE, 0},    // SETTING_DPS_ENDPOINT
            {5, ZONE_5_SIZE, 0},    // SETTING_SCOPE_ID
            {6, ZONE_6_SIZE, 0},    // SETTING_REGISTRATION_ID
            UNUSED_ZONE,            // SETTING_SYMMETRIC_KEY
            // DEVICE_CERT spans zones 0+7+8 = 2640 bytes total
            {0, ZONE_0_SIZE + ZONE_7_SIZE + ZONE_8_SIZE, 0}   // SETTING_DEVICE_CERT
        }
    }
};

void DeviceConfig_Init(ConnectionProfile profile)
{
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
    return s_activeProfile->mappings[setting].zoneIndex != 0xFF;
}

int DeviceConfig_GetMaxLen(SettingID setting)
{
    if (!DeviceConfig_IsSettingAvailable(setting))
    {
        return -1;
    }
    return s_activeProfile->mappings[setting].maxLen;
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
    
    if (len > mapping->maxLen)
    {
        return -1;
    }
    
    // Check if this is a multi-zone setting (CA_CERT, CLIENT_CERT, or DEVICE_CERT)
    if (setting == SETTING_CA_CERT && s_activeProfile->profile == PROFILE_MQTT_USERPASS_TLS)
    {
        // CA_CERT spans zones 0+7+8 for MQTT_USERPASS_TLS
        int written = 0;
        int remaining = len;
        
        // Write to zone 0 first (976 bytes)
        int size = (remaining > ZONE_0_SIZE) ? ZONE_0_SIZE : remaining;
        if (eeprom.write((uint8_t*)value, size, 0) != 0)
        {
            return -1;
        }
        written += size;
        remaining -= size;
        
        // Write to zone 7 if needed (784 bytes)
        if (remaining > 0)
        {
            size = (remaining > ZONE_7_SIZE) ? ZONE_7_SIZE : remaining;
            if (eeprom.write((uint8_t*)(value + written), size, 7) != 0)
            {
                return -1;
            }
            written += size;
            remaining -= size;
        }
        
        // Write to zone 8 if needed (880 bytes)
        if (remaining > 0)
        {
            size = (remaining > ZONE_8_SIZE) ? ZONE_8_SIZE : remaining;
            if (eeprom.write((uint8_t*)(value + written), size, 8) != 0)
            {
                return -1;
            }
        }
        
        return 0;
    }
    else if (setting == SETTING_CLIENT_CERT && s_activeProfile->profile == PROFILE_MQTT_MTLS)
    {
        // CLIENT_CERT spans zones 6+7 for MQTT_MTLS
        int written = 0;
        int remaining = len;
        
        // Write to zone 6 first (680 bytes)
        int size = (remaining > ZONE_6_SIZE) ? ZONE_6_SIZE : remaining;
        if (eeprom.write((uint8_t*)value, size, 6) != 0)
        {
            return -1;
        }
        written += size;
        remaining -= size;
        
        // Write to zone 7 if needed (784 bytes)
        if (remaining > 0)
        {
            size = (remaining > ZONE_7_SIZE) ? ZONE_7_SIZE : remaining;
            if (eeprom.write((uint8_t*)(value + written), size, 7) != 0)
            {
                return -1;
            }
        }
        
        return 0;
    }
    else if (setting == SETTING_DEVICE_CERT && 
             (s_activeProfile->profile == PROFILE_IOTHUB_CERT || 
              s_activeProfile->profile == PROFILE_DPS_CERT))
    {
        // DEVICE_CERT spans zones 0+7+8 for IOTHUB_CERT and DPS_CERT
        int written = 0;
        int remaining = len;
        
        // Write to zone 0 first (976 bytes)
        int size = (remaining > ZONE_0_SIZE) ? ZONE_0_SIZE : remaining;
        if (eeprom.write((uint8_t*)value, size, 0) != 0)
        {
            return -1;
        }
        written += size;
        remaining -= size;
        
        // Write to zone 7 if needed (784 bytes)
        if (remaining > 0)
        {
            size = (remaining > ZONE_7_SIZE) ? ZONE_7_SIZE : remaining;
            if (eeprom.write((uint8_t*)(value + written), size, 7) != 0)
            {
                return -1;
            }
            written += size;
            remaining -= size;
        }
        
        // Write to zone 8 if needed (880 bytes)
        if (remaining > 0)
        {
            size = (remaining > ZONE_8_SIZE) ? ZONE_8_SIZE : remaining;
            if (eeprom.write((uint8_t*)(value + written), size, 8) != 0)
            {
                return -1;
            }
        }
        
        return 0;
    }
    else
    {
        // Single-zone setting
        return eeprom.write((uint8_t*)value, len, mapping->zoneIndex);
    }
}

int DeviceConfig_Read(SettingID setting, char* buffer, int bufferSize)
{
    if (!DeviceConfig_IsSettingAvailable(setting) || buffer == NULL || bufferSize <= 0)
    {
        return -1;
    }
    
    EEPROMInterface eeprom;
    const ZoneMapping* mapping = &s_activeProfile->mappings[setting];
    
    // Check if this is a multi-zone setting
    if (setting == SETTING_CA_CERT && s_activeProfile->profile == PROFILE_MQTT_USERPASS_TLS)
    {
        // CA_CERT spans zones 0+7+8 for MQTT_USERPASS_TLS
        int totalRead = 0;
        int remaining = bufferSize;
        
        // Read from zone 0 first (976 bytes)
        int size = (remaining > ZONE_0_SIZE) ? ZONE_0_SIZE : remaining;
        if (eeprom.read((uint8_t*)buffer, size, 0, 0) == -1)
        {
            return -1;
        }
        totalRead += size;
        remaining -= size;
        
        // Read from zone 7 if needed (784 bytes)
        if (remaining > 0)
        {
            size = (remaining > ZONE_7_SIZE) ? ZONE_7_SIZE : remaining;
            if (eeprom.read((uint8_t*)(buffer + totalRead), size, 0, 7) == -1)
            {
                return -1;
            }
            totalRead += size;
            remaining -= size;
        }
        
        // Read from zone 8 if needed (880 bytes)
        if (remaining > 0)
        {
            size = (remaining > ZONE_8_SIZE) ? ZONE_8_SIZE : remaining;
            if (eeprom.read((uint8_t*)(buffer + totalRead), size, 0, 8) == -1)
            {
                return -1;
            }
            totalRead += size;
        }
        
        buffer[bufferSize - 1] = '\0';
        return totalRead;
    }
    else if (setting == SETTING_CLIENT_CERT && s_activeProfile->profile == PROFILE_MQTT_MTLS)
    {
        // CLIENT_CERT spans zones 6+7 for MQTT_MTLS
        int totalRead = 0;
        int remaining = bufferSize;
        
        // Read from zone 6 first (680 bytes)
        int size = (remaining > ZONE_6_SIZE) ? ZONE_6_SIZE : remaining;
        if (eeprom.read((uint8_t*)buffer, size, 0, 6) == -1)
        {
            return -1;
        }
        totalRead += size;
        remaining -= size;
        
        // Read from zone 7 if needed (784 bytes)
        if (remaining > 0)
        {
            size = (remaining > ZONE_7_SIZE) ? ZONE_7_SIZE : remaining;
            if (eeprom.read((uint8_t*)(buffer + totalRead), size, 0, 7) == -1)
            {
                return -1;
            }
            totalRead += size;
        }
        
        buffer[bufferSize - 1] = '\0';
        return totalRead;
    }
    else if (setting == SETTING_DEVICE_CERT && 
             (s_activeProfile->profile == PROFILE_IOTHUB_CERT || 
              s_activeProfile->profile == PROFILE_DPS_CERT))
    {
        // DEVICE_CERT spans zones 0+7+8 for IOTHUB_CERT and DPS_CERT
        int totalRead = 0;
        int remaining = bufferSize;
        
        // Read from zone 0 first (976 bytes)
        int size = (remaining > ZONE_0_SIZE) ? ZONE_0_SIZE : remaining;
        if (eeprom.read((uint8_t*)buffer, size, 0, 0) == -1)
        {
            return -1;
        }
        totalRead += size;
        remaining -= size;
        
        // Read from zone 7 if needed (784 bytes)
        if (remaining > 0)
        {
            size = (remaining > ZONE_7_SIZE) ? ZONE_7_SIZE : remaining;
            if (eeprom.read((uint8_t*)(buffer + totalRead), size, 0, 7) == -1)
            {
                return -1;
            }
            totalRead += size;
            remaining -= size;
        }
        
        // Read from zone 8 if needed (880 bytes)
        if (remaining > 0)
        {
            size = (remaining > ZONE_8_SIZE) ? ZONE_8_SIZE : remaining;
            if (eeprom.read((uint8_t*)(buffer + totalRead), size, 0, 8) == -1)
            {
                return -1;
            }
            totalRead += size;
        }
        
        buffer[bufferSize - 1] = '\0';
        return totalRead;
    }
    else
    {
        // Single-zone setting
        int result = eeprom.read((uint8_t*)buffer, bufferSize, 0, mapping->zoneIndex);
        if (result != -1)
        {
            buffer[bufferSize - 1] = '\0';
        }
        return result;
    }
}
