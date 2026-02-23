// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

/**
 * @file DeviceConfigFile.cpp
 * @brief Config-file storage for settings that overflow STSAFE EEPROM zones.
 *
 * Stores settings as key=value lines in /fs/device.cfg.
 * Reads scan the file line-by-line; writes update in-place or append.
 * Lines beginning with '#' are treated as comments and preserved.
 *
 * File format example:
 *   send_interval=30
 *   publish_topic=devices/mydevice/messages/events/
 *   subscribe_topic=devices/mydevice/messages/devicebound/#
 */

#define _DEVICE_CONFIG_IMPL
#include "DeviceConfigFile.h"
#include "DeviceConfigZones.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Path to the configuration file on the SFlash filesystem
#define CONFIG_FILE_PATH "/fs/device.cfg"

// Maximum line length in the config file (key + '=' + value + '\n' + '\0')
#define CONFIG_LINE_MAX  (64 + 1 + MAX_SUBSCRIBE_TOPIC_SIZE + 2)

// =============================================================================
// Setting ID → file key mapping
// =============================================================================

/**
 * @brief String key used in the config file for each SettingID.
 *
 * Only settings with FILE_ZONE mappings need a non-NULL key; the rest are
 * stored in EEPROM and this table is never consulted for them.
 */
static const char* const SETTING_FILE_KEYS[SETTING_COUNT] = {
    NULL,               // SETTING_WIFI_SSID         (EEPROM)
    NULL,               // SETTING_WIFI_PASSWORD      (EEPROM)
    NULL,               // SETTING_BROKER_URL         (EEPROM)
    NULL,               // SETTING_DEVICE_ID          (EEPROM)
    NULL,               // SETTING_DEVICE_PASSWORD    (EEPROM)
    NULL,               // SETTING_CA_CERT            (EEPROM)
    NULL,               // SETTING_CLIENT_CERT        (EEPROM)
    NULL,               // SETTING_CLIENT_KEY         (EEPROM)
    NULL,               // SETTING_CONNECTION_STRING  (EEPROM)
    NULL,               // SETTING_DPS_ENDPOINT       (EEPROM)
    NULL,               // SETTING_SCOPE_ID           (EEPROM)
    NULL,               // SETTING_REGISTRATION_ID    (EEPROM)
    NULL,               // SETTING_SYMMETRIC_KEY      (EEPROM)
    NULL,               // SETTING_DEVICE_CERT        (EEPROM)
    "send_interval",    // SETTING_SEND_INTERVAL      (config file)
    "publish_topic",    // SETTING_PUBLISH_TOPIC      (config file)
    "subscribe_topic",  // SETTING_SUBSCRIBE_TOPIC    (config file)
};

// =============================================================================
// Internal helpers
// =============================================================================

/**
 * @brief Get the file key string for a SettingID.
 * @return Key string or NULL if the setting has no file mapping.
 */
static const char* get_file_key(SettingID setting)
{
    if ((int)setting < 0 || setting >= SETTING_COUNT)
    {
        return NULL;
    }
    return SETTING_FILE_KEYS[setting];
}

/**
 * @brief Read the value for @p key from the config file into @p buffer.
 *
 * @return Number of bytes written to buffer (incl. '\0') if found,
 *         0 if key not found, -1 on I/O error.
 */
static int read_key_from_file(const char* key, char* buffer, int bufferSize)
{
    FILE* fp = fopen(CONFIG_FILE_PATH, "r");
    if (fp == NULL)
    {
        // File doesn't exist yet — treat as empty
        buffer[0] = '\0';
        return 0;
    }

    int keyLen = (int)strlen(key);
    char line[CONFIG_LINE_MAX];
    int found = 0;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        // Skip comments and blank lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
        {
            continue;
        }

        // Match "KEY="
        if (strncmp(line, key, keyLen) == 0 && line[keyLen] == '=')
        {
            const char* valStart = line + keyLen + 1;

            // Strip trailing newline characters
            int valLen = (int)strlen(valStart);
            while (valLen > 0 && (valStart[valLen - 1] == '\n' || valStart[valLen - 1] == '\r'))
            {
                valLen--;
            }

            int copyLen = (valLen < bufferSize - 1) ? valLen : bufferSize - 1;
            strncpy(buffer, valStart, copyLen);
            buffer[copyLen] = '\0';
            found = copyLen + 1;
            break;
        }
    }

    fclose(fp);

    if (!found)
    {
        buffer[0] = '\0';
    }
    return found;
}

