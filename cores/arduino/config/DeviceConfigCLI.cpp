// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "DeviceConfigCLI.h"
#include "DeviceConfig.h"
#include "UARTClass.h"
#include <string.h>
#include <stdlib.h>

extern UARTClass Serial;

// Helper function to convert escaped newlines to actual newlines
static void convert_escaped_newlines(char* str)
{
    if (str == NULL) return;
    
    char* src = str;
    char* dst = str;
    
    while (*src)
    {
        if (*src == '\\' && *(src + 1) == 'n')
        {
            *dst = '\n';
            src += 2;
        }
        else
        {
            *dst = *src;
            src++;
        }
        dst++;
    }
    *dst = '\0';
}

// Setting metadata for CLI commands
typedef struct {
    SettingID id;
    const char* label;
    const char* cmdName;
    bool isPrivacy;
} SettingMetadata;

static const SettingMetadata SETTING_METADATA[] = {
    {SETTING_WIFI_SSID, "WiFi SSID", "set_wifissid", false},
    {SETTING_WIFI_PASSWORD, "WiFi Password", "set_wifipwd", true},
    {SETTING_BROKER_URL, "MQTT Broker URL", "set_broker", false},
    {SETTING_DEVICE_ID, "Device ID", "set_deviceid", false},
    {SETTING_DEVICE_PASSWORD, "Device Password", "set_devicepwd", true},
    {SETTING_CA_CERT, "CA Certificate", "set_cacert", false},
    {SETTING_CLIENT_CERT, "Client Certificate", "set_clientcert", false},
    {SETTING_CLIENT_KEY, "Client Private Key", "set_clientkey", true},
    {SETTING_CONNECTION_STRING, "IoT Hub Connection String", "set_connstring", false},
    {SETTING_DPS_ENDPOINT, "DPS Endpoint", "set_dps_endpoint", false},
    {SETTING_SCOPE_ID, "DPS Scope ID", "set_scopeid", false},
    {SETTING_REGISTRATION_ID, "DPS Registration ID", "set_regid", false},
    {SETTING_SYMMETRIC_KEY, "DPS Symmetric Key", "set_symkey", true},
    {SETTING_DEVICE_CERT, "Device Certificate", "set_devicecert", false}
};

static const int SETTING_METADATA_COUNT = sizeof(SETTING_METADATA) / sizeof(SettingMetadata);

void config_print_help(void)
{
    Serial.printf("Configuration commands for profile '%s':\r\n", DeviceConfig_GetProfileName());
    
    for (int i = 0; i < SETTING_METADATA_COUNT; i++)
    {
        if (DeviceConfig_IsSettingAvailable(SETTING_METADATA[i].id))
        {
            int maxLen = DeviceConfig_GetMaxLen(SETTING_METADATA[i].id);
            Serial.printf(" - %s <%s>: Set %s (max %d bytes)\r\n",
                SETTING_METADATA[i].cmdName,
                "value",
                SETTING_METADATA[i].label,
                maxLen);
        }
    }
}

