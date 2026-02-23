// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

/**
 * @file DeviceConfigFile.h
 * @brief Config-file storage for settings that overflow STSAFE EEPROM zones.
 *
 * Settings whose ZoneMapping has FILE_ZONE_MARKER in zones[0] are stored in
 * a simple key=value text file at /fs/device.cfg on the SFlash filesystem.
 *
 * This provides unlimited expansion beyond the fixed STSAFE zone count while
 * keeping security-sensitive data (certificates, keys, passwords) in the
 * tamper-resistant STSAFE element.
 */

#ifndef __DEVICE_CONFIG_FILE_H__
#define __DEVICE_CONFIG_FILE_H__

#include "DeviceConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Save a setting value to the config file.
 *
 * Creates or updates the entry for @p setting in /fs/device.cfg.
 *
 * @param setting  Setting ID (must have FILE_ZONE mapping in active profile)
 * @param value    Null-terminated string value to store
 * @return 0 on success, -1 on failure
 */
int ConfigFile_Save(SettingID setting, const char* value);

/**
 * @brief Read a setting value from the config file.
 *
 * @param setting     Setting ID
 * @param buffer      Buffer to receive the value
 * @param bufferSize  Size of @p buffer in bytes
 * @return Number of bytes read (including null terminator) on success,
 *         0 if the key is not present, -1 on I/O failure
 */
int ConfigFile_Read(SettingID setting, char* buffer, int bufferSize);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_CONFIG_FILE_H__ */
