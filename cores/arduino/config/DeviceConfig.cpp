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

// =============================================================================
// Static buffers for loaded configuration
// =============================================================================
static char s_wifiSsid[ZONE_3_SIZE];
static char s_wifiPassword[ZONE_10_SIZE];
static char s_brokerUrl[ZONE_5_SIZE];
static char s_brokerHost[ZONE_5_SIZE];
static int  s_brokerPort = 8883;
static char s_caCert[ZONE_0_SIZE + ZONE_7_SIZE + ZONE_8_SIZE];
static char s_clientCert[ZONE_6_SIZE + ZONE_7_SIZE];
static char s_clientKey[ZONE_8_SIZE];
static char s_connectionString[ZONE_5_SIZE];
static char s_deviceId[256];

/**
 * @brief Parse broker URL into host and port
 * 
 * Supports formats:
 *   hostname
 *   hostname:port
 *   mqtts://hostname
 *   mqtts://hostname:port
 *   ssl://hostname:port
 */
static void parseBrokerUrl(const char* url)
{
    if (url == NULL || url[0] == '\0')
    {
        s_brokerHost[0] = '\0';
        s_brokerPort = 8883;
        return;
    }
    
    const char* hostStart = url;
    
    // Skip protocol prefix if present
    if (strncmp(url, "mqtts://", 8) == 0)
    {
        hostStart = url + 8;
    }
    else if (strncmp(url, "ssl://", 6) == 0)
    {
        hostStart = url + 6;
    }
    else if (strncmp(url, "mqtt://", 7) == 0)
    {
        hostStart = url + 7;
        s_brokerPort = 1883;  // Default non-TLS port
    }
    
    // Find port separator
    const char* portSep = strchr(hostStart, ':');
    
    if (portSep != NULL)
    {
        // Copy host portion
        int hostLen = portSep - hostStart;
        if (hostLen >= (int)sizeof(s_brokerHost))
        {
            hostLen = sizeof(s_brokerHost) - 1;
        }
        strncpy(s_brokerHost, hostStart, hostLen);
        s_brokerHost[hostLen] = '\0';
        
        // Parse port
        s_brokerPort = atoi(portSep + 1);
        if (s_brokerPort <= 0 || s_brokerPort > 65535)
        {
            s_brokerPort = 8883;
        }
    }
    else
    {
        // No port specified, use default
        strncpy(s_brokerHost, hostStart, sizeof(s_brokerHost) - 1);
        s_brokerHost[sizeof(s_brokerHost) - 1] = '\0';
    }
}

/**
 * @brief Extract CN (Common Name) from a PEM certificate
 * 
 * Looks for "CN=" in the certificate subject and extracts the value.
 * This is a simple text-based extraction that works for typical certs.
 */
static void extractCNFromCert(const char* cert, char* cnBuffer, int bufferSize)
{
    cnBuffer[0] = '\0';
    
    if (cert == NULL || cert[0] == '\0')
    {
        return;
    }
    
    // Look for "CN=" pattern (could be after "/" or ", ")
    const char* cnStart = strstr(cert, "CN=");
    if (cnStart == NULL)
    {
        cnStart = strstr(cert, "cn=");
    }
    
    if (cnStart != NULL)
    {
        cnStart += 3;  // Skip "CN="
        
        // Find end of CN value (terminated by '/', ',', '\n', '\r', or '"')
        int i = 0;
        while (cnStart[i] != '\0' && 
               cnStart[i] != '/' && 
               cnStart[i] != ',' && 
               cnStart[i] != '\n' && 
               cnStart[i] != '\r' &&
               cnStart[i] != '"' &&
               i < bufferSize - 1)
        {
            cnBuffer[i] = cnStart[i];
            i++;
        }
        cnBuffer[i] = '\0';
    }
}

/**
 * @brief Extract DeviceId from an IoT Hub connection string
 * 
 * Connection string format:
 * HostName=<hub>.azure-devices.net;DeviceId=<device>;SharedAccessKey=<key>
 */
static void extractDeviceIdFromConnectionString(const char* connStr, char* deviceId, int bufferSize)
{
    deviceId[0] = '\0';
    
    if (connStr == NULL || connStr[0] == '\0')
    {
        return;
    }
    
    // Look for "DeviceId="
    const char* start = strstr(connStr, "DeviceId=");
    if (start == NULL)
    {
        start = strstr(connStr, "deviceId=");
    }
    
    if (start != NULL)
    {
        start += 9;  // Skip "DeviceId="
        
        // Find end (terminated by ';' or end of string)
        int i = 0;
        while (start[i] != '\0' && start[i] != ';' && i < bufferSize - 1)
        {
            deviceId[i] = start[i];
            i++;
        }
        deviceId[i] = '\0';
    }
}

