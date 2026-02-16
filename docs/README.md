# Documentation

This directory contains detailed documentation for the MXChip AZ3166 IoT DevKit SDK.

---

## Getting Started

New to the MXChip AZ3166? Start here:

| Document | Description |
|----------|-------------|
| [Getting Started](GettingStarted.md) | Installation, device configuration, first sketch, CLI/Web UI reference, and architecture overview |

---

## Guides

| Document | Description |
|----------|-------------|
| [Boot Sequence](mxchip_boot_sequence.md) | Mermaid sequence diagrams of the full boot flow |
| [Custom Connection Profiles](CustomProfile.md) | Define custom EEPROM zone 
| [Building a Standalone Binary](BinaryBuild.md) | Guide for building a standalone firmware that is easily flashed to the device |
mappings for your project |
| [TLS Socket Modifications](TLSPATCH.md) | Detailed changes to the TLS layer from the original framework |

---

## Library Reference

API documentation for each bundled library:

| Library | Description |
|---------|-------------|
| [Audio](libraries/Audio.md) | Record and play audio using the onboard NAU88C10 codec |
| [AudioV2](libraries/AudioV2.md) | Enhanced audio with callback streaming, volume control, and ALC |
| [AzureIoT](libraries/AzureIoT.md) | Azure IoT Hub and DPS client over MQTT |
| [FileSystem](libraries/FileSystem.md) | FAT filesystem on onboard SPI flash |
| [PubSubClient](libraries/PubSubClient.md) | Lightweight MQTT 3.1.1 client |
| [Sensors](libraries/Sensors.md) | Drivers for all onboard sensors (temperature, humidity, pressure, IMU, magnetometer, IrDA, RGB LED) |
| [SPI](libraries/SPI.md) | Standard Arduino SPI Master interface |
| [WebSocket](libraries/WebSocket.md) | WebSocket client (RFC 6455) |
| [WiFi](libraries/WiFi.md) | WiFi station/AP, TCP client/server, TLS client, UDP |
| [Wire](libraries/Wire.md) | Standard Arduino I2C/TWI interface |

---

## Arduino Core

API documentation for the Arduino runtime in `cores/arduino/`:

| Document | Description |
|----------|-------------|
| [Arduino API](cores/ArduinoAPI.md) | Digital/analog I/O, timing, serial, interrupts, math, strings |
| [OLED Display](cores/OledDisplay.md) | 128×64 OLED screen driver (`Screen` global) |
| [EEPROM](cores/EEPROM.md) | STSAFE-A100 secure element storage zones |
| [TLS Socket](cores/TLSSocket.md) | mbedTLS-based secure socket (server-only and mutual TLS) |
| [Watchdog](cores/Watchdog.md) | Hardware independent watchdog timer (IWDG) |
| [DeviceConfig](cores/DeviceConfig.md) | Profile-based configuration system, validation, CLI, and web UI |
| [HTTP Client](cores/HTTPClient.md) | HTTP/HTTPS request client with URL parsing |
| [HTTP Server](cores/HTTPServer.md) | Embedded web server for device configuration |
| [NTP Client](cores/NTPClient.md) | UDP-based NTP time synchronization |
| [System Services](cores/SystemServices.md) | Boot sequence, WiFi, OTA updates, logging, DNS |

---

## System Platform

Platform support layer documentation from `system/`:

| Document | Description |
|----------|-------------|
| [mbed OS](system/MbedOS.md) | mbed OS 5.4.3 RTOS, drivers, networking, and TLS configuration |
| [WiFi Driver](system/WiFiDriver.md) | EMW10xx WiFi interface and lwIP stack |
| [Secure Element](system/SecureElement.md) | STSAFE-A100 HAL for secure storage |
| [Board Hardware](system/BoardHardware.md) | GPIO mapping, peripherals, clock, and Arduino connector |
| [MiCO Framework](system/MiCO.md) | MiCO OS abstraction (RTOS, WiFi, sockets, drivers) |
| [Utilities](system/Utilities.md) | Memory debug and diagnostic helpers |

---

## Board Variant

Board-specific pin definitions and memory layout from `variants/MXChip_AZ3166/`:

| Document | Description |
|----------|-------------|
| [MXChip AZ3166](variants/README.md) | Pin mapping, signal aliases, and linker script memory layout |


