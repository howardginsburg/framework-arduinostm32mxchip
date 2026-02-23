// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

/**
 * @file SFlashBlockDevice.h
 * @brief BlockDevice backed by the onboard SPI-flash filesystem partition.
 *
 * Canonical, single-copy implementation shared by the framework core
 * (SystemFileSystem) and the user-facing FileSystem library.  Placing this
 * file in cores/arduino/ ensures it is on the compiler include path
 * (-I{build.core.path}) for both core and library compilation units.
 */

#ifndef SFLASH_BLOCK_DEVICE_H
#define SFLASH_BLOCK_DEVICE_H

#include "BlockDevice.h"  // system/mbed-os/features/filesystem/bd/
#include "mico.h"
#include <stdlib.h>

class SFlashBlockDevice : public BlockDevice
{
public:
    SFlashBlockDevice();
    virtual ~SFlashBlockDevice();

    virtual int init();
    virtual int deinit();

    virtual int read(void *buffer, bd_addr_t addr, bd_size_t size);
    virtual int program(const void *buffer, bd_addr_t addr, bd_size_t size);
    virtual int erase(bd_addr_t addr, bd_size_t size);

    virtual bd_size_t get_read_size()    const;
    virtual bd_size_t get_program_size() const;
    virtual bd_size_t get_erase_size()   const;
    virtual bd_size_t size()             const;

private:
    mico_logic_partition_t *_partition;

    static void _eraseWrite(mico_partition_t partition,
                            volatile uint32_t *offset,
                            uint8_t *data, uint32_t size);
};

#endif /* SFLASH_BLOCK_DEVICE_H */
