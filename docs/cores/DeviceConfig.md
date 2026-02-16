# Device Configuration

Profile-based configuration system for managing device credentials and connection settings. Stores data in STSAFE-A100 EEPROM zones with validation, CLI commands, and web UI metadata.

> **Source:** [cores/arduino/config/](../../cores/arduino/config/)

---

## Overview

The DeviceConfig system provides:
- **Connection profiles** — predefined configurations for MQTT, Azure IoT Hub, and DPS
- **Zone mapping** — maps logical settings to physical EEPROM zones
- **Validation** — format and length checks for all setting types
- **CLI integration** — serial commands for reading/writing settings
- **Web UI metadata** — labels, placeholders, and field types for the config web server

---

## Connection Profiles

| Profile | Value | Description |
|---------|-------|-------------|
| `PROFILE_NONE` | 0 | No EEPROM — sketch provides all config |
| `PROFILE_MQTT_USERPASS` | 1 | MQTT with username/password |
| `PROFILE_MQTT_USERPASS_TLS` | 2 | MQTT with username/password over TLS |
| `PROFILE_MQTT_MTLS` | 3 | MQTT with mutual TLS |
| `PROFILE_IOTHUB_SAS` | 4 | Azure IoT Hub with SAS key |
| `PROFILE_IOTHUB_CERT` | 5 | Azure IoT Hub with X.509 certificate |
| `PROFILE_DPS_SAS` | 6 | Azure DPS with symmetric key (individual) |
| `PROFILE_DPS_CERT` | 7 | Azure DPS with X.509 certificate |
| `PROFILE_DPS_SAS_GROUP` | 8 | Azure DPS with symmetric key (group) |
| `PROFILE_CUSTOM` | 9 | User-defined profile |

Set the active profile at compile time:
```cpp
#define CONNECTION_PROFILE PROFILE_IOTHUB_SAS
```

---

## Setting IDs

| Setting | Description |
|---------|-------------|
| `SETTING_WIFI_SSID` | WiFi network name |
| `SETTING_WIFI_PASSWORD` | WiFi password |
| `SETTING_BROKER_URL` | MQTT broker URL |
| `SETTING_DEVICE_ID` | Device identifier |
| `SETTING_DEVICE_PASSWORD` | Device password |
| `SETTING_CA_CERT` | CA certificate (PEM) |
| `SETTING_CLIENT_CERT` | Client certificate (PEM) |
| `SETTING_CLIENT_KEY` | Client private key (PEM) |
| `SETTING_CONNECTION_STRING` | Azure IoT Hub connection string |
| `SETTING_DPS_ENDPOINT` | DPS endpoint URL |
| `SETTING_SCOPE_ID` | DPS scope ID |
| `SETTING_REGISTRATION_ID` | DPS registration ID |
| `SETTING_SYMMETRIC_KEY` | Symmetric key |
| `SETTING_DEVICE_CERT` | Device certificate (PEM) |

---

## Core API

```cpp
#include "config/DeviceConfig.h"
```

| Function | Description |
|----------|-------------|
| `void DeviceConfig_Init(ConnectionProfile profile)` | Initialize config system for a profile |
| `const char* DeviceConfig_GetProfileName(void)` | Get active profile name string |
| `ConnectionProfile DeviceConfig_GetActiveProfile(void)` | Get active profile enum value |
| `bool DeviceConfig_IsSettingAvailable(SettingID setting)` | Check if setting exists in active profile |
| `int DeviceConfig_GetMaxLen(SettingID setting)` | Max length for setting (-1 if unavailable) |
| `int DeviceConfig_Save(SettingID setting, const char *value)` | Save setting to EEPROM (0 = success) |
| `int DeviceConfig_Read(SettingID setting, char *buffer, int bufferSize)` | Read setting from EEPROM |
| `bool DeviceConfig_LoadAll(void)` | Load all settings into internal buffers |

### Accessor Functions

| Function | Description |
|----------|-------------|
| `const char* DeviceConfig_GetWifiSsid(void)` | Get WiFi SSID |
| `const char* DeviceConfig_GetWifiPassword(void)` | Get WiFi password |
| `const char* DeviceConfig_GetBrokerHost(void)` | Get broker hostname |
| `int DeviceConfig_GetBrokerPort(void)` | Get broker port |
| `const char* DeviceConfig_GetCACert(void)` | Get CA certificate |
| `const char* DeviceConfig_GetClientCert(void)` | Get client certificate |
| `const char* DeviceConfig_GetClientKey(void)` | Get client private key |
| `const char* DeviceConfig_GetDeviceId(void)` | Get device ID |

