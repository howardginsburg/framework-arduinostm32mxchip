# SPI Library

**Version:** 1.0 | **Author:** Arduino | **Category:** Communication | **Architecture:** stm32l4

Standard Arduino SPI Master library adapted for the STM32F412 (MXChip AZ3166). Provides the familiar Arduino SPI API.

---

## Quick Start

```cpp
#include <AZ3166SPI.h>

void setup() {
    SPI.begin();
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

    digitalWrite(CS_PIN, LOW);
    uint8_t response = SPI.transfer(0x42);
    digitalWrite(CS_PIN, HIGH);

    SPI.endTransaction();
}
```

---

## API Reference

### Class: `SPISettings`

| Constructor | Parameters | Description |
|-------------|-----------|-------------|
| `SPISettings()` | — | Default: 1 MHz, MSBFIRST, SPI_MODE0 |
| `SPISettings(clock, bitOrder, dataMode)` | `uint32_t`, `uint8_t`, `uint8_t` | Custom settings |

### Class: `SPIClass` (Global Instance: `SPI`)

| Method | Signature | Description |
|--------|-----------|-------------|
| `begin` | `void begin()` | Initialize SPI peripheral |
| `end` | `void end()` | De-initialize SPI peripheral |
| `beginTransaction` | `void beginTransaction(SPISettings settings)` | Start SPI transaction |
| `endTransaction` | `void endTransaction()` | End SPI transaction |
| `setBitOrder` | `void setBitOrder(uint8_t bitOrder)` | `MSBFIRST` or `LSBFIRST` |
| `setDataMode` | `void setDataMode(uint8_t dataMode)` | Set SPI mode (0–3) |
| `setFrequency` | `void setFrequency(uint32_t freq)` | Set clock frequency in Hz |
| `transfer` | `uint8_t transfer(uint8_t data)` | Transfer one byte (full-duplex) |

---

## Clock Divider Constants

These map to actual frequencies rather than AVR-style divider ratios:

| Define | Frequency | Description |
|--------|-----------|-------------|
| `SPI_CLOCK_DIV2` | 8 MHz | Fastest |
| `SPI_CLOCK_DIV4` | 4 MHz | |
| `SPI_CLOCK_DIV8` | 2 MHz | |
| `SPI_CLOCK_DIV16` | 1 MHz | Default |
| `SPI_CLOCK_DIV32` | 500 KHz | |
| `SPI_CLOCK_DIV64` | 250 KHz | |
| `SPI_CLOCK_DIV128` | 125 KHz | Slowest |

---

## SPI Modes

| Mode | CPOL | CPHA | Define |
|------|------|------|--------|
| 0 | 0 | 0 | `SPI_MODE0` |
| 1 | 0 | 1 | `SPI_MODE1` |
| 2 | 1 | 0 | `SPI_MODE2` |
| 3 | 1 | 1 | `SPI_MODE3` |

---

## Dependencies

- Arduino core
- mbed OS (`MbedSPI`)
