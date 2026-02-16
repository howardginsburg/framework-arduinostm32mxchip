# Arduino Core

The `cores/arduino/` directory contains the Arduino-compatible runtime for the MXChip AZ3166. It provides the standard Arduino API plus MXChip-specific functionality.

---

## Arduino API

| Document | Description |
|----------|-------------|
| [Arduino API](ArduinoAPI.md) | Digital/analog I/O, timing, serial, interrupts, math, strings, print/stream |

## Hardware Peripherals

| Document | Description |
|----------|-------------|
| [OLED Display](OledDisplay.md) | 128×64 OLED screen driver (`Screen` global) |
| [EEPROM](EEPROM.md) | STSAFE-A100 secure element storage zones |
| [TLS Socket](TLSSocket.md) | mbedTLS-based secure socket (server-only and mutual TLS) |
| [Watchdog](Watchdog.md) | Hardware independent watchdog timer (IWDG) |

## Device Configuration

| Document | Description |
|----------|-------------|
| [DeviceConfig](DeviceConfig.md) | Profile-based configuration system, zone mapping, validation, CLI, and UI metadata |

## Networking

| Document | Description |
|----------|-------------|
| [HTTP Client](HTTPClient.md) | HTTP and HTTPS request client with URL parsing |
| [HTTP Server](HTTPServer.md) | Embedded web server for device configuration UI |
| [NTP Client](NTPClient.md) | UDP-based NTP time synchronization |

## System Services

| Document | Description |
|----------|-------------|
| [System Services](SystemServices.md) | Boot sequence, WiFi management, OTA updates, timers, logging, DNS |
