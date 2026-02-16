# Board Variant — MXChip AZ3166

Arduino board variant definitions for the MXChip AZ3166 IoT DevKit.

> **Source:** [variants/MXChip_AZ3166/](../../variants/MXChip_AZ3166/)

---

## Pin Mapping

The AZ3166 exposes a micro:bit-style edge connector with the following Arduino pin definitions:

| Pin | STM32 | Capabilities |
|-----|-------|-------------|
| `PIN_0` | PB_0 | GPIO, ADC, SPI CS |
| `PIN_1` | PB_6 | GPIO, UART TX |
| `PIN_2` | PB_7 | GPIO, UART RX |
| `PIN_4` | PA_5 | GPIO, ADC |
| `PIN_5` | PA_4 | GPIO, ADC |
| `PIN_6` | PB_4 | GPIO, PWM |
| `PIN_7` | PB_3 | GPIO, PWM |
| `PIN_8` | PC_13 | GPIO |
| `PIN_9` | PB_10 | GPIO |
| `PIN_10` | PC_7 | GPIO, PWM |
| `PIN_11` | PA_10 | GPIO |
| `PIN_12` | PB_2 | GPIO |
| `PIN_13` | PB_13 | GPIO, SPI SCK |
| `PIN_14` | PB_14 | GPIO, SPI MISO |
| `PIN_15` | PB_15 | GPIO, SPI MOSI |
| `PIN_16` | PC_6 | GPIO |
| `PIN_19` | PB_8 | GPIO, I2C SCL |
| `PIN_20` | PB_9 | GPIO, I2C SDA |

Pins 3, 17, and 18 are not defined.

---

## Named Signal Aliases

From [PinNames.h](../../system/mbed-os/targets/TARGET_MXCHIP/TARGET_AZ3166/PinNames.h):

### LEDs

| Name | Pin | Description |
|------|-----|-------------|
| `LED_BUILTIN` | PC_13 | User LED |
| `LED_WIFI` | PB_2 | WiFi status LED |
| `LED_AZURE` | PA_15 | Azure connection LED |
| `LED_USER` | PC_13 | Same as LED_BUILTIN |
| `RGB_R` | PB_4 | RGB red channel |
| `RGB_G` | PB_3 | RGB green channel |
| `RGB_B` | PC_7 | RGB blue channel |

### Buttons

| Name | Pin |
|------|-----|
| `USER_BUTTON_A` | PA_4 |
| `USER_BUTTON_B` | PA_10 |

### Communication

| Name | Pin | Function |
|------|-----|----------|
| `SERIAL_TX` | PB_6 | UART transmit |
| `SERIAL_RX` | PB_7 | UART receive |
| `I2C_SCL` | PB_8 | I2C clock |
| `I2C_SDA` | PB_9 | I2C data |
| `SPI_MOSI` | PB_15 | SPI data out |
| `SPI_MISO` | PB_14 | SPI data in |
| `SPI_CLK` | PB_13 | SPI clock |
| `SPI_SS` | PB_0 | SPI chip select |

### Other

| Name | Pin | Function |
|------|-----|----------|
| `IR_DATA` | PB_10 | IR transmitter |
| `PWM_OUT` | PB_3 | PWM output (D9) |

---

## Memory Layout

From [AZ3166.ld](../../variants/MXChip_AZ3166/linker_scripts/gcc/AZ3166.ld):

| Region | Start | Size | Description |
|--------|-------|------|-------------|
| FLASH | `0x0800C000` | ~976 KB | Application code (48 KB reserved for bootloader) |
| RAM | `0x200001C4` | ~255.5 KB | SRAM (452 bytes reserved for vectors) |

Entry point: `Reset_Handler`

Standard sections: `.text`, `.ARM.extab`, `.ARM.exidx`, `.data`, `.bss`, `.heap`, `.stack`

Stack top is at the end of RAM.

---

## Files

| File | Description |
|------|-------------|
| [variant.h](../../variants/MXChip_AZ3166/variant.h) | Board variant header (includes pins_arduino.h) |
| [pins_arduino.h](../../variants/MXChip_AZ3166/pins_arduino.h) | Arduino edge-connector pin definitions |
| [AZ3166.ld](../../variants/MXChip_AZ3166/linker_scripts/gcc/AZ3166.ld) | GCC linker script |

---

## See Also

- [Board Hardware](../system/BoardHardware.md) — Full GPIO table, peripheral buses, and clock config
- [Arduino API](../cores/ArduinoAPI.md) — Digital/analog I/O using these pins
