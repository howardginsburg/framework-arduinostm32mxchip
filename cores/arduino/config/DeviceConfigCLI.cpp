// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "DeviceConfigCLI.h"
#include "DeviceConfig.h"
#include "SettingUI.h"
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

void config_print_help(void)
{
    Serial.printf("Configuration commands for profile '%s':\r\n", DeviceConfig_GetProfileName());
    
    for (int i = 0; i < (int)SETTING_UI_COUNT; i++)
    {
        if (DeviceConfig_IsSettingAvailable(SETTING_UI[i].id))
        {
            int maxLen = DeviceConfig_GetMaxLen(SETTING_UI[i].id);
            Serial.printf(" - %s <value>: Set %s (max %d bytes)\r\n",
                SETTING_UI[i].cliCommand,
                SETTING_UI[i].label,
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
    const SettingUIMetadata* meta = SettingUI_FindByCliCommand(cmdName);
    
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
        Serial.printf("Usage: %s <value>\r\n", meta->cliCommand);
        return true;
    }
    
    // For certificate and key settings, we need to handle escaped newlines
    bool isCertOrKey = SettingUI_IsMultiLine(meta);
    
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
            // Zero out memory for sensitive data before freeing
            if (SettingUI_IsSensitive(meta) || isCertOrKey)
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
        // Zero out memory for sensitive data before freeing
        if (SettingUI_IsSensitive(meta) || isCertOrKey)
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
    const SettingUIMetadata* meta = SettingUI_FindByCliCommand(cmdName);
    return SettingUI_IsSensitive(meta);
}

void config_show_status(void)
{
    Serial.printf("Configuration Status (Profile: %s):\r\n", DeviceConfig_GetProfileName());
    Serial.printf("================================\r\n");
    
    for (int i = 0; i < (int)SETTING_UI_COUNT; i++)
    {
        if (DeviceConfig_IsSettingAvailable(SETTING_UI[i].id))
        {
            char buffer[64];
            int result = DeviceConfig_Read(SETTING_UI[i].id, buffer, sizeof(buffer));
            
            Serial.printf("%s: ", SETTING_UI[i].label);
            
            if (result > 0 && buffer[0] != '\0')
            {
                if (SettingUI_IsSensitive(&SETTING_UI[i]))
                {
                    Serial.printf("SET (hidden)\r\n");
                }
                else if (SettingUI_IsMultiLine(&SETTING_UI[i]))
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
