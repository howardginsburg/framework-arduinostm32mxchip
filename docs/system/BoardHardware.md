# Board Hardware Configuration

GPIO mapping, peripheral assignments, clock configuration, and Arduino connector pinout for the MXChip AZ3166.

> **Source:** [system/az3166-driver/TARGET_AZ3166/](../../system/az3166-driver/TARGET_AZ3166/)

---

## Board Identity

| Constant | Value |
|----------|-------|
| `DEFAULT_NAME` | `"AZ3166"` |
| `MODEL` | `"AZ3166_1"` |
| `HARDWARE_REVISION` | `"1.0"` |
| `MCU_CLOCK_HZ` | 96,000,000 (96 MHz) |

---

## Clock Configuration

- **HSE:** External crystal oscillator
- **PLL:** M=13, N=192, P=4, Q=8, R=2
- **AHB:** sysclk / 1
- **APB1:** AHB / 2
- **APB2:** AHB / 1
- **Flash wait states:** 3 cycles
- **Tick rate:** 1000 Hz (`MICO_DEFAULT_TICK_RATE_HZ`)

---

## Arduino Connector Pinout

### Digital Pins

| Arduino Name | MICO GPIO | STM32 Pin | mbed Name | Function |
|-------------|-----------|-----------|-----------|----------|
| D0 / RXD | `MICO_GPIO_29` | PB_7 | `SERIAL_RX` | UART RX |
| D1 / TXD | `MICO_GPIO_30` | PB_6 | `SERIAL_TX` | UART TX |
| D4 | `MICO_GPIO_19` | PB_10 | — | GPIO, IR_DATA |
| D5 | `MICO_GPIO_16` | PC_13 | `LED_BUILTIN` | GPIO |
| D6 | `MICO_GPIO_14` | PC_0 | — | GPIO, WAKE_UP |
| D8 | `MICO_GPIO_2` | PB_2 | `LED_WIFI` | GPIO |
| D9 | `MICO_GPIO_27` | PB_3 | `PWM_OUT` | GPIO, PWM |
| D10 / CS | `MICO_GPIO_5` | PB_12 | `SPI_SS` | SPI chip select |
| D11 / MOSI | `MICO_GPIO_4` | PB_15 | `SPI_MOSI` | SPI data out |
| D12 / MISO | `MICO_GPIO_7` | PB_14 | `SPI_MISO` | SPI data in |
| D13 / SCK | `MICO_GPIO_6` | PB_13 | `SPI_CLK` | SPI clock |
| D14 / SDA | `MICO_GPIO_18` | PB_9 | `I2C_SDA` | I2C data |
| D15 / SCL | `MICO_GPIO_17` | PB_8 | `I2C_SCL` | I2C clock |

### Analog Pins

| Arduino Name | MICO ADC | STM32 Pin |
|-------------|----------|-----------|
| A2 | `MICO_ADC_1` | PA_4 |
| A3 | `MICO_ADC_2` | PA_5 |

A0, A1, A4, A5 are not connected.

---

## LEDs

| Name | mbed Name | STM32 Pin | Description |
|------|-----------|-----------|-------------|
| System LED | `LED_BUILTIN` | PC_13 | User-controllable LED |
| WiFi LED | `LED_WIFI` | PB_2 | WiFi status |
| Azure LED | `LED_AZURE` | PA_15 | Azure connection status |
| User LED | `LED_USER` | PC_13 | Same as LED_BUILTIN |
| RGB Red | `RGB_R` | PB_4 | RGB LED red channel |
| RGB Green | `RGB_G` | PB_3 | RGB LED green channel |
| RGB Blue | `RGB_B` | PC_7 | RGB LED blue channel |

---

## Buttons

| Name | mbed Name | STM32 Pin |
|------|-----------|-----------|
| Button A | `USER_BUTTON_A` | PA_4 |
| Button B | `USER_BUTTON_B` | PA_10 |

---

## Peripheral Buses

### UART

| Bus | Pins | Baud Rate | Usage |
|-----|------|-----------|-------|
| `MICO_UART_1` | PC_6 (TX), PC_7 (RX) | 115200 | STDIO / CLI |
| `MICO_UART_2` | PB_6 (TX), PB_7 (RX) | — | Arduino Serial |

### I2C

| Bus | Pins | Usage |
|-----|------|-------|
| `MICO_I2C_1` | PB_8 (SCL), PB_9 (SDA) | Sensors, OLED, STSAFE |

### SPI

| Bus | Pins | Usage |
|-----|------|-------|
| `MICO_SPI_1` | PB_13 (SCK), PB_14 (MISO), PB_15 (MOSI), PB_12 (SS) | Arduino SPI header |

### QSPI Flash

| Signal | Pin |
|--------|-----|
| CS | PC_11 |
| CLK | PB_1 |
| D0 | PA_6 |
| D1 | PA_7 |
| D2 | PC_4 |
| D3 | PC_5 |

### SDIO (WiFi)

| Signal | Pin |
|--------|-----|
| CLK | PC_12 |
| CMD | PD_2 |
| D0 | PC_8 |
| D1 | PC_9 |
| D2 | PC_10 |
| D3 | PB_5 |
| OOB_IRQ | PC_0 |

---

## Flash Partitions

| Partition | Description |
|-----------|-------------|
| `BOOTLOADER` | Boot loader |
| `APPLICATION` | Main application |
| `ATE` | Automatic Test Equipment |
| `OTA_TEMP` | OTA firmware staging |
| `RF_FIRMWARE` | WiFi chipset firmware |
| `PARAMETER_1` | System parameters |
| `PARAMETER_2` | User parameters |
| `FILESYS` | File system |

---

## Enabled Features

| Feature | Status |
|---------|--------|
| MCU RTC | Enabled (`MICO_ENABLE_MCU_RTC`) |
| Bluetooth | Enabled (`MICO_BLUETOOTH_ENABLE`) |
| NVIC Priority Bits | 4 (`CORTEX_NVIC_PRIO_BITS`) |
| Restore Default Timeout | 3000 ms |

---

## See Also

- [WiFi Driver](WiFiDriver.md) — SDIO WiFi interface
- [Secure Element](SecureElement.md) — I2C secure element
- [Board Variant](../variants/README.md) — Arduino pin definitions and linker script
