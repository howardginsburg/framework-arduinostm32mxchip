# MXChip AZ3166 IoT DevKit SDK (Community Fork)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A community-maintained fork of the [Microsoft Azure IoT DevKit SDK](https://github.com/microsoft/devkit-sdk) for the MXChip AZ3166 IoT DevKit, optimized for **PlatformIO** and general-purpose IoT development.

> **Note**: The original Microsoft SDK was archived in April 2023 and devices can no longer connect to Azure IoT Hub using the built-in libraries. This fork removes deprecated Azure IoT Hub dependencies and adds modern TLS/MQTT capabilities.

---

## Table of Contents

- [About the Hardware](#about-the-hardware)
- [What's Changed](#whats-changed)
- [Framework Features](#framework-features)
- [Connection Profiles](#connection-profiles)
- [Sample Projects](#sample-projects)
- [Installation](#installation)
- [WiFiClientSecure Usage](#wificlientsecure-usage)
- [Configuration CLI](#configuration-cli)
- [Web Configuration UI](#web-configuration-ui)
- [Building a Standalone Binary](#building-a-standalone-binary)
- [Architecture](#architecture)
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
| **Secure Storage** | STSAFE-A100 secure element with multiple storage zones |

---

## What's Changed

This fork includes significant modifications from the original SDK:

### Removed

| Component | Reason |
|-----------|--------|
| **Azure IoT Hub libraries** | The original built in libraries are outdated; devices can no longer connect |
| **Board telemetry collector** | Defunct Microsoft telemetry service |
| **Built-in MQTT library** | Original Paho library Did not support mTLS connections |

### Added

| Feature | Description |
|---------|-------------|
| **WiFiClientSecure** | Arduino-compatible TLS client for use with any MQTT library (e.g., PubSubClient) |
| **TLSSocket improvements** | Revamped the TLS socket layer to remediate bugs, etc when using MQTT mTLS (see [TLSPATCH.md](TLSPATCH.md)) |
| **Connection Profiles** | Pre-defined connection profiles for MQTT, Azure IoT Hub, and Azure DPS |
| **DeviceConfig System** | Unified configuration storage with multi-zone support for large certificates |
| **SensorManager** | Simple API for all onboard sensors (temperature, humidity, pressure, IMU, magnetometer) |
| **AzureIoT Library** | Complete Azure IoT Hub client with DPS provisioning, SAS token generation, and Device Twin support |
| **PubSubClient Library** | Bundled MQTT client library (Nick O'Leary, MIT license) — no external lib_deps needed |
| **Updated Web Configuration UI** | Browser-based setup via WiFi AP mode |
| **Input Validation** | Centralized validation for URLs, certificates, and connection strings |
| **Updated CLI commands** | Extensible serial commands for all configuration settings |

---

## Framework Features

The framework provides two key abstractions that simplify building IoT projects:

### DeviceConfig

The `DeviceConfig` system provides unified access to configuration stored in the STSAFE secure element. Configuration is loaded automatically at startup based on your connection profile.

```cpp
#include <DeviceConfig.h>

// WiFi credentials
const char* ssid = DeviceConfig_GetWifiSsid();
const char* password = DeviceConfig_GetWifiPassword();

// MQTT/TLS settings
const char* host = DeviceConfig_GetBrokerHost();
int port = DeviceConfig_GetBrokerPort();
const char* caCert = DeviceConfig_GetCACert();
const char* clientCert = DeviceConfig_GetClientCert();
const char* clientKey = DeviceConfig_GetClientKey();

// Device identity (extracted from cert CN or connection string)
const char* deviceId = DeviceConfig_GetDeviceId();
```

### SensorManager

The `Sensors` global provides easy access to all onboard sensors. It's initialized automatically at startup.

```cpp
#include <SensorManager.h>

// Individual readings
float temp = Sensors.getTemperature();      // °C
float humidity = Sensors.getHumidity();     // %RH
float pressure = Sensors.getPressure();     // hPa

// Motion sensors
int32_t ax, ay, az;
Sensors.getAccelerometer(ax, ay, az);       // mg

int32_t gx, gy, gz;
Sensors.getGyroscope(gx, gy, gz);           // mdps

int32_t mx, my, mz;
Sensors.getMagnetometer(mx, my, mz);        // mGauss

// Read all sensors at once
SensorData data = Sensors.readAll();

// Generate JSON telemetry
char json[512];
Sensors.toJson(json, sizeof(json));
```

| Sensor | Measurements | Units |
|--------|--------------|-------|
| HTS221 | Temperature, Humidity | °C, %RH |
| LPS22HB | Barometric Pressure | hPa |
| LSM6DSL | Accelerometer, Gyroscope | mg, mdps |
| LIS2MDL | Magnetometer | mGauss |

### AzureIoT Library

The `AzureIoT` library provides a complete Azure IoT Hub client with DPS provisioning support. It handles credential loading, SAS token generation, MQTT connection management, telemetry, C2D messages, and Device Twin operations. Select a connection profile via build flags and the library handles everything else.

```cpp
#include <AzureIoTHub.h>

void onC2D(const char* topic, const char* payload, unsigned int len) {
    Serial.println(payload);
}

void setup() {
    WiFi.begin();  // credentials from EEPROM

    azureIoTInit();       // loads config, provisions via DPS if needed
    azureIoTConnect();    // connects to IoT Hub over MQTT
    azureIoTSetC2DCallback(onC2D);
}

void loop() {
    azureIoTLoop();       // process MQTT messages
    azureIoTSendTelemetry("{\"temp\":25.3}");
    delay(10000);
}
```

The library is split into focused modules:

| Module | Purpose |
|--------|---------|
| `AzureIoTHub.h/.cpp` | Public API — init, connect, telemetry, twin, callbacks |
| `AzureIoTDPS.h/.cpp` | Device Provisioning Service registration over MQTT |
| `AzureIoTCrypto.h/.cpp` | SAS token generation, HMAC-SHA256, group key derivation |
| `AzureIoTConfig.h` | Protocol constants and Azure root CA certificate |

---

## Connection Profiles

The SDK supports multiple connection profiles that determine which settings are available and how EEPROM zones are allocated:

| Profile | Description | Settings |
|---------|-------------|----------|
| `PROFILE_NONE` | No EEPROM - config in code | None |
| `PROFILE_MQTT_USERPASS` | Basic MQTT | WiFi, Broker URL, Device ID, Password |
| `PROFILE_MQTT_USERPASS_TLS` | MQTT over TLS | WiFi, Broker URL, Device ID, Password, CA Cert |
| `PROFILE_MQTT_MTLS` | MQTT with mutual TLS | WiFi, Broker URL, CA Cert, Client Cert, Client Key |
| `PROFILE_IOTHUB_SAS` | Azure IoT Hub (SAS) | WiFi, Connection String |
| `PROFILE_IOTHUB_CERT` | Azure IoT Hub (X.509) | WiFi, Connection String, Device Cert |
| `PROFILE_DPS_SAS` | Azure DPS (Symmetric Key) | WiFi, DPS Endpoint, Scope ID, Registration ID, Symmetric Key |
| `PROFILE_DPS_SAS_GROUP` | Azure DPS (Group SAS) | WiFi, DPS Endpoint, Scope ID, Registration ID, Symmetric Key |
| `PROFILE_DPS_CERT` | Azure DPS (X.509) | WiFi, DPS Endpoint, Scope ID, Registration ID, Device Cert |

### Setting the Profile

Add a build flag to your `platformio.ini`:

```ini
[env:mxchip_az3166]
platform = ststm32
board = mxchip_az3166
framework = arduino

build_flags =
    -DCONNECTION_PROFILE=PROFILE_MQTT_MTLS

platform_packages =
    framework-arduinostm32mxchip@https://github.com/howardginsburg/framework-arduinostm32mxchip.git
```


## Sample Projects

These complete sample projects demonstrate the framework capabilities:

### [MXChipSecureMQTTDemo](https://github.com/howardginsburg/MXChipSecureMQTTDemo)

A demonstration of mutual TLS (mTLS) MQTT connectivity using X.509 client certificate authentication. This project showcases bidirectional publish/subscribe support with real sensor telemetry (temperature, humidity, pressure) sent as JSON every 5 seconds. Features include automatic WiFi/MQTT reconnection, OLED display for status and readings, and RGB LED indicators for connection state. Includes comprehensive setup instructions for Azure Event Grid MQTT broker with client certificate authentication.

### [MXChipIoTHubDemo](https://github.com/howardginsburg/MXChipIoTHubDemo)

A pure MQTT implementation for connecting to Azure IoT Hub without requiring the deprecated Azure SDK that was a part of the original framework. Uses the framework's built-in AzureIoT library for direct MQTT communication with full IoT Hub functionality: Device-to-Cloud (D2C) telemetry, Cloud-to-Device (C2D) message receiving, and Device Twin support for reported/desired properties. Supports individual and group enrollment via DPS with automatic device key derivation. This lightweight approach demonstrates how to implement Azure IoT Hub connectivity using standard MQTT topics and SAS token authentication.

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

Add this to your `platformio.ini`, setting the connection profile for your use case (see [Connection Profiles](#connection-profiles)):

```ini
[env:mxchip_az3166]
platform = ststm32
board = mxchip_az3166
framework = arduino

build_flags =
    -DCONNECTION_PROFILE=PROFILE_MQTT_MTLS

platform_packages =
    framework-arduinostm32mxchip@https://github.com/howardginsburg/framework-arduinostm32mxchip.git
```

---

## WiFiClientSecure Usage

The `WiFiClientSecure` class provides Arduino-compatible TLS connections:

### Basic Example with PubSubClient (mTLS Profile)

This example uses `PROFILE_MQTT_MTLS`. WiFi credentials, broker URL, certificates, and keys are loaded from EEPROM (configured via CLI or Web UI). The framework automatically initializes DeviceConfig on startup.

```cpp
#include <AZ3166WiFi.h>
#include <AZ3166WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DeviceConfig.h>

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi using stored credentials
    WiFi.begin(DeviceConfig_GetWifiSsid(), DeviceConfig_GetWifiPassword());
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    // Configure TLS with certificates from EEPROM
    wifiClient.setCACert(DeviceConfig_GetCACert());
    wifiClient.setCertificate(DeviceConfig_GetClientCert());
    wifiClient.setPrivateKey(DeviceConfig_GetClientKey());
    
    // Connect to broker (host/port parsed from stored broker URL)
    mqttClient.setServer(DeviceConfig_GetBrokerHost(), DeviceConfig_GetBrokerPort());
}

void loop() {
    if (!mqttClient.connected()) {
        // Device ID extracted from client certificate CN
        mqttClient.connect(DeviceConfig_GetDeviceId());
    }
    
    mqttClient.publish("telemetry", "Hello from MXChip!");
    mqttClient.loop();
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

## Web Configuration UI

The SDK includes a browser-based configuration interface accessible via WiFi AP mode.

### Enabling AP Mode

1. Hold **Button A** during boot, or
2. Call `WiFi.beginAP("DevKit-Setup")` in your code

### Accessing the UI

1. Connect to the device's WiFi access point
2. Navigate to `http://192.168.0.1` in your browser
3. Configure settings using the web form
4. Click **Save** to store settings to EEPROM

### Features

- **Profile-aware**: Only shows settings relevant to the active connection profile
- **Input validation**: Validates URLs, certificates, and connection strings before saving
- **Visual feedback**: Shows validation errors and success status for each field
- **Responsive design**: Works on mobile and desktop browsers

---

## Configuration CLI

Access the configuration console by holding **Button A** while pressing **Reset**. Connect via serial (115200 baud).

### Available Commands

Commands vary based on the active connection profile.

| Command | Description | Profiles |
|---------|-------------|----------|
| `help` | Show all available commands | All |
| `version` | Display SDK and firmware versions | All |
| `exit` | Reboot the device | All |
| `scan` | Scan for available WiFi networks | All |
| `status` | Show current configuration status | All |
| **WiFi Configuration** | | |
| `set_wifissid <ssid>` | Set WiFi network name | All (except NONE) |
| `set_wifipwd <password>` | Set WiFi password | All (except NONE) |
| **MQTT Configuration** | | |
| `set_broker <url>` | Set MQTT broker URL (e.g., `mqtts://broker:8883`) | MQTT profiles |
| `set_deviceid <id>` | Set device/client ID | MQTT_USERPASS* |
| `set_devicepwd <password>` | Set device password | MQTT_USERPASS* |
| **Azure IoT Hub** | | |
| `set_connstring <conn>` | Set IoT Hub connection string | IOTHUB_* |
| **Azure DPS** | | |
| `set_dps_endpoint <url>` | Set DPS global endpoint | DPS_* |
| `set_scopeid <id>` | Set DPS ID Scope | DPS_* |
| `set_regid <id>` | Set DPS registration ID | DPS_* |
| `set_symkey <key>` | Set DPS symmetric key | DPS_SAS, DPS_SAS_GROUP |
| **Certificate Configuration** | | |
| `set_cacert "<pem>"` | Set CA/root certificate | TLS profiles |
| `set_clientcert "<pem>"` | Set client certificate | MTLS/CERT profiles |
| `set_clientkey "<pem>"` | Set client private key | MQTT_MTLS |
| `set_devicecert "<pem>"` | Set device certificate | IOTHUB_CERT, DPS_CERT |

### Entering Certificates and Keys via CLI

PEM-formatted values (certificates and private keys) require special handling in the CLI because they contain spaces and newlines.

**Rules:**
1. The value **must be quoted** — PEM headers like `BEGIN EC PRIVATE KEY` contain spaces, which would split the argument
2. Use `\n` to represent newlines — the CLI converts `\n` sequences to actual newline characters
3. The entire PEM content goes on a single line between the quotes

**Example — Setting an EC private key:**
```
set_clientkey "-----BEGIN EC PRIVATE KEY-----\nMIGkAgEBBDA...base64data...\n-----END EC PRIVATE KEY-----"
```

**Example — Setting a CA certificate:**
```
set_cacert "-----BEGIN CERTIFICATE-----\nMIIC...base64data...\n-----END CERTIFICATE-----"
```

**Preparing PEM files for CLI entry:**

Convert a multi-line PEM file to a single-line string with `\n` markers:

```bash
# Linux/macOS
awk 'NR>1{printf "\\n"} {printf "%s",$0}' cert.pem

# PowerShell
(Get-Content cert.pem -Raw).TrimEnd() -replace "`n","\n" -replace "`r",""
```

Then paste the output between quotes after the CLI command.

> **Tip**: The Web Configuration UI provides a simpler way to enter certificates — just paste the full multi-line PEM content directly into the text area.

### Input Validation

All settings are validated before saving:

| Setting Type | Validation Rules |
|--------------|------------------|
| Broker URL | Must be valid URL format with scheme (mqtt://, mqtts://, ssl://) |
| PEM Certificate | Must have `-----BEGIN CERTIFICATE-----` header/footer |
| PEM Private Key | Must have `-----BEGIN ... PRIVATE KEY-----` header/footer |
| IoT Hub Connection String | Must contain `HostName=`, `DeviceId=`, and authentication |
| DPS Endpoint | Must be valid HTTPS URL |

Invalid entries are rejected with descriptive error messages.

---

## Building a Standalone Binary

If you need to produce a single `.bin` file that combines the bootloader and your application firmware for drag-and-drop flashing over USB, see [bootloader/BinaryBuild.md](bootloader/BinaryBuild.md) for instructions.

---

## Architecture

### File Organization

The configuration system is split into focused modules:

```
cores/arduino/config/
├── DeviceConfig.h           # Public API and profile definitions
├── DeviceConfig.cpp         # Storage layer (EEPROM read/write)
├── DeviceConfigZones.h      # Zone constants and mapping macros
├── DeviceConfigRuntime.cpp  # Runtime loading and getter functions
├── DeviceConfigCLI.h/cpp    # Serial CLI interface
├── SettingUI.h              # Shared UI metadata for CLI and Web
└── SettingValidator.h       # Input validation functions
```

The Azure IoT library provides reusable IoT Hub and DPS functionality:

```
libraries/AzureIoT/src/
├── AzureIoTHub.h/cpp        # Public API (init, connect, telemetry, twin)
├── AzureIoTDPS.h/cpp        # DPS registration over MQTT
├── AzureIoTCrypto.h/cpp     # SAS tokens, HMAC-SHA256, group key derivation
└── AzureIoTConfig.h         # Protocol constants and root CA certificate

libraries/PubSubClient/src/
├── PubSubClient.h/cpp       # Bundled MQTT client (MIT, Nick O'Leary)
```

### EEPROM Zone Allocation

The STSAFE secure element provides multiple storage zones with fixed sizes:

| Zone | Size | Usage |
|------|------|-------|
| 0 | 976 bytes | Large certificates (part 1) |
| 2 | 192 bytes | Medium storage |
| 3 | 120 bytes | WiFi SSID |
| 5 | 584 bytes | Broker URL / Connection string |
| 6 | 680 bytes | Device ID / Certificates |
| 7 | 784 bytes | Large certificates (part 2) |
| 8 | 880 bytes | Large certificates (part 3) / Client keys |
| 10 | 88 bytes | WiFi password |

### Multi-Zone Certificate Storage

Large certificates (up to ~2.6KB) span multiple zones:

```cpp
// Example: CA certificate uses zones 0, 7, and 8
// Total: 976 + 784 + 880 = 2640 bytes available
```

The zone mapping is data-driven, allowing different profiles to allocate zones differently based on which settings they need.

### Validation Layer

All input validation is centralized in `SettingValidator.h`:

```cpp
#include "SettingValidator.h"

ValidationResult result = Validator_ValidateSetting(SETTING_CA_CERT, userInput);
if (result != VALIDATE_OK) {
    const char* errorMsg = Validator_GetErrorMessage(result);
    // Handle validation error
}
```

---

## License

This project is licensed under the MIT License - see the original [Microsoft devkit-sdk](https://github.com/microsoft/devkit-sdk) for details.

Portions of this code are Copyright (c) Microsoft Corporation.