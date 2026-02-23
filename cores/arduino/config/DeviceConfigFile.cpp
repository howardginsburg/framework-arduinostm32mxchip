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
#include "SystemFileSystem.h"
#include "File.h"
#include <string.h>
#include <stdlib.h>

using namespace mbed;

// Filename within the mounted filesystem (leading '/' required by ChaN follow_path with _FS_RPATH=0)
#define CONFIG_FILE_NAME "/device.cfg"

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
 * Uses the mbed::File C++ API directly on the mounted FATFileSystem
 * to bypass the POSIX path-routing layer (fopen).
 *
 * @return Number of bytes written to buffer (incl. '\0') if found,
 *         0 if key not found or file absent, -1 on I/O error.
 */
static int read_key_from_file(const char* key, char* buffer, int bufferSize)
{
    FileSystem* fs = SystemFileSystem_GetFS();
    if (fs == NULL)
    {
        buffer[0] = '\0';
        return 0;
    }

    File f;
    if (f.open(fs, CONFIG_FILE_NAME, O_RDONLY) != 0)
    {
        // File doesn't exist yet — treat as empty
        buffer[0] = '\0';
        return 0;
    }

    int keyLen = (int)strlen(key);
    char line[CONFIG_LINE_MAX];
    int linePos = 0;
    int found = 0;
    char ch;

    while (f.read(&ch, 1) == 1)
    {
        if (ch == '\n' || ch == '\r')
        {
            if (linePos > 0)
            {
                line[linePos] = '\0';

                if (line[0] != '#' &&
                    linePos >= keyLen + 1 &&
                    strncmp(line, key, keyLen) == 0 &&
                    line[keyLen] == '=')
                {
                    const char* valStart = line + keyLen + 1;
                    int valLen = (int)strlen(valStart);
                    int copyLen = (valLen < bufferSize - 1) ? valLen : bufferSize - 1;
                    strncpy(buffer, valStart, copyLen);
                    buffer[copyLen] = '\0';
                    found = copyLen + 1;
                    break;
                }
                linePos = 0;
            }
        }
        else if (linePos < (int)(sizeof(line) - 1))
        {
            line[linePos++] = ch;
        }
    }

    // Handle last line with no trailing newline
    if (!found && linePos > 0)
    {
        line[linePos] = '\0';
        if (line[0] != '#' &&
            linePos >= keyLen + 1 &&
            strncmp(line, key, keyLen) == 0 &&
            line[keyLen] == '=')
        {
            const char* valStart = line + keyLen + 1;
            int valLen = (int)strlen(valStart);
            int copyLen = (valLen < bufferSize - 1) ? valLen : bufferSize - 1;
            strncpy(buffer, valStart, copyLen);
            buffer[copyLen] = '\0';
            found = copyLen + 1;
        }
    }

    f.close();

    if (!found)
    {
        buffer[0] = '\0';
    }
    return found;
}

/**
 * @brief Write (create or update) @p key=@p value in the config file.
 *
 * Uses the mbed::File C++ API directly to bypass the POSIX path-routing layer.
 *
 * Strategy:
 *   1. Read the existing file into a temporary buffer.
 *   2. Replace the matching key line if found, or append it.
 *   3. Rewrite the file (O_WRONLY | O_CREAT | O_TRUNC).
 *
 * @return 0 on success, -1 on failure.
 */
static int write_key_to_file(const char* key, const char* value)
{
    FileSystem* fs = SystemFileSystem_GetFS();
    if (fs == NULL)
    {
        return -1;
    }

    // -------------------------------------------------------------------------
    // Step 1 – Read existing content
    // -------------------------------------------------------------------------
    char* existingContent = NULL;
    int fileSize = 0;

    {
        File rf;
        if (rf.open(fs, CONFIG_FILE_NAME, O_RDONLY) == 0)
        {
            fileSize = (int)rf.size();
            if (fileSize > 0 && fileSize <= 65536)
            {
                existingContent = (char*)malloc(fileSize + 1);
                if (existingContent != NULL)
                {
                    int readBytes = (int)rf.read(existingContent, fileSize);
                    existingContent[readBytes] = '\0';
                    fileSize = readBytes;
                }
            }
            rf.close();
        }
    }

    // -------------------------------------------------------------------------
    // Step 2 – Build new content
    // -------------------------------------------------------------------------
    int keyLen   = (int)strlen(key);
    int valueLen = (int)strlen(value);
    int newSize  = fileSize + keyLen + 1 + valueLen + 2 + 1;
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
                strcat(newContent, key);
                strcat(newContent, "=");
                strcat(newContent, value);
                strcat(newContent, "\n");
                replaced = true;
            }
            else
            {
                int copyLen = atEof ? lineLen : lineLen + 1;
                strncat(newContent, lineStart, copyLen);
            }

            if (atEof) break;
            lineStart = lineEnd + 1;
        }
        free(existingContent);
    }

    if (!replaced)
    {
        strcat(newContent, key);
        strcat(newContent, "=");
        strcat(newContent, value);
        strcat(newContent, "\n");
    }

    // -------------------------------------------------------------------------
    // Step 3 – Rewrite file
    // -------------------------------------------------------------------------
    File wf;
    int err = wf.open(fs, CONFIG_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC);
    if (err != 0)
    {
        free(newContent);
        return -1;
    }

    int toWrite = (int)strlen(newContent);
    int written = (int)wf.write(newContent, toWrite);
    wf.close();
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
