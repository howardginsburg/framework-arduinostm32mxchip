# Creating a Custom Connection Profile

This guide explains how to define a custom connection profile for the MXChip AZ3166 framework **entirely within your own project** — no framework source edits required.

A connection profile controls which configuration settings are available and how they are stored in the STSAFE secure element's EEPROM zones. The built-in profiles cover common MQTT and Azure IoT patterns, but the underlying storage is just key-value pairs in EEPROM zones — you can store **any** string data. A custom profile lets you repurpose these zones for any scenario: REST API endpoints, LoRa gateway addresses, database credentials, license keys, or anything else your application needs.

---

## Table of Contents

- [How It Works](#how-it-works)
- [Step 1: Create custom_profile.h](#step-1-create-custom_profileh)
- [Step 2: Set the Build Flag](#step-2-set-the-build-flag)
- [Step 3: Use in Your Sketch](#step-3-use-in-your-sketch)
- [Custom UI Metadata](#custom-ui-metadata)
- [Reference: Available Settings](#reference-available-settings)
- [Reference: EEPROM Zones](#reference-eeprom-zones)
- [Reference: Zone Mapping Macros](#reference-zone-mapping-macros)
- [Example: Full Custom Profile Walkthrough](#example-full-custom-profile-walkthrough)
- [Example: Custom UI Labels](#example-custom-ui-labels)
- [Example: Multi-Zone Certificate Storage](#example-multi-zone-certificate-storage)
- [Example: Using File-Backed Settings](#example-using-file-backed-settings)
- [Migrating from a Built-In Profile](#migrating-from-a-built-in-profile)
- [Troubleshooting](#troubleshooting)
- [Validation Behavior in Custom Profiles](#validation-behavior-in-custom-profiles)
- [Tips and Constraints](#tips-and-constraints)

---

## How It Works

The framework supports a `PROFILE_CUSTOM` connection profile that loads its zone mapping from a header file in **your project** rather than from the built-in profile table.

At compile time, the framework's `DeviceConfig.cpp` uses `__has_include("custom_profile.h")` to check if your project provides a custom profile definition. If it finds one and `CONNECTION_PROFILE` is set to `PROFILE_CUSTOM`, the framework uses your profile definition instead of the built-in ones.

The only things you need are:

1. A `custom_profile.h` file in your project's `include/` or `src/` directory
2. The build flag `-DCONNECTION_PROFILE=PROFILE_CUSTOM` in `platformio.ini`

No framework files need to be modified. The CLI, web UI, and all `DeviceConfig_*` functions automatically adapt to your custom profile's settings.

---

## Step 1: Create custom_profile.h

Create a file named `custom_profile.h` in your project's `include/` directory (or `src/` — anywhere PlatformIO adds to the include path).

This file must define a `const ProfileDefinition` variable named `CUSTOM_PROFILE`. The framework will use this variable when `PROFILE_CUSTOM` is active.

```c
#ifndef CUSTOM_PROFILE_H
#define CUSTOM_PROFILE_H

#include <DeviceConfig.h>
#include <DeviceConfigZones.h>

/**
 * Custom profile definition.
 *
 * The mappings array has exactly SETTING_COUNT (17) entries, one per SettingID,
 * in the order they appear in the SettingID enum. Every slot must be filled —
 * use UNUSED_ZONE for settings your profile doesn't need.
 */
static const ProfileDefinition CUSTOM_PROFILE = {
    PROFILE_CUSTOM,
    "My Custom Profile",                    // Name shown in CLI and web UI
    "Description of this custom profile",   // Description
    {
        // Mapping order must match the SettingID enum exactly:
        ZONE(3, ZONE_3_SIZE),               // SETTING_WIFI_SSID        → Zone 3 (120 bytes)
        ZONE(10, ZONE_10_SIZE),             // SETTING_WIFI_PASSWORD     → Zone 10 (88 bytes)
        ZONE(5, ZONE_5_SIZE),               // SETTING_BROKER_URL        → Zone 5 (584 bytes)  — can store any URL
        ZONE(2, ZONE_2_SIZE),               // SETTING_DEVICE_ID         → Zone 2 (192 bytes)  — can store any string
        ZONE(6, ZONE_6_SIZE),               // SETTING_DEVICE_PASSWORD   → Zone 6 (680 bytes)  — can store any credential
        UNUSED_ZONE,                        // SETTING_CA_CERT           — not used
        UNUSED_ZONE,                        // SETTING_CLIENT_CERT       — not used
        UNUSED_ZONE,                        // SETTING_CLIENT_KEY        — not used
        UNUSED_ZONE,                        // SETTING_CONNECTION_STRING  — not used
        UNUSED_ZONE,                        // SETTING_DPS_ENDPOINT      — not used
        UNUSED_ZONE,                        // SETTING_SCOPE_ID          — not used
        UNUSED_ZONE,                        // SETTING_REGISTRATION_ID   — not used
        UNUSED_ZONE,                        // SETTING_SYMMETRIC_KEY     — not used
        UNUSED_ZONE,                        // SETTING_DEVICE_CERT       — not used
        UNUSED_ZONE,                        // SETTING_SEND_INTERVAL     — not used (file-backed)
        UNUSED_ZONE,                        // SETTING_PUBLISH_TOPIC     — not used (file-backed)
        UNUSED_ZONE                         // SETTING_SUBSCRIBE_TOPIC   — not used (file-backed)
    }
};

#endif // CUSTOM_PROFILE_H
```

The framework discovers this file automatically via `__has_include("custom_profile.h")` at compile time.

---

## Step 2: Set the Build Flag

In your `platformio.ini`, set the connection profile to `PROFILE_CUSTOM`:

```ini
[env:mxchip_az3166]
platform = ststm32
board = mxchip_az3166
framework = arduino

build_flags =
    -DCONNECTION_PROFILE=PROFILE_CUSTOM

platform_packages =
    framework-arduinostm32mxchip@https://github.com/howardginsburg/framework-arduinostm32mxchip.git
```

At startup the framework calls `DeviceConfig_Init(PROFILE_CUSTOM)`, which picks up your `CUSTOM_PROFILE` definition. The CLI, web UI, and all `DeviceConfig_*` getter functions will use your zone mappings.

---

## Step 3: Use in Your Sketch

Your sketch code works exactly the same as with any built-in profile. The `DeviceConfig_*` functions read from whichever zones your profile mapped. Since the underlying storage is generic, you can use the zones for any purpose — the setting names are just identifiers for EEPROM slots:

```cpp
#include <AZ3166WiFi.h>
#include <HTTPClient.h>
#include <DeviceConfig.h>

void setup() {
    WiFi.begin(DeviceConfig_GetWifiSsid(), DeviceConfig_GetWifiPassword());
}

void loop() {
    // Read the API endpoint URL (stored in SETTING_BROKER_URL zone)
    char apiUrl[584];
    DeviceConfig_Read(SETTING_BROKER_URL, apiUrl, sizeof(apiUrl));

    // Read the device name (stored in SETTING_DEVICE_ID zone)
    char deviceName[192];
    DeviceConfig_Read(SETTING_DEVICE_ID, deviceName, sizeof(deviceName));

    // Read the API key (stored in SETTING_DEVICE_PASSWORD zone)
    char apiKey[680];
    DeviceConfig_Read(SETTING_DEVICE_PASSWORD, apiKey, sizeof(apiKey));

    // Use them however your application needs
    HTTPClient http;
    http.begin(apiUrl);
    http.addHeader("X-API-Key", apiKey);
    http.addHeader("X-Device-Name", deviceName);
    http.POST("{\"temperature\": 22.5}");
    http.end();

    delay(60000);
}
```

The CLI automatically shows only the commands for settings your profile enables. For the example above, `help` would list `set_wifissid`, `set_wifipwd`, `set_broker`, `set_deviceid`, and `set_devicepwd`.

---

## Custom UI Metadata

By default, the CLI and web UI use the built-in labels, command names, and form fields (e.g., `set_deviceid`, "Device ID"). If you want to **rename** these to match your application's domain — for example, relabeling "Device ID" as "Station ID" or changing the CLI command from `set_broker` to `set_endpoint` — you can define a `CUSTOM_PROFILE_UI` array in your `custom_profile.h`.

This is entirely optional. If you don't define `CUSTOM_PROFILE_UI`, the built-in labels are used and everything works normally.

### Defining Custom UI

Add a `CUSTOM_PROFILE_UI` array and `CUSTOM_PROFILE_UI_COUNT` macro to your `custom_profile.h`. Each entry is a `SettingUIMetadata` struct that specifies the label, CLI command, web form name, placeholder, default value, and field type for one setting.

You only need entries for settings your profile actually uses. The array order determines display order in the CLI help and web form.

```c
#include <SettingUI.h>   // For SettingUIMetadata and UIFieldType

static const SettingUIMetadata CUSTOM_PROFILE_UI[] = {
    // WiFi (keep standard names)
    {SETTING_WIFI_SSID,     "WiFi SSID",     "set_wifissid", "SSID", "WiFi Network Name",              NULL, UI_FIELD_TEXT},
    {SETTING_WIFI_PASSWORD,  "WiFi Password", "set_wifipwd",  "PASS", "WiFi Password",                  NULL, UI_FIELD_TEXT},

    // Repurposed settings with custom labels
    {SETTING_BROKER_URL,    "API Endpoint",   "set_endpoint", "Endpoint",    "https://api.example.com/v1/data",        NULL,  UI_FIELD_TEXT},
    {SETTING_DEVICE_ID,     "Station ID",     "set_station",  "StationID",   "Station Identifier",                     NULL,  UI_FIELD_TEXT},
    {SETTING_CA_CERT,       "Server CA Cert", "set_serverca", "ServerCA",    "Server CA Certificate (PEM)",            NULL,  UI_FIELD_TEXTAREA},
};
#define CUSTOM_PROFILE_UI_COUNT (sizeof(CUSTOM_PROFILE_UI) / sizeof(SettingUIMetadata))
```

### SettingUIMetadata Fields

| Field | Description | Example |
|-------|-------------|---------|
| `id` | `SettingID` enum value — must match a zone-mapped setting in your profile | `SETTING_BROKER_URL` |
| `label` | Human-readable label shown in CLI help and web UI | `"Server URL"` |
| `cliCommand` | CLI command name (typed in serial console to set the value) | `"set_server"` |
| `webFormName` | HTML form field name (used in POST data parsing) | `"ServerURL"` |
| `webPlaceholder` | Placeholder text shown in the web form input | `"Server URL"` |
| `defaultValue` | Pre-filled default value in web forms (`NULL` for none) | `NULL` |
| `fieldType` | `UI_FIELD_TEXT` for single-line, `UI_FIELD_TEXTAREA` for multi-line (certs/keys) | `UI_FIELD_TEXT` |

### How It Works

When `DeviceConfig_Init(PROFILE_CUSTOM)` runs, the framework checks if `CUSTOM_PROFILE_UI_COUNT` is defined. If so, it calls `SettingUI_SetCustomUI()` to replace the active UI metadata table. All CLI commands, web forms, and status displays then use your custom labels and commands instead of the built-in ones.

### Reading Data in Your Sketch

Regardless of UI relabeling, you read data using the standard `SettingID` enum values:

```cpp
// These use SettingID — always works regardless of UI labels
char stationId[192];
DeviceConfig_Read(SETTING_DEVICE_ID, stationId, sizeof(stationId));

char apiEndpoint[584];
DeviceConfig_Read(SETTING_BROKER_URL, apiEndpoint, sizeof(apiEndpoint));

char apiKey[680];
DeviceConfig_Read(SETTING_DEVICE_PASSWORD, apiKey, sizeof(apiKey));
```

---

## Reference: Available Settings

These are the setting IDs you can map to zones. Every `ProfileDefinition.mappings` array must have exactly one entry per setting, in enum order:

| Index | SettingID | Built-in Purpose | Custom Profile Use |
|-------|-----------|------------------|-------------------|
| 0 | `SETTING_WIFI_SSID` | WiFi network name | WiFi SSID (usually keep as-is) |
| 1 | `SETTING_WIFI_PASSWORD` | WiFi password | WiFi password (usually keep as-is) |
| 2 | `SETTING_BROKER_URL` | MQTT broker URL | Any URL or long string (584 bytes) |
| 3 | `SETTING_DEVICE_ID` | Device/client identifier | Any short string (192 bytes) |
| 4 | `SETTING_DEVICE_PASSWORD` | Device password | Any credential or string (680 bytes) |
| 5 | `SETTING_CA_CERT` | CA/root certificate (PEM) | Any large text (976 bytes) |
| 6 | `SETTING_CLIENT_CERT` | Client certificate (PEM) | Any large text (can span zones) |
| 7 | `SETTING_CLIENT_KEY` | Client private key (PEM) | Any large text (880 bytes) |
| 8 | `SETTING_CONNECTION_STRING` | Azure IoT Hub connection string | Any string (584 bytes) |
| 9 | `SETTING_DPS_ENDPOINT` | Azure DPS global endpoint | Any URL or string (584 bytes) |
| 10 | `SETTING_SCOPE_ID` | Azure DPS ID Scope | Any short string (192 bytes) |
| 11 | `SETTING_REGISTRATION_ID` | Azure DPS registration ID | Any string (680 bytes) |
| 12 | `SETTING_SYMMETRIC_KEY` | Azure DPS symmetric key | Any string (784 bytes) |
| 13 | `SETTING_DEVICE_CERT` | Device certificate for Azure (PEM) | Any large text (can span zones) |
| 14 | `SETTING_SEND_INTERVAL` | Message send interval (seconds) | Use `FILE_ZONE(MAX_SEND_INTERVAL_SIZE)` or `UNUSED_ZONE` |
| 15 | `SETTING_PUBLISH_TOPIC` | MQTT publish topic | Use `FILE_ZONE(MAX_PUBLISH_TOPIC_SIZE)` or `UNUSED_ZONE` |
| 16 | `SETTING_SUBSCRIBE_TOPIC` | MQTT subscribe topic | Use `FILE_ZONE(MAX_SUBSCRIBE_TOPIC_SIZE)` or `UNUSED_ZONE` |

---

## Reference: EEPROM Zones

The STSAFE secure element has fixed-size storage zones. These sizes are hardware constants and cannot be changed:

| Zone | Size (bytes) | Defined As | Notes |
|------|-------------|------------|-------|
| 0 | 976 | `ZONE_0_SIZE` | Large — good for certificates |
| 2 | 192 | `ZONE_2_SIZE` | Medium — IDs, short strings |
| 3 | 120 | `ZONE_3_SIZE` | WiFi SSID (by convention) |
| 5 | 584 | `ZONE_5_SIZE` | URLs, connection strings |
| 6 | 680 | `ZONE_6_SIZE` | Medium-large — certificates, IDs |
| 7 | 784 | `ZONE_7_SIZE` | Large — certificates, keys |
| 8 | 880 | `ZONE_8_SIZE` | Large — certificates, keys |
| 10 | 88 | `ZONE_10_SIZE` | Small — WiFi password (by convention) |

**Total available:** 4,304 bytes across 8 zones.

---

## Reference: Zone Mapping Macros

These macros (from `DeviceConfigZones.h`) simplify zone mapping definitions in your `custom_profile.h`:

| Macro | Usage | Description |
|-------|-------|-------------|
| `UNUSED_ZONE` | Setting not used | Marks the setting as unavailable in this profile |
| `ZONE(z, s)` | Single zone | Maps to zone `z` with size `s` |
| `ZONE2(z1, s1, z2, s2)` | Two zones | Data spans `z1` then `z2` sequentially |
| `ZONE3(z1, s1, z2, s2, z3, s3)` | Three zones | Data spans `z1`, `z2`, then `z3` |
| `FILE_ZONE(s)` | Config-file setting | Stores the value in `/fs/device.cfg` instead of EEPROM; `s` is the max length in bytes |

For multi-zone settings, data is written and read sequentially — the first `s1` bytes go to `z1`, the next `s2` bytes to `z2`, etc. There is no overhead or padding between zones.

---

## Example: Full Custom Profile Walkthrough

Suppose you're building an environmental monitoring station that posts sensor readings to a REST API over HTTPS. You need to store an API endpoint URL, an API key, a station identifier, and a TLS certificate for the server. None of the built-in profiles are designed for this — they're all MQTT or Azure-specific — but the EEPROM zones can store any string data.

### 1. Plan zone allocation

The setting names come from the framework's `SettingID` enum, but they're just labels for EEPROM storage slots. You can store anything in them:

| Your Data | Stored In | Zone(s) | Available Bytes |
|-----------|-----------|---------|----------------|
| WiFi SSID | `SETTING_WIFI_SSID` | Zone 3 | 120 |
| WiFi Password | `SETTING_WIFI_PASSWORD` | Zone 10 | 88 |
| API Endpoint URL | `SETTING_BROKER_URL` | Zone 5 | 584 |
| Station ID | `SETTING_DEVICE_ID` | Zone 2 | 192 |
| API Key | `SETTING_DEVICE_PASSWORD` | Zone 6 | 680 |
| Server CA Cert | `SETTING_CA_CERT` | Zone 0 | 976 |

**Rule:** No two settings in the same profile may share a zone.

### 2. Create `include/custom_profile.h`

```c
#ifndef CUSTOM_PROFILE_H
#define CUSTOM_PROFILE_H

#include <DeviceConfig.h>
#include <DeviceConfigZones.h>

static const ProfileDefinition CUSTOM_PROFILE = {
    PROFILE_CUSTOM,
    "REST API Sensor Station",
    "Posts sensor data to a REST API over HTTPS",
    {
        ZONE(3, ZONE_3_SIZE),                    // SETTING_WIFI_SSID       → WiFi SSID
        ZONE(10, ZONE_10_SIZE),                  // SETTING_WIFI_PASSWORD   → WiFi password
        ZONE(5, ZONE_5_SIZE),                    // SETTING_BROKER_URL      → API endpoint URL
        ZONE(2, ZONE_2_SIZE),                    // SETTING_DEVICE_ID       → Station ID
        ZONE(6, ZONE_6_SIZE),                    // SETTING_DEVICE_PASSWORD → API key
        ZONE(0, ZONE_0_SIZE),                    // SETTING_CA_CERT         → Server CA certificate
        UNUSED_ZONE,                             // SETTING_CLIENT_CERT
        UNUSED_ZONE,                             // SETTING_CLIENT_KEY
        UNUSED_ZONE,                             // SETTING_CONNECTION_STRING
        UNUSED_ZONE,                             // SETTING_DPS_ENDPOINT
        UNUSED_ZONE,                             // SETTING_SCOPE_ID
        UNUSED_ZONE,                             // SETTING_REGISTRATION_ID
        UNUSED_ZONE,                             // SETTING_SYMMETRIC_KEY
        UNUSED_ZONE,                             // SETTING_DEVICE_CERT
        UNUSED_ZONE,                             // SETTING_SEND_INTERVAL     (file-backed, not used)
        UNUSED_ZONE,                             // SETTING_PUBLISH_TOPIC     (file-backed, not used)
        UNUSED_ZONE                              // SETTING_SUBSCRIBE_TOPIC   (file-backed, not used)
    }
};

#endif // CUSTOM_PROFILE_H
```

### 3. Configure `platformio.ini`

```ini
[env:mxchip_az3166]
platform = ststm32
board = mxchip_az3166
framework = arduino

build_flags =
    -DCONNECTION_PROFILE=PROFILE_CUSTOM

platform_packages =
    framework-arduinostm32mxchip@https://github.com/howardginsburg/framework-arduinostm32mxchip.git
```

### 4. Use in your sketch

```cpp
#include <AZ3166WiFi.h>
#include <HTTPClient.h>
#include <DeviceConfig.h>
#include <Sensor.h>

void setup() {
    WiFi.begin(DeviceConfig_GetWifiSsid(), DeviceConfig_GetWifiPassword());
}

void loop() {
    // Read configuration from EEPROM
    char apiUrl[584];
    char stationId[192];
    char apiKey[680];
    DeviceConfig_Read(SETTING_BROKER_URL, apiUrl, sizeof(apiUrl));
    DeviceConfig_Read(SETTING_DEVICE_ID, stationId, sizeof(stationId));
    DeviceConfig_Read(SETTING_DEVICE_PASSWORD, apiKey, sizeof(apiKey));

    // Read sensors
    float temperature = 0, humidity = 0;
    readHTS221(temperature, humidity);

    // POST to REST API
    char payload[256];
    snprintf(payload, sizeof(payload),
        "{\"station\":\"%s\",\"temp\":%.1f,\"humidity\":%.1f}",
        stationId, temperature, humidity);

    HTTPClient http;
    http.begin(apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", apiKey);
    http.POST(payload);
    http.end();

    delay(60000);  // Post every minute
}
```

The CLI automatically shows: `set_wifissid`, `set_wifipwd`, `set_broker`, `set_deviceid`, `set_devicepwd`, and `set_cacert` — matching exactly the settings your profile enables.

### Project layout

```
sensor-station/
├── include/
│   └── custom_profile.h      ← Your profile definition
├── src/
│   └── main.cpp               ← Your sketch
└── platformio.ini              ← CONNECTION_PROFILE=PROFILE_CUSTOM
```

---

## Example: Custom UI Labels

Building on the sensor station walkthrough above, the default CLI commands are `set_broker`, `set_deviceid`, and `set_devicepwd` — which make no sense for a REST API use case. Custom UI metadata lets you replace these with domain-appropriate names.

### `include/custom_profile.h`

```c
#ifndef CUSTOM_PROFILE_H
#define CUSTOM_PROFILE_H

#include <DeviceConfig.h>
#include <DeviceConfigZones.h>
#include <SettingUI.h>

// --- Zone mappings (same as before) ---
static const ProfileDefinition CUSTOM_PROFILE = {
    PROFILE_CUSTOM,
    "REST API Sensor Station",
    "Posts sensor data to a REST API over HTTPS",
    {
        ZONE(3, ZONE_3_SIZE),                    // SETTING_WIFI_SSID       → WiFi SSID
        ZONE(10, ZONE_10_SIZE),                  // SETTING_WIFI_PASSWORD   → WiFi password
        ZONE(5, ZONE_5_SIZE),                    // SETTING_BROKER_URL      → API endpoint URL
        ZONE(2, ZONE_2_SIZE),                    // SETTING_DEVICE_ID       → Station ID
        ZONE(6, ZONE_6_SIZE),                    // SETTING_DEVICE_PASSWORD → API key
        ZONE(0, ZONE_0_SIZE),                    // SETTING_CA_CERT         → Server CA certificate
        UNUSED_ZONE,                             // SETTING_CLIENT_CERT
        UNUSED_ZONE,                             // SETTING_CLIENT_KEY
        UNUSED_ZONE,                             // SETTING_CONNECTION_STRING
        UNUSED_ZONE,                             // SETTING_DPS_ENDPOINT
        UNUSED_ZONE,                             // SETTING_SCOPE_ID
        UNUSED_ZONE,                             // SETTING_REGISTRATION_ID
        UNUSED_ZONE,                             // SETTING_SYMMETRIC_KEY
        UNUSED_ZONE,                             // SETTING_DEVICE_CERT
        UNUSED_ZONE,                             // SETTING_SEND_INTERVAL     (file-backed, not used)
        UNUSED_ZONE,                             // SETTING_PUBLISH_TOPIC     (file-backed, not used)
        UNUSED_ZONE                              // SETTING_SUBSCRIBE_TOPIC   (file-backed, not used)
    }
};

// --- Custom UI metadata ---
static const SettingUIMetadata CUSTOM_PROFILE_UI[] = {
    {SETTING_WIFI_SSID,       "WiFi SSID",        "set_wifissid",   "SSID",       "WiFi Network Name",                       NULL, UI_FIELD_TEXT},
    {SETTING_WIFI_PASSWORD,   "WiFi Password",    "set_wifipwd",    "PASS",       "WiFi Password",                           NULL, UI_FIELD_TEXT},
    {SETTING_BROKER_URL,      "API Endpoint",     "set_endpoint",   "Endpoint",   "https://api.example.com/v1/telemetry",    NULL, UI_FIELD_TEXT},
    {SETTING_DEVICE_ID,       "Station ID",       "set_station",    "StationID",  "Station Identifier (e.g., station-42)",   NULL, UI_FIELD_TEXT},
    {SETTING_DEVICE_PASSWORD, "API Key",          "set_apikey",     "APIKey",     "API Key",                                 NULL, UI_FIELD_TEXT},
    {SETTING_CA_CERT,         "Server CA Cert",   "set_serverca",   "ServerCA",   "Server CA Certificate (PEM)",             NULL, UI_FIELD_TEXTAREA},
};
#define CUSTOM_PROFILE_UI_COUNT (sizeof(CUSTOM_PROFILE_UI) / sizeof(SettingUIMetadata))

#endif // CUSTOM_PROFILE_H
```

Now the CLI and web UI use your domain language:

### Resulting CLI

```
Configuration commands for profile 'REST API Sensor Station':
 - set_wifissid <value>: Set WiFi SSID (max 120 bytes)
 - set_wifipwd <value>: Set WiFi Password (max 88 bytes)
 - set_endpoint <value>: Set API Endpoint (max 584 bytes)
 - set_station <value>: Set Station ID (max 192 bytes)
 - set_apikey <value>: Set API Key (max 680 bytes)
 - set_serverca <value>: Set Server CA Cert (max 976 bytes)
```

The web configuration page also uses these labels and placeholder text automatically — a user configuring the device sees "API Endpoint" and "Station ID" instead of "Broker URL" and "Device ID".

---

## Example: Multi-Zone Certificate Storage

If your custom profile needs to store large certificates (e.g., a CA certificate chain), you can span data across multiple EEPROM zones using the `ZONE2()` or `ZONE3()` macros. The framework writes and reads the data sequentially across zones with no padding.

For example, a CA certificate up to 2,640 bytes can span zones 0, 7, and 8:

```c
static const ProfileDefinition CUSTOM_PROFILE = {
    PROFILE_CUSTOM,
    "mTLS Gateway Client",
    "Connects to a custom gateway with mutual TLS",
    {
        ZONE(3, ZONE_3_SIZE),                                           // SETTING_WIFI_SSID
        ZONE(10, ZONE_10_SIZE),                                         // SETTING_WIFI_PASSWORD
        ZONE(5, ZONE_5_SIZE),                                           // SETTING_BROKER_URL      → Gateway URL
        ZONE(2, ZONE_2_SIZE),                                           // SETTING_DEVICE_ID        → Client name
        UNUSED_ZONE,                                                    // SETTING_DEVICE_PASSWORD
        ZONE3(0, ZONE_0_SIZE, 7, ZONE_7_SIZE, 8, ZONE_8_SIZE),         // SETTING_CA_CERT          → Large CA cert (2,640 bytes)
        ZONE2(6, ZONE_6_SIZE, 7, ZONE_7_SIZE),                         // SETTING_CLIENT_CERT      → Client cert (1,464 bytes)
        ZONE(8, ZONE_8_SIZE),                                           // SETTING_CLIENT_KEY       → Client key (880 bytes)
        UNUSED_ZONE,                                                    // SETTING_CONNECTION_STRING
        UNUSED_ZONE,                                                    // SETTING_DPS_ENDPOINT
        UNUSED_ZONE,                                                    // SETTING_SCOPE_ID
        UNUSED_ZONE,                                                    // SETTING_REGISTRATION_ID
        UNUSED_ZONE,                                                    // SETTING_SYMMETRIC_KEY
        UNUSED_ZONE,                                                    // SETTING_DEVICE_CERT
        UNUSED_ZONE,                                                    // SETTING_SEND_INTERVAL
        UNUSED_ZONE,                                                    // SETTING_PUBLISH_TOPIC
        UNUSED_ZONE                                                     // SETTING_SUBSCRIBE_TOPIC
    }
};
```

> **Zone sharing across settings:** In the example above, zones 7 and 8 appear in both `SETTING_CA_CERT` (as parts of a 3-zone span) and `SETTING_CLIENT_CERT` / `SETTING_CLIENT_KEY`. This is valid only if you never store the CA cert and client cert/key simultaneously at their full sizes. If you need all three at maximum capacity, assign non-overlapping zones to each setting.

---

## Example: Using File-Backed Settings

If your custom profile needs configurable operational settings (like a polling interval or topic names), use the `FILE_ZONE()` macro to store them in `/fs/device.cfg` instead of EEPROM:

```c
static const ProfileDefinition CUSTOM_PROFILE = {
    PROFILE_CUSTOM,
    "MQTT Sensor Node",
    "Publishes sensor data to an MQTT broker with configurable topics",
    {
        ZONE(3, ZONE_3_SIZE),                       // SETTING_WIFI_SSID
        ZONE(10, ZONE_10_SIZE),                     // SETTING_WIFI_PASSWORD
        ZONE(5, ZONE_5_SIZE),                       // SETTING_BROKER_URL
        ZONE(2, ZONE_2_SIZE),                       // SETTING_DEVICE_ID
        ZONE(6, ZONE_6_SIZE),                       // SETTING_DEVICE_PASSWORD
        UNUSED_ZONE,                                // SETTING_CA_CERT
        UNUSED_ZONE,                                // SETTING_CLIENT_CERT
        UNUSED_ZONE,                                // SETTING_CLIENT_KEY
        UNUSED_ZONE,                                // SETTING_CONNECTION_STRING
        UNUSED_ZONE,                                // SETTING_DPS_ENDPOINT
        UNUSED_ZONE,                                // SETTING_SCOPE_ID
        UNUSED_ZONE,                                // SETTING_REGISTRATION_ID
        UNUSED_ZONE,                                // SETTING_SYMMETRIC_KEY
        UNUSED_ZONE,                                // SETTING_DEVICE_CERT
        FILE_ZONE(MAX_SEND_INTERVAL_SIZE),          // SETTING_SEND_INTERVAL    → /fs/device.cfg
        FILE_ZONE(MAX_PUBLISH_TOPIC_SIZE),          // SETTING_PUBLISH_TOPIC    → /fs/device.cfg
        FILE_ZONE(MAX_SUBSCRIBE_TOPIC_SIZE)         // SETTING_SUBSCRIBE_TOPIC  → /fs/device.cfg
    }
};
```

This enables the `set_interval`, `set_pubtopic`, and `set_subtopic` CLI commands for your custom profile. In your sketch:

```cpp
int interval = DeviceConfig_GetSendInterval();       // Default: 30 seconds
const char* pubTopic = DeviceConfig_GetPublishTopic();
```

---

## Migrating from a Built-In Profile

If you start with a built-in profile (e.g., `PROFILE_MQTT_USERPASS`) and later switch to `PROFILE_CUSTOM`:

- **EEPROM data persists across profile changes** — if your custom profile maps the same settings to the same zones as the original profile, the stored values carry over automatically.
- **If zone assignments differ**, the old data remains in the original zones but the new profile reads from different zones. Re-enter values via the CLI or web UI.
- **File-backed settings** (`/fs/device.cfg`) are profile-independent — they persist regardless of which profile is active.

To plan a smooth migration, compare the zone assignments of your built-in profile (documented in `DeviceConfig.cpp`) with your planned custom profile assignments.

---

## Troubleshooting

### "error: 'CUSTOM_PROFILE' was not declared in this scope"

The framework couldn't find your profile definition. Check:
1. The file is named exactly **`custom_profile.h`** (the framework uses `__has_include("custom_profile.h")`)
2. The file is in your project's `include/` or `src/` directory (must be on PlatformIO's include path)
3. The variable is named exactly **`CUSTOM_PROFILE`** — the framework references this name directly
4. You have `-DCONNECTION_PROFILE=PROFILE_CUSTOM` in your `platformio.ini` `build_flags`

### "expected '}' before end of input" or array size errors

Your mappings array must have exactly **17 entries** — one for each `SettingID` in enum order, including the three file-backed settings at the end (indices 14–16). A common mistake is using the old count of 14 from earlier framework versions. Use `UNUSED_ZONE` for settings your profile doesn't need.

### Data appears corrupted or reads back wrong values

- **Zone overlap:** Two settings in your profile may be mapped to the same zone. Each zone can only be assigned to one setting. The framework does not detect this at compile time — overlapping zones silently corrupt data.
- **Profile change:** If you switched from a built-in profile, old data remains in zones. The new profile may interpret it differently. Re-enter values via CLI.
- **Direct EEPROM writes:** If your sketch also uses `EEPROMInterface` directly, it may overwrite zones that `DeviceConfig` manages.

### Validation rejects values that should be valid

Format validation is tied to the `SettingID`, not the profile. When you repurpose a setting (e.g., storing a REST URL in `SETTING_BROKER_URL`), the framework's validators still apply the original format rules. See [Validation Behavior in Custom Profiles](#validation-behavior-in-custom-profiles) below for details and workarounds.

---

## Validation Behavior in Custom Profiles

Format validation in the framework is dispatched by `SettingID`, **not** by the active profile. This means repurposed settings still run their original validators when saved via the CLI or `DeviceConfig_Save()`. Understanding this is important when storing non-standard data in built-in setting slots.

### Settings with format validators

| SettingID | Validator | What it checks | Custom profile impact |
|-----------|-----------|-----------------|----------------------|
| `SETTING_BROKER_URL` | `Validator_BrokerUrl` | Accepts `mqtt://`, `mqtts://`, `ssl://` prefixes or bare `hostname:port`. **Rejects** `https://`, `http://`, or any URL with a non-numeric port suffix. | If storing a REST URL like `https://api.example.com`, use the bare hostname (`api.example.com`) or `api.example.com:443` instead. |
| `SETTING_CA_CERT` | `Validator_PemCertificate` | Requires `-----BEGIN CERTIFICATE-----` / `-----END CERTIFICATE-----` markers. | Only store PEM-formatted certificates. Cannot be repurposed for arbitrary text. |
| `SETTING_CLIENT_CERT` | `Validator_PemCertificate` | Same as CA cert. | Same constraint. |
| `SETTING_DEVICE_CERT` | `Validator_PemCertificate` | Same as CA cert. | Same constraint. |
| `SETTING_CLIENT_KEY` | `Validator_PemPrivateKey` | Requires PEM private key markers (RSA, EC, or PKCS#8). | Only store PEM-formatted private keys. |
| `SETTING_CONNECTION_STRING` | `Validator_IotHubConnectionString` | Requires `HostName=`, `DeviceId=`, and either `SharedAccessKey=` or `x509=true`. | Cannot be repurposed for arbitrary strings. |
| `SETTING_SCOPE_ID` | `Validator_DpsScopeId` | Checks for `0ne` prefix (but accepts any non-empty string). | Effectively no constraint — safe to repurpose. |
| `SETTING_SEND_INTERVAL` | `Validator_Numeric` | Must be a valid integer string. | Only store numeric values. |

### Settings without format validators

These settings only have length checks and can store any string data:

`SETTING_WIFI_SSID`, `SETTING_WIFI_PASSWORD`, `SETTING_DEVICE_ID`, `SETTING_DEVICE_PASSWORD`, `SETTING_DPS_ENDPOINT`, `SETTING_REGISTRATION_ID`, `SETTING_SYMMETRIC_KEY`, `SETTING_PUBLISH_TOPIC`, `SETTING_SUBSCRIBE_TOPIC`

### Best practice for custom profiles

When planning zone assignments, prefer settings **without** format validators for general-purpose string storage. Use `SETTING_DEVICE_ID`, `SETTING_DEVICE_PASSWORD`, `SETTING_DPS_ENDPOINT`, `SETTING_REGISTRATION_ID`, or `SETTING_SYMMETRIC_KEY` for arbitrary data. Reserve cert/key settings for actual PEM data, and use `SETTING_BROKER_URL` only for values that pass the broker URL validator (bare hostnames with optional numeric ports).

---

## Tips and Constraints

- **`SETTING_COUNT` is now 17.** Every `CUSTOM_PROFILE` mappings array must have exactly 17 entries — one for each `SettingID` up to and including `SETTING_SUBSCRIBE_TOPIC`. The three new entries (indices 14–16) are for the file-backed operational settings; use `UNUSED_ZONE` for them unless your custom profile needs to expose those settings.
- **File-backed settings in custom profiles.** The `FILE_ZONE(s)` macro (defined in `DeviceConfigZones.h`) marks a setting as stored in `/fs/device.cfg` rather than EEPROM. You can use it for the three operational settings (`SETTING_SEND_INTERVAL`, `SETTING_PUBLISH_TOPIC`, `SETTING_SUBSCRIBE_TOPIC`) in your custom profile if you want to expose them through the CLI or web UI.
- **No framework edits required.** Everything is defined in your project via `custom_profile.h` and the build flag.
- **Zone exclusivity:** Each zone can only be assigned to one setting per profile. The framework does not detect conflicts at compile time — overlapping zones will silently corrupt data.
- **WiFi convention:** Zones 3 and 10 are conventionally used for WiFi SSID and password. You can reassign them, but the web UI WiFi scanner assumes these zones.
- **Certificate sizing:** PEM certificates are typically 1,200–2,000 bytes. EC keys are ~250 bytes; RSA-2048 keys are ~1,700 bytes. Plan your zone allocation accordingly.
- **Multi-zone data flow:** Data is written and read sequentially across zones with no headers or padding.
- **Variable name must be `CUSTOM_PROFILE`.** The framework looks for this exact name when `PROFILE_CUSTOM` is active.
- **File name must be `custom_profile.h`.** The framework uses `__has_include("custom_profile.h")` to discover it.
- **Custom UI is optional.** If you don't define `CUSTOM_PROFILE_UI` and `CUSTOM_PROFILE_UI_COUNT`, the built-in labels and CLI commands are used. Only define them when you want to rename settings.
- **Custom UI entries must reference mapped settings.** Each `SettingUIMetadata` entry's `id` should correspond to a setting that has a zone mapping in your `CUSTOM_PROFILE`. Entries for unmapped settings are harmless but will show as "not available" in the CLI.