bool DeviceConfig_LoadAll(void)
{
    if (s_activeProfile == NULL)
    {
        return false;
    }
    
    bool success = true;
    
    // Load WiFi SSID (required for all profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_WIFI_SSID))
    {
        if (DeviceConfig_Read(SETTING_WIFI_SSID, s_wifiSsid, sizeof(s_wifiSsid)) < 0)
        {
            s_wifiSsid[0] = '\0';
            success = false;
        }
    }
    
    // Load WiFi Password (required for all profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_WIFI_PASSWORD))
    {
        if (DeviceConfig_Read(SETTING_WIFI_PASSWORD, s_wifiPassword, sizeof(s_wifiPassword)) < 0)
        {
            s_wifiPassword[0] = '\0';
            success = false;
        }
    }
    
    // Load Broker URL (for MQTT profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_BROKER_URL))
    {
        if (DeviceConfig_Read(SETTING_BROKER_URL, s_brokerUrl, sizeof(s_brokerUrl)) >= 0)
        {
            parseBrokerUrl(s_brokerUrl);
        }
        else
        {
            s_brokerUrl[0] = '\0';
            s_brokerHost[0] = '\0';
            success = false;
        }
    }
    
    // Load CA Certificate (for TLS profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_CA_CERT))
    {
        if (DeviceConfig_Read(SETTING_CA_CERT, s_caCert, sizeof(s_caCert)) < 0)
        {
            s_caCert[0] = '\0';
            success = false;
        }
    }
    
    // Load Client Certificate (for mTLS profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_CLIENT_CERT))
    {
        if (DeviceConfig_Read(SETTING_CLIENT_CERT, s_clientCert, sizeof(s_clientCert)) < 0)
        {
            s_clientCert[0] = '\0';
            success = false;
        }
    }
    
    // Load Client Private Key (for mTLS profiles)
    if (DeviceConfig_IsSettingAvailable(SETTING_CLIENT_KEY))
    {
        if (DeviceConfig_Read(SETTING_CLIENT_KEY, s_clientKey, sizeof(s_clientKey)) < 0)
        {
            s_clientKey[0] = '\0';
            success = false;
        }
    }
    
    // Load Connection String (for IoT Hub SAS profile)
    if (DeviceConfig_IsSettingAvailable(SETTING_CONNECTION_STRING))
    {
        if (DeviceConfig_Read(SETTING_CONNECTION_STRING, s_connectionString, sizeof(s_connectionString)) < 0)
        {
            s_connectionString[0] = '\0';
            success = false;
        }
    }
    
    // Load/derive Device ID based on profile type
    s_deviceId[0] = '\0';
    if (s_activeProfile->profile == PROFILE_MQTT_MTLS || 
        s_activeProfile->profile == PROFILE_IOTHUB_CERT ||
        s_activeProfile->profile == PROFILE_DPS_CERT)
    {
        // Extract CN from client certificate
        if (s_clientCert[0] != '\0')
        {
            extractCNFromCert(s_clientCert, s_deviceId, sizeof(s_deviceId));
        }
    }
    else if (s_activeProfile->profile == PROFILE_IOTHUB_SAS)
    {
        // Extract DeviceId from connection string
        if (s_connectionString[0] != '\0')
        {
            extractDeviceIdFromConnectionString(s_connectionString, s_deviceId, sizeof(s_deviceId));
        }
    }
    else if (DeviceConfig_IsSettingAvailable(SETTING_DEVICE_ID))
    {
        // Read directly from EEPROM for username/password profiles
        DeviceConfig_Read(SETTING_DEVICE_ID, s_deviceId, sizeof(s_deviceId));
    }
    
    return success;
}

const char* DeviceConfig_GetWifiSsid(void)
{
    return s_wifiSsid;
}

const char* DeviceConfig_GetWifiPassword(void)
{
    return s_wifiPassword;
}

const char* DeviceConfig_GetBrokerHost(void)
{
    return s_brokerHost;
}

int DeviceConfig_GetBrokerPort(void)
{
    return s_brokerPort;
}

const char* DeviceConfig_GetCACert(void)
{
    return s_caCert;
}

const char* DeviceConfig_GetClientCert(void)
{
    return s_clientCert;
}

const char* DeviceConfig_GetClientKey(void)
{
    return s_clientKey;
}

const char* DeviceConfig_GetDeviceId(void)
{
    return s_deviceId;
}
