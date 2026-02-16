# Wire Library

**Version:** 1.0 | **Author:** Arduino | **Category:** Communication | **Architecture:** stm32l4

Standard Arduino I2C/TWI (Two Wire Interface) library for the MXChip AZ3166. Supports master and slave modes with two hardware I2C buses.

---

## Quick Start

```cpp
#include <Wire.h>

void setup() {
    Wire.begin();  // Initialize as master
}

void loop() {
    Wire.beginTransmission(0x48);  // Address an I2C device
    Wire.write(0x00);              // Register address
    Wire.endTransmission();

    Wire.requestFrom(0x48, 2);    // Request 2 bytes
    while (Wire.available()) {
        int val = Wire.read();
        Serial.println(val);
    }
    delay(1000);
}
```

---

## API Reference

### Class: `TwoWire`

#### Initialization

| Method | Signature | Description |
|--------|-----------|-------------|
| Constructor | `TwoWire(I2CName i2c_instance)` | Specify I2C peripheral instance |
| `begin` | `void begin()` | Initialize as master |
| `begin` | `void begin(unsigned char addr)` | Initialize as slave at address |
| `begin` | `void begin(int addr)` | Initialize as slave at address |
| `end` | `void end()` | De-initialize I2C |
| `setClock` | `void setClock(uint32_t freq)` | Set clock speed (Hz) |

#### Master Transmit

| Method | Signature | Description |
|--------|-----------|-------------|
| `beginTransmission` | `void beginTransmission(unsigned char addr)` | Start write to slave |
| `endTransmission` | `unsigned char endTransmission(void)` | End write, send stop |
| `endTransmission` | `unsigned char endTransmission(unsigned char sendStop)` | End write, optional stop/restart |
| `write` | `size_t write(unsigned char data)` | Write one byte |
| `write` | `size_t write(const unsigned char* data, size_t quantity)` | Write multiple bytes |

`endTransmission` return values:
| Value | Meaning |
|-------|---------|
| 0 | Success |
| 1 | Data too long |
| 2 | NACK on address |
| 3 | NACK on data |
| 4 | Other error |

#### Master Receive

| Method | Signature | Description |
|--------|-----------|-------------|
| `requestFrom` | `uint8_t requestFrom(uint8_t addr, uint8_t quantity)` | Request bytes from slave |
| `requestFrom` | `uint8_t requestFrom(uint8_t addr, uint8_t quantity, uint8_t sendStop)` | With stop control |
| `requestFrom` | `uint8_t requestFrom(uint8_t addr, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop)` | With internal address |
| `available` | `int available()` | Bytes available to read |
| `read` | `int read()` | Read one byte |
| `peek` | `int peek()` | Peek at next byte without consuming it |

#### Slave Callbacks

| Method | Signature | Description |
|--------|-----------|-------------|
| `onReceive` | `void onReceive(void (*handler)(int))` | Set slave receive callback |
| `onRequest` | `void onRequest(void (*handler)(void))` | Set slave request callback |

#### Utility

| Method | Signature | Description |
|--------|-----------|-------------|
| `flush` | `void flush()` | Flush buffers |

---

## Global Instances

| Instance | Description |
|----------|-------------|
| `Wire` | Primary I2C bus |
| `Wire1` | Secondary I2C bus |

---

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `BUFFER_LENGTH` | 32 | Internal buffer size (bytes) |
| `MASTER_ADDRESS` | 0x33 | Default master address |
| `I2C_OK` | 0 | Success return code |
| `I2C_TIMEOUT` | 1 | Timeout return code |

---

## Dependencies

- mbed I2C HAL (`i2c_api.h`)
