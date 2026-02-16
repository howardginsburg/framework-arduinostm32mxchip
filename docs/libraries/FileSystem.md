# FileSystem Library

**Version:** 0.0.1 | **Author:** Microsoft | **Category:** Data Storage | **Architecture:** stm32f4

FAT filesystem driver for the onboard SPI flash (SFlash). Provides a block device interface and filesystem info utilities.

---

## Quick Start

```cpp
#include <SFlashBlockDevice.h>
#include <fatfs_exfuns.h>

SFlashBlockDevice flash;

void setup() {
    flash.init();

    // Get filesystem info
    filesystem_info info = fatfs_get_info();
    Serial.printf("Total: %d%c, Free: %d%c\n",
        info.total_space, info.unit,
        info.free_space, info.unit);
}
```

---

## API Reference

### Class: `SFlashBlockDevice`

Extends the mbed `BlockDevice` interface to provide read/write/erase access to the onboard SPI flash.

| Method | Signature | Description |
|--------|-----------|-------------|
| `init` | `virtual int init()` | Initialize the block device |
| `deinit` | `virtual int deinit()` | Deinitialize the block device |
| `read` | `virtual int read(void* buffer, bd_addr_t addr, bd_size_t size)` | Read data from flash |
| `program` | `virtual int program(const void* buffer, bd_addr_t addr, bd_size_t size)` | Write data (sector must be erased first) |
| `erase` | `virtual int erase(bd_addr_t addr, bd_size_t size)` | Erase sectors |
| `get_read_size` | `virtual bd_size_t get_read_size() const` | Minimum read block size |
| `get_program_size` | `virtual bd_size_t get_program_size() const` | Minimum program block size |
| `get_erase_size` | `virtual bd_size_t get_erase_size() const` | Minimum erase block size |
| `size` | `virtual bd_size_t size() const` | Total device capacity in bytes |

### Filesystem Info

```cpp
typedef struct {
    int total_space;
    int free_space;
    char unit;          // 'K' for KB, 'M' for MB
} filesystem_info;

filesystem_info fatfs_get_info();
```

---

## Usage Notes

- The `program` method requires that the target region has been erased first. Flash memory can only be written to erased sectors.
- The device uses FatFS internally for filesystem operations.
- Block sizes are determined by the SPI flash hardware.

---

## Examples

- **FileSystem** — Basic filesystem initialization and info retrieval

---

## Dependencies

- mbed OS (`BlockDevice` interface)
- FatFS (`ff.h`)
- MiCO SPI flash driver
