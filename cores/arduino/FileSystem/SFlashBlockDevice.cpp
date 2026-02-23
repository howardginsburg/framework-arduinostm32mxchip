// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "SFlashBlockDevice.h"

#define SFLASH_SECTOR_SIZE  512u
#define SFLASH_FLASH_SECTOR 4096u

SFlashBlockDevice::SFlashBlockDevice() : _partition(NULL) {}
SFlashBlockDevice::~SFlashBlockDevice() {}

int SFlashBlockDevice::init()
{
    _partition = MicoFlashGetInfo((mico_partition_t)MICO_PARTITION_FILESYS);
    return (_partition != NULL) ? 0 : -1;
}

int SFlashBlockDevice::deinit() { return 0; }

int SFlashBlockDevice::read(void *buffer, bd_addr_t addr, bd_size_t size)
{
    uint32_t sector = (uint32_t)(addr / SFLASH_SECTOR_SIZE);
    uint32_t count  = (uint32_t)(size / SFLASH_SECTOR_SIZE);
    uint8_t *buf    = (uint8_t *)buffer;

    for (; count > 0; count--)
    {
        uint32_t offset = sector * SFLASH_SECTOR_SIZE;
        MicoFlashRead((mico_partition_t)MICO_PARTITION_FILESYS,
                      &offset, buf, SFLASH_SECTOR_SIZE);
        sector++;
        buf += SFLASH_SECTOR_SIZE;
    }
    return 0;
}

int SFlashBlockDevice::program(const void *buffer, bd_addr_t addr, bd_size_t size)
{
    uint32_t sector    = (uint32_t)(addr / SFLASH_SECTOR_SIZE);
    uint32_t count     = (uint32_t)(size / SFLASH_SECTOR_SIZE);
    const uint8_t *buf = (const uint8_t *)buffer;

    for (; count > 0; count--)
    {
        volatile uint32_t offset = sector * SFLASH_SECTOR_SIZE;
        _eraseWrite((mico_partition_t)MICO_PARTITION_FILESYS,
                    &offset, (uint8_t *)buf, SFLASH_SECTOR_SIZE);
        sector++;
        buf += SFLASH_SECTOR_SIZE;
    }
    return 0;
}

int SFlashBlockDevice::erase(bd_addr_t /*addr*/, bd_size_t /*size*/) { return 0; }

bd_size_t SFlashBlockDevice::get_read_size()    const { return SFLASH_SECTOR_SIZE; }
bd_size_t SFlashBlockDevice::get_program_size() const { return SFLASH_SECTOR_SIZE; }
bd_size_t SFlashBlockDevice::get_erase_size()   const { return SFLASH_SECTOR_SIZE; }

bd_size_t SFlashBlockDevice::size() const
{
    return (_partition != NULL) ? _partition->partition_length : 0;
}

/*static*/
void SFlashBlockDevice::_eraseWrite(mico_partition_t partition,
                                    volatile uint32_t *offset,
                                    uint8_t *data, uint32_t size)
{
    uint32_t f_sector = (*offset) >> 12;
    uint32_t f_addr   = f_sector << 12;
    uint16_t s_sector = (uint16_t)((*offset) & 0x0F00u);

    uint8_t *sector_buf = (uint8_t *)malloc(SFLASH_FLASH_SECTOR);
    if (sector_buf == NULL)
    {
        return;
    }

    MicoFlashRead(partition, &f_addr, sector_buf, SFLASH_FLASH_SECTOR);

    uint32_t pos;
    for (pos = 0; pos < size; pos++)
    {
        if (sector_buf[s_sector + pos] != 0xFF) break;
    }

    if (pos != size)
    {
        f_addr -= SFLASH_FLASH_SECTOR;
        MicoFlashErase(partition, f_addr, size);
        for (pos = 0; pos < size; pos++)
        {
            sector_buf[s_sector + pos] = data[pos];
        }
        MicoFlashWrite(partition, &f_addr, sector_buf, SFLASH_FLASH_SECTOR);
    }
    else
    {
        MicoFlashWrite(partition, offset, data, size);
    }

    free(sector_buf);
}
