// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

/**
 * @file SystemFileSystem.cpp
 * @brief Framework-level SFlash FAT filesystem initialisation.
 *
 * All mico / mbed-os headers used here are already in the core include path
 * (see compiler.libstm.c.flags in platform.txt).
 */

#include "SystemFileSystem.h"
#include "SFlashBlockDevice.h"  // cores/arduino/FileSystem/ – shared with the FileSystem library
#include "FATFileSystem.h"      // system/mbed-os/features/filesystem/fat/
#include "Arduino.h"            // Serial
#include "ff.h"                 // ChaN FatFs direct API (for f_mkfs)

// =============================================================================
// Public API
// =============================================================================

// Module-level pointer set after a successful mount so that
// SystemFileSystem_GetFS() can return it without needing to reach inside
// another function's local scope.
static FATFileSystem *s_fs = NULL;

mbed::FileSystem* SystemFileSystem_GetFS(void)
{
    return s_fs;
}

int SystemFileSystem_Mount(void)
{
    if (s_fs != NULL)
    {
        return 0;
    }

    // Function-local statics: constructed on first call (inside main(), after
    // the mbed-RTOS scheduler is running) — avoids static-init ordering issues.
    static SFlashBlockDevice bd;
    static FATFileSystem           fs("fs");

    bd.init();

    // Lazy mount — always succeeds (just registers the filesystem object with
    // ChaN; actual FAT read is deferred to first file access).
    // Do NOT use force=true: on an unformatted partition it would return
    // -ENOENT right away, and after f_mkfs it returns -EINVAL because ChaN
    // leaves FatFs[0]->fs_type > 0 after writing the FAT structures.
    int err = fs.mount(&bd);
    if (err != 0)
    {
        Serial.printf("[FS] Mount registration failed (%d)\r\n", err);
        return -1;
    }

    // Probe: open a tiny file to trigger ChaN's deferred mount.
    // If the partition was never formatted, ChaN returns FR_NO_FILESYSTEM which
    // mbed maps to -ENOENT (-2).
    {
        File probe;
        int perr = probe.open(&fs, "/_p.tmp", O_WRONLY | O_CREAT | O_TRUNC);
        if (perr == 0)
        {
            // Partition is healthy — clean up the probe file
            probe.close();
            fs.remove("/_p.tmp");
        }
        else if (perr == -2)
        {
            // Unformatted (or corrupted) partition.
            // FATFileSystem::format() allocates a second FATFileSystem instance
            // internally; with _VOLUMES=1, that fails with -ENOMEM because
            // drive 0 is already owned by fs("fs").
            // Use ChaN's f_mkfs() directly on drive "0:" instead.
            Serial.print("[FS] Formatting /fs...");
            FRESULT fr = f_mkfs("0:", 0, 0);
            if (fr != FR_OK)
            {
                Serial.printf("failed (FR=%d)\r\n", (int)fr);
                return -1;
            }
            Serial.println("done.");

            // After f_mkfs, ChaN leaves FatFs[0]->fs_type > 0 (it mounts
            // internally while writing FAT structures).  Unmount + lazy
            // remount to reset the ChaN state so the next file access does a
            // clean deferred-mount and finds the freshly written FAT.
            fs.unmount();
            err = fs.mount(&bd);
            if (err != 0)
            {
                Serial.printf("[FS] Mount after format failed (%d)\r\n", err);
                return -1;
            }
        }
        else
        {
            Serial.printf("[FS] Probe open failed unexpectedly (%d)\r\n", perr);
            return -1;
        }
    }

    s_fs = &fs;
    Serial.println("[FS] /fs mounted.");
    return 0;
}
