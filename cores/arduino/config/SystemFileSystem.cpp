// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

/**
 * @file SystemFileSystem.cpp
 * @brief Framework-level SFlash FAT filesystem initialisation.
 *
 * Provides a self-contained block device implementation (equivalent to the
 * FileSystem library's SFlashBlockDevice) compiled entirely within the core so
 * that DeviceConfig file-backed settings do not depend on a user library.
 *
 * All mico / mbed-os headers used here are already in the core include path
 * (see compiler.libstm.c.flags in platform.txt).
 */

#include "SystemFileSystem.h"
#include "FATFileSystem.h"   // system/mbed-os/features/filesystem/fat/
#include "BlockDevice.h"     // system/mbed-os/features/filesystem/bd/
#include "Arduino.h"         // Serial
#include "mico.h"
#include "ff.h"              // ChaN FatFs direct API (for f_mkfs)
#include <stdlib.h>

// =============================================================================
// Inline SFlash block device
// (equivalent to libraries/FileSystem/src/SFlashBlockDevice, duplicated here
//  so the core does not depend on a user library)
// =============================================================================

#define SYSFS_SECTOR_SIZE   512u
#define SYSFS_FLASH_SECTOR  4096u

class SystemSFlashBlockDevice : public BlockDevice
{
public:
    SystemSFlashBlockDevice() : _partition(NULL) {}
    virtual ~SystemSFlashBlockDevice() {}

    virtual int init()
    {
        _partition = MicoFlashGetInfo((mico_partition_t)MICO_PARTITION_FILESYS);
        return (_partition != NULL) ? 0 : -1;
    }

    virtual int deinit() { return 0; }

    virtual int read(void *buffer, bd_addr_t addr, bd_size_t size)
    {
        uint32_t sector = (uint32_t)(addr / SYSFS_SECTOR_SIZE);
        uint32_t count  = (uint32_t)(size / SYSFS_SECTOR_SIZE);
        uint8_t *buf    = (uint8_t *)buffer;

        for (; count > 0; count--)
        {
            uint32_t offset = sector * SYSFS_SECTOR_SIZE;
            MicoFlashRead((mico_partition_t)MICO_PARTITION_FILESYS,
                          &offset, buf, SYSFS_SECTOR_SIZE);
            sector++;
            buf += SYSFS_SECTOR_SIZE;
        }
        return 0;
    }

    virtual int program(const void *buffer, bd_addr_t addr, bd_size_t size)
    {
        uint32_t sector   = (uint32_t)(addr / SYSFS_SECTOR_SIZE);
        uint32_t count    = (uint32_t)(size / SYSFS_SECTOR_SIZE);
        const uint8_t *buf = (const uint8_t *)buffer;

        for (; count > 0; count--)
        {
            volatile uint32_t offset = sector * SYSFS_SECTOR_SIZE;
            _eraseWrite((mico_partition_t)MICO_PARTITION_FILESYS,
                        &offset, (uint8_t *)buf, SYSFS_SECTOR_SIZE);
            sector++;
            buf += SYSFS_SECTOR_SIZE;
        }
        return 0;
    }

    virtual int erase(bd_addr_t /*addr*/, bd_size_t /*size*/) { return 0; }

    virtual bd_size_t get_read_size()    const { return SYSFS_SECTOR_SIZE; }
    virtual bd_size_t get_program_size() const { return SYSFS_SECTOR_SIZE; }
    virtual bd_size_t get_erase_size()   const { return SYSFS_SECTOR_SIZE; }

    virtual bd_size_t size() const
    {
        return (_partition != NULL) ? _partition->partition_length : 0;
    }

private:
    mico_logic_partition_t *_partition;

    static void _eraseWrite(mico_partition_t partition,
                            volatile uint32_t *offset,
                            uint8_t *data, uint32_t size)
    {
        uint32_t f_sector = (*offset) >> 12;
        uint32_t f_addr   = f_sector << 12;
        uint16_t s_sector = (uint16_t)((*offset) & 0x0F00u);

        uint8_t *sector_buf = (uint8_t *)malloc(SYSFS_FLASH_SECTOR);
        if (sector_buf == NULL)
        {
            return;
        }

        MicoFlashRead(partition, &f_addr, sector_buf, SYSFS_FLASH_SECTOR);

        uint32_t pos;
        for (pos = 0; pos < size; pos++)
        {
            if (sector_buf[s_sector + pos] != 0xFF) break;
        }

        if (pos != size)
        {
            f_addr -= SYSFS_FLASH_SECTOR;
            MicoFlashErase(partition, f_addr, size);
            for (pos = 0; pos < size; pos++)
            {
                sector_buf[s_sector + pos] = data[pos];
            }
            MicoFlashWrite(partition, &f_addr, sector_buf, SYSFS_FLASH_SECTOR);
        }
        else
        {
            MicoFlashWrite(partition, offset, data, size);
        }

        free(sector_buf);
    }
};

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
    static SystemSFlashBlockDevice bd;
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
