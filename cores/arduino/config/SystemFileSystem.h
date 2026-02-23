// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

/**
 * @file SystemFileSystem.h
 * @brief Framework-level SFlash FAT filesystem initialisation.
 *
 * Mounts the onboard SPI flash as a FAT filesystem at /fs/.  Called by the
 * framework before DeviceConfig_LoadAll() so that file-backed DeviceConfig
 * settings (SETTING_SEND_INTERVAL, SETTING_PUBLISH_TOPIC, etc.) are readable
 * and writable from the first moment the device boots.
 *
 * If the flash partition has never been formatted the filesystem is formatted
 * automatically on this first call.
 *
 * NOTE: Do not create a separate FATFileSystem("fs") instance in your sketch
 * or via the FileSystem library — the framework already owns this mount point.
 */

#ifndef __SYSTEM_FILE_SYSTEM_H__
#define __SYSTEM_FILE_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Mount the SFlash FAT filesystem at /fs/.
 *
 * Safe to call multiple times; subsequent calls are no-ops.
 *
 * @return 0 on success, -1 on failure.
 */
int SystemFileSystem_Mount(void);

#ifdef __cplusplus
}

// C++ only – returns the mounted FATFileSystem instance for direct file I/O.
// Returns NULL if the filesystem has not been successfully mounted.
#include "FATFileSystem.h"
mbed::FileSystem* SystemFileSystem_GetFS(void);

#endif  /* __cplusplus */

#endif /* __SYSTEM_FILE_SYSTEM_H__ */