bool config_dispatch_command(const char* cmdName, int argc, char** argv)
{
    if (cmdName == NULL)
    {
        return false;
    }
    
    // Find matching setting
    const SettingMetadata* meta = NULL;
    for (int i = 0; i < SETTING_METADATA_COUNT; i++)
    {
        if (strcmp(SETTING_METADATA[i].cmdName, cmdName) == 0)
        {
            meta = &SETTING_METADATA[i];
            break;
        }
    }
    
    if (meta == NULL)
    {
        return false;  // Not a config command
    }
    
    // Check if setting is available in the active profile
    if (!DeviceConfig_IsSettingAvailable(meta->id))
    {
        Serial.printf("ERROR: %s is not available in the current profile (%s)\r\n",
            meta->label, DeviceConfig_GetProfileName());
        return true;  // Command was recognized, just not available
    }
    
    // Check argument
    if (argc < 2 || argv[1] == NULL)
    {
        Serial.printf("Usage: %s <value>\r\n", meta->cmdName);
        return true;
    }
    
    // For certificate and key settings, we need to handle escaped newlines
    bool isCertOrKey = (meta->id == SETTING_CA_CERT || 
                        meta->id == SETTING_CLIENT_CERT || 
                        meta->id == SETTING_CLIENT_KEY ||
                        meta->id == SETTING_DEVICE_CERT);
    
    char* value = argv[1];
    char* allocatedValue = NULL;
    
    if (isCertOrKey)
    {
        // Make a copy and convert escaped newlines
        allocatedValue = (char*)malloc(strlen(argv[1]) + 1);
        if (allocatedValue == NULL)
        {
            Serial.printf("ERROR: Out of memory\r\n");
            return true;
        }
        strcpy(allocatedValue, argv[1]);
        convert_escaped_newlines(allocatedValue);
        value = allocatedValue;
    }
    
    // Validate length
    int len = strlen(value);
    int maxLen = DeviceConfig_GetMaxLen(meta->id);
    if (len == 0 || len > maxLen)
    {
        Serial.printf("ERROR: Invalid length. Max %d bytes, got %d\r\n", maxLen, len);
        if (allocatedValue != NULL)
        {
            // Zero out memory for keys before freeing
            if (meta->id == SETTING_CLIENT_KEY || meta->id == SETTING_SYMMETRIC_KEY)
            {
                memset(allocatedValue, 0, len);
            }
            free(allocatedValue);
        }
        return true;
    }
    
    // Save the setting
    int result = DeviceConfig_Save(meta->id, value);
    
    // Clean up allocated memory
    if (allocatedValue != NULL)
    {
        // Zero out memory for keys before freeing
        if (meta->id == SETTING_CLIENT_KEY || meta->id == SETTING_SYMMETRIC_KEY)
        {
            memset(allocatedValue, 0, len);
        }
        free(allocatedValue);
    }
    
    if (result == 0)
    {
        Serial.printf("INFO: Set %s successfully", meta->label);
        if (isCertOrKey)
        {
            Serial.printf(" (%d bytes)", len);
        }
        Serial.printf("\r\n");
    }
    else
    {
        Serial.printf("ERROR: Failed to save %s\r\n", meta->label);
    }
    
    return true;
}

bool config_is_privacy_command(const char* cmdName)
{
    if (cmdName == NULL)
    {
        return false;
    }
    
    for (int i = 0; i < SETTING_METADATA_COUNT; i++)
    {
        if (strcmp(SETTING_METADATA[i].cmdName, cmdName) == 0)
        {
            return SETTING_METADATA[i].isPrivacy;
        }
    }
    
    return false;
}

void config_show_status(void)
{
    Serial.printf("Configuration Status (Profile: %s):\r\n", DeviceConfig_GetProfileName());
    Serial.printf("================================\r\n");
    
    for (int i = 0; i < SETTING_METADATA_COUNT; i++)
    {
        if (DeviceConfig_IsSettingAvailable(SETTING_METADATA[i].id))
        {
            char buffer[64];
            int result = DeviceConfig_Read(SETTING_METADATA[i].id, buffer, sizeof(buffer));
            
            Serial.printf("%s: ", SETTING_METADATA[i].label);
            
            if (result > 0 && buffer[0] != '\0')
            {
                if (SETTING_METADATA[i].isPrivacy)
                {
                    Serial.printf("SET (hidden)\r\n");
                }
                else if (SETTING_METADATA[i].id == SETTING_CA_CERT || 
                         SETTING_METADATA[i].id == SETTING_CLIENT_CERT ||
                         SETTING_METADATA[i].id == SETTING_DEVICE_CERT)
                {
                    Serial.printf("SET (starts with: %.20s...)\r\n", buffer);
                }
                else
                {
                    Serial.printf("%s\r\n", buffer);
                }
            }
            else
            {
                Serial.printf("NOT SET\r\n");
            }
        }
    }
}
