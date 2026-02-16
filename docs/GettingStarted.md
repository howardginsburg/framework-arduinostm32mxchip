# Getting Started

This guide walks you through setting up the MXChip AZ3166 IoT DevKit with PlatformIO, configuring your connection, and running your first project.

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [Step 1: Update Firmware](#step-1-update-firmware)
- [Step 2: Create a PlatformIO Project](#step-2-create-a-platformio-project)
- [Step 3: Choose a Connection Profile](#step-3-choose-a-connection-profile)
- [Step 4: Configure Your Device](#step-4-configure-your-device)
- [Step 5: Write Your First Sketch](#step-5-write-your-first-sketch)
- [WiFiClientSecure Usage](#wificlientsecure-usage)
- [Configuration CLI Reference](#configuration-cli-reference)
- [Web Configuration UI](#web-configuration-ui)
- [Architecture](#architecture)

---

## Prerequisites

- **Visual Studio Code** with the **PlatformIO** extension installed
- **MXChip AZ3166** development board
- **USB Micro-B cable** for connecting the board to your computer
- A WiFi network (2.4 GHz — the board does not support 5 GHz)

---

## Step 1: Update Firmware

If your board has outdated firmware (especially if you've previously run Eclipse RTOS or original Microsoft samples), update it first.

1. Download [devkit-firmware-2.0.0.bin](../firmware/devkit-firmware-2.0.0.bin)
2. Connect your AZ3166 via USB — it mounts as a removable drive
3. Copy the `.bin` file to the drive root
4. The board will automatically flash and reboot (the RGB LED will blink during the update)

> **How do I check my firmware version?** Connect via serial at 115200 baud and enter the CLI by holding **Button A** while pressing **Reset**. The `version` command displays the current firmware and SDK versions.

---

## Step 2: Create a PlatformIO Project

Create a new PlatformIO project targeting the AZ3166:

```bash
pio init --board mxchip_az3166
```

Or use the PlatformIO Home UI in VS Code: **PlatformIO Home → New Project → Board: MXChip AZ3166**.

Configure your `platformio.ini` to use this framework:

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

The `platform_packages` line tells PlatformIO to pull this community fork instead of the default (archived) framework.

---

## Step 3: Choose a Connection Profile

Connection profiles determine which settings are stored in the secure EEPROM and how zone storage is allocated. Set the profile via the `-DCONNECTION_PROFILE=` build flag in `platformio.ini`.

| Profile | Use Case | Required Settings |
|---------|----------|-------------------|
| `PROFILE_NONE` | Config in code, no EEPROM | None |
| `PROFILE_MQTT_USERPASS` | Basic MQTT broker | WiFi, Broker URL, Device ID, Password |
| `PROFILE_MQTT_USERPASS_TLS` | MQTT over TLS | WiFi, Broker URL, Device ID, Password, CA Cert |
| `PROFILE_MQTT_MTLS` | MQTT with mutual TLS | WiFi, Broker URL, CA Cert, Client Cert, Client Key |
| `PROFILE_IOTHUB_SAS` | Azure IoT Hub (SAS key) | WiFi, Connection String |
| `PROFILE_IOTHUB_CERT` | Azure IoT Hub (X.509) | WiFi, Connection String, Device Cert |
| `PROFILE_DPS_SAS` | Azure DPS (individual key) | WiFi, DPS Endpoint, Scope ID, Registration ID, Symmetric Key |
| `PROFILE_DPS_SAS_GROUP` | Azure DPS (group key) | WiFi, DPS Endpoint, Scope ID, Registration ID, Symmetric Key |
| `PROFILE_DPS_CERT` | Azure DPS (X.509) | WiFi, DPS Endpoint, Scope ID, Registration ID, Device Cert |
| `PROFILE_CUSTOM` | Your own zone mapping | Defined in your `custom_profile.h` |

**Choosing a profile:**

- For **generic MQTT brokers** (Mosquitto, HiveMQ, EMQX), start with `PROFILE_MQTT_USERPASS_TLS` or `PROFILE_MQTT_MTLS` depending on your authentication method.
- For **Azure IoT Hub**, use `PROFILE_IOTHUB_SAS` for the simplest setup, or `PROFILE_DPS_SAS` / `PROFILE_DPS_CERT` for production scenarios with automatic provisioning.
- For **development/testing** where you hard-code credentials, use `PROFILE_NONE`.
- For unique requirements, see the [Custom Connection Profiles](CustomProfile.md) guide.

---

## Step 4: Configure Your Device

Once your project builds and uploads, you need to configure WiFi credentials and connection settings. There are two methods:

### Option A: Serial CLI

1. Hold **Button A** while pressing **Reset** to enter configuration mode
2. Open a serial monitor at **115200 baud**
3. Type `help` to see available commands for your profile
4. Configure settings:

```
set_wifissid MyNetwork
set_wifipwd MyPassword
set_broker mqtts://broker.example.com:8883
```

5. Type `exit` to reboot with the new settings

### Option B: Web Configuration UI

1. Hold **Button A** during boot to enter AP mode
2. Connect your computer/phone to the device's WiFi access point (named `AZ-XXXX`)
3. Open `http://192.168.0.1` in a browser
4. Fill in the form fields and click **Save**
5. The device reboots and connects with the new settings

See [Configuration CLI Reference](#configuration-cli-reference) and [Web Configuration UI](#web-configuration-ui) below for full details.

---

## Step 5: Write Your First Sketch

Here's a minimal sketch that connects to WiFi, reads a sensor, and displays data on the OLED:

```cpp
#include <Arduino.h>
#include <AZ3166WiFi.h>
#include <SensorManager.h>

void setup() {
    Serial.begin(115200);
    Screen.init();
    Screen.clean();
    Screen.print(0, "Connecting...");

    // Connect using credentials stored in EEPROM
    WiFi.begin();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Screen.print(0, "WiFi Connected");
    Screen.print(1, WiFi.localIP().get_address());
}

void loop() {
    float temp = Sensors.getTemperature();
    float humidity = Sensors.getHumidity();

    char buf[32];
    snprintf(buf, sizeof(buf), "Temp: %.1f C", temp);
    Screen.print(2, buf);
    snprintf(buf, sizeof(buf), "Hum:  %.1f %%", humidity);
    Screen.print(3, buf);

    Serial.printf("Temperature: %.1f C, Humidity: %.1f %%\n", temp, humidity);
    delay(2000);
}
```

After uploading, the OLED should show your WiFi IP and live sensor readings.

---

## WiFiClientSecure Usage

The `WiFiClientSecure` class provides Arduino-compatible TLS connections, making it easy to use with libraries like PubSubClient.

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

## Configuration CLI Reference

Access the configuration console by holding **Button A** while pressing **Reset**. Connect via serial at **115200 baud**.

### General Commands

| Command | Description |
|---------|-------------|
| `help` | Show all available commands for the active profile |
| `version` | Display SDK and firmware versions |
| `status` | Show current configuration values |
| `scan` | Scan for available WiFi networks |
| `exit` | Save and reboot the device |

### Setting Commands

Commands vary based on the active connection profile.

| Command | Description | Profiles |
|---------|-------------|----------|
| **WiFi** | | |
| `set_wifissid <ssid>` | Set WiFi network name | All (except NONE) |
| `set_wifipwd <password>` | Set WiFi password | All (except NONE) |
| **MQTT** | | |
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
| **Certificates** | | |
| `set_cacert "<pem>"` | Set CA/root certificate | TLS profiles |
| `set_clientcert "<pem>"` | Set client certificate | MTLS/CERT profiles |
| `set_clientkey "<pem>"` | Set client private key | MQTT_MTLS |
| `set_devicecert "<pem>"` | Set device certificate | IOTHUB_CERT, DPS_CERT |

### Entering Certificates and Keys via CLI

PEM-formatted values (certificates and private keys) require special handling because they contain spaces and newlines.

**Rules:**
1. The value **must be quoted** — PEM headers like `BEGIN EC PRIVATE KEY` contain spaces
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

> **Tip**: The Web Configuration UI provides a much simpler way to enter certificates — just paste the full multi-line PEM content directly into the text area.

### Input Validation

All settings are validated before saving:

| Setting Type | Validation Rules |
|--------------|------------------|
| Broker URL | Must be valid URL format with scheme (`mqtt://`, `mqtts://`, `ssl://`) |
| PEM Certificate | Must have `-----BEGIN CERTIFICATE-----` header/footer |
| PEM Private Key | Must have `-----BEGIN ... PRIVATE KEY-----` header/footer |
| IoT Hub Connection String | Must contain `HostName=`, `DeviceId=`, and authentication |
| DPS Endpoint | Must be valid HTTPS URL |

Invalid entries are rejected with descriptive error messages.

---

## Web Configuration UI

The SDK includes a browser-based configuration interface accessible via WiFi AP mode.

### Entering AP Mode

There are two ways to enable the web configuration server:

1. **Button shortcut:** Hold **Button A** during boot
2. **Programmatic:** Call `WiFi.beginAP("DevKit-Setup")` in your code

When AP mode starts, the OLED displays the AP name and IP address.

### Connecting to the UI

1. On your computer or phone, connect to the device's WiFi access point (named `AZ-XXXX`)
2. Open `http://192.168.0.1` in a browser
3. The form displays only the settings relevant to your active connection profile
4. Fill in each field and click **Save**
5. The device stores settings to EEPROM and reboots

### Features

- **Profile-aware form** — Only shows fields relevant to the active connection profile
- **Input validation** — Validates URLs, certificates, and connection strings before saving, with inline error messages
- **Multi-line certificate input** — Paste full PEM content directly into text areas (no `\n` escaping needed)
- **Responsive layout** — Works on mobile and desktop browsers
- **Visual feedback** — Success and error indicators for each field

---

## Custom Connection Profiles

If the built-in profiles don't match your project's requirements, you can define a custom profile entirely within your own project — no framework edits needed. Create a `custom_profile.h` header in your project's `include/` directory with your zone mappings, and set `-DCONNECTION_PROFILE=PROFILE_CUSTOM` in `platformio.ini`. The framework discovers it automatically at compile time.

See [CustomProfile.md](CustomProfile.md) for step-by-step instructions.

---

## Building a Standalone Binary

If you need to produce a single `.bin` file that combines the bootloader and your application firmware for drag-and-drop flashing over USB, see [BinaryBuild.md](BinaryBuild.md) for instructions.

---

## Architecture

### Configuration System

The DeviceConfig system is split into focused modules:

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

### Azure IoT Library

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

Large certificates (up to ~2.6KB) span multiple zones automatically. The zone mapping is data-driven, allowing different profiles to allocate zones differently based on which settings they need.

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

## Next Steps

- Browse the [Sample Projects](../README.md#sample-projects) for complete working examples
- Explore the [Library Reference](README.md) for API details on all bundled libraries
- Read the [Arduino Core](cores/README.md) docs for low-level API reference
- See [Board Hardware](system/BoardHardware.md) for pin mappings and peripheral details