/**
 * @brief Write (create or update) @p key=@p value in the config file.
 *
 * Strategy:
 *   1. Read the existing file into a temporary buffer.
 *   2. Replace the matching key line if found, or append it.
 *   3. Rewrite the file atomically (overwrite with "w").
 *
 * @return 0 on success, -1 on failure.
 */
static int write_key_to_file(const char* key, const char* value)
{
    // -------------------------------------------------------------------------
    // Step 1 – Read existing content
    // -------------------------------------------------------------------------
    char* existingContent = NULL;
    long fileSize = 0;

    FILE* fp = fopen(CONFIG_FILE_PATH, "r");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (fileSize > 0)
        {
            // Sanity-check to avoid overflow in fileSize + 1
            if (fileSize > 65536L)
            {
                fclose(fp);
                return -1;
            }
            existingContent = (char*)malloc(fileSize + 1);
            if (existingContent == NULL)
            {
                fclose(fp);
                return -1;
            }
            int readBytes = (int)fread(existingContent, 1, fileSize, fp);
            existingContent[readBytes] = '\0';
        }
        fclose(fp);
    }

    // -------------------------------------------------------------------------
    // Step 2 – Build new content
    // -------------------------------------------------------------------------
    int keyLen   = (int)strlen(key);
    int valueLen = (int)strlen(value);
    // Allocate worst-case: existing content + new line
    int newSize = (int)fileSize + keyLen + 1 + valueLen + 2 + 1;
    char* newContent = (char*)malloc(newSize);
    if (newContent == NULL)
    {
        free(existingContent);
        return -1;
    }
    newContent[0] = '\0';

    bool replaced = false;

    if (existingContent != NULL)
    {
        char* lineStart = existingContent;
        while (*lineStart != '\0')
        {
            // Find end of this line
            char* lineEnd = lineStart;
            while (*lineEnd != '\0' && *lineEnd != '\n')
            {
                lineEnd++;
            }
            int lineLen = (int)(lineEnd - lineStart);
            bool atEof  = (*lineEnd == '\0');

            if (!replaced && lineLen >= keyLen + 1 &&
                strncmp(lineStart, key, keyLen) == 0 &&
                lineStart[keyLen] == '=')
            {
                // Replace this line
                strcat(newContent, key);
                strcat(newContent, "=");
                strcat(newContent, value);
                strcat(newContent, "\n");
                replaced = true;
            }
            else
            {
                // Keep original line (including newline if present)
                int copyLen = atEof ? lineLen : lineLen + 1;
                strncat(newContent, lineStart, copyLen);
            }

            if (atEof)
            {
                break;
            }
            lineStart = lineEnd + 1;
        }
    }

    if (!replaced)
    {
        // Append new key
        strcat(newContent, key);
        strcat(newContent, "=");
        strcat(newContent, value);
        strcat(newContent, "\n");
    }

    free(existingContent);

    // -------------------------------------------------------------------------
    // Step 3 – Rewrite file
    // -------------------------------------------------------------------------
    fp = fopen(CONFIG_FILE_PATH, "w");
    if (fp == NULL)
    {
        free(newContent);
        return -1;
    }

    int toWrite = (int)strlen(newContent);
    int written = (int)fwrite(newContent, 1, toWrite, fp);
    fclose(fp);
    free(newContent);

    return (written == toWrite) ? 0 : -1;
}

// =============================================================================
// Public API
// =============================================================================

int ConfigFile_Save(SettingID setting, const char* value)
{
    const char* key = get_file_key(setting);
    if (key == NULL || value == NULL)
    {
        return -1;
    }
    return write_key_to_file(key, value);
}

int ConfigFile_Read(SettingID setting, char* buffer, int bufferSize)
{
    const char* key = get_file_key(setting);
    if (key == NULL || buffer == NULL || bufferSize <= 0)
    {
        return -1;
    }
    return read_key_from_file(key, buffer, bufferSize);
}
