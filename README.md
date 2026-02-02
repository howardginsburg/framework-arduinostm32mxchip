# MXChip AZ3166 IoT DevKit SDK (Community Fork)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A community-maintained fork of the [Microsoft Azure IoT DevKit SDK](https://github.com/microsoft/devkit-sdk) for the MXChip AZ3166 IoT DevKit, optimized for **PlatformIO** and general-purpose IoT development.

> **Note**: The original Microsoft SDK was archived in April 2023 and devices can no longer connect to Azure IoT Hub using the built-in libraries. This fork removes deprecated Azure IoT Hub dependencies and adds modern TLS/MQTT capabilities.

---

## Table of Contents

- [About the Hardware](#about-the-hardware)
- [What's Changed](#whats-changed)
- [Sample Projects](#sample-projects)
- [Installation](#installation)
- [WiFiClientSecure Usage](#wificlientsecure-usage)
- [Configuration CLI](#configuration-cli)
- [License](#license)

---

## About the Hardware

The **MXChip AZ3166 IoT DevKit** features ARM Cortex-M processors with:

| Component | Details |
|-----------|---------|
| **Processor** | ST Microelectronics STM32F412
| **WiFi** | Cypress BCM43362
| **Display** | 128×64 OLED screen |
| **Audio** | Headphone/speaker output, stereo microphone |
| **Sensors** | Humidity, temperature, pressure, accelerometer, gyroscope, magnetometer |
| **Buttons** | A, B, and Reset |
| **LEDs** | RGB LED, user LED, WiFi LED |

---

## What's Changed

This fork includes significant modifications from the original SDK:

### Removed

| Component | Reason |
|-----------|--------|
| **Azure IoT Hub libraries** | The built in libraries are outdated; devices can no longer connect |
| **Board telemetry collector** | Defunct Microsoft telemetry service |
| **Built-in MQTT library** | Did not support TLS connections |

### Added

| Feature | Description |
|---------|-------------|
| **WiFiClientSecure** | Arduino-compatible TLS client for use with any MQTT library (e.g., PubSubClient) |
| **TLSSocket improvements** | Revamped the TLS socket layer to remediate bugs, etc when using MQTT mTLS (see [TLSPATCH.md](TLSPATCH.md)) |
| **MQTT configuration storage** | EEPROM functions for broker address, device ID, and password |
| **CLI commands** | New serial commands for MQTT and certificate configuration |
| **Certificate management** | Store CA certs, client certs, and private keys in secure EEPROM |

---

## Sample Projects

These complete sample projects demonstrate the framework capabilities:

### [MXChipSecureMQTTDemo](https://github.com/howardginsburg/MXChipSecureMQTTDemo)

A demonstration of mutual TLS (mTLS) MQTT connectivity using X.509 client certificate authentication. This project showcases bidirectional publish/subscribe support with real sensor telemetry (temperature, humidity, pressure) sent as JSON every 5 seconds. Features include automatic WiFi/MQTT reconnection, OLED display for status and readings, and RGB LED indicators for connection state. Includes comprehensive setup instructions for Azure Event Grid MQTT broker with client certificate authentication.

### [MXChipIoTHubDemo](https://github.com/howardginsburg/MXChipIoTHubDemo)

A pure MQTT implementation for connecting to Azure IoT Hub without requiring the deprecated Azure SDK that was a part of the original framework. Uses PubSubClient for direct MQTT communication with full IoT Hub functionality: Device-to-Cloud (D2C) telemetry, Cloud-to-Device (C2D) message receiving, and Device Twin support for reported/desired properties. This lightweight approach demonstrates how to implement Azure IoT Hub connectivity from scratch using standard MQTT topics and SAS token authentication.

---

## Installation

### Prerequisites

- **Visual Studio Code** with the **PlatformIO** extension
- **MXChip AZ3166** with firmware 2.0.0 or later

### Step 1: Update Firmware

If your board has outdated firmware (especially if you've run Eclipse RTOS samples), update it:

1. Download [devkit-firmware-2.0.0.bin](firmware/devkit-firmware-2.0.0.bin)
2. Connect your AZ3166 via USB (it mounts as a drive)
3. Copy the `.bin` file to the drive root
4. The board will automatically flash and reboot

### Step 2: Create PlatformIO Project

```bash
# Create a new project or use an existing one
pio init --board mxchip_az3166
```

### Step 3: Configure platformio.ini

Add this to your `platformio.ini`:

```ini
[env:mxchip_az3166]
platform = ststm32
board = mxchip_az3166
framework = arduino

platform_packages =
    framework-arduinostm32mxchip@https://github.com/howardginsburg/framework-arduinostm32mxchip.git
```

---

## WiFiClientSecure Usage

The `WiFiClientSecure` class provides Arduino-compatible TLS connections:

### Basic Example with PubSubClient

```cpp
#include <AZ3166WiFi.h>
#include <AZ3166WiFiClientSecure.h>
#include <PubSubClient.h>

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

// Root CA certificate (PEM format)
const char* rootCA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDdzCCAl+gAwIBAgIEAgAAuTANBg...\n" \
"-----END CERTIFICATE-----\n";

void setup() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    // Configure TLS
    wifiClient.setCACert(rootCA);
    // Optional: for mutual TLS
    // wifiClient.setCertificate(clientCert);
    // wifiClient.setPrivateKey(clientKey);
    
    mqttClient.setServer("your-broker.com", 8883);
}

void loop() {
    if (!mqttClient.connected()) {
        mqttClient.connect("clientId", "username", "password");
    }
    
    mqttClient.publish("topic", "Hello from MXChip!");
    delay(5000);
}
```

### WiFiClientSecure API

| Method | Description |
|--------|-------------|
| `setCACert(const char* ca)` | Set root CA certificate for server verification |
| `setCertificate(const char* cert)` | Set client certificate for mutual TLS |
| `setPrivateKey(const char* key)` | Set client private key for mutual TLS |
| `setInsecure()` | Disable certificate verification (not recommended) |
| `setTimeout(unsigned int ms)` | Set socket timeout in milliseconds |

---

## Configuration CLI

Access the configuration console by holding **Button A** while pressing **Reset**. Connect via serial (115200 baud).

### Available Commands

| Command | Description |
|---------|-------------|
| `help` | Show all available commands |
| `version` | Display SDK and firmware versions |
| `exit` | Reboot the device |
| `scan` | Scan for available WiFi networks |
| **WiFi Configuration** | |
| `set_wifissid <ssid>` | Set WiFi network name |
| `set_wifipwd <password>` | Set WiFi password |
| **MQTT Configuration** | |
| `set_mqtt <url>` | Set MQTT broker address |
| `set_deviceid <id>` | Set device/client ID |
| `set_device_pwd <password>` | Set device password |
| **Certificate Configuration** | |
| `set_cacert "<pem>"` | Set CA certificate (use `\n` for newlines) |
| `set_clientcert "<pem>"` | Set client certificate |
| `set_clientkey "<pem>"` | Set client private key |
| `cert_status` | Show certificate storage status |
| **Security** | |
| `enable_secure <level>` | Enable secure channel encryption |

---

## License

This project is licensed under the MIT License - see the original [Microsoft devkit-sdk](https://github.com/microsoft/devkit-sdk) for details.

Portions of this code are Copyright (c) Microsoft Corporation.