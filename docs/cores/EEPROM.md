# EEPROM Interface

Secure persistent storage via the STSAFE-A100 I2C secure element. Provides zone-based read/write and convenience methods for common credentials.

> **Source:** [cores/arduino/EEPROMInterface.h](../../cores/arduino/EEPROMInterface.h)

> **Prefer DeviceConfig for new code.** The [DeviceConfig](DeviceConfig.md) system is the recommended high-level API for reading and writing device configuration. It handles profile-based zone selection, multi-zone spanning for large values, file-backed settings, validation, and runtime buffering automatically. Use `EEPROMInterface` directly only for diagnostics, factory provisioning, or storage needs outside the 17 `SettingID` slots.

> **⚠️ Zone safety:** Writing directly to an EEPROM zone that the active connection profile also manages will silently corrupt the corresponding `DeviceConfig` setting. Always check the active profile's zone assignments (see [DeviceConfig — Zone Mapping](DeviceConfig.md#zone-mapping)) before using `EEPROMInterface` for direct writes.

---

## Usage

```cpp
#include <EEPROMInterface.h>

EEPROMInterface eeprom;
```

---

## Methods

### Low-Level Zone Access

| Method | Description |
|--------|-------------|
| `int write(uint8_t *dataBuff, int buffSize, uint8_t dataZoneIndex)` | Write data to a zone. Returns 0 on success, -1 on fail. |
| `int read(uint8_t *dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex)` | Read data from a zone. Returns bytes read or -1. |
| `int enableHostSecureChannel(int level = 1, uint8_t *key = NULL)` | Enable encrypted I2C. Level 1=hardcode, 2=user key (32 bytes), 3=random. |

> **Secure channel note:** `enableHostSecureChannel()` encrypts the I2C bus between the STM32 and the STSAFE chip, protecting data in transit on the PCB. This is a hardware-level transport feature and is rarely needed for typical applications. It does not affect how `DeviceConfig` operates — `DeviceConfig` uses the same `EEPROMInterface` instance and works transparently whether the secure channel is enabled or not.

### Credential Helpers

> **Legacy API.** These convenience methods use hardcoded zone assignments that predate the profile-based `DeviceConfig` system. They remain available for backward compatibility, but new code should use `DeviceConfig_Save()` / `DeviceConfig_Read()` instead, which route to the correct zone based on the active profile.

All return 0 on success, -1 on failure.

| Method | Description |
|--------|-------------|
| `saveWiFiSetting(char *ssid, char *pwd)` | Save WiFi SSID and password |
| `readWiFiSetting(char *ssid, int ssidSize, char *pwd, int pwdSize)` | Read WiFi credentials |
| `saveDeviceConnectionString(char *connStr)` | Save Azure IoT Hub connection string |
| `readDeviceConnectionString(char *buf, int bufSize)` | Read IoT Hub connection string |
| `saveX509Cert(char *cert)` | Save X.509 certificate |
| `readX509Cert(char *cert, int bufSize)` | Read X.509 certificate |
| `saveMQTTAddress(char *addr)` | Save MQTT broker address |
| `readMQTTAddress(char *addr, int bufSize)` | Read MQTT broker address |
| `saveDeviceID(char *id)` | Save device ID |
| `readDeviceID(char *id, int bufSize)` | Read device ID |
| `saveDevicePassword(char *pwd)` | Save device password |
| `readDevicePassword(char *pwd, int bufSize)` | Read device password |
| `saveClientCert(char *cert)` | Save client certificate (≤192 bytes) |
| `readClientCert(char *cert, int bufSize)` | Read client certificate |
| `saveClientKey(char *key)` | Save client private key (≤880 bytes) |
| `readClientKey(char *key, int bufSize)` | Read client private key |

---

## Storage Zones

The STSAFE secure element has fixed-size storage zones. These sizes are hardware constants. The "Purpose" column shows the conventional usage, but **actual zone assignments vary by connection profile** — a zone labeled "Client certificate" below may store a Device ID or API key in a different profile. See [DeviceConfig — Zone Mapping](DeviceConfig.md#zone-mapping) for profile-specific assignments.

| Zone | Size (bytes) | Conventional Purpose |
|------|-------------|---------|
| 0 | 976 | General / X.509 cert (part 1) |
| 2 | 192 | Client certificate / short strings |
| 3 | 120 | WiFi SSID |
| 5 | 584 | Azure IoT Hub connection string / URLs |
| 6 | 680 | DPS UDS / device IDs |
| 7 | 784 | X.509 cert (part 2) |
| 8 | 880 | Client private key |
| 10 | 88 | WiFi password |

Zones 1, 4, and 9 are not usable (size 0).

---

## Size Limits

| Constant | Value | Description |
|----------|-------|-------------|
| `WIFI_SSID_MAX_LEN` | 32 | Max WiFi SSID length |
| `WIFI_PWD_MAX_LEN` | 64 | Max WiFi password length |
| `AZ_IOT_HUB_MAX_LEN` | 512 | Max IoT Hub connection string |
| `DPS_UDS_MAX_LEN` | 64 | Max DPS UDS length |
| `AZ_IOT_X509_MAX_LEN` | 2639 | Max X.509 cert (zones 0+7+8−1) |
| `MQTT_MAX_LEN` | 512 | Max MQTT broker address |
| `DEVICE_ID_MAX_LEN` | 64 | Max device ID |
| `DEVICE_PASSWORD_MAX_LEN` | 64 | Max device password |
| `CLIENT_CERT_MAX_LEN` | 192 | Max client certificate |
| `CLIENT_KEY_MAX_LEN` | 880 | Max client key |
| `EEPROM_DEFAULT_LEN` | 200 | Default buffer length |

---

## See Also

- [DeviceConfig](DeviceConfig.md) — High-level configuration system built on top of EEPROM zones