---

## Zone Mapping

Each profile defines which EEPROM zones store which settings. Mappings support up to `MAX_ZONES_PER_SETTING` zones per setting for large values that span multiple zones.

```c
typedef struct {
    uint8_t zones[MAX_ZONES_PER_SETTING];    // Zone indices (0xFF = unused)
    uint16_t zoneSizes[MAX_ZONES_PER_SETTING];
} ZoneMapping;
```

### Combined Buffer Sizes

| Constant | Value | Description |
|----------|-------|-------------|
| `MAX_CA_CERT_SIZE` | 2640 | CA certificate buffer |
| `MAX_CLIENT_CERT_SIZE` | 1464 | Client certificate buffer |
| `MAX_DEVICE_CERT_SIZE` | 2640 | Device certificate buffer |
| `MAX_CLIENT_KEY_SIZE` | 880 | Client private key buffer |

---

## Validation

```cpp
#include "config/SettingValidator.h"
```

| Result Code | Description |
|-------------|-------------|
| `VALIDATE_OK` | Value is valid |
| `VALIDATE_ERROR_NULL` | Null pointer |
| `VALIDATE_ERROR_EMPTY` | Empty string |
| `VALIDATE_ERROR_TOO_LONG` | Exceeds max length |
| `VALIDATE_ERROR_INVALID_FORMAT` | Format check failed |
| `VALIDATE_ERROR_MISSING_REQUIRED` | Required setting missing |
| `VALIDATE_ERROR_SETTING_UNAVAILABLE` | Setting not in active profile |

### Validator Functions

| Function | Description |
|----------|-------------|
| `ValidationResult Validator_ValidateSetting(SettingID setting, const char *value)` | Full validation with type checks |
| `ValidationResult Validator_CheckLength(SettingID setting, const char *value)` | Length-only check |
| `ValidationResult Validator_BrokerUrl(const char *url)` | MQTT broker URL format |
| `ValidationResult Validator_PemCertificate(const char *pem)` | PEM certificate format |
| `ValidationResult Validator_PemPrivateKey(const char *pem)` | PEM private key format |
| `ValidationResult Validator_IotHubConnectionString(const char *str)` | IoT Hub connection string |
| `ValidationResult Validator_DpsScopeId(const char *id)` | DPS scope ID format |
| `const char* Validator_GetErrorMessage(ValidationResult result)` | Human-readable error |

---

## CLI Commands

```cpp
#include "config/DeviceConfigCLI.h"
```

| Function | Description |
|----------|-------------|
| `void config_print_help(void)` | Print help for all config commands |
| `bool config_dispatch_command(const char *cmdName, int argc, char **argv)` | Dispatch a CLI config command |
| `void config_show_status(void)` | Show all current setting values |

---

## Web UI Metadata

```cpp
#include "config/SettingUI.h"
```

### Field Types

| Type | Description |
|------|-------------|
| `UI_FIELD_TEXT` | Single-line text input |
| `UI_FIELD_TEXTAREA` | Multi-line text area (for certificates/keys) |

### Metadata Structure

```c
typedef struct {
    SettingID id;
    const char *label;           // Display label
    const char *cliCommand;      // CLI command name
    const char *webFormName;     // HTML form field name
    const char *webPlaceholder;  // Placeholder text
    const char *defaultValue;    // Default value
    UIFieldType fieldType;       // TEXT or TEXTAREA
} SettingUIMetadata;
```

### Functions

| Function | Description |
|----------|-------------|
| `const SettingUIMetadata* SettingUI_GetActiveArray(void)` | Get active UI metadata array |
| `int SettingUI_GetActiveCount(void)` | Number of UI entries |
| `void SettingUI_SetCustomUI(const SettingUIMetadata *ui, int count)` | Override with custom UI |
| `const SettingUIMetadata* SettingUI_FindById(SettingID id)` | Find by setting ID |
| `const SettingUIMetadata* SettingUI_FindByCliCommand(const char *cmd)` | Find by CLI command |
| `const SettingUIMetadata* SettingUI_FindByFormName(const char *name)` | Find by web form name |
| `bool SettingUI_IsMultiLine(const SettingUIMetadata *meta)` | True if textarea field |

---

## See Also

- [EEPROM](EEPROM.md) — Underlying storage zones
- [HTTP Server](HTTPServer.md) — Web configuration server
- [System Services](SystemServices.md) — WiFi connection using saved credentials
